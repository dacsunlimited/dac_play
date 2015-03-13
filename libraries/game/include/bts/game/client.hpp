#pragma once

#include <bts/blockchain/address.hpp>
#include <bts/blockchain/meta_game_operations.hpp>
#include <bts/mail/message.hpp>
#include <bts/mail/config.hpp>

#include <map>

#include <fc/signals.hpp>

namespace bts { namespace game {
   using namespace bts::blockchain;
    
   namespace detail { class client_impl; }
    
   class client
   {
   public:
      client();
      virtual ~client();
       
      void     open(const fc::path& data_dir);
      
      bool     scan_create_game( const create_game_operation& op );
      
      void     close();
      
      fc::path get_data_dir()const;
      
      // TODO: store the script to related game id
      fc::signal<void( std::string, std::string )> game_claimed_script;
       
   private:
      std::shared_ptr<detail::client_impl> my;
   };
   
   typedef std::shared_ptr<client> game_client_ptr;
} } // bts::game
