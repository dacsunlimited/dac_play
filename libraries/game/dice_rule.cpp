#include <bts/game/dice_rule.hpp>
#include <bts/game/game_operations.hpp>
#include <bts/game/rule_record.hpp>
#include <bts/blockchain/chain_interface.hpp>
#include <bts/blockchain/exceptions.hpp>
#include <bts/blockchain/time.hpp>
#include <bts/wallet/exceptions.hpp>

#include <sstream>

namespace bts { namespace game {
    using namespace bts::blockchain;
    using namespace bts::wallet;

    bts::blockchain::balance_id_type  dice_rule::balance_id()const
    {
        return condition.get_address();
    }
    
    bts::blockchain::address dice_rule::owner()const
    {
        // TODO: Only support withdraw signature for now
        if( condition.type == bts::blockchain::withdraw_signature_type )
            return condition.as<bts::blockchain::withdraw_with_signature>().owner;
        return bts::blockchain::address();
    }
    
    dice_rule::dice_rule( const bts::blockchain::address& owner, bts::blockchain::share_type amnt, uint32_t o , uint32_t g)
    {
        FC_ASSERT( amnt > 0 );
        amount = amnt;
        odds = o;
        guess = g;
        // TODO: Dice specify the slate_id, if slate_id is added make sure the one in scan_rule_resule_transaction is updated too.
        condition = bts::blockchain::withdraw_condition( bts::blockchain::withdraw_with_signature( owner ), 1);
    }
    
    void dice_rule::evaluate( transaction_evaluation_state& eval_state )
    {
        if( this->odds < 1 || this->odds < this->guess || this->guess < 1)
            FC_CAPTURE_AND_THROW( invalid_dice_odds, (odds) );
        
        auto dice_asset_record = eval_state._current_state->get_asset_record( "DICE" );
        if( !dice_asset_record )
            FC_CAPTURE_AND_THROW( unknown_asset_symbol, ( eval_state.trx.id() ) );
        
        /*
         * For each transaction, there must be only one dice operatiion exist
         */
        auto cur_record = eval_state._current_state->get_rule_data_record( type, eval_state.trx.id()._hash[0] );
        if( cur_record )
            FC_CAPTURE_AND_THROW( duplicate_dice_in_transaction, ( eval_state.trx.id() ) );
        
        rule_dice_record cur_data;
        
        // this does not means the balance are now stored in balance record, just over pass the api
        // the dice record are not in any balance record, they are over-fly-on-sky..
        // TODO: Dice Review
        eval_state.sub_balance(this->balance_id(), asset( this->amount, dice_asset_record->id ));
        
        cur_data.id               = eval_state.trx.id();
        cur_data.amount           = this->amount;
        cur_data.owner            = this->owner();
        cur_data.odds             = this->odds;
        cur_data.guess            = this->guess;
        
        cur_record = rule_data_record(cur_data);
        
        eval_state._current_state->store_rule_data_record(type, cur_data.id._hash[0], *cur_record );
    }
    
