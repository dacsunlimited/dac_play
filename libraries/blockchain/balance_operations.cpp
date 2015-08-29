#include <bts/blockchain/balance_operations.hpp>
#include <bts/blockchain/exceptions.hpp>
#include <bts/blockchain/pending_chain_state.hpp>

#include <bts/blockchain/fork_blocks.hpp>
#include <bts/utilities/combinatorics.hpp>

#include <fc/crypto/aes.hpp>

namespace bts { namespace blockchain {
    
    const note_type public_note::type       = public_type;
    const note_type secret_note::type       = secret_type;
    const packet_condition_type packet_rp_condition::type      = packet_rp_type;
    
    secret_note note_message::encrypt( const fc::ecc::private_key& owner_private_key)const
    {
        public_key_type  owner_public_key   = owner_private_key.get_public_key();
        auto shared_secret = owner_private_key.get_shared_secret( owner_public_key );
        secret_note result;
        result.data = fc::aes_encrypt( shared_secret, fc::raw::pack( *this ) );
        return  result;
    }
    
    note_message secret_note::decrypt( const fc::ecc::private_key& e )const
    {
        auto shared_secret = e.get_shared_secret(e.get_public_key());
        return decrypt(shared_secret);
    }
    
    note_message secret_note::decrypt(const fc::sha512& shared_secret) const
    {
        auto decrypted_data = fc::aes_decrypt( shared_secret, data );
        return fc::raw::unpack<note_message>( decrypted_data );
    }

   asset balance_record::calculate_yield( fc::time_point_sec now, share_type amount, share_type yield_pool, share_type share_supply )const
   {
      if( amount <= 0 )       return asset(0, condition.asset_id);
      if( share_supply <= 0 ) return asset(0, condition.asset_id);
      if( yield_pool <= 0 )   return asset(0, condition.asset_id);

      auto elapsed_time = (now - deposit_date);
      if( elapsed_time <= fc::seconds( BTS_BLOCKCHAIN_MIN_YIELD_PERIOD_SEC ) )
          return asset(0, condition.asset_id);

      fc::uint128 current_supply( share_supply - yield_pool );
      if( current_supply <= 0)
          return asset(0, condition.asset_id);

      fc::uint128 fee_fund( yield_pool );
      fc::uint128 amount_withdrawn( amount );
      amount_withdrawn *= 1000000;

      //
      // numerator in the following expression is at most
      // BTS_BLOCKCHAIN_MAX_SHARES * 1000000 * BTS_BLOCKCHAIN_MAX_SHARES
      // thus it cannot overflow
      //
      fc::uint128 yield = (amount_withdrawn * fee_fund) / current_supply;

      /**
       *  If less than 1 year, the 80% of yield are scaled linearly with time and 20% scaled by time^2,
       *  this should produce a yield curve that is "80% simple interest" and 20% simulating compound
       *  interest.
       */
      if( elapsed_time < fc::seconds( BTS_BLOCKCHAIN_MAX_YIELD_PERIOD_SEC ) )
      {
         fc::uint128 original_yield = yield;
         // discount the yield by 80%
         yield *= 8;
         yield /= 10;
         //
         // yield largest value in preceding multiplication is at most
         // BTS_BLOCKCHAIN_MAX_SHARES * 1000000 * BTS_BLOCKCHAIN_MAX_SHARES * 8
         // thus it cannot overflow
         //

         fc::uint128 delta_yield = original_yield - yield;

         // yield == amount withdrawn / total usd  *  fee fund * fraction_of_year
         yield *= elapsed_time.to_seconds();
         yield /= BTS_BLOCKCHAIN_MAX_YIELD_PERIOD_SEC;

         delta_yield *= elapsed_time.to_seconds();
         delta_yield /= BTS_BLOCKCHAIN_MAX_YIELD_PERIOD_SEC;

         delta_yield *= elapsed_time.to_seconds();
         delta_yield /= BTS_BLOCKCHAIN_MAX_YIELD_PERIOD_SEC;

         yield += delta_yield;
      }

      yield /= 1000000;

      if((yield > 0) && (yield < fc::uint128_t(yield_pool)))
         return asset( yield.to_uint64(), condition.asset_id );
      return asset( 0, condition.asset_id );
   }

   balance_id_type deposit_operation::balance_id()const
   {
      return condition.get_address();
   }

   deposit_operation::deposit_operation( const address& owner,
                                         const asset& amnt,
                                         slate_id_type slate_id )
   {
      FC_ASSERT( amnt.amount > 0 );
      amount = amnt.amount;
      condition = withdraw_condition( withdraw_with_signature( owner ),
                                      amnt.asset_id, slate_id );
   }

