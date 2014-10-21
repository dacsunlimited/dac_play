#pragma once

#include <bts/blockchain/operations.hpp>
#include <bts/blockchain/types.hpp>
#include <bts/blockchain/withdraw_types.hpp>
#include <bts/game/games.hpp>

namespace bts { namespace game {
    using namespace bts::blockchain;
    
    struct game_operation
    {
        static const operation_type_enum type;
        
        bts::game::game  game;
        
        game_operation(){}
        
        game_operation( const bts::game::game& g)
        {
            game = g;
        }
        
        void evaluate( transaction_evaluation_state& eval_state );
    };
} } // bts::game

FC_REFLECT( bts::game::game_operation, (game) )
