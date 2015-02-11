#include <bts/blockchain/time.hpp>

#include <bts/game/v8_game.hpp>
#include <bts/game/v8_api.hpp>

namespace bts { namespace game {
   using namespace v8;
   
   static bool first_engine = []()->bool{
      v8::Isolate* isolate = Isolate::GetCurrent();
      if ( isolate == NULL )
      {
          isolate = v8::Isolate::New();
          isolate->Enter();
      }
      
      init_class_template( isolate );
      
      return true;
   }();
   
   v8_game_engine::v8_game_engine(uint8_t rule_type) : _rule_type(rule_type)
   {
      // TODO: Init and load the script defined for rules
   }
   
   void v8_game_engine::evaluate( transaction_evaluation_state& eval_state)
   {
      v8::Isolate* isolate = Isolate::GetCurrent();
      v8::Isolate::Scope isolate_scope(isolate);
      v8::HandleScope handle_scope(isolate);
      
      v8::Handle<v8::Context> context = CreateShellContext(isolate);
      if (context.IsEmpty()) {
          fprintf(stderr, "Error creating context\n");
      }
      v8::Context::Scope context_scope(context);
      
      //context->Global()->Set(String::NewFromUtf8(isolate, "scan_result_trx"), External::New(isolate, rtrx));
      
      context->Global()->Set(String::NewFromUtf8(isolate, "evaluate_block_num"), v8_evalstate::New(isolate, eval_state.shared_from_this()));
      
      v8::TryCatch try_catch;
      v8::Handle<v8::Script> script = v8::Script::Compile(
                                                          String::NewFromUtf8(
                                                                              isolate,
                                                                              "'TODO';scan_result(scan_rtx, scan_result_block_num, scan_result_block_time, scan_result_received_time, scan_result_trx_index, scan_w);"), String::NewFromUtf8(isolate, "rule.execute")
                                                          );
      script->Run();
   }
   
   wallet_transaction_record v8_game_engine::play( chain_database_ptr blockchain, bts::wallet::wallet_ptr w, const variant& var, bool sign )
   {
      // TODO: Now we have the assumption that the asset id equals the game id.
      signed_transaction     trx;
      unordered_set<address> required_signatures;
      
      trx.expiration = now() + w->get_transaction_expiration();
      
      // TODO: adjust fee based upon blockchain price per byte and
      // the size of trx... 'recursively'
      auto required_fees = w->get_transaction_fee();
      
      {
          // TODO
      }
      
      auto record = wallet_transaction_record();
      // create the operations according to rule_type and input types
      // then how to map the structs?
      //trx.operations.push_back( game_operation(bts::game::dice_rule(address( play_account->active_key() ), amount_to_play, d_input.odds, d_input.guess ))//slate_id 0 );
      
      auto entry = bts::wallet::ledger_entry();
      //entry.from_account = play_account->active_key();
      //entry.to_account = play_account->active_key();
      //entry.memo = "play dice";
      
      record.ledger_entries.push_back( entry );
      record.fee = required_fees;
      
      if( sign ) w->sign_transaction( trx, required_signatures );
      
      record.trx = trx;
      
      return record;
   }
   
   bool v8_game_engine::scan( wallet_transaction_record& trx_rec, bts::wallet::wallet_ptr w )
   {
    
   }
   
   bool v8_game_engine::scan_result( const rule_result_transaction& rtrx,
                    uint32_t block_num,
                    const time_point_sec& block_time,
                    const uint32_t trx_index, bts::wallet::wallet_ptr w)
   {
      v8::Isolate* isolate = Isolate::GetCurrent();
      v8::Isolate::Scope isolate_scope(isolate);
      v8::HandleScope handle_scope(isolate);
      
      v8::Handle<v8::Context> context = CreateShellContext(isolate);
      if (context.IsEmpty()) {
          fprintf(stderr, "Error creating context\n");
          // TODO: throw errors
          return false;
      }
      v8::Context::Scope context_scope(context);
      
      //context->Global()->Set(String::NewFromUtf8(isolate, "scan_result_trx"), External::New(isolate, rtrx));
      
      context->Global()->Set(String::NewFromUtf8(isolate, "scan_result_block_num"), Integer::New(isolate, block_num));
      
      //associates our internal field pointing to 'p' with the "point" name inside the context
      //this enable usage of point inside this context without need to create a new one
      context->Global()->Set(String::NewFromUtf8(isolate, "scan_result_block_time"), String::NewFromUtf8(isolate, block_time.to_iso_string().c_str()));
      
      context->Global()->Set(String::NewFromUtf8(isolate, "scan_result_trx_index"), Integer::New(isolate, trx_index));
      v8::TryCatch try_catch;
      v8::Handle<v8::Script> script = v8::Script::Compile(String::NewFromUtf8(isolate, "'TODO';scan_result(scan_rtx, scan_result_block_num, scan_result_block_time, scan_result_trx_index, scan_w);"), String::NewFromUtf8(isolate, "rule.execute"));
      v8::Handle<v8::Value> result = script->Run();
      
      return result->ToBoolean(isolate)->BooleanValue();
   }
   
   void v8_game_engine::execute( chain_database_ptr blockchain, uint32_t block_num, const pending_chain_state_ptr& pending_state )
   {
      v8::Isolate* isolate = Isolate::GetCurrent();
      v8::Isolate::Scope isolate_scope(isolate);
      v8::HandleScope handle_scope(isolate);
      
      v8::Handle<v8::Context> context = CreateShellContext(isolate);
      if (context.IsEmpty()) {
          fprintf(stderr, "Error creating context\n");
      }
      v8::Context::Scope context_scope(context);
      
      //associates our internal field pointing to 'p' with the "point" name inside the context
      //this enable usage of point inside this context without need to create a new one
      context->Global()->Set(String::NewFromUtf8(isolate, "execute_blockchain"), v8_blockchain::New(isolate, blockchain, block_num));
      
      context->Global()->Set(String::NewFromUtf8(isolate, "execute_block_num"), Integer::New(isolate, block_num));
      
      context->Global()->Set(String::NewFromUtf8(isolate, "execute_pendingstate"), v8_chainstate::New(isolate, pending_state));
      
      // TODO: get rule execute
      
      // begin script execution
      v8::TryCatch try_catch;
      v8::Handle<v8::Script> script = v8::Script::Compile(String::NewFromUtf8(isolate, "'TODO';execute(execute_blockchain, execute_block_num, execute_pendingstate);"), String::NewFromUtf8(isolate, "rule.execute"));
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
