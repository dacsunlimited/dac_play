#include <bts/blockchain/game_record.hpp>

namespace bts { namespace blockchain {

    game_record game_record::make_null()const
    {
        game_record cpy(*this);
        cpy.issuer_account_id = -1;
        return cpy;
    }
    
    const game_db_interface& game_record::db_interface( const chain_interface& db )
    { try {
        return db._game_db_interface;
    } FC_CAPTURE_AND_RETHROW() }
    
    ogame_record game_db_interface::lookup( const game_id_type id )const
    { try {
        return lookup_by_id( id );
    } FC_CAPTURE_AND_RETHROW( (id) ) }
    
    ogame_record game_db_interface::lookup( const string& symbol )const
    { try {
        return lookup_by_symbol( symbol );
    } FC_CAPTURE_AND_RETHROW( (symbol) ) }
    
    void game_db_interface::store( const game_record& record )const
    { try {
        const ogame_record prev_record = lookup( record.id );
        if( prev_record.valid() )
        {
            if( prev_record->symbol != record.symbol )
                erase_from_symbol_map( prev_record->symbol );
        }
        
        insert_into_id_map( record.id, record );
        insert_into_symbol_map( record.symbol, record.id );
    } FC_CAPTURE_AND_RETHROW( (record) ) }
    
    void game_db_interface::remove( const account_id_type id )const
    { try {
        const ogame_record prev_record = lookup( id );
        if( prev_record.valid() )
        {
            erase_from_id_map( id );
            erase_from_symbol_map( prev_record->symbol );
        }
    } FC_CAPTURE_AND_RETHROW( (id) ) }

}} // bts::blockchain
