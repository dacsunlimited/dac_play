#pragma once

#include <bts/blockchain/operations.hpp>
#include <bts/blockchain/types.hpp>
#include <bts/blockchain/withdraw_types.hpp>
#include <bts/game/rule_record.hpp>

namespace bts { namespace game {
    using namespace bts::blockchain;
    
    /**
     *  Creates / defines an asset type but does not
     *  allocate it to anyone. Use issue_asset_operation
     */
    struct create_game_operation
    {
        static const operation_type_enum type;

        /**
         * Names are a more complete description and may
         * contain any kind of characters or spaces.
         */
        std::string      name;
        /**
         *  Describes the asset and its purpose.
         */
        std::string      description;
        /**
         * Other information relevant to this asset.
         */
        fc::variant      public_data;

        /**
         *  Game only be issued by individuals that
         *  have registered a name.
         */
        account_id_type  owner_account_id;

        /** The url of the game's rule script */
        std::string      script_code;

        void evaluate( transaction_evaluation_state& eval_state ) const;
    };

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

FC_REFLECT( bts::game::create_game_operation,
            (name)
            (description)
            (public_data)
            (owner_account_id)
            (script_code)
            )

FC_REFLECT( bts::game::game_operation, (input) )
