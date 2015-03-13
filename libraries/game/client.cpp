#include <bts/blockchain/operation_factory.hpp>

#include <bts/game/rule_record.hpp>
#include <bts/game/rule_factory.hpp>
#include <bts/game/game_operations.hpp>
#include <bts/game/client.hpp>

#include <bts/game/v8_api.hpp>
#include <bts/game/http_downloader.hpp>

#include <fc/io/json.hpp>
#include <fc/network/http/connection.hpp>
#include <fc/network/resolve.hpp>
#include <fc/network/url.hpp>
#include <fc/reflect/variant.hpp>
#include <fc/thread/non_preemptable_scope_check.hpp>

#include <iostream>
#include <fstream>

namespace bts { namespace game {
   using namespace bts::blockchain;
   
   const operation_type_enum game_operation::type              = game_op_type;
   
   namespace detail {
      class client_impl {
         
      public:
         client*                 self;
         
         v8::Platform*           _platform;
         
         fc::path                _data_dir;
         
         boost::signals2::scoped_connection   _http_callback_signal_connection;
         
         client_impl(client* self)
         : self(self)
         {}
         ~client_impl(){
            v8::V8::Dispose();
            v8::V8::ShutdownPlatform();
            delete _platform;
         }
         
         void open(const fc::path& data_dir) {
            try {
               v8::V8::InitializeICU();
               _platform = v8::platform::CreateDefaultPlatform();
               v8::V8::InitializePlatform(_platform);
               v8::V8::Initialize();
               
               v8::Isolate* isolate = v8::Isolate::GetCurrent();
               if ( isolate == NULL )
               {
                  isolate = v8::Isolate::New();
                  isolate->Enter();
               }
               
               v8_api::init_class_template( isolate );
               
               // TODO: loop through all the rule scripts and register them, each rule instance is supposed to have their own context
               // TODO: To check whether the wallet and blockchain object are the same with the ones that should be used in script.
               //_archive.open(data_dir / "script");
               // TODO delete cpp version of dice_rule
               //bts::game::rule_factory::instance().register_rule(1, std::make_shared< v8_game_engine > (1, self));
               
               bts::blockchain::operation_factory::instance().register_operation<game_operation>();
               
               game_executors::instance().register_game_executor(
                     std::function<void( chain_database_ptr, uint32_t, const pending_chain_state_ptr&)>(std::bind(&rule_factory::execute, rule_factory::instance(), std::placeholders::_1, std::placeholders::_2, std::placeholders::_3 ) )
                                                                 );
               
               _http_callback_signal_connection =
               self->game_claimed_script.connect(
                                                 [=]( std::string url, std::string hash) { this->script_http_callback( url, hash ); } );
               
               // Testing
               self->game_claimed_script("http://www.dacsunlimited.com/games/dice_rule.js", "000");
            } catch (...) {
            }
         }
         
         // Download/curl file from remote and save to data_dir
         // example:
         void script_http_callback( const std::string url, std::string hash )
         {
            ilog("The url is ${url}, the hash is ${hash}", ("url", url)("hash", hash) );
            fc::async( [=]()
                      {
                         const std::shared_ptr<http_downloader> downloader_ptr = std::make_shared<http_downloader>();
                         auto content = downloader_ptr->download(url);
                         
                         if ( !fc::exists(self->get_data_dir()) ) {
                            fc::create_directories(self->get_data_dir());
                         }
                         
                         ilog ("The data dir is ${d}", ("d", self->get_data_dir() ));
                         
                         fc::path script_filename = self->get_data_dir() / ( hash + ".js");
                         
                         ilog ("The script_filename ${d}", ("d", script_filename ));
                         
                         std::ofstream script_file(script_filename.string());
                         
                         script_file << content;
                      }
                      );
         }
      };
   }
   
   
   client::client()
   : my(new detail::client_impl(this))
   {
   }
   
   client::~client()
   {
      close();
   }
   
   void client::open(const path& data_dir) {
      my->_data_dir = data_dir;
      my->open(data_dir);
   };
   
   bool client::scan_create_game( const create_game_operation& op )
   {
      // Scan the create game operation and download the script from remote
      
      game_claimed_script(op.script_url, op.script_hash);
      
      return false;
   }
   
   fc::path client::get_data_dir()const
   {
      return my->_data_dir;
   }
   
   void    client::close()
   {
      
   }
} } // bts:game
