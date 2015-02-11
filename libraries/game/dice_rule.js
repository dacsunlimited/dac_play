// require("play.js")

var BTS_BLOCKCHAIN_NUM_DELEGATES = 101	
var BTS_BLOCKCHAIN_NUM_DICE = BTS_BLOCKCHAIN_NUM_DELEGATES / 10;
var BTS_BLOCKCHAIN_DICE_RANGE = 10000;
var BTS_BLOCKCHAIN_DICE_HOUSE_EDGE = 0;

/*
 * return
 * { operations, ledger_entries, required_signatures}
 */
global.play = function( blockchain, wallet, input, sign )
    {   try {
        dice_input d_input;
        
        fc::from_variant(params, d_input);
        
        // TODO: return wallet_record
        
        //FC_ASSERT( d_input.amount > 0 );
        //FC_ASSERT( d_input.odds > 0 );
        
        
        var asset_rec = blockchain.get_asset_record( "DICE" );
        //FC_ASSERT( asset_rec.valid() );
        
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
        
        // TODO: Dice, specify to account, the receiver who can claim jackpot
        FC_ASSERT( amount_to_play > 0 );
		
        
    } FC_CAPTURE_AND_RETHROW( (params) )}

/*
 * 
 */
global.evaluate = function( eval_state, eval_state_current_state )
{
        if( this->odds < 1 || this->odds < this->guess || this->guess < 1)
            FC_CAPTURE_AND_THROW( invalid_dice_odds, (odds) );
        
		// TODO: support symbol string as parameter
        auto dice_asset_record = eval_state_current_state.get_asset_record( "DICE" );
        if( !dice_asset_record )
            FC_CAPTURE_AND_THROW( unknown_asset_symbol, ( eval_state.trx.id() ) );
        
        /*
         * For each transaction, there must be only one dice operatiion exist
         */
        auto cur_record = eval_state_current_state.get_rule_data_record( type, eval_state.trx.id()._hash[0] );
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
        
        eval_state_current_state.store_rule_data_record(type, cur_data.id._hash[0], *cur_record );
    }

global.execute = function (blockchain, block_num, pending_state) {
	if (block_num <= BTS_BLOCKCHAIN_NUM_DICE)
	{
   	 return
	}

	var block_random_num = blockchain_context.get_current_random_seed();

	var range = BTS_BLOCKCHAIN_DICE_RANGE;

	var block_num_of_dice = block_num - BTS_BLOCKCHAIN_NUM_DICE;

	var block_of_dice = blockchain_context.getblock(block_num_of_dice);
	
	var trxs = block_of_dice.get_transactions();
	
	var trx;
	for (trx in trxs)
	{
		var id = trx.id();
		// TODO: define the type
		var rule_data = blockchain_context.get_rule_data_record(type, id.hash(0));
		
		
		if (rule_data)
		{
			// TODO hash to be defined in V8
			var dice_random_num = id.hash(0);
			
			// win condition
            var lucky_number = ( ( ( block_random_num % range ) + ( dice_random_num % range ) ) % range ) * (rule_data.odds);
            var guess = rule_data.guess;
            var jackpot = 0;
            if ( lucky_number >= (guess - 1) * range && lucky_number < guess * range )
            {
                jackpot = rule_data.amount * (rule_data.odds) * (100 - BTS_BLOCKCHAIN_DICE_HOUSE_EDGE) / 100;
                
                // add the jackpot to the accout's balance, give the jackpot from virtul pool to winner
                   
                // TODO: Dice, what should be the slate_id for the withdraw_with_signature, if need, we can set to the jackpot owner?
                var jackpot_balance_address = V8_Global_Get_Balance_ID_For_Owner(rule_data.owner, 1);
                var jackpot_payout = pending_state.get_balance_record( jackpot_balance_address );
                if( !jackpot_payout )
                    jackpot_payout = balance_record( rule_data.owner, asset(0, 1), 1);
                jackpot_payout.balance += jackpot;
                jackpot_payout.last_update = Date.now();
                   
                pending_state.store_balance_record( jackpot_payout );
                   
                shares_created += jackpot;
            }
               
            // balance destroyed
            shares_destroyed += rule_data.amount;
            
			// remove the dice_record from pending state after execute the jackpot
            pending_state.store_rule_data_record(type, id._hash[0], null);
               
            var dice_trx = {};
            dice_trx.play_owner = d_data.owner;
            dice_trx.jackpot_owner = d_data.owner;
            dice_trx.play_amount = d_data.amount;
            dice_trx.jackpot_received = jackpot;
            dice_trx.odds = d_data.odds;
            dice_trx.lucky_number = (lucky_number / range) + 1;
               
               
            rule_result_transactions.push_back(rule_result_transaction(dice_trx));
		}
	}
	
	pending_state.set_rule_result_transactions( rule_result_transactions );
        
	auto base_asset_record = pending_state.get_asset_record( asset_id_type(1) );
	// FC_ASSERT( base_asset_record.valid() );
	base_asset_record.current_share_supply += (shares_created - shares_destroyed);
	pending_state.store_asset_record( base_asset_record );
}

global.scan_result = function( rule_result_transaction, block_num, block_time, trx_index, wallet)
{
	try {
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
            record.received_time = received_time;
            
            w->store_transaction( record );
        }
        
        return true;
        
} FC_CAPTURE_AND_RETHROW((rtrx)) 
}
    
global.scan = function( wallet_transaction_record, wallet )
{
	/*
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
	*/
}
