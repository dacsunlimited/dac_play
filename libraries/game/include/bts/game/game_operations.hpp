#pragma once

#include <bts/blockchain/operations.hpp>
#include <bts/blockchain/types.hpp>
#include <bts/blockchain/withdraw_types.hpp>
#include <bts/game/rule_record.hpp>

namespace bts { namespace game {
    using namespace bts::blockchain;
    
    struct game_operation
    {
        static const operation_type_enum type;
        
        bts::game::game_input  input;
        
        game_operation(){}
        
        game_operation( const bts::game::game_input& i)
        {
            input = i;
        }
        
        void evaluate( transaction_evaluation_state& eval_state ) const;
    };
} } // bts::game

FC_REFLECT( bts::game::game_operation, (input) )
