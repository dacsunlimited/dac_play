#include <bts/blockchain/note_record.hpp>
#include <bts/blockchain/chain_interface.hpp>

namespace bts { namespace blockchain {

public_key_type note_record::signer_key()const
{ try {
   FC_ASSERT( signer.valid() );
   fc::sha256 digest;
   if( !message->data.empty() ) digest = fc::sha256::hash( string(message->data.begin(), message->data.end()).c_str(), message->data.size() );
   return fc::ecc::public_key( *signer, digest );
} FC_CAPTURE_AND_RETHROW() }

void note_record::sanity_check( const chain_interface& db )const
{ try {
    FC_ASSERT( index.account_id == 0 || db.lookup<account_record>( abs( index.account_id ) ).valid() );
    FC_ASSERT( amount.amount > 0 );
    FC_ASSERT( amount.asset_id == 0 || db.lookup<asset_record>( amount.asset_id ).valid() );
} FC_CAPTURE_AND_RETHROW( (*this) ) }

onote_record note_record::lookup( const chain_interface& db, const note_index& index )
{ try {
    return db.note_lookup_by_index( index );
} FC_CAPTURE_AND_RETHROW( (index) ) }

void note_record::store( chain_interface& db, const note_index& index, const note_record& record )
{ try {
    db.note_insert_into_index_map( index, record );
} FC_CAPTURE_AND_RETHROW( (index)(record) ) }

void note_record::remove( chain_interface& db, const note_index& index )
{ try {
    const onote_record prev_record = db.lookup<note_record>( index );
    if( prev_record.valid() )
        db.note_erase_from_index_map( index );
} FC_CAPTURE_AND_RETHROW( (index) ) }

} } // bts::blockchain