   void deposit_operation::evaluate( transaction_evaluation_state& eval_state )const
   { try {
       if( this->amount <= 0 )
          FC_CAPTURE_AND_THROW( negative_deposit, (amount) );

       switch( withdraw_condition_types( this->condition.type ) )
       {
          case withdraw_signature_type:
          case withdraw_multisig_type:
          case withdraw_escrow_type:
             break;
          default:
             FC_CAPTURE_AND_THROW( invalid_withdraw_condition, (*this) );
       }

       const balance_id_type deposit_balance_id = this->balance_id();

       obalance_record cur_record = eval_state.pending_state()->get_balance_record( deposit_balance_id );
       if( !cur_record.valid() )
       {
          cur_record = balance_record( this->condition );
          if( this->condition.type == withdraw_escrow_type )
             cur_record->meta_data = variant_object("creating_transaction_id", eval_state.trx.id() );
       }

       if( cur_record->balance == 0 )
       {
          cur_record->deposit_date = eval_state.pending_state()->now();
       }
       else
       {
          fc::uint128 old_sec_since_epoch( cur_record->deposit_date.sec_since_epoch() );
          fc::uint128 new_sec_since_epoch( eval_state.pending_state()->now().sec_since_epoch() );

          fc::uint128 avg = (old_sec_since_epoch * cur_record->balance) + (new_sec_since_epoch * this->amount);
          avg /= (cur_record->balance + this->amount);

          cur_record->deposit_date = time_point_sec( avg.to_integer() );
       }

       cur_record->balance += this->amount;
       eval_state.sub_balance( asset( this->amount, cur_record->asset_id() ) );

       if( cur_record->condition.asset_id == 0 && cur_record->condition.slate_id )
          eval_state.adjust_vote( cur_record->condition.slate_id, this->amount );

       cur_record->last_update = eval_state.pending_state()->now();

       const oasset_record asset_rec = eval_state.pending_state()->get_asset_record( cur_record->condition.asset_id );
       FC_ASSERT( asset_rec.valid() );

       if( asset_rec->is_market_issued() )
       {
           FC_ASSERT( cur_record->condition.slate_id == 0 );
       }

       const auto& owners = cur_record->owners();
       for( const address& owner : owners )
       {
           FC_ASSERT( asset_rec->address_is_whitelisted( owner ) );
       }

       eval_state.pending_state()->store_balance_record( *cur_record );
   } FC_CAPTURE_AND_RETHROW( (*this) ) }

   void withdraw_operation::evaluate( transaction_evaluation_state& eval_state )const
   { try {
       if( this->amount <= 0 )
          FC_CAPTURE_AND_THROW( negative_withdraw, (amount) );

      obalance_record current_balance_record = eval_state.pending_state()->get_balance_record( this->balance_id );
      if( !current_balance_record.valid() )
         FC_CAPTURE_AND_THROW( unknown_balance_record, (balance_id) );

      if( this->amount > current_balance_record->get_spendable_balance( eval_state.pending_state()->now() ).amount )
         FC_CAPTURE_AND_THROW( insufficient_funds, (current_balance_record)(amount) );

      auto asset_rec = eval_state.pending_state()->get_asset_record( current_balance_record->condition.asset_id );
      FC_ASSERT( asset_rec.valid() );

      const bool authority_is_retracting = asset_rec->flag_is_active( asset_record::retractable_balances )
                                           && eval_state.verify_authority( asset_rec->authority );

      if( !authority_is_retracting )
      {
         FC_ASSERT( !asset_rec->flag_is_active( asset_record::halted_withdrawals ) );

         switch( (withdraw_condition_types)current_balance_record->condition.type )
         {
            case withdraw_signature_type:
            {
                const withdraw_with_signature condition = current_balance_record->condition.as<withdraw_with_signature>();
                const address owner = condition.owner;
                if( !eval_state.check_signature( owner ) )
                    FC_CAPTURE_AND_THROW( missing_signature, (owner) );
                break;
            }

            case withdraw_vesting_type:
            {
                const withdraw_vesting condition = current_balance_record->condition.as<withdraw_vesting>();
                const address owner = condition.owner;
                if( !eval_state.check_signature( owner ) )
                    FC_CAPTURE_AND_THROW( missing_signature, (owner) );
                break;
            }

            case withdraw_multisig_type:
            {
               auto multisig = current_balance_record->condition.as<withdraw_with_multisig>();

               uint32_t valid_signatures = 0;
               for( const auto& sig : multisig.owners )
                    valid_signatures += eval_state.check_signature( sig );

               if( valid_signatures < multisig.required )
                   FC_CAPTURE_AND_THROW( missing_signature, (valid_signatures)(multisig) );
               break;
            }

            default:
               FC_CAPTURE_AND_THROW( invalid_withdraw_condition, (current_balance_record->condition) );
         }
      }

      // update delegate vote on withdrawn account..
      if( current_balance_record->condition.asset_id == 0 && current_balance_record->condition.slate_id )
         eval_state.adjust_vote( current_balance_record->condition.slate_id, -this->amount );

      if( asset_rec->is_market_issued() )
      {
         auto yield = current_balance_record->calculate_yield( eval_state.pending_state()->now(),
                                                               current_balance_record->balance,
                                                               asset_rec->collected_fees,
                                                               asset_rec->current_supply );
         if( yield.amount > 0 )
         {
            asset_rec->collected_fees       -= yield.amount;
            current_balance_record->balance += yield.amount;
            current_balance_record->deposit_date = eval_state.pending_state()->now();
            eval_state.yield_claimed[ current_balance_record->asset_id() ] += yield.amount;
            eval_state.pending_state()->store_asset_record( *asset_rec );
         }
      }

      current_balance_record->balance -= this->amount;
      current_balance_record->last_update = eval_state.pending_state()->now();
      eval_state.pending_state()->store_balance_record( *current_balance_record );

      if( asset_rec->withdrawal_fee != 0 && !eval_state.verify_authority( asset_rec->authority ) )
          eval_state.min_fees[ asset_rec->id ] = std::max( asset_rec->withdrawal_fee, eval_state.min_fees[ asset_rec->id ] );

      eval_state.add_balance( asset( this->amount, current_balance_record->condition.asset_id ) );
   } FC_CAPTURE_AND_RETHROW( (*this) ) }

