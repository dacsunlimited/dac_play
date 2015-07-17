#include <bts/blockchain/packet_record.hpp>
#include <bts/blockchain/chain_interface.hpp>

namespace bts { namespace blockchain {

    // Using next_available_claim_index() >= claim_statuses.size() claim that the packet is out
    uint32_t      packet_record::next_available_claim_index() const
    {
        uint32_t index = 0;
        for ( uint32_t i = 0; i < claim_statuses.size(); i ++ )
        {
            if ( claim_statuses[i].account_id == -1) {
                return index;
            }
            else
            {
                index ++;
            }
        }
        
        return index;
    }
    
    bool          packet_record::is_unclaimed_empty() const
    {
        return next_available_claim_index() >= claim_statuses.size();
    }
    
    asset         packet_record::left_packet_amount() const
    {
        asset left_amount(0, amount.asset_id);
        for ( uint32_t i = 0; i < claim_statuses.size(); i ++ )
        {
            if ( claim_statuses[i].account_id == -1) {
                left_amount += claim_statuses[i].amount;
            }
            else
            {
                // pass, already be claimed;
            }
        }
        
        return left_amount;
    }
    
void packet_record::sanity_check( const chain_interface& db )const
{ try {
    FC_ASSERT( from_account_id == 0 || db.lookup<account_record>( abs( from_account_id ) ).valid() );
    FC_ASSERT( amount.amount > 0 );
    FC_ASSERT( amount.asset_id == 0 || db.lookup<asset_record>( amount.asset_id ).valid() );
    for ( auto status : claim_statuses )
    {
        FC_ASSERT( status.amount.asset_id == amount.asset_id );
    }
} FC_CAPTURE_AND_RETHROW( (*this) ) }

opacket_record packet_record::lookup( const chain_interface& db, const packet_id_type& id )
{ try {
    return db.packet_lookup_by_index( id );
} FC_CAPTURE_AND_RETHROW( (id) ) }

void packet_record::store( chain_interface& db, const packet_id_type& id, const packet_record& record )
{ try {
    db.packet_insert_into_index_map( id, record );
} FC_CAPTURE_AND_RETHROW( (id)(record) ) }

void packet_record::remove( chain_interface& db, const packet_id_type& id )
{ try {
    const opacket_record prev_record = db.lookup<packet_record>( id );
    if( prev_record.valid() )
        db.packet_erase_from_index_map( id );
} FC_CAPTURE_AND_RETHROW( (id) ) }

} } // bts::blockchain
