#pragma once

#include <bts/blockchain/types.hpp>

namespace bts { namespace blockchain {

struct operation_reward_record;
typedef optional<operation_reward_record> ooperation_reward_record;

class chain_interface;
struct operation_reward_record
{
    operation_id_type    id;
    map<asset_id_type, share_type> fees;

    void sanity_check( const chain_interface& )const;
    static ooperation_reward_record lookup( const chain_interface&, const operation_id_type );
    static void store( chain_interface&, const operation_id_type, const operation_reward_record& );
    static void remove( chain_interface&, const operation_id_type );
};

class operation_reward_db_interface
{
    friend struct operation_reward_record;

    virtual ooperation_reward_record operation_reward_lookup_by_id( const operation_id_type )const = 0;
    virtual void operation_reward_insert_into_id_map( const operation_id_type, const operation_reward_record& ) = 0;
    virtual void operation_reward_erase_from_id_map( const operation_id_type ) = 0;
};
    
    struct operation_reward_transaction
    {
        address                                   reward_owner;
        operation_id_type                         op_type;
        asset                                     reward;
        string                                    info;
    };

} } // bts::blockchain

FC_REFLECT( bts::blockchain::operation_reward_record, (id)(fees) )
FC_REFLECT( bts::blockchain::operation_reward_transaction, (reward_owner)(op_type)(reward)(info) )
FC_REFLECT_TYPENAME( std::vector<bts::blockchain::operation_reward_transaction> )