   void burn_operation::evaluate( transaction_evaluation_state& eval_state )const
   { try {
      if( this->amount.amount <= 0 )
         FC_CAPTURE_AND_THROW( negative_deposit, (amount) );

      if( !message.empty() )
          FC_ASSERT( amount.asset_id == 0 );

      if( amount.asset_id == 0 )
      {
          const size_t message_kb = (message.size() / 1024) + 1;
          const share_type required_fee = message_kb * BTS_BLOCKCHAIN_MIN_BURN_FEE;

          FC_ASSERT( amount.amount >= required_fee, "Message of size ${s} KiB requires at least ${a} satoshis to be burned!",
                     ("s",message_kb)("a",required_fee) );
      }

      oasset_record asset_rec = eval_state.pending_state()->get_asset_record( amount.asset_id );
      FC_ASSERT( asset_rec.valid() );
      FC_ASSERT( !asset_rec->is_market_issued() );

      asset_rec->current_supply -= this->amount.amount;
      eval_state.sub_balance( this->amount );

      eval_state.pending_state()->store_asset_record( *asset_rec );

      if( account_id != 0 ) // you can offer burnt offerings to God if you like... otherwise it must be an account
      {
          const oaccount_record account_rec = eval_state.pending_state()->get_account_record( abs( this->account_id ) );
          FC_ASSERT( account_rec.valid() );
      }

      burn_record record;
      record.index.account_id = account_id;
      record.index.transaction_id = eval_state.trx.id();
      record.amount = amount;
      record.message = message;
      record.signer = message_signature;

      FC_ASSERT( !eval_state.pending_state()->get_burn_record( record.index ).valid() );

      eval_state.pending_state()->store_burn_record( std::move( record ) );
   } FC_CAPTURE_AND_RETHROW( (*this) ) }
    
