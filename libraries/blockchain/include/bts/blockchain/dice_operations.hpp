#pragma once

#include <bts/blockchain/operations.hpp>
#include <bts/blockchain/types.hpp>

namespace bts { namespace blockchain {
    
    struct dice_operation
    {
        static const operation_type_enum type;
        dice_operation():amount(0){}
        
        share_type          amount;
        uint64_t       odds;
        
        /**
         *  Only registered accounts can play dice
         */
        account_id_type        dice_account_id;
        
        
        void evaluate( transaction_evaluation_state& eval_state );
    };
    
    /*
     * TODO: to replace with withdraw_pay_operation
     */
    /*
    struct jackpot_operation
    {
        static const operation_type_enum type;
        jackpot_operation():amount(0){}
        
        jackpot_operation( share_type amount_to_withdraw,
                               account_id_type id )
        :amount(amount_to_withdraw),account_id(id) {}
        
        share_type                       amount;
        account_id_type                  account_id;
        
        void evaluate( transaction_evaluation_state& eval_state );
    };
     */
} } // bts::blockchain 

FC_REFLECT( bts::blockchain::dice_operation, (amount)(odds)(dice_account_id) )
//FC_REFLECT( bts::blockchain::jackpot_operation, (amount)(account_id) )

