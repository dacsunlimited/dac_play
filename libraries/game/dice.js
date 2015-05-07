// This is the rule definition for demo dice game
// All data structrue are defined in JSON in javascript, and being mapped to variant object in C++, be packed before storing in database
// Data structures that is need by this game
/******************
    struct dice_data
    {
        dice_data(){}
        
        dice_id_type        id = dice_id_type();
        address             owner;
        share_type          amount;
        uint32_t            odds;
        uint32_t            guess;
    };
    
    struct dice_rule
    {
        static const uint8_t    type;
        
        dice_rule():amount(0), odds(1){}
        
        dice_rule( const bts::blockchain::address& owner, bts::blockchain::share_type amnt, uint32_t odds = 2, uint32_t g = 1 );
        
        // This util method should be provided as util methods in JSON and C++
        bts::blockchain::address owner()const;
        
        // owner is just the hash of the condition
        bts::blockchain::balance_id_type                balance_id()const;
        
        bts::blockchain::share_type          amount;
        uint32_t            odds;
        uint32_t            guess;
        
        // the condition that the funds may be withdrawn,this is only necessary if the address is new.
        bts::blockchain::withdraw_condition  condition;
    };
    
    struct dice_input
    {
        dice_input() {}
        dice_input( std::string name, double a, uint32_t o, uint32_t g )
        :from_account_name(name), amount(a), odds(o), guess(g) {}
        
        std::string     from_account_name;
        double          amount;
        uint32_t        odds;
        uint32_t        guess;
    };
    
    struct dice_transaction
    {
        static const uint8_t    type;
        
        dice_transaction(){}
        
        address                                   play_owner;
        address                                   jackpot_owner;
        share_type                                play_amount;
        share_type                                jackpot_received;
        uint32_t                                  odds;
        uint32_t                                  lucky_number;
    };
    const uint8_t dice_rule::type = dice_rule_type;
    const uint8_t dice_transaction::type = dice_rule_type;
**************/
// require("play.js")
// TODO: Add the substitute for FC_CAPTURE_AND_THROW and FC_ASSERT
// TODO: Mapping between js object and C++ variant
// TODO: Input: {game_input}, Output: {operation_game_data, wallet_transaction_record, game_result_transaction}

var

PLAY = PLAY || {},

versoin = "0.0.1",

BTS_BLOCKCHAIN_NUM_DELEGATES = 101,

BTS_BLOCKCHAIN_NUM_DICE = BTS_BLOCKCHAIN_NUM_DELEGATES / 10,

BTS_BLOCKCHAIN_DICE_RANGE = 10000,

BTS_BLOCKCHAIN_DICE_HOUSE_EDGE = 0,

// TODO: We should not have the assumption that the asset id equals to game id.
dice_game = {
        asset_id : 1,
        asset : "DICE",
        game_symbol: "DICE"
    },

	game_type = 1;

/*
 * Play this game with input in the context to blockchain and wallet
 * 
 * input is a variant object passed in by v8 api {TODO: How to covert variant in C++ to js object here}
 * sign indicates that the result transaction should be signed or not
 * { operations, ledger_entries, required_signatures}
 */
PLAY.play = function (blockchain, wallet, input, record, trx) {
    //try {  
    
    // V8_Vaild
    //FC_ASSERT( input.amount > 0 );
    //FC_ASSERT( input.odds > 0 );
    
    var required_signatures = [];   // TODO: js array to unordered_set<address>
    
    // V8_API: wallet::get_transaction_fee
    var required_fees = wallet.get_transaction_fee();
    
    // V8_API: blockchain::get_asset_record
    // TODO: permission limitation to other assets.
    var asset_record = blockchain.get_asset_record(dice_game.asset);
    //FC_ASSERT( asset_rec.valid() );
    
    // V8_API: asset_record.precision
    // TODO: access property precision for asset record
    var amount_to_play = input.amount * asset_record.precision;
    // V8_Valid
    // FC_ASSERT( amount_to_play > 0 );
    
    // V8_API: constructor for asset, and accessor to id
    var chips_to_play = asset(amount_to_play, asset_record.id);
    
    // V8_Vaild
    //if( ! blockchain->is_valid_account_name( input.from_account_name ) )
    //    FC_THROW_EXCEPTION( bts::wallet::invalid_name, "Invalid account name!", ("dice_account_name",d_input.from_account_name) );
    
    // V8_API: blockchain::get_account_record
    var play_account = blockchain.get_account_record(input.from_account_name);
    
    // TODO make sure it is using account active key    
    wallet.withdraw_to_transaction(chips_to_play,
                                    input.from_account_name,
                                    trx,
                                    required_signatures);
        
    wallet.withdraw_to_transaction( required_fees,
                                    input.from_account_name,
                                    trx,
                                    required_signatures );
    
    // V8_API: play_account::active_key
    required_signatures.push( play_account.active_key() );
    
    // TODO: Dice, specify to account, the receiver who can claim jackpot
    
    // bts::game::dice_rule(address( play_account->active_key() ), amount_to_play, input.odds, input.guess )
	// TODO: changed to game_input
    var dice_rule = {
        address : address( play_account.active_key() ),     // TODO, address constructor
        amount  : amount_to_play,
        odds    : input.odds,
        guess   : input.guess
    };
    
    // slate_id 0, TODO: game_operation constructor, trx operations accessor
    // TODO: then how to map the structs, delete bts::game::rule in C++, replace with a rule_id and variant
    // trx.operations.push_back( game_operation(bts::game::dice_rule(address( play_account->active_key() ), amount_to_play, d_input.odds, d_input.guess ))//slate_id 0 );
    // create the operations according to rule_type and input types, ?
    
    // V8_API: trx.operations
    // V8_API: operations::push ?? if operations is array this might not needed.
    // V8_API: contructor game_operation()
    trx.operations.push( game_operation(dice_rule) );

    // json to bts::wallet::ledger_entry, might similar to json --> variant --> ledger_entry.from_variant
    var entry = {
        from_account : play_account.active_key(),
        to_account : play_account.active_key(),
        memo : "play dice"
    }

    record.ledger_entries.push(entry);
    record.fee = required_fees;
        
    //} FC_CAPTURE_AND_RETHROW( (params) )
};

