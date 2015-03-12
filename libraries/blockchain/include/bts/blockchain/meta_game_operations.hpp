#pragma once

#include <bts/blockchain/operations.hpp>
#include <bts/blockchain/account_record.hpp>
#include <bts/blockchain/types.hpp>

namespace bts { namespace blockchain {

   /**
    *  Creates / defines an asset type but does not
    *  allocate it to anyone. Use issue_asset_operation
    */
   struct create_game_operation
   {
       static const operation_type_enum type;

       /**
        * Symbols may only contain A-Z and 0-9 and up to 5
        * characters and must be unique.
        */
       std::string      symbol;

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
        *  Assets can only be issued by individuals that
        *  have registered a name.
        */
       account_id_type  issuer_account_id;

       /** The id of the asset used to play in this game */
       asset_id_type    asset_id;

       /** The url of the game's rule script */
       std::string      script_url;
      
       /** The hash of the game's rule script */
       std::string      script_hash;

       void evaluate( transaction_evaluation_state& eval_state ) const;
   };

} } // bts::blockchain

FC_REFLECT( bts::blockchain::create_game_operation,
            (symbol)
            (name)
            (description)
            (public_data)
            (issuer_account_id)
            (asset_id)
            (script_url)
            (script_hash)
            )
