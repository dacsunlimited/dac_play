#include <bts/blockchain/ad_record.hpp>
#include <bts/blockchain/chain_interface.hpp>

namespace bts { namespace blockchain {

public_key_type ad_record::signer_key()const
{ try {
    FC_ASSERT( signer.valid() );
    fc::sha256 digest;
    if( !message.empty() ) digest = fc::sha256::hash( message.c_str(), message.size() );
    return fc::ecc::public_key( *signer, digest );
} FC_CAPTURE_AND_RETHROW() }

void ad_record::sanity_check( const chain_interface& db )const
{ try {
    FC_ASSERT( index.account_id == 0 || db.lookup<account_record>( abs( index.account_id ) ).valid() );
    FC_ASSERT( amount.amount > 0 );
    FC_ASSERT( amount.asset_id == 0 || db.lookup<asset_record>( amount.asset_id ).valid() );
} FC_CAPTURE_AND_RETHROW( (*this) ) }

oad_record ad_record::lookup( const chain_interface& db, const ad_index& index )
{ try {
    return db.ad_lookup_by_index( index );
} FC_CAPTURE_AND_RETHROW( (index) ) }

void ad_record::store( chain_interface& db, const ad_index& index, const ad_record& record )
{ try {
    db.ad_insert_into_index_map( index, record );
} FC_CAPTURE_AND_RETHROW( (index)(record) ) }

void ad_record::remove( chain_interface& db, const ad_index& index )
{ try {
    const oad_record prev_record = db.lookup<ad_record>( index );
    if( prev_record.valid() )
        db.ad_erase_from_index_map( index );
} FC_CAPTURE_AND_RETHROW( (index) ) }

} } // bts::blockchain
