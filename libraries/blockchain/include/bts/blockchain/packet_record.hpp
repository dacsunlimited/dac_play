#pragma once

#include <bts/blockchain/asset.hpp>

namespace bts { namespace blockchain {

struct packet_record;
typedef fc::optional<packet_record> opacket_record;

struct red_packet_status
{
    asset                                    amount;
    account_id_type                          account_id;
    transaction_id_type                      transaction_id;
};
    
class chain_interface;
struct packet_record
{
    packet_id_type                           id;
    asset                                    amount;
    account_id_type                          from_account_id;
    public_key_type                          claim_public_key;
    string                                   message;
    vector< red_packet_status >              claim_statuses;
    
    uint32_t      next_available_claim_index() const;
    
    bool          is_unclaimed_empty() const;
    
    asset         left_packet_amount() const;

    void sanity_check( const chain_interface& )const;
    static opacket_record lookup( const chain_interface&, const packet_id_type& );
    static void store( chain_interface&, const packet_id_type&, const packet_record& );
    static void remove( chain_interface&, const packet_id_type& );
};

class packet_db_interface
{
    friend struct packet_record;

    virtual opacket_record packet_lookup_by_index( const packet_id_type& )const = 0;
    virtual void packet_insert_into_index_map( const packet_id_type&, const packet_record& ) = 0;
    virtual void packet_erase_from_index_map( const packet_id_type& ) = 0;
};

} } // bts::blockchain

FC_REFLECT( bts::blockchain::red_packet_status, (amount)(account_id)(transaction_id) )
FC_REFLECT( bts::blockchain::packet_record, (id)(amount)(from_account_id)(claim_public_key)(message)(claim_statuses) )