    void ad_operation::evaluate( transaction_evaluation_state& eval_state )const
    { try {
        if( this->amount.amount <= 0 )
            FC_CAPTURE_AND_THROW( negative_deposit, (amount) );
        
        FC_ASSERT( !message.empty() );
        
        FC_ASSERT( amount.asset_id == 0 );
        
        
#ifndef WIN32
#warning [HARDFORK] Remove this check after PDV_V0_1_4_FORK_BLOCK_NUM has passed
#endif
        share_type required_fee = 0;
        if( eval_state.pending_state()->get_head_block_num() >= PDV_V0_1_4_FORK_BLOCK_NUM )
        {
            // 1 PLS for each 400 byte
            const size_t message_kb = (message.size() / 400) + 1;
            required_fee = message_kb * BTS_BLOCKCHAIN_MIN_AD_FEE;
            FC_ASSERT( amount.amount >= required_fee, "Message of size ${s} KiB requires at least ${a} satoshis to be pay!",
                      ("s",message_kb)("a",required_fee) );
        }
        else
        {
            // 4 PLS for each 1024 byte
            const size_t message_kb = (message.size() / 1024) + 1;
            required_fee = message_kb * BTS_BLOCKCHAIN_MIN_AD_FEE * 5;
            FC_ASSERT( amount.amount >= required_fee, "Message of size ${s} KiB requires at least ${a} satoshis to be pay!",
                      ("s",message_kb)("a",required_fee) );
        }
        
        
        // half of the note fees goto collected fees(delegate pay), other go to ad owner
        eval_state.min_fees[amount.asset_id] += required_fee;
        
        FC_ASSERT( owner_account_id != 0 );
        const oaccount_record owner_account_rec = eval_state.pending_state()->get_account_record( abs( this->owner_account_id ) );
        FC_ASSERT( owner_account_rec.valid() );
        
        auto owner_address = owner_account_rec->active_address();
        auto ad_income_balance = eval_state.pending_state()->get_balance_record(withdraw_condition( withdraw_with_signature(owner_address), 0 ).get_address());
        if( !ad_income_balance )
            ad_income_balance = balance_record( owner_address, asset(0, 0), 0 );
        
        auto ad_pay = amount.amount - required_fee;
        ad_income_balance->balance += ad_pay;
        ad_income_balance->last_update = eval_state.pending_state()->now();
        ad_income_balance->deposit_date = eval_state.pending_state()->now();
        
        eval_state.pending_state()->store_balance_record( *ad_income_balance );
        
        eval_state.sub_balance( asset(ad_pay, amount.asset_id) );
        
        // checking the signature of the publisher.
        FC_ASSERT( publisher_account_id != 0 );
        const oaccount_record publisher_account_rec = eval_state.pending_state()->get_account_record( abs( this->publisher_account_id ) );
        FC_ASSERT( publisher_account_rec.valid() );
        
        // update rp
        eval_state.rp_account_id = abs( this->publisher_account_id );
        
        // Enable this in later HARDFORK Change
        // if ( !eval_state.check_signature( publisher_account_rec->owner_key ) )
        //    FC_CAPTURE_AND_THROW( missing_signature, (publisher_account_rec->owner_key) );
        
        ad_record record;
        record.index.account_id = owner_account_id;
        record.index.transaction_id = eval_state.trx.id();
        record.publisher_id = publisher_account_id;
        record.amount = amount;
        record.message = message;
        record.signer = message_signature;
        
        // the message must be signed by the claimed publisher account
        FC_ASSERT( publisher_account_rec->active_key() == record.signer_key() );
        
        FC_ASSERT( !eval_state.pending_state()->get_ad_record( record.index ).valid() );
        
        eval_state.pending_state()->store_ad_record( std::move( record ) );
    } FC_CAPTURE_AND_RETHROW( (*this) ) }
    
    void note_operation::evaluate( transaction_evaluation_state& eval_state )const
    { try {
#ifndef WIN32
#warning [HARDFORK] Remove this check after PDV_V0_1_0_FORK_BLOCK_NUM has passed
#endif
        FC_ASSERT( eval_state.pending_state()->get_head_block_num() >= PDV_V0_1_0_FORK_BLOCK_NUM );
        if( this->amount.amount <= 0 )
            FC_CAPTURE_AND_THROW( negative_deposit, (amount) );
        
        FC_ASSERT( !message->data.empty() );
        FC_ASSERT( amount.asset_id == 0 );
        
        const size_t message_kb = (message->data.size() / 1024) + 1;
        const share_type required_fee = message_kb * BTS_BLOCKCHAIN_MIN_NOTE_FEE;
        
        FC_ASSERT( amount.amount >= required_fee, "Message of size ${s} KiB requires at least ${a} satoshis to be burned!",
                  ("s",message_kb)("a",required_fee) );
        
        // 30% of the note fees goto collected fees(delegate pay), other go to the operation pool
        eval_state.min_fees[amount.asset_id] += amount.amount * 3 / 10;
        
        // TODO: instead of burn, the left will go to a fee pool attached to this operation.
        auto op_reward_record = eval_state.pending_state()->get_operation_reward_record(note_op_type);
        auto reward_fee = amount.amount - amount.amount * 3 / 10;
        op_reward_record->fees[amount.asset_id] += reward_fee;
        eval_state.sub_balance( asset(reward_fee, amount.asset_id) );
        eval_state.pending_state()->store_operation_reward_record( *op_reward_record );
        
        // the transaction check the signature of the owner
        const oaccount_record account_rec = eval_state.pending_state()->get_account_record( abs( this->owner_account_id ) );
        FC_ASSERT( account_rec.valid() );
        
        // update rp
        eval_state.rp_account_id = abs( this->owner_account_id );
        
        // Enable this in later HARDFORK Change
        //if ( !eval_state.check_signature( account_rec->owner_key ) )
        //    FC_CAPTURE_AND_THROW( missing_signature, (account_rec->owner_key) );
        
        note_record record;
        record.index.account_id = owner_account_id;
        record.index.transaction_id = eval_state.trx.id();
        record.amount = amount;
        record.message = message;
        record.signer = message_signature;
        
        // verify the signature of the message, the message signer must be the account_id's active key
        // TODO: REVIEW, the active key could be changed before the delegete include this transaction
        FC_ASSERT( account_rec->active_key() == record.signer_key() );
        
        FC_ASSERT( !eval_state.pending_state()->get_note_record( record.index ).valid() );
        
        eval_state.pending_state()->store_note_record( std::move( record ) );
    } FC_CAPTURE_AND_RETHROW( (*this) ) }
    
