#include <bts/game/game_operations.hpp>
#include <bts/blockchain/chain_interface.hpp>
#include <bts/blockchain/exceptions.hpp>
#include <bts/game/client.hpp>
#include <bts/game/v8_game.hpp>
#include <bts/blockchain/fork_blocks.hpp>

namespace bts { namespace game {
    using namespace bts::blockchain;
    
    void create_game_operation::evaluate( transaction_evaluation_state& eval_state ) const
    { try {
        #ifndef WIN32
        #warning [HARDFORK] Remove this check after PLS_V0_4_0_FORK_BLOCK_NUM has passed
        #endif
        FC_ASSERT( eval_state.pending_state()->get_head_block_num() >= PLS_V0_4_0_FORK_BLOCK_NUM );
        
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
        
        // Work around, only the genesis account can create game for v0.4.0
        if ( owner_account_record->name != "play-bitsha256" )
        {
            FC_CAPTURE_AND_THROW( invalid_account_name, (owner_account_record->name) );
        }
        
        eval_state.account_or_any_parent_has_signed(*owner_account_record);
        
        const asset reg_fee( eval_state.pending_state()->get_game_registration_fee( this->name.size() ), 0 );
        eval_state.min_fees[reg_fee.asset_id] += reg_fee.amount;
        
        game_record new_record;
        new_record.id                     = eval_state.pending_state()->new_game_id();
        new_record.name                   = this->name;
        new_record.description            = this->description;
        new_record.public_data            = this->public_data;
        new_record.owner_account_id       = this->owner_account_id;
        new_record.script_code            = this->script_code;
        new_record.registration_date      = eval_state.pending_state()->now();
        new_record.last_update            = new_record.registration_date;
        
        eval_state.pending_state()->store_game_record( new_record );
        
        // only for debuging (download the script)
        // bts::game::client::get_current().game_claimed_script( script_code, name );
        
        try {
            bts::game::client::get_current().get_v8_engine( name );
        }
        catch (const game_engine_not_found& e)
        {
            wlog("game engine note found, failed to init for unknown reason during evaluate operation");
        }
        
    } FC_CAPTURE_AND_RETHROW( (*this) ) }
    
    void game_update_operation::evaluate( transaction_evaluation_state& eval_state ) const
    { try {
        #ifndef WIN32
        #warning [HARDFORK] Remove this check after PLS_V0_4_0_FORK_BLOCK_NUM has passed
        #endif
        FC_ASSERT( eval_state.pending_state()->get_head_block_num() >= PLS_V0_4_0_FORK_BLOCK_NUM );
        
        ogame_record current_game_record = eval_state.pending_state()->get_game_record( this->game_id );
        
        if( ! current_game_record.valid() )
            FC_CAPTURE_AND_THROW( unknown_game, (this->game_id) );
        
        oaccount_record owner_account_record = eval_state.pending_state()->get_account_record( current_game_record->owner_account_id );
        if( NOT owner_account_record.valid() )
            FC_CAPTURE_AND_THROW( unknown_account_id, (current_game_record->owner_account_id) );
        
        eval_state.account_or_any_parent_has_signed(*owner_account_record);
        
        // TODO: update later, how to decide the fee of update games
        const asset reg_fee( eval_state.pending_state()->get_game_registration_fee( current_game_record->name.size() ), 0 );
        eval_state.min_fees[reg_fee.asset_id] += reg_fee.amount;
        
        current_game_record->description            = this->description;
        current_game_record->public_data            = this->public_data;
        current_game_record->script_code            = this->script_code;
        current_game_record->last_update            = eval_state.pending_state()->now();
        
        eval_state.pending_state()->store_game_record( *current_game_record );
        
    } FC_CAPTURE_AND_RETHROW( (*this) ) }

    /**
     *  @note in this method we are using 'this->' to refer to member variables for
     *  clarity.
     */
    void game_play_operation::evaluate( transaction_evaluation_state& eval_state ) const
    { try {
        #ifndef WIN32
        #warning [HARDFORK] Remove this check after PLS_V0_4_0_FORK_BLOCK_NUM has passed
        #endif
        
        FC_ASSERT( eval_state.pending_state()->get_head_block_num() >= PLS_V0_4_0_FORK_BLOCK_NUM );
        
        // TODO: shoud provide with game_input
        auto ogame = eval_state.pending_state()->get_game_record( input.game_id );
        
        if( NOT ogame.valid() )
            FC_CAPTURE_AND_THROW( unknown_game, (input.game_id) );
        
        bts::game::client::get_current().get_v8_engine( ogame->name )->evaluate( eval_state, input.game_id, input.data );

    } FC_CAPTURE_AND_RETHROW( (*this) ) }

} } // bts::game
