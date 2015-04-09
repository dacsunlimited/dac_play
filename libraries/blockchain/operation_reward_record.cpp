#include <bts/blockchain/chain_interface.hpp>
#include <bts/blockchain/operation_reward_record.hpp>

#include <fc/crypto/sha256.hpp>
#include <fc/io/raw.hpp>

namespace bts { namespace blockchain {

void operation_reward_record::sanity_check( const chain_interface& db )const
{ try {
    //FC_ASSERT( !fees.empty() );
} FC_CAPTURE_AND_RETHROW( (*this) ) }

ooperation_reward_record operation_reward_record::lookup( const chain_interface& db, const operation_type id )
{ try {
    return db.operation_reward_lookup_by_id( id );
} FC_CAPTURE_AND_RETHROW( (id) ) }

void operation_reward_record::store( chain_interface& db, const operation_type id, const operation_reward_record& record )
{ try {
    db.operation_reward_insert_into_id_map( id, record );
} FC_CAPTURE_AND_RETHROW( (id)(record) ) }

void operation_reward_record::remove( chain_interface& db, const operation_type id )
{ try {
    const ooperation_reward_record prev_record = db.lookup<operation_reward_record>( id );
    if( prev_record.valid() )
        db.operation_reward_erase_from_id_map( id );
} FC_CAPTURE_AND_RETHROW( (id) ) }

} } // bts::blockchain
