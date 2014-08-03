#include <bts/blockchain/dice_operations.hpp>
#include <bts/blockchain/chain_interface.hpp>
#include <bts/blockchain/exceptions.hpp>

namespace bts { namespace blockchain {

/**
 *  @note in this method we are using 'this->' to refer to member variables for
 *  clarity.
 */
void dice_operation::evaluate( transaction_evaluation_state& eval_state )
{ try {
    if( this->odds < 1 )
        FC_CAPTURE_AND_THROW( invalid_dice_odds, (odds) );
    
    auto dice_account_record = eval_state._current_state->get_account_record( this->dice_account_id );
    if( NOT dice_account_record )
        FC_CAPTURE_AND_THROW( unknown_account_id, (this->dice_account_id) );
    
    /*
     * For each transaction, there must be only one dice operatiion exist
     */
    auto cur_record = eval_state._current_state->get_dice_record( eval_state.trx.id() );
    if( cur_record )
        FC_CAPTURE_AND_THROW( duplicate_dice_in_transaction, ( eval_state.trx.id() ) );
    
    if( NOT eval_state.check_signature( dice_account_record->active_address() ) )
    {
        FC_CAPTURE_AND_THROW( missing_signature, (dice_account_record->active_key()) );
    }
    
    if( dice_account_record->is_retracted() ) FC_CAPTURE_AND_THROW( account_retracted, (dice_account_record) );
    
    // this does not means the balance are now stored in balance record, just over pass the api
    // the dice record are not in any balance record, they are over-fly-on-sky..
    // TODO: Dice Review
    eval_state.sub_balance(address(), asset( this->amount, 0 ));
    
    dice_record new_record;
    new_record.id               = eval_state.trx.id();
    new_record.account_id       = this->dice_account_id;
    new_record.amount          = this->amount;
    new_record.odds             = this->odds;
    
    eval_state._current_state->store_dice_record( new_record );
    
} FC_CAPTURE_AND_RETHROW( (*this) ) }

} } // bts::blockchain