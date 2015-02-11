#include <bts/blockchain/operation_factory.hpp>

#include <bts/game/rule_record.hpp>
#include <bts/game/rule_factory.hpp>
#include <bts/game/dice_rule.hpp>
#include <bts/game/game_operations.hpp>
#include <bts/game/client.hpp>
#include <bts/game/v8_api.hpp>

namespace bts { namespace game {
   using namespace bts::blockchain;
   
   const uint8_t dice_rule::type = dice_rule_type;
   const uint8_t dice_transaction::type = dice_rule_type;
   
   const operation_type_enum game_operation::type              = game_op_type;
   
   namespace detail {
      class client_impl {
         
      public:
         client*                 self;
         
         v8::Platform*           _platform;
         
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
               
               init_class_template( isolate );
               
               fc::path script_1(data_dir / "rule_1.js");
               v8::Handle<v8::String> source = ReadFile( isolate, script_1.to_native_ansi_path().c_str() );
               if (source.IsEmpty()) {
                  isolate->ThrowException( v8::String::NewFromUtf8(isolate, "Error loading file" ) );
               }
               
               // TODO: loop through all the rule scripts and register them, each rule instance is supposed to have their own context
               // TODO: To check whether the wallet and blockchain object are the same with the ones that should be used in script.
               //_archive.open(data_dir / "script");
               // TODO delete cpp version of dice_rule
               bts::game::rule_factory::instance().register_rule(1, std::make_shared< v8_game_engine > (1));
               
               bts::blockchain::operation_factory::instance().register_operation<game_operation>();
               
               game_executors::instance().register_game_executor(
                                                                 std::function<void( chain_database_ptr, uint32_t, const pending_chain_state_ptr&)>(std::bind(&rule_factory::execute, rule_factory::instance(), std::placeholders::_1, std::placeholders::_2, std::placeholders::_3))
                                                                 );
            } catch (...) {
            }
         }
      };
   }
   
   
   client::client()
   : my(new detail::client_impl(this))
   {
   }
   
   void client::open(const path& data_dir) {
      my->open(data_dir);
   };
} } // bts:game
