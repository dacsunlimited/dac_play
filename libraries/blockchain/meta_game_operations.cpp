#include <bts/blockchain/meta_game_operations.hpp>
#include <bts/blockchain/chain_interface.hpp>
#include <bts/blockchain/exceptions.hpp>

namespace bts { namespace blockchain {

   void create_game_operation::evaluate( transaction_evaluation_state& eval_state ) const
   { try {
      if( NOT eval_state._current_state->is_valid_symbol_name( this->symbol ) )
          FC_CAPTURE_AND_THROW( invalid_asset_symbol, (symbol) );

      ogame_record current_game_record = eval_state._current_state->get_game_record( this->symbol );
      if( current_game_record.valid() )
          FC_CAPTURE_AND_THROW( game_symbol_in_use, (symbol) );

      if( this->name.empty() )
          FC_CAPTURE_AND_THROW( invalid_game_name, (this->name) );

      const game_id_type game_id = eval_state._current_state->last_game_id() + 1;
      current_game_record = eval_state._current_state->get_game_record( game_id );
      if( current_game_record.valid() )
          FC_CAPTURE_AND_THROW( game_id_in_use, (game_id) );

      oaccount_record issuer_account_record = eval_state._current_state->get_account_record( this->issuer_account_id );
      if( NOT issuer_account_record.valid() )
           FC_CAPTURE_AND_THROW( unknown_account_id, (issuer_account_id) );
       
      oasset_record game_asset_record = eval_state._current_state->get_asset_record( this->asset_id );
      FC_ASSERT( game_asset_record.valid() );
       
      oaccount_record game_asset_issuer_account_record = eval_state._current_state->get_account_record( game_asset_record->issuer_account_id );
      FC_ASSERT( game_asset_issuer_account_record.valid() );
       
      if( !eval_state.check_signature( game_asset_issuer_account_record->active_key() ) )
           FC_CAPTURE_AND_THROW( missing_signature, (game_asset_issuer_account_record) );
      
      // TODO: replace with game fee
      const asset reg_fee( eval_state._current_state->get_asset_registration_fee( this->symbol.size() ), 0 );
      eval_state.required_fees += reg_fee;

      game_record new_record;
      new_record.id                     = eval_state._current_state->new_game_id();
      new_record.symbol                 = this->symbol;
      new_record.name                   = this->name;
      new_record.description            = this->description;
      new_record.public_data            = this->public_data;
      new_record.issuer_account_id      = this->issuer_account_id;
      new_record.asset_id               = this->asset_id;
      new_record.script_url             = this->script_url;
      new_record.script_hash            = this->script_hash;
      new_record.registration_date      = eval_state._current_state->now();
      new_record.last_update            = new_record.registration_date;

      eval_state._current_state->store_game_record( new_record );
   } FC_CAPTURE_AND_RETHROW( (*this) ) }
} } // bts::blockchain