    void red_packet_operation::evaluate( transaction_evaluation_state& eval_state )const
    { try {
        if( this->amount.amount <= 0 )
            FC_CAPTURE_AND_THROW( negative_deposit, (amount) );
        
        FC_ASSERT( !message.empty() );
        
        FC_ASSERT( amount.asset_id == 0 );
        FC_ASSERT( this->count > 0 );
        FC_ASSERT( this->count <= 100 );
        
        FC_ASSERT( message.size() < 100 );
        
        const share_type required_fee = BTS_BLOCKCHAIN_MIN_RED_PACKET_FEE;
        
        FC_ASSERT( amount.amount >= required_fee + count * BTS_BLOCKCHAIN_MIN_RED_PACKET_UNIT, "The paid amount must larger than the sum of required fee and count * BTS_BLOCKCHAIN_MIN_RED_PACKET_UNIT!",
                  ("a",required_fee + count * BTS_BLOCKCHAIN_MIN_RED_PACKET_UNIT) );
        // half of the note fees goto collected fees(delegate pay), other go to ad owner
        eval_state.min_fees[amount.asset_id] += required_fee;
        
        // the transaction check the signature of the owner
        const oaccount_record account_rec = eval_state.pending_state()->get_account_record( abs( this->from_account_id ) );
        FC_ASSERT( account_rec.valid() );
        
        // update rp
        eval_state.rp_account_id = abs( this->from_account_id );
        
        if ( !eval_state.check_signature( account_rec->owner_key ) )
            FC_CAPTURE_AND_THROW( missing_signature, (account_rec->owner_key) );
        
        packet_record record;
        record.id = random_id;
        record.amount = amount;
        record.from_account_id = from_account_id;
        record.claim_public_key = claim_public_key;
        record.message = message;
        record.claim_condition = claim_condition;
        
        share_type packet_unit = BTS_BLOCKCHAIN_MIN_RED_PACKET_UNIT;
        
        // using random id as the distribution for the packet allocation, remove dusty
        uint32_t total_space = amount.amount / packet_unit;
        
        uint64_t rand = random_id._hash[0];
#ifndef WIN32
#warning [HARDFORK] Remove this check after PDV_V0_2_0_FORK_BLOCK_NUM has passed
#endif
        if ( eval_state.pending_state()->get_head_block_num() > PDV_V0_2_0_FORK_BLOCK_NUM )
        {
            while ( total_space > (count + 1) * 10) {
                total_space = total_space / 10;
                packet_unit = packet_unit * 10;
            }
            
            fc::sha256 rand_seed = fc::sha256::hash( random_id );
            rand = rand_seed._hash[0];
        }
        
        asset used_packet_amount( total_space * packet_unit, amount.asset_id );
        eval_state.sub_balance( used_packet_amount );
        
        uint16_t MAX_UINT16_T = 65535;
        FC_ASSERT( total_space < MAX_UINT16_T );
        
        if ( count > 1 )
        {
            // TODO: testing this, try convert 2 uint32_t to 1 uint64_t(share_type)
            // selecting [count - 1] from [1 ...... random_space - 1]
            // 0 -> v[0] + 1
            // v[0] -> v[1] + 1
            // ...
            // v[count-2] + 1 -> total_space
            // lucky_guys is possible from 0 .... random_space - 2
            uint16_t k = count - 1;
            uint16_t N = total_space - 1;
            auto lucky_guys = bts::utilities::unranking(
                                                        rand % bts::utilities::cnr( N, k ), k, N);
            red_packet_status first_status;
            first_status.amount = asset( (lucky_guys[0] + 1 - 0) * packet_unit, amount.asset_id);
            first_status.account_id = -1;
            
            record.claim_statuses.push_back( first_status );
            
            for ( uint16_t i = 0; i < (k - 1); i ++ )
            {
                red_packet_status status;
                status.amount = asset( (lucky_guys[i + 1] - lucky_guys[i]) * packet_unit, amount.asset_id);
                status.account_id = -1;
                record.claim_statuses.push_back( status );
            }
            
            red_packet_status last_status;
            last_status.amount = asset( (total_space - ( lucky_guys[k-1] + 1)) * packet_unit, amount.asset_id );
            last_status.account_id = -1;
            
            record.claim_statuses.push_back( last_status );
        }
        else  // count == 1
        {
            red_packet_status only_status;
            only_status.amount = asset( total_space * packet_unit, amount.asset_id);
            only_status.account_id = -1;
            
            record.claim_statuses.push_back( only_status );
        }
        
        
        FC_ASSERT( record.left_packet_amount() == used_packet_amount , "The record is ${record}, the ammount is ${a}", ("record", record) );
        
        FC_ASSERT( !eval_state.pending_state()->get_packet_record( record.id ).valid() );
        
        eval_state.pending_state()->store_packet_record( std::move( record ) );
        
    } FC_CAPTURE_AND_RETHROW( (*this) ) }
    