    void dice_rule::execute( chain_database_ptr blockchain, uint32_t block_num, const pending_chain_state_ptr& pending_state )
    {
        // TODO: review that wether the jackpot should before update random seed or after that
        // do not need to claim for the first BTS_BLOCKCHAIN_NUM_DELEGATES + 1 blocks, no dice action in genesis
        if (block_num <= BTS_BLOCKCHAIN_NUM_DICE)
            return;
        
        auto current_random_seed = blockchain->get_current_random_seed();
        uint32_t block_random_num = current_random_seed._hash[0];
        
        uint32_t range = BTS_BLOCKCHAIN_DICE_RANGE;
        
        uint32_t block_num_of_dice = block_num - BTS_BLOCKCHAIN_NUM_DICE;
        auto block_of_dice = blockchain->get_block(block_num_of_dice);
        
        share_type shares_destroyed = 0;
        share_type shares_created = 0;
        vector<rule_result_transaction> rule_result_transactions;
        for( const auto& trx : block_of_dice.user_transactions )
        {
            auto id = trx.id();
            auto rule_record = blockchain->get_rule_data_record(type, id._hash[0]);
            
            if ( !!rule_record ) {
                auto d_data = rule_record->as<rule_dice_record>();
                
                uint32_t dice_random_num = id._hash[0];
                
                // win condition
                uint32_t lucky_number = ( ( ( block_random_num % range ) + ( dice_random_num % range ) ) % range ) * (d_data.odds);
                uint32_t guess = d_data.guess;
                share_type jackpot = 0;
                if ( lucky_number >= (guess - 1) * range && lucky_number < guess * range )
                {
                    jackpot = d_data.amount * (d_data.odds) * (100 - BTS_BLOCKCHAIN_DICE_HOUSE_EDGE) / 100;
                    
                    // add the jackpot to the accout's balance, give the jackpot from virtul pool to winner
                    
                    // TODO: Dice, what should be the slate_id for the withdraw_with_signature, if need, we can set to the jackpot owner?
                    auto jackpot_balance_address = withdraw_condition( withdraw_with_signature(d_data.owner), 1 ).get_address();
                    auto jackpot_payout = pending_state->get_balance_record( jackpot_balance_address );
                    if( !jackpot_payout )
                        jackpot_payout = balance_record( d_data.owner, asset(0, 1), 1);
                    jackpot_payout->balance += jackpot;
                    jackpot_payout->last_update = pending_state->now();
                    
                    pending_state->store_balance_record( *jackpot_payout );
                    
                    // TODO: Dice, add the virtual transactions just like market transactions
                    
                    // balance created
                    
                    shares_created += jackpot;
                }
                
                // balance destroyed
                shares_destroyed += d_data.amount;
                // remove the dice_record from pending state after execute the jackpot
                pending_state->store_rule_data_record(type, id._hash[0], rule_record->make_null());
                
                dice_transaction dice_trx;
                dice_trx.play_owner = d_data.owner;
                dice_trx.jackpot_owner = d_data.owner;
                dice_trx.play_amount = d_data.amount;
                dice_trx.jackpot_received = jackpot;
                dice_trx.odds = d_data.odds;
                dice_trx.lucky_number = (lucky_number / range) + 1;
                
                
                rule_result_transactions.push_back(rule_result_transaction(dice_trx));
            }
        }
        
        pending_state->set_rule_result_transactions( std::move( rule_result_transactions ) );
        
        // TODO: Dice what if the accumulated_fees become negetive which is possible in theory
        // const auto prev_accumulated_fees = pending_state->get_accumulated_fees();
        //pending_state->set_accumulated_fees( prev_accumulated_fees - jackpot );
        
        // update the current share supply
        // TODO: we can also add this(or part of this) house edge to the accumulated fees too, which means pay house edge to the delegates, instead of destroy it(pay dividends to all share holders)
        // TODO: Dice The destoy part is not only the delegate now, but also the house edge, so should reflect it on ui.
        auto base_asset_record = pending_state->get_asset_record( asset_id_type(1) );
        FC_ASSERT( base_asset_record.valid() );
        base_asset_record->current_share_supply += (shares_created - shares_destroyed);
        pending_state->store_asset_record( *base_asset_record );
    }
    
    bool dice_rule::scan_result( const rule_result_transaction& rtrx,
                     uint32_t block_num,
                     const time_point_sec& block_time,
                     const uint32_t trx_index, bts::wallet::wallet_ptr w)
    { try {
        auto gtrx = rtrx.as<dice_transaction>();
        const auto win = ( gtrx.jackpot_received != 0 );
        const auto play_result = string( win ? "win" : "lose" );
        
        // TODO: Dice, play owner might be different with jackpot owner
        auto okey_jackpot = w->get_wallet_key_for_address( gtrx.jackpot_owner );
        if( okey_jackpot && okey_jackpot->has_private_key() )
        {
            auto jackpot_account_key = w->get_wallet_key_for_address( okey_jackpot->account_address );
            
            
            // auto bal_id = withdraw_condition(withdraw_with_signature(gtrx.jackpot_owner), 1 ).get_address();
            // auto bal_rec = _blockchain->get_balance_record( bal_id );
            
            /* What we paid */
            /*
             auto out_entry = ledger_entry();
             out_entry.from_account = jackpot_account_key;
             out_entry.amount = asset( trx.play_amount );
             std::stringstream out_memo_ss;
             out_memo_ss << "play dice with odds: " << trx.odds;
             out_entry.memo = out_memo_ss.str();
             */
            
            /* What we received */
            auto in_entry = ledger_entry();
            in_entry.to_account = jackpot_account_key->public_key;
            in_entry.amount = asset(gtrx.jackpot_received, 1);
            
            std::stringstream in_memo_ss;
            in_memo_ss << play_result << ", jackpot lucky number: " << gtrx.lucky_number;
            in_entry.memo = in_memo_ss.str();
            
            /* Construct a unique record id */
            std::stringstream id_ss;
            id_ss << block_num << string(gtrx.jackpot_owner) << trx_index;
            
            // TODO: Don't blow away memo, etc.
            auto record = wallet_transaction_record();
            record.record_id = fc::ripemd160::hash( id_ss.str() );
            record.block_num = block_num;
            record.is_virtual = true;
            record.is_confirmed = true;
            record.is_market = true;
            //record.ledger_entries.push_back( out_entry );
            record.ledger_entries.push_back( in_entry );
            record.fee = asset(0);    // TODO: Dice, do we need fee for claim jackpot? may be later we'll support part to delegates
            record.created_time = block_time;
            record.received_time = block_time;
            
            w->store_transaction( record );
        }
        
        return true;
        
    } FC_CAPTURE_AND_RETHROW((rtrx)) }
    
