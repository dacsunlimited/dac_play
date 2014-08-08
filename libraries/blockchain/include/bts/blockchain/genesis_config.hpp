#pragma once

#include <bts/blockchain/types.hpp>
#include <fc/time.hpp>

namespace bts { namespace blockchain {

  struct name_config
  {
     name_config():delegate_pay_rate(255){}

     std::string        name;
     public_key_type    owner;
     int                delegate_pay_rate;
  };

  struct asset_config // these are all market-issued assets
  {
     std::string       symbol;
     std::string       name;
     std::string       description;
     uint64_t          precision;
     uint64_t          init_supply;
     uint64_t          init_collateral;
  };
  
  struct genesis_block_config
  {
     fc::time_point_sec                         timestamp;
     std::vector<asset_config>                  chip_assets;
     std::vector<name_config>                   names;
     std::vector<std::pair<pts_address,double>> balances;
  };

} } // bts::blockchain

FC_REFLECT( bts::blockchain::name_config, (name)(owner)(delegate_pay_rate) )
FC_REFLECT( bts::blockchain::asset_config, (symbol)(name)(description)(precision)(init_supply)(init_collateral) )
FC_REFLECT( bts::blockchain::genesis_block_config, (timestamp)(chip_assets)(names)(balances) )
