#include <bts/blockchain/chain_interface.hpp>
#include <bts/blockchain/exceptions.hpp>
#include <bts/blockchain/market_engine.hpp>
#include <bts/blockchain/market_operations.hpp>

namespace bts { namespace blockchain {

   /**
    *  If the amount is negative then it will withdraw/cancel the bid assuming
    *  it is signed by the owner and there is sufficient funds.
    *
    *  If the amount is positive then it will add funds to the bid.
    */
   void bid_operation::evaluate( transaction_evaluation_state& eval_state )
   { try {
      if( this->bid_index.order_price == price() )
         FC_CAPTURE_AND_THROW( zero_price, (bid_index.order_price) );


      auto owner = this->bid_index.owner;

      auto base_asset_rec = eval_state._current_state->get_asset_record( bid_index.order_price.base_asset_id );
      auto quote_asset_rec = eval_state._current_state->get_asset_record( bid_index.order_price.quote_asset_id );
      FC_ASSERT( base_asset_rec.valid() );
      FC_ASSERT( quote_asset_rec.valid() );
      if( base_asset_rec->is_restricted() )
         FC_ASSERT( eval_state._current_state->get_authorization( base_asset_rec->id, owner ) );
      if( quote_asset_rec->is_restricted() )
         FC_ASSERT( eval_state._current_state->get_authorization( quote_asset_rec->id, owner ) );

      bool issuer_override = quote_asset_rec->is_retractable() && eval_state.verify_authority( quote_asset_rec->authority );

      if( !issuer_override && !eval_state.check_signature( owner ) )
         FC_CAPTURE_AND_THROW( missing_signature, (bid_index.owner) );

      asset delta_amount  = this->get_amount();

      eval_state.validate_asset( delta_amount );

      auto current_bid   = eval_state._current_state->get_bid_record( this->bid_index );

      if( this->amount == 0 ) FC_CAPTURE_AND_THROW( zero_amount );
      if( this->amount <  0 ) // withdraw
      {
          if( NOT current_bid )
             FC_CAPTURE_AND_THROW( unknown_market_order, (bid_index) );

          if( llabs(this->amount) > current_bid->balance )
             FC_CAPTURE_AND_THROW( insufficient_funds, (amount)(current_bid->balance) );

          // add the delta amount to the eval state that we withdrew from the bid
          eval_state.add_balance( -delta_amount );
      }
      else // this->amount > 0 - deposit
      {
          if( NOT current_bid )  // then initialize to 0
            current_bid = order_record();
          // sub the delta amount from the eval state that we deposited to the bid
          eval_state.sub_balance( balance_id_type(), delta_amount );
      }

      current_bid->last_update = eval_state._current_state->now();
      current_bid->balance     += this->amount;

      eval_state._current_state->store_bid_record( this->bid_index, *current_bid );

      //auto check   = eval_state._current_state->get_bid_record( this->bid_index );
   } FC_CAPTURE_AND_RETHROW( (*this) ) }

   /**
    *  If the amount is negative then it will withdraw/cancel the bid assuming
    *  it is signed by the owner and there is sufficient funds.
    *
    *  If the amount is positive then it will add funds to the bid.
    */
   void relative_bid_operation::evaluate( transaction_evaluation_state& eval_state )
   { try {
      if( this->bid_index.order_price == price() )
         FC_CAPTURE_AND_THROW( zero_price, (bid_index.order_price) );

      auto owner = this->bid_index.owner;
      if( !eval_state.check_signature( owner ) )
         FC_CAPTURE_AND_THROW( missing_signature, (bid_index.owner) );

      asset delta_amount  = this->get_amount();

      eval_state.validate_asset( delta_amount );
      auto quote_asset_rec = eval_state._current_state->get_asset_record( bid_index.order_price.quote_asset_id );
      FC_ASSERT( quote_asset_rec->is_market_issued() );
      FC_ASSERT( bid_index.order_price.base_asset_id == 0 ); // NOTE: only allowing issuance against base asset

      auto current_bid   = eval_state._current_state->get_relative_bid_record( this->bid_index );

      if( this->amount == 0 ) FC_CAPTURE_AND_THROW( zero_amount );
      if( this->amount <  0 ) // withdraw
      {
          if( NOT current_bid )
             FC_CAPTURE_AND_THROW( unknown_market_order, (bid_index) );

          if( llabs(this->amount) > current_bid->balance )
             FC_CAPTURE_AND_THROW( insufficient_funds, (amount)(current_bid->balance) );

          // add the delta amount to the eval state that we withdrew from the bid
          eval_state.add_balance( -delta_amount );
      }
      else // this->amount > 0 - deposit
      {
          if( NOT current_bid )  // then initialize to 0
            current_bid = order_record();
          // sub the delta amount from the eval state that we deposited to the bid
          eval_state.sub_balance( balance_id_type(), delta_amount );
      }

      current_bid->last_update = eval_state._current_state->now();
      current_bid->balance     += this->amount;
      current_bid->limit_price =  this->limit_price;

      eval_state._current_state->store_relative_bid_record( this->bid_index, *current_bid );

      //auto check   = eval_state._current_state->get_bid_record( this->bid_index );
   } FC_CAPTURE_AND_RETHROW( (*this) ) }


