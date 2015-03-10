#pragma once

#include <bts/blockchain/address.hpp>
#include <bts/mail/message.hpp>
#include <bts/mail/config.hpp>

#include <map>

namespace bts { namespace game {
    
   namespace detail { class client_impl; }
    
   class client
   {
   public:
      client();
      virtual ~client() {}
       
      void     open(const fc::path& data_dir);
      
      fc::path get_data_dir()const;
       
   private:
      std::shared_ptr<detail::client_impl> my;
   };
   
   typedef std::shared_ptr<client> game_client_ptr;
} } // bts::game