/*
 * Instead of defining the evaluate function as a method of rule, pass the rule to the function
 */
PLAY.evaluate = function(self, eval_state, eval_state_current_state){
    // V8_Valid
    //if( self.odds < 1 || self.odds < self.guess || self.guess < 1)
    //    FC_CAPTURE_AND_THROW( invalid_dice_odds, (odds) );
        
    // V8_API: eval_state_current_state::get_asset_record
    var dice_asset_record = eval_state_current_state.get_asset_record(dice_game.asset);
    // V8_Valid
    //if( !dice_asset_record )
        //FC_CAPTURE_AND_THROW( unknown_asset_symbol, ( eval_state.trx.id() ) );
    
    // For each transaction, there must be only one dice operatiion exist
    // TODO: improve the rule id representation for rule record
    // V8_API: eval_state_current_state::get_game_data_record
    var cur_record = eval_state_current_state.get_game_data_record(game_type, eval_state.trx.id()._hash[0]);
    
    // V8_Valid
    //if( cur_record )
        //FC_CAPTURE_AND_THROW( duplicate_dice_in_transaction, ( eval_state.trx.id() ) );
    
    
    
    // TODO: Game Logic: this does not means the balance are now stored in balance record, just over pass the api
    // the dice record are not in any balance record, they are over-fly-on-sky.
    // V8_API: dice_asset_record.id
    // V8_API: self::balance_id and self.amount what the self here.
    // V8_API: eval_state::sub_balance
    eval_state.sub_balance( asset(self.amount, dice_asset_record.id));
    
    
    // rule_dice_record cur_data;
    var dice_record = {
        id : eval_state.trx.id(),
        amount : self.amount,
        owner : self.owner(),
        odds : self.odds,
        guess : self.guess
    };
    
    // TODO: Game Logic: remove game_data_record
    // V8_API: eval_state_current_state::store_game_data_record
    eval_state_current_state.store_game_data_record(game_type, cur_data.id._hash[0], game_data_record(dice_record));
};

// game execute during extain chain and deterministrix transaction apply
PLAY.execute = function (blockchain, block_num, pending_state){
	if (block_num <= BTS_BLOCKCHAIN_NUM_DICE){
   	    return;
	}

	var block_random_num = blockchain_context.get_current_random_seed();

	var range = BTS_BLOCKCHAIN_DICE_RANGE;

	var block_num_of_dice = block_num - BTS_BLOCKCHAIN_NUM_DICE;

	var block_of_dice = blockchain_context.getblock(block_num_of_dice);
	
	var trxs = block_of_dice.get_transactions();
	
	for (var trx in trxs)
	{
		var id = trx.id();
		// TODO: define the type
		var game_data = blockchain_context.get_game_data_record(type, id.hash(0));
        
		if (game_data)
		{
			// TODO hash to be defined in V8
			var dice_random_num = id.hash(0);
			
			// win condition
            var lucky_number = ( ( ( block_random_num % range ) + ( dice_random_num % range ) ) % range ) * (game_data.odds);
            var guess = game_data.guess;
            var jackpot = 0;
            if ( lucky_number >= (guess - 1) * range && lucky_number < guess * range )
            {
                jackpot = game_data.amount * (game_data.odds) * (100 - BTS_BLOCKCHAIN_DICE_HOUSE_EDGE) / 100;
                
                // add the jackpot to the accout's balance, give the jackpot from virtul pool to winner
                   
                // TODO: Dice, what should be the slate_id for the withdraw_with_signature, if need, we can set to the jackpot owner?
                var jackpot_balance_address = V8_Global_Get_Balance_ID_For_Owner(game_data.owner, dice_game.asset_id);
                var jackpot_payout = pending_state.get_balance_record( jackpot_balance_address );
                if( !jackpot_payout )
                    jackpot_payout = balance_record( game_data.owner, asset(0, dice_game.asset_id), dice_game.asset_id);
                jackpot_payout.balance += jackpot;
                jackpot_payout.last_update = Date.now();
                   
                pending_state.store_balance_record( jackpot_payout );
                   
                shares_created += jackpot;
            }
               
            // balance destroyed
            shares_destroyed += game_data.amount;
            
			// remove the dice_record from pending state after execute the jackpot
            pending_state.store_game_data_record(type, id._hash[0], null);
               
            var dice_trx = {
                play_owner : game_data.owner,
                jackpot_owner : game_data.owner,
                play_amount : game_data.amount,
                jackpot_received : jackpot,
                odds : game_data.odds,
                lucky_number : (lucky_number / range) + 1
            };

            // TODO: There is no necessary for game_result_transaction to exsit anymore, dice_trx are directly stored as variant
            game_result_transactions.push(game_result_transaction(dice_trx));
		}
	}
	
	pending_state.set_game_result_transactions( game_result_transactions );
    
    // TODO: what is asset_id_type?
	var base_asset_record = pending_state.get_asset_record( asset_id_type(1) );
	// FC_ASSERT( base_asset_record.valid() );
	base_asset_record.current_share_supply += (shares_created - shares_destroyed);
	pending_state.store_asset_record( base_asset_record );
};

