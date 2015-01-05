// require("play.js")

var BTS_BLOCKCHAIN_NUM_DELEGATES = 101	
var BTS_BLOCKCHAIN_NUM_DICE = BTS_BLOCKCHAIN_NUM_DELEGATES / 10;
var BTS_BLOCKCHAIN_DICE_RANGE = 10000;
var BTS_BLOCKCHAIN_DICE_HOUSE_EDGE = 0;

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
