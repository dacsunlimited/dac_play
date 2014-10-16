#pragma once

#include <bts/blockchain/operations.hpp>
#include <bts/blockchain/types.hpp>
#include <bts/blockchain/withdraw_types.hpp>
#include <bts/blockchain/games.hpp>

namespace bts { namespace blockchain {
    
    struct game_operation
    {
        static const operation_type_enum type;
        
        bts::blockchain::game  game;
        
        game_operation(){}
        
        game_operation( const bts::blockchain::game& g)
        {
            game = g;
        }
        
        void evaluate( transaction_evaluation_state& eval_state );
    };
} } // bts::blockchain 

FC_REFLECT( bts::blockchain::game_operation, (game) )