PLAY.scan_result = function( game_result_trx, block_num, block_time, trx_index, wallet)
{
    //try {
    // auto gtrx = rtrx.as<dice_transaction>(); game_result_trx now is a variant/js_object it self, so no need to convert
    var win = ( game_result_trx.jackpot_received != 0 );
    var play_result = win ? "win" : "lose";
    
    // TODO: Dice, play owner might be different with jackpot owner
    // TODO: Accessor get_wallet_key_for_address for wallet
    // TODO: Accessor has_private_key for wallet_key
    // TODO: Property account_address for wallet_key
    var okey_jackpot = wallet.get_wallet_key_for_address( game_result_trx.jackpot_owner );
    if( okey_jackpot && okey_jackpot.has_private_key() )
    {
        var jackpot_account_key = wallet.get_wallet_key_for_address( okey_jackpot.account_address );
        
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
        var ledger_entries = [];
        
        ledger_entries.push({
            // TODO Property public_key for wallet_key
            // TODO: Constructor for asset()
            to_account : jackpot_account_key.public_key,
            amount : asset(game_result_trx.jackpot_received, 1),
            memo : play_result + ", jackpot lucky number: " + game_result_trx.lucky_number
        }
        );
        
        // TODO: Don't blow away memo, etc.
        var wallet_transaction_record = {
            //  Construct a unique record id, TODO: js method for fc::ripemd160::hash, could refer bitshares-js repository
            // TODO: record_id : fc::ripemd160::hash( "" + block_num + game_result_trx.jackpot_owner + trx_index ),
            block_num : block_num,
            is_virtual : true,
            is_confirmed : true,
            is_market : true,
            ledger_entries : ledger_entries,
            // TODO: Constructor for asset()
            // TODO: Dice, do we need fee for claim jackpot? may be later we'll support part to delegates
            fee : asset(0),
            created_time : block_time,
            received_time : received_time
        };
        
        // TODO: Accessor store_transaction for wallet
        wallet.store_transaction( wallet_transaction_record );
    }
    
    return true;
    
    //} FC_CAPTURE_AND_RETHROW((rtrx)) 
};
    
PLAY.scan = function( rule, wallet_transaction_record, wallet )
{
    // TODO: Accessor to type, (withdraw_condition_types) 
    // TODO: Define withdraw type constants
    switch( rule.condition.type )
    {
        case withdraw_null_type:
        {
            // TODO
            // FC_THROW( "withdraw_null_type not implemented!" );
            break;
        }
        case withdraw_signature_type:
        {
            // TODO: auto condtion = rule.condition.as<withdraw_with_signature>();
            // TODO: lookup if cached key and work with it only
            // if( _wallet_db.has_private_key( deposit.owner ) )
            if( rule.condtion.memo )
            {
                // TODO: TITAN, FC_THROW( "withdraw_option_type not implemented!" );
                break;
            } else
            {
                
                var opt_key_rec = wallet.get_wallet_key_for_address(rule.condtion.owner);
                if( opt_key_rec.valid() && opt_key_rec.has_private_key() )
                {
                    // TODO: Refactor this
                    for( var entry in trx_rec.ledger_entries )
                    {
                        // TODO: Read Accessor to_account
                        if( !entry.to_account.valid() )
                        {
                            // TODO: Write Accessor to following properties
                            // TODO: Read Accessor to public_key
                            entry.to_account = opt_key_rec.public_key;
                            // TODO: Constructor asset( amount, 1 )
                            entry.amount = asset( self.amount, dice_game.asset_id );
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
            // TODO: FC_THROW( "unknown withdraw condition type!" );
            break;
        }
    }
   
   return false;
}
