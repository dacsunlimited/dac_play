#include <bts/game/game_operations.hpp>
#include <bts/blockchain/chain_interface.hpp>
#include <bts/blockchain/exceptions.hpp>
#include <bts/game/client.hpp>
#include <bts/game/v8_game.hpp>

namespace bts { namespace game {
    using namespace bts::blockchain;
    
    void create_game_operation::evaluate( transaction_evaluation_state& eval_state ) const
    { try {
        ogame_record current_game_record = eval_state.pending_state()->get_game_record( this->name );
        if( current_game_record.valid() )
            FC_CAPTURE_AND_THROW( game_name_in_use, (name) );
        
        if( this->name.empty() )
            FC_CAPTURE_AND_THROW( invalid_game_name, (this->name) );
        
        if( !eval_state.pending_state()->is_valid_account_name( this->name ) )
            FC_THROW_EXCEPTION( invalid_game_name, "Invalid name for a game!", ("game_name",this->name) );
        
        const game_id_type game_id = eval_state.pending_state()->last_game_id() + 1;
        current_game_record = eval_state.pending_state()->get_game_record( game_id );
        if( current_game_record.valid() )
            FC_CAPTURE_AND_THROW( game_id_in_use, (game_id) );
        
        oaccount_record owner_account_record = eval_state.pending_state()->get_account_record( this->owner_account_id );
        if( NOT owner_account_record.valid() )
            FC_CAPTURE_AND_THROW( unknown_account_id, (owner_account_id) );
        
        const asset reg_fee( eval_state.pending_state()->get_game_registration_fee( this->name.size() ), 0 );
        eval_state.min_fees[reg_fee.asset_id] += reg_fee.amount;
        
        game_record new_record;
        new_record.id                     = eval_state.pending_state()->new_game_id();
        new_record.name                   = this->name;
        new_record.description            = this->description;
        new_record.public_data            = this->public_data;
        new_record.owner_account_id       = this->owner_account_id;
        new_record.script_url             = this->script_url;
        new_record.script_hash            = this->script_hash;
        new_record.registration_date      = eval_state.pending_state()->now();
        new_record.last_update            = new_record.registration_date;
        
        eval_state.pending_state()->store_game_record( new_record );
        
        // TODO: maybe it is better to move following steps to blockchain.extend_chain, in case script download failed or other exception.
        // Downloading the script file, and init the script engine.
        bts::game::client::get_current().game_claimed_script( script_url, name, script_hash );
        bts::game::client::get_current().get_v8_engine( name );
    } FC_CAPTURE_AND_RETHROW( (*this) ) }

    /**
     *  @note in this method we are using 'this->' to refer to member variables for
     *  clarity.
     */
    void game_operation::evaluate( transaction_evaluation_state& eval_state ) const
    { try {
        // TODO: shoud provide with game_input
        auto ogame = eval_state.pending_state()->get_game_record( input.game_id );
        
        if( NOT ogame.valid() )
            FC_CAPTURE_AND_THROW( unknown_game, (input.game_id) );
        
        bts::game::client::get_current().get_v8_engine( ogame->name )->evaluate( eval_state );

    } FC_CAPTURE_AND_RETHROW( (*this) ) }

} } // bts::game
