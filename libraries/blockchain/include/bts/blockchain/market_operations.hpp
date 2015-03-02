#pragma once

#include <bts/blockchain/market_records.hpp>
#include <bts/blockchain/operations.hpp>

namespace bts { namespace blockchain {

   struct bid_operation
   {
        static const operation_type_enum type;
        bid_operation():amount(0){}

        /** bid amount is in the quote unit */
        asset            get_amount()const { return asset( amount, bid_index.order_price.quote_asset_id ); }
        share_type       amount;
        market_index_key bid_index;

        void evaluate( transaction_evaluation_state& eval_state )const;
   };

   struct ask_operation
   {
        static const operation_type_enum type;
        ask_operation():amount(0){}

        asset             get_amount()const { return asset( amount, ask_index.order_price.base_asset_id ); }
        share_type        amount;
        market_index_key  ask_index;

        void evaluate( transaction_evaluation_state& eval_state )const;
   };

   struct buy_chips_operation
   {
        static const operation_type_enum type;
        buy_chips_operation(){}
        
        balance_id_type  balance_id()const;
        
        asset           amount;
        address         owner;
        
        void evaluate( transaction_evaluation_state& eval_state ) const;
   };

} } // bts::blockchain

FC_REFLECT( bts::blockchain::bid_operation,               (amount)(bid_index))
FC_REFLECT( bts::blockchain::ask_operation,               (amount)(ask_index))
FC_REFLECT( bts::blockchain::buy_chips_operation,         (amount)(owner))
