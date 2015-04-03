#pragma once

#include <bts/blockchain/asset.hpp>
#include <bts/blockchain/balance_operations.hpp>

namespace bts { namespace blockchain {

struct note_index
{
    account_id_type     account_id;
    transaction_id_type transaction_id;

    friend bool operator < ( const note_index& a, const note_index& b )
    {
        return std::tie( a.account_id, a.transaction_id ) < std::tie( b.account_id, b.transaction_id );
    }

    friend bool operator == ( const note_index& a, const note_index& b )
    {
        return std::tie( a.account_id, a.transaction_id ) == std::tie( b.account_id, b.transaction_id );
    }
};

struct note_record;
typedef fc::optional<note_record> onote_record;

class chain_interface;
struct note_record
{
    note_index                  index;
    asset                       amount;
    string                      message;
    optional<signature_type>    signer;
    optional<message_meta_info> meta_data;

    public_key_type signer_key()const;

    void sanity_check( const chain_interface& )const;
    static onote_record lookup( const chain_interface&, const note_index& );
    static void store( chain_interface&, const note_index&, const note_record& );
    static void remove( chain_interface&, const note_index& );
};

class note_db_interface
{
    friend struct note_record;

    virtual onote_record note_lookup_by_index( const note_index& )const = 0;
    virtual void note_insert_into_index_map( const note_index&, const note_record& ) = 0;
    virtual void note_erase_from_index_map( const note_index& ) = 0;
};

} } // bts::blockchain

FC_REFLECT( bts::blockchain::note_index, (account_id)(transaction_id) )
FC_REFLECT( bts::blockchain::note_record, (index)(amount)(message)(signer)(meta_data) )
