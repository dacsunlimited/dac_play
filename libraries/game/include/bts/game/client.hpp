#pragma once

#include <bts/blockchain/address.hpp>
#include <bts/mail/message.hpp>
#include <bts/mail/config.hpp>

#include <bts/blockchain/exceptions.hpp>
#include <bts/blockchain/transaction_evaluation_state.hpp>
#include <bts/blockchain/chain_interface.hpp>
#include <bts/blockchain/game_interface.hpp>

#include <map>

#include <fc/signals.hpp>

namespace bts { namespace game {
   using namespace bts::blockchain;
    
   class v8_game_engine;
   typedef std::shared_ptr<v8_game_engine> v8_game_engine_ptr;
   struct create_game_operation;
   namespace detail { class client_impl; }
    
   class client : public game_interface
   {
   public:
      client(chain_database_ptr chain);
      virtual ~client();
       
      void     open(const fc::path& data_dir);
      
      void     close();
      
      fc::path get_data_dir()const;
       
       chain_database_ptr get_chain_database();
       
       v8_game_engine_ptr get_v8_engine(const std::string& game_name);
       
       bool reinstall_game_engine(const std::string& game_name);
       
       void execute( chain_database_ptr blockchain, uint32_t block_num, const pending_chain_state_ptr& pending_state );
      
       
       void* get_isolate(/*const std::string& game_name*/);
       
       static client& get_current();
       
      // TODO: store the script to related game id
      fc::signal<void( std::string, std::string)> game_claimed_script;
       
   private:
      std::shared_ptr<detail::client_impl> my;
      static client* current;
   };
   
   typedef std::shared_ptr<client> game_client_ptr;
} } // bts::game