    void claim_packet_operation::evaluate( transaction_evaluation_state& eval_state )const
    { try {
        const oaccount_record account_rec = eval_state.pending_state()->get_account_record( abs( this->to_account_id ) );
        FC_ASSERT( account_rec.valid() );
        
        opacket_record packet_rec = eval_state.pending_state()->get_packet_record( random_id );
        FC_ASSERT( packet_rec.valid() );
        
        if ( !eval_state.check_signature( account_rec->owner_key ) )
            FC_CAPTURE_AND_THROW( missing_signature, (account_rec->owner_key) );
        
        if ( !eval_state.check_signature( packet_rec->claim_public_key ) )
            FC_CAPTURE_AND_THROW( missing_signature, (packet_rec->claim_public_key) );
        
        if ( packet_rec->claim_condition.valid() ) {
            auto condition = packet_rec->claim_condition;
            if ( condition->type == packet_rp_type) {
                auto rp_condition = condition->as<packet_rp_condition>();
                
                FC_ASSERT( account_rec->stats_info.rp >= rp_condition.rp, "The account to claim the red packet must meet the claim condition." );
            }
        }
        
        if ( packet_rec->is_unclaimed_empty() )
        {
            // pass, do nothing
        }
        else
        {
            auto index = packet_rec->next_available_claim_index();
            
            for ( uint32_t i = 0; i < index; i ++ )
            {
                FC_ASSERT( packet_rec->claim_statuses[i].account_id != this->to_account_id, "This account already claimed this packet!");
            }
            
            asset packet_amount = packet_rec->claim_statuses[index].amount;
            
            eval_state.add_balance( packet_amount );
            
            auto to_address = account_rec->active_address();
            auto claim_balance = eval_state.pending_state()->get_balance_record(withdraw_condition( withdraw_with_signature(to_address), packet_amount.asset_id ).get_address());
            if( !claim_balance )
                claim_balance = balance_record( to_address, asset(0, packet_amount.asset_id), 0 );
            
            claim_balance->balance += packet_amount.amount;
            claim_balance->last_update = eval_state.pending_state()->now();
            claim_balance->deposit_date = eval_state.pending_state()->now();
            
            eval_state.pending_state()->store_balance_record( *claim_balance );
            eval_state.sub_balance( packet_amount );
            
            packet_rec->claim_statuses[index].account_id = this->to_account_id;
            packet_rec->claim_statuses[index].transaction_id = eval_state.trx.id();
            
            eval_state.pending_state()->store_packet_record( *packet_rec );
        }
        
    } FC_CAPTURE_AND_RETHROW( (*this) ) }

