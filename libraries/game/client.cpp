#include <bts/blockchain/operation_factory.hpp>

#include <bts/game/rule_record.hpp>
#include <bts/game/game_operations.hpp>
#include <bts/game/client.hpp>

#include <bts/game/v8_api.hpp>
#include <bts/game/http_downloader.hpp>
#include <bts/game/v8_game.hpp>

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
   
   const operation_type_enum create_game_operation::type        = create_game_operation_type;
   const operation_type_enum game_operation::type               = game_op_type;
    
    client* client::current = nullptr;
   
   namespace detail {
      class client_impl {
          
      public:
         client*                 self;
         
         v8::Platform*           _platform;
          
          // For each game we should create a different isolate instance,
          // and do exit/dispose operation for each of them
          //
          // Here I just fixed the process hang issue.
          v8::Isolate* _isolate;
         
         fc::path                _data_dir;
          
         std::unordered_map<std::string, v8_game_engine_ptr > _engines;
         
         boost::signals2::scoped_connection   _http_callback_signal_connection;
         
         client_impl(client* self)
         : self(self)
         {}
         ~client_impl(){
             // explicitly release enginer obj here
             // before we release isolate things.
             
             for(auto itr=_engines.begin(); itr!=_engines.end(); itr++)
             //for(auto e: _engines) not work???
             {
 
                 //int count = itr->second.use_count();
                 itr->second.reset();
             }
             
             _isolate->Exit();
             _isolate->Dispose();
             
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
               
               _isolate = v8::Isolate::GetCurrent();
               if ( _isolate == NULL )
               {
                  _isolate = v8::Isolate::New();
                  _isolate->Enter();
               }
               
               ilog("Init class templat for game client" );
               
               v8_api::init_class_template( _isolate );

               
               // TODO: each rule instance is supposed to have their own context
               // TODO: To check whether the wallet and blockchain object are the same with the ones that should be used in script.
               
               bts::blockchain::operation_factory::instance().register_operation<game_operation>();
               bts::blockchain::operation_factory::instance().register_operation<create_game_operation>();
               
               game_executors::instance().register_game_executor(
                     std::function<void( chain_database_ptr, uint32_t, const pending_chain_state_ptr&)>(
                std::bind(&client::execute, self, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3 ) )
                                                                 );
               
               _http_callback_signal_connection =
               self->game_claimed_script.connect(
                                                 [=]( std::string url, std::string name, std::string hash) { this->script_http_callback( url, name, hash ); } );
                
            } catch (...) {
            }
         }
          
          void   register_game_engine(const std::string& game_name, v8_game_engine_ptr engine_ptr )
          {
              FC_ASSERT( _engines.find( game_name ) == _engines.end(),
                        "Game Name already Registered ${name}", ("name", game_name) );
              
              if(engine_ptr != NULL)
              {
                  _engines[game_name] = engine_ptr;
              }
          }
          
          void init_game_engine_if_not_exist(const std::string& game_name)
          {
              auto itr = _engines.find( game_name );
              
              if( itr == _engines.end() )
              {
                  try
                  {
                      register_game_engine(game_name, std::make_shared< v8_game_engine > (game_name, self));
                  } catch ( const fc::exception& e )
                  {
                      wlog("game engine register failed: ${x}", ("x",e.to_detail_string()));
                  }
              }
          }
         
         // Download/curl file from remote and save to data_dir
         // example:
          void script_http_callback( const std::string url, std::string game_name, std::string script_hash )
         {
            ilog("The url is ${url}, the hash is ${hash}", ("url", url)("game_name", game_name)("hash", script_hash) );
            fc::async( [=]()
                      {
                         const std::shared_ptr<http_downloader> downloader_ptr = std::make_shared<http_downloader>();
                         auto content = downloader_ptr->download(url);
                         
                         if ( !fc::exists(self->get_data_dir()) ) {
                            fc::create_directories(self->get_data_dir());
                         }
                         
                         ilog ("The data dir is ${d}", ("d", self->get_data_dir() ));
                         
                         fc::path script_filename = self->get_data_dir() / ( game_name + ".js");
                         
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
       
       current = this;
   };
   
   bool client::scan_create_game( const create_game_operation& op )
   {
      // Scan the create game operation and download the script from remote
      
       // TODO: Check this, now moved to create_game_operation.eval(...)
      // game_claimed_script( op.script_url, op.name, op.script_hash );
       
      // my->init_game_engine_if_not_exist( op.name );
      
      return false;
   }
   
   fc::path client::get_data_dir()const
   {
      return my->_data_dir;
   }
    
    void client::execute( chain_database_ptr blockchain, uint32_t block_num, const pending_chain_state_ptr& pending_state )
    {
        auto games = blockchain->get_games("", -1);
        
        for ( const auto& g : games)
        {
            auto v8_game_engine = get_v8_engine( g.name );
            
            v8_game_engine->execute( blockchain, block_num, pending_state );
        }
    }
    
    v8_game_engine_ptr client::get_v8_engine(const std::string& game_name)
    {
        my->init_game_engine_if_not_exist(game_name);
        
        auto itr = my->_engines.find( game_name );
        if( itr == my->_engines.end() )
            FC_THROW_EXCEPTION( bts::blockchain::game_engine_not_found, "", ("game_name", game_name) );
        return itr->second;
    }
    
    client& client::get_current()
    {
        return *current;
    }
   
   void    client::close()
   {
      
   }
} } // bts:game
