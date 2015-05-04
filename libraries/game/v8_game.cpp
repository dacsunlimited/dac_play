#include <bts/blockchain/time.hpp>

#include <bts/game/v8_helper.hpp>
#include <bts/game/v8_api.hpp>
#include <bts/game/v8_game.hpp>
#include <bts/game/client.hpp>

#include <fc/log/logger.hpp>

namespace bts { namespace game {
   
   namespace detail {
      class v8_game_engine_impl {
         
      public:
         bts::game::v8_game_engine* self;
         bts::game::client*         _client;
         uint8_t                    _rule_type;
         Isolate*                            _isolate;
         v8::Persistent<Context>             _context;
         
         v8_game_engine_impl(v8_game_engine* self, bts::game::client* client)
         : self(self), _client(client)
         {
             //_context.Reset();
         }
         
         ~v8_game_engine_impl(){
             
            _context.Reset();

         }
         
         void init()
         {
            // Refer http://v8.googlecode.com/svn/trunk/samples/process.cc
            // TODO, read from the script according to rule type
            fc::path script_1(_client->get_data_dir() / "dice.js");
            _isolate = v8::Isolate::GetCurrent();
            
            v8::Locker locker(_isolate);
             
            HandleScope handle_scope(_isolate);
            v8::Handle<v8::Context> context = v8_helper::CreateShellContext(_isolate);
            if (context.IsEmpty()) {
                fprintf(stderr, "Error creating context\n");
                FC_CAPTURE_AND_THROW(failed_game_engine_init);
            }
            _context.Reset(_isolate, context);
            
            Context::Scope context_scope(context);
            
            ilog("The path is ${s}", ("s", script_1.to_native_ansi_path().c_str() ));
            
            v8::Handle<v8::String> source = v8_helper::ReadFile( _isolate, script_1.to_native_ansi_path().c_str() );
            
             if (source.IsEmpty()) {
                 GetIsolate()->ThrowException( v8::String::NewFromUtf8(GetIsolate(), "Error loading file" ) );
                 FC_CAPTURE_AND_THROW(failed_game_engine_init);
             }
             
            String::Utf8Value utf8_source(source);
            ilog("The source is ${s}", ("s", std::string(*utf8_source) ));
            
            Handle<Script> script = Script::Compile(source);
            
            // Run the script to get the result.
            Handle<Value> result = script->Run();
            
            String::Utf8Value utf8(result);
            printf("%s\n", *utf8);
         }
         
         Isolate* GetIsolate() { return _isolate; }
      };
   }
   
   v8_game_engine::v8_game_engine(uint8_t rule_type, bts::game::client* client): my(new detail::v8_game_engine_impl(this, client))
   {
      my->_rule_type = rule_type;
      my->init();
     
   }
   
    // TODO: shoud provide with game_input
   void v8_game_engine::evaluate( transaction_evaluation_state& eval_state )
   {
      // TODO: what is isolate scope.
      // v8::Isolate::Scope isolate_scope(isolate);
       v8::Locker locker(my->GetIsolate());
      v8::HandleScope handle_scope(my->GetIsolate());
      v8::Local<v8::Context> context = v8::Local<v8::Context>::New(my->GetIsolate(), my->_context);
      // Entering the context
      Context::Scope context_scope(context);
      //context->Global()->Set(String::NewFromUtf8(isolate, "scan_result_trx"), External::New(isolate, rtrx));
      
      // TODO: Rewriting the global of the context
      // context->Global()->Set(String::NewFromUtf8(my->GetIsolate(), "evaluate_block_num"), v8_evalstate::New(my->GetIsolate(), eval_state.shared_from_this()));
      
      v8::TryCatch try_catch;
      v8::Handle<v8::Script> script = v8::Script::Compile(
                           String::NewFromUtf8(
                           my->GetIsolate(),
                           "scan_result(scan_rtx, scan_result_block_num, scan_result_block_time, scan_result_received_time, scan_result_trx_index, scan_w);"),
                           String::NewFromUtf8(my->GetIsolate(), "rule.execute")
                                                          );
      script->Run();
   }
   