   /**
    *  If the amount is negative then it will withdraw/cancel the bid assuming
    *  it is signed by the owner and there is sufficient funds.
    *
    *  If the amount is positive then it will add funds to the bid.
    */
   void ask_operation::evaluate( transaction_evaluation_state& eval_state )
   { try {
      if( this->ask_index.order_price == price() )
         FC_CAPTURE_AND_THROW( zero_price, (ask_index.order_price) );
      
      auto owner = this->ask_index.owner;

      auto base_asset_rec = eval_state._current_state->get_asset_record( ask_index.order_price.base_asset_id );
      auto quote_asset_rec = eval_state._current_state->get_asset_record( ask_index.order_price.quote_asset_id );
      FC_ASSERT( base_asset_rec.valid() );
      FC_ASSERT( quote_asset_rec.valid() );
      if( base_asset_rec->is_restricted() )
         FC_ASSERT( eval_state._current_state->get_authorization( base_asset_rec->id, owner ) );
      if( quote_asset_rec->is_restricted() )
         FC_ASSERT( eval_state._current_state->get_authorization( quote_asset_rec->id, owner ) );

      bool issuer_override = base_asset_rec->is_retractable() && eval_state.verify_authority( base_asset_rec->authority );

      if( !issuer_override && !eval_state.check_signature( owner ) )
         FC_CAPTURE_AND_THROW( missing_signature, (ask_index.owner) );


      asset delta_amount  = this->get_amount();

      eval_state.validate_asset( delta_amount );

      auto current_ask   = eval_state._current_state->get_ask_record( this->ask_index );


      if( this->amount == 0 ) FC_CAPTURE_AND_THROW( zero_amount );
      if( this->amount <  0 ) // withdraw
      {
          if( NOT current_ask )
             FC_CAPTURE_AND_THROW( unknown_market_order, (ask_index) );

          if( llabs(this->amount) > current_ask->balance )
             FC_CAPTURE_AND_THROW( insufficient_funds, (amount)(current_ask->balance) );

          // add the delta amount to the eval state that we withdrew from the ask
          eval_state.add_balance( -delta_amount );
      }
      else // this->amount > 0 - deposit
      {
          if( NOT current_ask )  // then initialize to 0
            current_ask = order_record();
          // sub the delta amount from the eval state that we deposited to the ask
          eval_state.sub_balance( balance_id_type(), delta_amount );
      }

      current_ask->last_update = eval_state._current_state->now();
      current_ask->balance     += this->amount;
      FC_ASSERT( current_ask->balance >= 0, "", ("current_ask",current_ask)  );

      eval_state._current_state->store_ask_record( this->ask_index, *current_ask );
   } FC_CAPTURE_AND_RETHROW( (*this) ) }

