#pragma once
#include <bts/blockchain/types.hpp>
#include <bts/blockchain/withdraw_types.hpp>
#include <bts/blockchain/transaction.hpp>

namespace bts { namespace blockchain {
    
    struct dice_record
    {
        dice_record()
        :id(0),account_id(0), ammount(0){}
        
        bool is_null()const;
        
        dice_record make_null()const;
        
        dice_id_type        id;
        account_id_type     account_id;
        share_type          ammount;
    };
    typedef fc::optional<dice_record> odice_record;
    
} } // bts::blockchain

FC_REFLECT( bts::blockchain::dice_record,
           (id)(account_id)(ammount)
           )