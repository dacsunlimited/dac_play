#include <bts/blockchain/game_record.hpp>
#include <bts/blockchain/chain_interface.hpp>

namespace bts { namespace blockchain {

    game_record game_record::make_null()const
    {
        game_record cpy(*this);
        cpy.issuer_account_id = -1;
        return cpy;
    }

    void game_record::sanity_check( const chain_interface& db )const
    { try {
        FC_ASSERT( id >= 0 );
        FC_ASSERT( !symbol.empty() );
        FC_ASSERT( !name.empty() );
        FC_ASSERT( id == 0 || db.lookup<account_record>( issuer_account_id ).valid() );
        // TODO: Check asset id and rule id
    } FC_CAPTURE_AND_RETHROW( (*this) ) }
   
   ogame_record game_record::lookup( const chain_interface& db, const game_id_type id)
   {
      try {
         return db.game_lookup_by_id( id );
      } FC_CAPTURE_AND_RETHROW( (id) ) }
   
   ogame_record game_record::lookup( const chain_interface& db, const string& symbol)
   {
      try {
         return db.game_lookup_by_symbol( symbol );
      } FC_CAPTURE_AND_RETHROW( (symbol) ) }
   
   void game_record::store( chain_interface& db, const game_id_type id, const game_record& record)
   {
      try {
         const oasset_record prev_record = db.lookup<asset_record>( id );
         if( prev_record.valid() )
         {
            if( prev_record->symbol != record.symbol )
               db.game_erase_from_symbol_map( prev_record->symbol );
         }
         
         db.game_insert_into_id_map( id, record );
         db.game_insert_into_symbol_map( record.symbol, id );
      } FC_CAPTURE_AND_RETHROW( (id)(record) ) }
   
   void game_record::remove( chain_interface& db, const game_id_type id)
   {
      try {
         const ogame_record prev_record = db.lookup<game_record>( id );
         if( prev_record.valid() )
         {
            db.game_erase_from_id_map( id );
            db.game_erase_from_symbol_map( prev_record->symbol );
         }
      } FC_CAPTURE_AND_RETHROW( (id) ) }
   
}} // bts::blockchain
