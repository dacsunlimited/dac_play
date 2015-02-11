#include <bts/blockchain/operation_factory.hpp>

#include <bts/game/rule_record.hpp>
#include <bts/game/rule_factory.hpp>
#include <bts/game/dice_rule.hpp>
#include <bts/game/game_operations.hpp>
#include <bts/game/client.hpp>

namespace bts { namespace game {
   using namespace bts::blockchain;
   
   const uint8_t dice_rule::type = dice_rule_type;
   const uint8_t dice_transaction::type = dice_rule_type;
   
   const operation_type_enum game_operation::type              = game_op_type;

   static bool first_chain = []()->bool{
       bts::game::rule_factory::instance().register_rule<dice_rule>();
       
       bts::blockchain::operation_factory::instance().register_operation<game_operation>();
       
       game_executors::instance().register_game_executor(
                                                         std::function<void( chain_database_ptr, uint32_t, const pending_chain_state_ptr&)>(std::bind(&rule_factory::execute, rule_factory::instance(), std::placeholders::_1, std::placeholders::_2, std::placeholders::_3))
                                                         );
   
       return true;
   }();
   
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
               //_archive.open(data_dir / "archive");
               v8::V8::InitializeICU();
               _platform = v8::platform::CreateDefaultPlatform();
               v8::V8::InitializePlatform(_platform);
               v8::V8::Initialize();
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