   wallet_transaction_record v8_game_engine::play( chain_database_ptr blockchain, bts::wallet::wallet_ptr w, const variant& var, bool sign )
   {
      
      signed_transaction     trx;
      unordered_set<address> required_signatures;
      
      trx.expiration = now() + w->get_transaction_expiration();
      
      auto record = wallet_transaction_record();
      
       auto isolate = my->GetIsolate();
       v8::Locker locker(isolate);
      v8::HandleScope handle_scope(my->GetIsolate());
      v8::Local<v8::Context> context = v8::Local<v8::Context>::New(my->GetIsolate(), my->_context);
      // Entering the context
      Context::Scope context_scope(context);
         
      v8::TryCatch try_catch;
      v8::Handle<v8::Script> script = v8::Script::Compile(
                                                          String::NewFromUtf8(
                                                                                 my->GetIsolate(),
                                                                                 "scan_result(scan_rtx, scan_result_block_num, scan_result_block_time, scan_result_received_time, scan_result_trx_index, scan_w);"),
                                                             String::NewFromUtf8(my->GetIsolate(), "rule.execute")
                                                             );
      v8::Local<Value> result = script->Run();
      
      if (result.IsEmpty()) {
         // The TryCatch above is still in effect and will have caught the error.
         String::Utf8Value error(try_catch.Exception());
         elog("Error when running the script. The error is ${e}", ("e", *error));
      }
      
      if( sign )
         w->sign_transaction( trx, required_signatures );
      
      record.trx = trx;
      return record;
   }
   
   bool v8_game_engine::scan( wallet_transaction_record& trx_rec, bts::wallet::wallet_ptr w )
   {
      
      HandleScope handle_scope(my->GetIsolate());
      // TODO
      return false;
   }
   
   bool v8_game_engine::scan_result( const game_result_transaction& rtrx,
                    uint32_t block_num,
                    const time_point_sec& block_time,
                    const uint32_t trx_index, bts::wallet::wallet_ptr w)
   {
       v8::Locker locker(my->GetIsolate());
      v8::HandleScope handle_scope(my->GetIsolate());
      v8::Local<v8::Context> context = v8::Local<v8::Context>::New(my->GetIsolate(), my->_context);
      v8::Context::Scope context_scope(context);
      
      //context->Global()->Set(String::NewFromUtf8(isolate, "scan_result_trx"), External::New(isolate, rtrx));
      
      context->Global()->Set(String::NewFromUtf8(my->GetIsolate(), "scan_result_block_num"), Integer::New(my->GetIsolate(), block_num));
      
      //associates our internal field pointing to 'p' with the "point" name inside the context
      //this enable usage of point inside this context without need to create a new one
      context->Global()->Set(String::NewFromUtf8(my->GetIsolate(), "scan_result_block_time"), String::NewFromUtf8(my->GetIsolate(), block_time.to_iso_string().c_str()));
      
      context->Global()->Set(String::NewFromUtf8(my->GetIsolate(), "scan_result_trx_index"), Integer::New(my->GetIsolate(), trx_index));
      
      v8::TryCatch try_catch;
      v8::Handle<v8::Script> script = v8::Script::Compile(String::NewFromUtf8(my->GetIsolate(), "scan_result(scan_rtx, scan_result_block_num, scan_result_block_time, scan_result_trx_index, scan_w);"), String::NewFromUtf8(my->GetIsolate(), "rule.execute"));
      v8::Handle<v8::Value> result = script->Run();
      
      return result->ToBoolean(my->GetIsolate())->BooleanValue();
   }
   
   void v8_game_engine::execute( chain_database_ptr blockchain, uint32_t block_num, const pending_chain_state_ptr& pending_state )
   {
      v8::Locker locker(my->GetIsolate());
      v8::HandleScope handle_scope(my->GetIsolate());
      v8::Local<v8::Context> context = v8::Local<v8::Context>::New(my->GetIsolate(), my->_context);
      v8::Context::Scope context_scope(context);
      
      //associates our internal field pointing to 'p' with the "point" name inside the context
      //this enable usage of point inside this context without need to create a new one
      context->Global()->Set(String::NewFromUtf8(my->GetIsolate(), "execute_blockchain"), v8_blockchain::New(my->GetIsolate(), blockchain, block_num));
      
      context->Global()->Set(String::NewFromUtf8(my->GetIsolate(), "execute_block_num"), Integer::New(my->GetIsolate(), block_num));
      
      context->Global()->Set(String::NewFromUtf8(my->GetIsolate(), "execute_pendingstate"), v8_chainstate::New(my->GetIsolate(), pending_state));
      
      // TODO: get rule execute
      
      // begin script execution
      v8::TryCatch try_catch;
      v8::Handle<v8::Script> script = v8::Script::Compile(String::NewFromUtf8(my->GetIsolate(), "execute(execute_blockchain, execute_block_num, execute_pendingstate);"), String::NewFromUtf8(my->GetIsolate(), "rule.execute"));
      script->Run();
      
      /*
      if (script.IsEmpty())
      {
          // TODO: throw exception
      }
      else
      {
          v8::Handle<v8::Value> result = script->Run();
          if (result.IsEmpty())
          {
              assert(try_catch.HasCaught());
              // TODO: throw exception
          }
          else
          {
              assert(!try_catch.HasCaught());
              if (!result->IsUndefined()) {
                  // TOOD: return the result
              }
          }
      }*/
   }
} } // bts::game