   void release_escrow_operation::evaluate( transaction_evaluation_state& eval_state )const
   { try {
      auto escrow_balance_record = eval_state.pending_state()->get_balance_record( this->escrow_id );
      FC_ASSERT( escrow_balance_record.valid() );

      if( this->amount_to_receiver < 0 )
         FC_CAPTURE_AND_THROW( negative_withdraw, (amount_to_receiver) );

      if( this->amount_to_sender < 0 )
         FC_CAPTURE_AND_THROW( negative_withdraw, (amount_to_sender) );

      if( !eval_state.check_signature( this->released_by ) )
         FC_ASSERT( false, "transaction not signed by releasor" );

      auto escrow_condition = escrow_balance_record->condition.as<withdraw_with_escrow>();
      auto total_released = uint64_t(amount_to_sender) + uint64_t(amount_to_receiver);

      FC_ASSERT( total_released <= escrow_balance_record->balance );
      FC_ASSERT( total_released >= amount_to_sender ); // check for addition overflow
      FC_ASSERT( total_released >= amount_to_receiver ); // check for addition overflow

      escrow_balance_record->balance -= total_released;
      auto asset_rec = eval_state.pending_state()->get_asset_record( escrow_balance_record->condition.asset_id );

      if( amount_to_sender > 0 )
          FC_ASSERT( asset_rec->address_is_whitelisted( escrow_condition.sender ) );
      if( amount_to_receiver > 0 )
          FC_ASSERT( asset_rec->address_is_whitelisted( escrow_condition.receiver ) );

      const bool authority_is_retracting = asset_rec->flag_is_active( asset_record::retractable_balances )
                                           && eval_state.verify_authority( asset_rec->authority );

      if( escrow_condition.sender == this->released_by )
      {
         FC_ASSERT( amount_to_sender == 0 );
         FC_ASSERT( amount_to_receiver <= escrow_balance_record->balance );

         if( !eval_state.check_signature( escrow_condition.sender ) && !authority_is_retracting)
             FC_CAPTURE_AND_THROW( missing_signature, (escrow_condition.sender) );

         balance_record new_balance_record( escrow_condition.receiver,
                                            asset( amount_to_receiver, escrow_balance_record->asset_id() ),
                                            escrow_balance_record->slate_id() );
         auto current_receiver_balance = eval_state.pending_state()->get_balance_record( new_balance_record.id());

         if( current_receiver_balance )
            current_receiver_balance->balance += amount_to_receiver;
         else
            current_receiver_balance = new_balance_record;

          eval_state.pending_state()->store_balance_record( *current_receiver_balance );
      }
      else if( escrow_condition.receiver == this->released_by )
      {
         FC_ASSERT( amount_to_receiver == 0 );
         FC_ASSERT( amount_to_sender <= escrow_balance_record->balance );

         if( !eval_state.check_signature( escrow_condition.receiver ) && !authority_is_retracting)
             FC_CAPTURE_AND_THROW( missing_signature, (escrow_condition.receiver) );

         balance_record new_balance_record( escrow_condition.sender,
                                            asset( amount_to_sender, escrow_balance_record->asset_id() ),
                                            escrow_balance_record->slate_id() );
         auto current_sender_balance = eval_state.pending_state()->get_balance_record( new_balance_record.id());

         if( current_sender_balance )
            current_sender_balance->balance += amount_to_sender;
         else
            current_sender_balance = new_balance_record;

         eval_state.pending_state()->store_balance_record( *current_sender_balance );
      }
      else if( escrow_condition.escrow == this->released_by )
      {
         if( !eval_state.check_signature( escrow_condition.escrow ) && !authority_is_retracting )
             FC_CAPTURE_AND_THROW( missing_signature, (escrow_condition.escrow) );
         // get a balance record for the receiver, create it if necessary and deposit funds
         {
            balance_record new_balance_record( escrow_condition.receiver,
                                               asset( amount_to_receiver, escrow_balance_record->asset_id() ),
                                               escrow_balance_record->slate_id() );
            auto current_receiver_balance = eval_state.pending_state()->get_balance_record( new_balance_record.id());

            if( current_receiver_balance )
               current_receiver_balance->balance += amount_to_receiver;
            else
               current_receiver_balance = new_balance_record;
            eval_state.pending_state()->store_balance_record( *current_receiver_balance );
         }
         //  get a balance record for the sender, create it if necessary and deposit funds
         {
            balance_record new_balance_record( escrow_condition.sender,
                                               asset( amount_to_sender, escrow_balance_record->asset_id() ),
                                               escrow_balance_record->slate_id() );
            auto current_sender_balance = eval_state.pending_state()->get_balance_record( new_balance_record.id());

            if( current_sender_balance )
               current_sender_balance->balance += amount_to_sender;
            else
               current_sender_balance = new_balance_record;
            eval_state.pending_state()->store_balance_record( *current_sender_balance );
         }
      }
      else if( address() == this->released_by )
      {
         if( !eval_state.check_signature( escrow_condition.sender ) && !authority_is_retracting)
             FC_CAPTURE_AND_THROW( missing_signature, (escrow_condition.sender) );
         if( !eval_state.check_signature( escrow_condition.receiver ) && !authority_is_retracting)
             FC_CAPTURE_AND_THROW( missing_signature, (escrow_condition.receiver) );
         // get a balance record for the receiver, create it if necessary and deposit funds
         {
            balance_record new_balance_record( escrow_condition.receiver,
                                               asset( amount_to_receiver, escrow_balance_record->asset_id() ),
                                               escrow_balance_record->slate_id() );
            auto current_receiver_balance = eval_state.pending_state()->get_balance_record( new_balance_record.id());

            if( current_receiver_balance )
               current_receiver_balance->balance += amount_to_receiver;
            else
               current_receiver_balance = new_balance_record;
            eval_state.pending_state()->store_balance_record( *current_receiver_balance );
         }
         //  get a balance record for the sender, create it if necessary and deposit funds
         {
            balance_record new_balance_record( escrow_condition.sender,
                                               asset( amount_to_sender, escrow_balance_record->asset_id() ),
                                               escrow_balance_record->slate_id() );
            auto current_sender_balance = eval_state.pending_state()->get_balance_record( new_balance_record.id());

            if( current_sender_balance )
               current_sender_balance->balance += amount_to_sender;
            else
               current_sender_balance = new_balance_record;
            eval_state.pending_state()->store_balance_record( *current_sender_balance );
         }
      }
      else
      {
          FC_ASSERT( false, "not released by a party to the escrow transaction" );
      }

      eval_state.pending_state()->store_balance_record( *escrow_balance_record );
   } FC_CAPTURE_AND_RETHROW( (*this) ) }

