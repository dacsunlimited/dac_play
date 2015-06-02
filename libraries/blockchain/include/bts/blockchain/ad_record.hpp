#pragma once

#include <bts/blockchain/asset.hpp>
#include <bts/blockchain/balance_operations.hpp>

namespace bts { namespace blockchain {

struct ad_index
{
    account_id_type     account_id;
    transaction_id_type transaction_id;

    friend bool operator < ( const ad_index& a, const ad_index& b )
    {
        return std::tie( a.account_id, a.transaction_id ) < std::tie( b.account_id, b.transaction_id );
    }

    friend bool operator == ( const ad_index& a, const ad_index& b )
    {
        return std::tie( a.account_id, a.transaction_id ) == std::tie( b.account_id, b.transaction_id );
    }
};

struct ad_record;
typedef fc::optional<ad_record> oad_record;

class chain_interface;
struct ad_record
{
    ad_index                    index;
    asset                       amount;
    account_id_type             publisher_id;
    optional<signature_type>    signer;
    string            message;

    public_key_type signer_key()const;

    void sanity_check( const chain_interface& )const;
    static oad_record lookup( const chain_interface&, const ad_index& );
    static void store( chain_interface&, const ad_index&, const ad_record& );
    static void remove( chain_interface&, const ad_index& );
};

class ad_db_interface
{
    friend struct ad_record;

    virtual oad_record ad_lookup_by_index( const ad_index& )const = 0;
    virtual void ad_insert_into_index_map( const ad_index&, const ad_record& ) = 0;
    virtual void ad_erase_from_index_map( const ad_index& ) = 0;
};

} } // bts::blockchain

FC_REFLECT( bts::blockchain::ad_index, (account_id)(transaction_id) )
FC_REFLECT( bts::blockchain::ad_record, (index)(amount)(publisher_id)(message)(signer) )
