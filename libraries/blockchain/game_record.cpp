#include <bts/blockchain/game_record.hpp>
#include <bts/blockchain/chain_interface.hpp>

namespace bts { namespace blockchain {

    game_record game_record::make_null()const
    {
        game_record cpy(*this);
        cpy.owner_account_id = -1;
        return cpy;
    }

    void game_record::sanity_check( const chain_interface& db )const
    { try {
        FC_ASSERT( id >= 0 );
        FC_ASSERT( !name.empty() );
        FC_ASSERT( id == 0 || db.lookup<account_record>( owner_account_id ).valid() );
        // TODO: Check asset id and rule id
    } FC_CAPTURE_AND_RETHROW( (*this) ) }
   
   ogame_record game_record::lookup( const chain_interface& db, const game_id_type id)
   {
      try {
         return db.game_lookup_by_id( id );
      } FC_CAPTURE_AND_RETHROW( (id) ) }
   
   ogame_record game_record::lookup( const chain_interface& db, const string& name)
   {
      try {
         return db.game_lookup_by_name( name );
      } FC_CAPTURE_AND_RETHROW( (name) ) }
   
   void game_record::store( chain_interface& db, const game_id_type id, const game_record& record)
   {
      try {
         const ogame_record prev_record = db.lookup<game_record>( id );
         if( prev_record.valid() )
         {
            if( prev_record->name != record.name )
               db.game_erase_from_name_map( prev_record->name );
         }
         
         db.game_insert_into_id_map( id, record );
         db.game_insert_into_name_map( record.name, id );
      } FC_CAPTURE_AND_RETHROW( (id)(record) ) }
   
   void game_record::remove( chain_interface& db, const game_id_type id)
   {
      try {
         const ogame_record prev_record = db.lookup<game_record>( id );
         if( prev_record.valid() )
         {
            db.game_erase_from_id_map( id );
            db.game_erase_from_name_map( prev_record->name );
         }
      } FC_CAPTURE_AND_RETHROW( (id) ) }
   
}} // bts::blockchain
