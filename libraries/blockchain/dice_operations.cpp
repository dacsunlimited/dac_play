#include <bts/blockchain/dice_operations.hpp>
#include <bts/blockchain/chain_interface.hpp>
#include <bts/blockchain/exceptions.hpp>

namespace bts { namespace blockchain {
    
    balance_id_type  dice_operation::balance_id()const
    {
        return condition.get_address();
    }

    dice_operation::dice_operation( const address& owner, share_type amnt, uint32_t o )
    {
        FC_ASSERT( amnt > 0 );
        amount = amnt;
        odds = o;
        // TODO: Dice specify the slate_id, if slate_id is added make sure the one in scan_jackpot_transaction is updated too.
        condition = withdraw_condition( withdraw_with_signature( owner ), 0);
    }
    
/**
 *  @note in this method we are using 'this->' to refer to member variables for
 *  clarity.
 */
void dice_operation::evaluate( transaction_evaluation_state& eval_state )
{ try {
    if( this->odds < 1 )
        FC_CAPTURE_AND_THROW( invalid_dice_odds, (odds) );
    
    /*
     * For each transaction, there must be only one dice operatiion exist
     */
    auto cur_record = eval_state._current_state->get_dice_record( eval_state.trx.id() );
    if( cur_record )
        FC_CAPTURE_AND_THROW( duplicate_dice_in_transaction, ( eval_state.trx.id() ) );
    
    cur_record = dice_record( this->condition );
    
    // this does not means the balance are now stored in balance record, just over pass the api
    // the dice record are not in any balance record, they are over-fly-on-sky..
    // TODO: Dice Review
    eval_state.sub_balance(this->balance_id(), asset( this->amount, 0 ));
    
    cur_record->id               = eval_state.trx.id();
    cur_record->amount           = this->amount;
    cur_record->odds             = this->odds;
    
    eval_state._current_state->store_dice_record( *cur_record );
    
} FC_CAPTURE_AND_RETHROW( (*this) ) }

} } // bts::blockchain