   /**
    *  If the amount is negative then it will withdraw/cancel the bid assuming
    *  it is signed by the owner and there is sufficient funds.
    *
    *  If the amount is positive then it will add funds to the bid.
    */
   void relative_ask_operation::evaluate( transaction_evaluation_state& eval_state )
   { try {
      if( this->ask_index.order_price == price() )
         FC_CAPTURE_AND_THROW( zero_price, (ask_index.order_price) );

      FC_ASSERT( ask_index.order_price.quote_asset_id > ask_index.order_price.base_asset_id );

      auto owner = this->ask_index.owner;
      if( !eval_state.check_signature( owner ) )
         FC_CAPTURE_AND_THROW( missing_signature, (ask_index.owner) );

      asset delta_amount  = this->get_amount();

      eval_state.validate_asset( delta_amount );

      auto quote_asset_rec = eval_state._current_state->get_asset_record( ask_index.order_price.quote_asset_id );
      FC_ASSERT( quote_asset_rec->is_market_issued() );
      FC_ASSERT( ask_index.order_price.base_asset_id == 0 ); // NOTE: only allowing issuance against base asset

      auto current_ask   = eval_state._current_state->get_ask_record( this->ask_index );


      if( this->amount == 0 ) FC_CAPTURE_AND_THROW( zero_amount );
      if( this->amount <  0 ) // withdraw
      {
          if( NOT current_ask )
             FC_CAPTURE_AND_THROW( unknown_market_order, (ask_index) );

          if( llabs(this->amount) > current_ask->balance )
             FC_CAPTURE_AND_THROW( insufficient_funds, (amount)(current_ask->balance) );

          // add the delta amount to the eval state that we withdrew from the ask
          eval_state.add_balance( -delta_amount );
      }
      else // this->amount > 0 - deposit
      {
          if( NOT current_ask )  // then initialize to 0
            current_ask = order_record();
          // sub the delta amount from the eval state that we deposited to the ask
          eval_state.sub_balance( balance_id_type(), delta_amount );
      }

      current_ask->last_update = eval_state._current_state->now();
      current_ask->balance     += this->amount;
      current_ask->limit_price =  this->limit_price;
      FC_ASSERT( current_ask->balance >= 0, "", ("current_ask",current_ask)  );

      eval_state._current_state->store_relative_ask_record( this->ask_index, *current_ask );
   } FC_CAPTURE_AND_RETHROW( (*this) ) }
    
    balance_id_type  buy_chips_operation::balance_id()const
    {
        withdraw_condition condition(withdraw_with_signature( this->owner ), this->amount.asset_id);
        return condition.get_address();
    }
    
    void buy_chips_operation::evaluate( transaction_evaluation_state& eval_state )
    { try {
        if ( this->amount.amount == 0) {
            FC_CAPTURE_AND_THROW( zero_amount );
        }
        
        if( this->amount.amount < 0 )
            FC_CAPTURE_AND_THROW( negative_deposit );
        
        auto  asset_to_buy = eval_state._current_state->get_asset_record( this->amount.asset_id );

        FC_ASSERT( asset_to_buy.valid() );
        FC_ASSERT( asset_to_buy->is_chip_asset(), "${symbol} is not a chip asset", ("symbol",asset_to_buy->symbol) );
        
        double price = (asset_to_buy->current_collateral * 1.0) / asset_to_buy->current_share_supply;
        share_type collateral_to_add = this->amount.amount * price;
        
        asset cost_shares( collateral_to_add, 0);
        
        eval_state.sub_balance( this->owner, cost_shares);
        
        // deposit amount of chips to the owner. and update the asset record's current collateral
        
        withdraw_condition condition( withdraw_with_signature( this->owner ), this->amount.asset_id);
        
        auto chips_balance_id = condition.get_address();
        auto cur_record = eval_state._current_state->get_balance_record( chips_balance_id );
        if( !cur_record )
        {
            cur_record = balance_record( condition );
        }
        cur_record->last_update   = eval_state._current_state->now();
        cur_record->balance       += this->amount.amount;
        // because this is system created from vitual, so no need to sub_balance
        eval_state._current_state->store_balance_record( *cur_record );
        
        asset_to_buy->current_collateral += collateral_to_add;
        asset_to_buy->current_share_supply += this->amount.amount;
        // TODO: Dice, do we need to update the asset 0's current supply, no. Because it is just become the collateral of chips.
        
        eval_state._current_state->store_asset_record( *asset_to_buy );
    } FC_CAPTURE_AND_RETHROW( (*this) ) }

} } // bts::blockchain
