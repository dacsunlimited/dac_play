#include <bts/game/dice_game.hpp>
#include <bts/blockchain/chain_interface.hpp>
#include <bts/blockchain/exceptions.hpp>


namespace bts { namespace game {
    using namespace bts::blockchain;

    bts::blockchain::balance_id_type  dice_game::balance_id()const
    {
        return condition.get_address();
    }
    
    bts::blockchain::address dice_game::owner()const
    {
        if( condition.type == bts::blockchain::withdraw_signature_type )
            return condition.as<bts::blockchain::withdraw_with_signature>().owner;
        return bts::blockchain::address();
    }
    
    dice_game::dice_game( const bts::blockchain::address& owner, bts::blockchain::share_type amnt, uint32_t o , uint32_t g)
    {
        FC_ASSERT( amnt > 0 );
        amount = amnt;
        odds = o;
        guess = g;
        // TODO: Dice specify the slate_id, if slate_id is added make sure the one in scan_jackpot_transaction is updated too.
        condition = bts::blockchain::withdraw_condition( bts::blockchain::withdraw_with_signature( owner ), 1);
    }
    
    void dice_game::evaluate( transaction_evaluation_state& eval_state )
    {
        if( this->odds < 1 || this->odds < this->guess || this->guess < 1)
            FC_CAPTURE_AND_THROW( invalid_dice_odds, (odds) );
        
        auto dice_asset_record = eval_state._current_state->get_asset_record( "DICE" );
        if( !dice_asset_record )
            FC_CAPTURE_AND_THROW( unknown_asset_symbol, ( eval_state.trx.id() ) );
        
        /*
         * For each transaction, there must be only one dice operatiion exist
         */
        auto cur_record = eval_state._current_state->get_dice_record( eval_state.trx.id() );
        if( cur_record )
            FC_CAPTURE_AND_THROW( duplicate_dice_in_transaction, ( eval_state.trx.id() ) );
        
        cur_record = dice_record();
        
        // this does not means the balance are now stored in balance record, just over pass the api
        // the dice record are not in any balance record, they are over-fly-on-sky..
        // TODO: Dice Review
        eval_state.sub_balance(this->balance_id(), asset( this->amount, dice_asset_record->id ));
        
        cur_record->id               = eval_state.trx.id();
        cur_record->amount           = this->amount;
        cur_record->owner            = this->owner();
        cur_record->odds             = this->odds;
        cur_record->guess            = this->guess;
        
        eval_state._current_state->store_dice_record( *cur_record );
    }

} } // bts::game