    bool dice_rule::scan( wallet_transaction_record& trx_rec, bts::wallet::wallet_ptr w )
    {
         switch( (withdraw_condition_types) condition.type )
         {
             case withdraw_null_type:
             {
                 FC_THROW( "withdraw_null_type not implemented!" );
                 break;
             }
             case withdraw_signature_type:
             {
                 auto condtion = condition.as<withdraw_with_signature>();
                 // TODO: lookup if cached key and work with it only
                 // if( _wallet_db.has_private_key( deposit.owner ) )
                 if( condtion.memo )
                 {
                     // TODO: TITAN, FC_THROW( "withdraw_option_type not implemented!" );
                     break;
                 } else
                 {
                     
                     auto opt_key_rec = w->get_wallet_key_for_address(condtion.owner);
                     if( opt_key_rec.valid() && opt_key_rec->has_private_key() )
                     {
                         // TODO: Refactor this
                         for( auto& entry : trx_rec.ledger_entries )
                         {
                             if( !entry.to_account.valid() )
                             {
                                 entry.to_account = opt_key_rec->public_key;
                                 entry.amount = asset( amount, 1 );
                                 entry.memo = "play dice";
                                 return true;
                             }
                         }
                     }
                 }
                 break;
             }
             case withdraw_multisig_type:
             {
                 // TODO: FC_THROW( "withdraw_multi_sig_type not implemented!" );
                 break;
             }
             case withdraw_password_type:
             {
                 // TODO: FC_THROW( "withdraw_password_type not implemented!" );
                 break;
             }
             default:
             {
                 FC_THROW( "unknown withdraw condition type!" );
                 break;
             }
         }
        
        return false;
    }
    
    wallet_transaction_record dice_rule::play( chain_database_ptr blockchain, bts::wallet::wallet_ptr w, const variant& params, bool sign )
    {   try {
        dice_input d_input;
        
        fc::from_variant(params, d_input);
        
        
        // TODO: return wallet_record
        
        FC_ASSERT( d_input.amount > 0 );
        FC_ASSERT( d_input.odds > 0 );
        
        // TODO: Now we have the assumption that the asset id equals the game id.
        signed_transaction     trx;
        unordered_set<address> required_signatures;
        
        trx.expiration = now() + w->get_transaction_expiration();
        
        // TODO: adjust fee based upon blockchain price per byte and
        // the size of trx... 'recursively'
        auto required_fees = w->get_transaction_fee();
        
        
        const auto asset_rec = blockchain->get_asset_record( "DICE" );
        FC_ASSERT( asset_rec.valid() );
        
        share_type amount_to_play = d_input.amount * asset_rec->precision;
        
        // dice asset is 1
        asset chips_to_play(amount_to_play, asset_rec->id);
        
        if( ! blockchain->is_valid_account_name( d_input.from_account_name ) )
            FC_THROW_EXCEPTION( bts::wallet::invalid_name, "Invalid account name!", ("dice_account_name",d_input.from_account_name) );
        
        
        auto play_account = blockchain->get_account_record( d_input.from_account_name );
        // TODO make sure it is using account active key
        
        w->withdraw_to_transaction( chips_to_play,
                                    d_input.from_account_name,
                                    trx,
                                    required_signatures );
        
        w->withdraw_to_transaction( required_fees,
                                    d_input.from_account_name,
                                    trx,
                                    required_signatures );
        
        //check this way to avoid overflow
        required_signatures.insert( play_account->active_key() );
        
        auto record = wallet_transaction_record();
        
        // TODO: Dice, specify to account, the receiver who can claim jackpot
        FC_ASSERT( amount_to_play > 0 );
        trx.operations.push_back(
                                 game_operation(bts::game::dice_rule(address( play_account->active_key() ), amount_to_play, d_input.odds, d_input.guess ))//slate_id 0
                                 );
        
        auto entry = bts::wallet::ledger_entry();
        entry.from_account = play_account->active_key();
        entry.to_account = play_account->active_key();
        entry.memo = "play dice";
        
        
        record.ledger_entries.push_back( entry );
        record.fee = required_fees;
        
        if( sign ) w->sign_transaction( trx, required_signatures );
        
        record.trx = trx;
        
        return record;
    } FC_CAPTURE_AND_RETHROW( (params) )}

} } // bts::game

namespace fc {
    void to_variant( const bts::game::dice_input& var,  variant& vo )
    {
        mutable_variant_object obj("from_account_name",var.from_account_name);
        obj("amount", var.amount)
        ("odds",var.odds)
        ("guess",var.guess);
        vo = std::move( obj );
    }
    
    void from_variant( const variant& var,  bts::game::dice_input& vo )
    { try {
        const variant_object& obj = var.get_object();
        if( obj.contains( "from_account_name" ) )
            vo.from_account_name = obj[ "from_account_name" ].as_string();
        if( obj.contains( "amount" ) )
            vo.amount = obj["amount"].as_double();
        if( obj.contains( "odds" ) )
            vo.odds = obj["odds"].as_uint64();
        if( obj.contains( "guess" ) )
            vo.guess = obj["guess"].as_uint64();
    } FC_RETHROW_EXCEPTIONS( warn, "unable to convert variant to dice_input", ("variant",var) ) }
    
} // fc