   void update_balance_vote_operation::evaluate( transaction_evaluation_state& eval_state )const
   { try {
      auto current_balance_record = eval_state.pending_state()->get_balance_record( this->balance_id );
      FC_ASSERT( current_balance_record.valid(), "No such balance!" );
      FC_ASSERT( current_balance_record->condition.asset_id == 0, "Only BTS balances can have restricted owners." );
      FC_ASSERT( current_balance_record->condition.type == withdraw_signature_type, "Restricted owners not enabled for anything but basic balances" );

      auto last_update_secs = current_balance_record->last_update.sec_since_epoch();
      ilog("last_update_secs is: ${secs}", ("secs", last_update_secs) );

      auto balance = current_balance_record->balance;
      auto fee = BTS_BLOCKCHAIN_PRECISION / 2;
      FC_ASSERT( balance > fee );

      auto asset_rec = eval_state.pending_state()->get_asset_record( current_balance_record->condition.asset_id );
      if( asset_rec->is_market_issued() ) FC_ASSERT( current_balance_record->condition.slate_id == 0 );

      if( current_balance_record->condition.slate_id )
      {
          eval_state.adjust_vote( current_balance_record->condition.slate_id, -balance );
      }
      current_balance_record->balance -= balance;
      current_balance_record->last_update = eval_state.pending_state()->now();

      ilog("I'm storing a balance record whose last update is: ${secs}", ("secs", current_balance_record->last_update) );
      eval_state.pending_state()->store_balance_record( *current_balance_record );

      auto new_restricted_owner = current_balance_record->restricted_owner;
      auto new_slate = current_balance_record->condition.slate_id;

      if( this->new_restricted_owner.valid() && (this->new_restricted_owner != new_restricted_owner) )
      {
          ilog("@n new restricted owner specified and its not the existing one");
          for( const auto& owner : current_balance_record->owners() ) //eventually maybe multisig can delegate vote
          {
              if( !eval_state.check_signature( owner ) )
                  FC_CAPTURE_AND_THROW( missing_signature, (owner) );
          }
          new_restricted_owner = this->new_restricted_owner;
          new_slate = this->new_slate;
      }
      else // NOT this->new_restricted_owner.valid() || (this->new_restricted_owner == new_restricted_owner)
      {
          auto restricted_owner = current_balance_record->restricted_owner;
          /*
          FC_ASSERT( restricted_owner.valid(),
                     "Didn't specify a new restricted owner, but one currently exists." );
                     */
          ilog( "@n now: ${secs}", ("secs", eval_state.pending_state()->now().sec_since_epoch()) );
          ilog( "@n last update: ${secs}", ("secs", last_update_secs ) );
          FC_ASSERT( eval_state.pending_state()->now().sec_since_epoch() - last_update_secs
                     >= BTS_BLOCKCHAIN_VOTE_UPDATE_PERIOD_SEC,
                     "You cannot update your vote this frequently with only the voting key!" );

          if( NOT eval_state.check_signature( *restricted_owner ) )
          {
              const auto& owners = current_balance_record->owners();
              for( const auto& owner : owners ) //eventually maybe multisig can delegate vote
              {
                  if( NOT eval_state.check_signature( owner ) )
                      FC_CAPTURE_AND_THROW( missing_signature, (owner) );
              }
          }
          new_slate = this->new_slate;
      }

      const auto owner = current_balance_record->owner();
      FC_ASSERT( owner.valid() );
      withdraw_condition new_condition( withdraw_with_signature( *owner ), 0, new_slate );
      balance_record newer_balance_record( new_condition );
      auto new_balance_record = eval_state.pending_state()->get_balance_record( newer_balance_record.id() );
      if( !new_balance_record.valid() )
          new_balance_record = current_balance_record;
      new_balance_record->condition = new_condition;

      if( new_balance_record->balance == 0 )
      {
         new_balance_record->deposit_date = eval_state.pending_state()->now();
      }
      else
      {
         fc::uint128 old_sec_since_epoch( current_balance_record->deposit_date.sec_since_epoch() );
         fc::uint128 new_sec_since_epoch( eval_state.pending_state()->now().sec_since_epoch() );

         fc::uint128 avg = (old_sec_since_epoch * new_balance_record->balance) + (new_sec_since_epoch * balance);
         avg /= (new_balance_record->balance + balance);

         new_balance_record->deposit_date = time_point_sec( avg.to_integer() );
      }

      new_balance_record->last_update = eval_state.pending_state()->now();
      new_balance_record->balance += (balance - fee);
      new_balance_record->restricted_owner = new_restricted_owner;

      eval_state.add_balance( asset(fee, 0) );

      // update delegate vote on deposited account..
      if( new_balance_record->condition.slate_id )
         eval_state.adjust_vote( new_balance_record->condition.slate_id, (balance-fee) );

      ilog("I'm storing a balance record whose last update is: ${secs}", ("secs", new_balance_record->last_update) );
      eval_state.pending_state()->store_balance_record( *new_balance_record );

   } FC_CAPTURE_AND_RETHROW( (*this) ) }

   void limit_fee_operation::evaluate( transaction_evaluation_state& eval_state )const
   { try {
       FC_ASSERT( eval_state.max_fees.count( this->max_fee.asset_id ) == 0 );
       eval_state.max_fees[ this->max_fee.asset_id ] = this->max_fee.amount;
   } FC_CAPTURE_AND_RETHROW( (*this) ) }

} } // bts::blockchain
