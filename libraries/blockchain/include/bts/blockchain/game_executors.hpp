#pragma once

#include <bts/blockchain/exceptions.hpp>
#include <bts/blockchain/transaction_evaluation_state.hpp>
#include <bts/blockchain/chain_interface.hpp>
#include <bts/blockchain/chain_database.hpp>

namespace bts { namespace blockchain {

   /**
    * @class game_executors
    *
    *  Enables call executors in games
    *  
    */
   class game_executors
   {
       public:
          static game_executors& instance();

          void   register_game_executor(std::function<void( chain_database_ptr, uint32_t, const pending_chain_state_ptr&)> executor_f)
          {
             _executors.push_back( executor_f );
          }

          void execute( chain_database_ptr blockchain, uint32_t block_num, const pending_chain_state_ptr& pending_state )
          {
             for ( const auto& f : _executors )
             {
                f(blockchain, block_num, pending_state);
             }
          }
       
       private:
          std::vector<std::function<void( chain_database_ptr, uint32_t, const pending_chain_state_ptr&)>>  _executors;
   };

} } // bts:blockchain

