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
       std::string      script_url;
      
       /** The hash of the game's rule script */
       std::string      script_hash;

       void evaluate( transaction_evaluation_state& eval_state ) const;
   };

} } // bts::blockchain

FC_REFLECT( bts::blockchain::create_game_operation,
            (name)
            (description)
            (public_data)
            (issuer_account_id)
            (asset_id)
            (script_url)
            (script_hash)
            )
