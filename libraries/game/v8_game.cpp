#include <bts/blockchain/time.hpp>

#include <bts/game/v8_helper.hpp>
#include <bts/game/v8_api.hpp>
#include <bts/game/v8_game.hpp>
#include <bts/game/client.hpp>
#include <bts/game/game_operations.hpp>

#include <fc/log/logger.hpp>

namespace bts { namespace game {
   
   namespace detail {
      class v8_game_engine_impl {
         
      public:
         bts::game::v8_game_engine*         self;
         bts::game::client*                 _client;
         std::string                       _game_name;
         Isolate*                           _isolate;
         v8::Persistent<Context>            _context;
         
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
            // Deprecated: fc::path script_path( _client->get_data_dir() / (_game_name + ".js") );
            _isolate = v8::Isolate::GetCurrent();
            
            v8::Locker locker(_isolate);
             
            HandleScope handle_scope(_isolate);
            v8::TryCatch try_catch( _isolate );
            v8::Handle<v8::Context> context = v8_helper::CreateShellContext(_isolate);
            if (context.IsEmpty()) {
                String::Utf8Value error(try_catch.Exception());
                wlog("Error creating context in game ${name}, error is ${e}", ("name", _game_name)("e", *error));
                FC_CAPTURE_AND_THROW(failed_game_engine_init);
            }
            _context.Reset(_isolate, context);
            
            Context::Scope context_scope(context);
            
            ilog("The game is ${s}", ("s", _game_name ));
             
             auto ogame_rec = _client->get_chain_database()->get_game_record( _game_name );
             FC_ASSERT( ogame_rec.valid() );
             
            
            //v8::Handle<v8::String> source = v8_helper::ReadFile( _isolate, script_path.to_native_ansi_path().c_str() );
             v8::Handle<v8::String> source = v8::String::NewFromUtf8( GetIsolate(), ogame_rec->script_code.c_str() );
             
             if (source.IsEmpty()) {
                 wlog("The souce is empty, error loading script code");
                 GetIsolate()->ThrowException( v8::String::NewFromUtf8(GetIsolate(), "Error loading file" ) );
                 String::Utf8Value error(try_catch.Exception());
                 FC_CAPTURE_AND_THROW(failed_loading_source_file, (_game_name)(*error));
             }
             
            String::Utf8Value utf8_source(source);
            Handle<Script> script = Script::Compile(source);
            if ( script.IsEmpty() )
            {
                // The TryCatch above is still in effect and will have caught the error.
                FC_CAPTURE_AND_THROW(failed_compile_script, (*utf8_source)(v8_helper::ReportException(GetIsolate(), &try_catch)));
            } else
            {
                // Run the script to get the result.
                Handle<Value> result = script->Run();
                
                if ( result.IsEmpty() )
                {
                    FC_CAPTURE_AND_THROW(failed_run_script, (v8_helper::ReportException(GetIsolate(), &try_catch)));
                } else
                {
                    wlog("The result of the running of script is ${s}", ( "s",  v8_helper::ToCString(String::Utf8Value(result)) ));
                }
            }
         }
         
         Isolate* GetIsolate() { return _isolate; }
      };
   }
   
    v8_game_engine::v8_game_engine(std::string game_name, bts::game::client* client): my(new detail::v8_game_engine_impl(this, client))
   {
      my->_game_name = game_name;
      my->init();
     
   }
   
    // TODO: shoud provide with game_input
   void v8_game_engine::evaluate( transaction_evaluation_state& eval_state, game_id_type game_id, const variant& var)
   {
       auto isolate = my->GetIsolate();
       v8::Locker locker( isolate );
       v8::HandleScope handle_scope( isolate );
       v8::Local<v8::Context> context = v8::Local<v8::Context>::New(my->GetIsolate(), my->_context);
      
       // Entering the context
       Context::Scope context_scope(context);
       
       wlog("Start evaluating the game.. with var ${v}", ("v", var));
      
       // TODO: Rewriting the global of the context, Is the Global() geting the local context or top global context?
       context->Global()->Set(String::NewFromUtf8(my->GetIsolate(), "$eval_state"), v8_evalstate::New( isolate, &eval_state ));
       context->Global()->Set(String::NewFromUtf8(my->GetIsolate(), "$pending_state"), v8_chainstate::New( isolate, eval_state.pending_state()->shared_from_this() ));
       auto _input = var; // TODO: convert/parse it to a v8 javascript object
       context->Global()->Set(String::NewFromUtf8(isolate, "$input"),  v8_helper::cpp_to_json(isolate, _input));
      
       v8::TryCatch try_catch;
       
       auto source = "PLAY.evaluate($eval_state, $pending_state, $input);";
       v8::Handle<v8::Script> script = v8::Script::Compile( String::NewFromUtf8( my->GetIsolate(), source) );
       if ( script.IsEmpty() )
       {
           String::Utf8Value error(try_catch.Exception());
           FC_CAPTURE_AND_THROW(failed_compile_script, (source)(*error));
       } else
       {
           // Run the script to get the result.
           Handle<Value> result = script->Run();
           
           if ( result.IsEmpty() )
           {
               FC_CAPTURE_AND_THROW(failed_run_script, (v8_helper::ReportException(my->GetIsolate(), &try_catch)));
           } else
           {
               wlog("The result of the running of script is ${s}", ( "s",  v8_helper::ToCString(String::Utf8Value(result)) ));
               
               auto result_obj = v8_helper::json_to_cpp<variant>( isolate, result );
               
               FC_ASSERT( result_obj.is_object() );
               FC_ASSERT( result_obj.get_object().contains( "to_balances" ) );
               FC_ASSERT( result_obj.get_object().contains( "datas" ) );
               auto to_balances_var = result_obj.get_object()["to_balances"];
               FC_ASSERT( to_balances_var.is_array() );
               
               for ( auto to_balance : to_balances_var.get_array() )
               {
                   FC_ASSERT( to_balance.is_object() );
                   FC_ASSERT( to_balance.get_object().contains( "owner") );
                   FC_ASSERT( to_balance.get_object()["owner"].is_string() );
                   FC_ASSERT( to_balance.get_object().contains( "asset") );
                   address owner( to_balance.get_object()["owner"].as_string() );
                   auto asset_amount = to_balance.get_object()["asset"].as<asset>();
                   
                   auto game_asset = eval_state.pending_state()->get_asset_record( asset_amount.asset_id );
                   
                   FC_ASSERT( game_asset.valid() );
                   FC_ASSERT( game_asset->is_game_issued() , "game asset is ${g}", ("g", *game_asset) );
                   FC_ASSERT( game_asset->issuer.issuer_id == game_id, "Adding the game asset must get the permission, with the condition that the issuer is this game." );
                   
                   withdraw_condition to_condition( withdraw_with_signature(owner), asset_amount.asset_id);
                   
                   obalance_record to_record = eval_state.pending_state()->get_balance_record( to_condition.get_address() );
                   if( !to_record.valid() )
                   {
                       to_record = balance_record( to_condition );
                   }
                   
                   if( to_record->balance == 0 )  // Not possible for current consensus
                   {
                       to_record->deposit_date = eval_state.pending_state()->now();
                   }
                   else
                   {
                       fc::uint128 old_sec_since_epoch( to_record->deposit_date.sec_since_epoch() );
                       fc::uint128 new_sec_since_epoch( eval_state.pending_state()->now().sec_since_epoch() );
                       
                       fc::uint128 avg = (old_sec_since_epoch * to_record->balance) + (new_sec_since_epoch * asset_amount.amount);
                       avg /= (to_record->balance + asset_amount.amount);
                       
                       to_record->deposit_date = time_point_sec( avg.to_integer() );
                   }
                   
                   to_record->balance += asset_amount.amount;
                   eval_state.sub_balance( asset_amount );
                   
                   to_record->last_update = eval_state.pending_state()->now();
                   
                   eval_state.pending_state()->store_balance_record( *to_record );
               }
               
               auto datas_var = result_obj.get_object()["datas"];
               FC_ASSERT( datas_var.is_array() );
               
               for ( auto data : datas_var.get_array() )
               {
                   if (data.is_object())
                   {
                       FC_ASSERT( data.get_object().contains( "index") );
                       FC_ASSERT( data.get_object()["index"].is_numeric() );
                       
                       game_data_record game_data;
                       game_data.game_id = game_id;
                       game_data.data = data;
                       
                       eval_state.pending_state()->store_game_data_record(game_id, game_data.get_game_data_index(), game_data );
                   } else if ( data.is_numeric() ) // TODO: Remove game_data_record, when directly return a number
                   {
                       data_id_type data_id = data.as<data_id_type>();
                       game_data_record game_data;
                       game_data.data = fc::mutable_variant_object( "index", data_id );
                       game_data.make_null();
                       
                       eval_state.pending_state()->store_game_data_record(game_id, data_id, game_data );
                   }
                   
               }
           }
       }
   }
   
   wallet_transaction_record v8_game_engine::play( game_id_type game_id, chain_database_ptr blockchain, bts::wallet::wallet_ptr w, const variant& var, bool sign )
   {
       signed_transaction     trx;
       unordered_set<address> required_signatures;
      
       trx.expiration = now() + w->get_transaction_expiration();
       
       const auto required_fees = w->get_transaction_fee();
      
       auto record = wallet_transaction_record();
      
       auto isolate = my->GetIsolate();
       v8::Locker locker(isolate);
       v8::HandleScope handle_scope( isolate );
       v8::Local<v8::Context> context = v8::Local<v8::Context>::New(isolate, my->_context);
       // Entering the context
       Context::Scope context_scope(context);
       
       wlog("Start playing game.. with var ${v}", ("v", var));
       
       context->Global()->Set( String::NewFromUtf8(isolate, "$blockchain"), v8_blockchain::New(isolate, blockchain, blockchain->get_head_block_num()) );
       context->Global()->Set( String::NewFromUtf8(isolate, "$wallet"), v8_wallet::New(isolate, w) );
       
       auto _input = var; // TODO: convert/parse it to a v8 javascript object
       context->Global()->Set(String::NewFromUtf8(isolate, "$input"),  v8_helper::cpp_to_json(isolate, _input));
       
       v8::TryCatch try_catch;
       auto source =  "PLAY.play($blockchain, $wallet, $input);";
       
       vector<play_code> codes;
       v8::Handle<v8::Script> script = v8::Script::Compile( String::NewFromUtf8( my->GetIsolate(), source) );
       if ( script.IsEmpty() )
       {
           String::Utf8Value error(try_catch.Exception());
           FC_CAPTURE_AND_THROW(failed_compile_script, (source)(*error));
       } else
       {
           // Run the script to get the result.
           Handle<Value> result = script->Run();
           
           if ( result.IsEmpty() )
           {
               FC_CAPTURE_AND_THROW(failed_run_script, ( v8_helper::ReportException(isolate, &try_catch) ));
           } else
           {
               wlog("The result of the running of script is ${s}", ( "s",  v8_helper::ToCString(String::Utf8Value(result)) ));
               FC_ASSERT(result->IsArray(), "The script result should be array!");
               
               while ( result->IsArray() )
               {
                   v8::Handle<v8::Array> array = v8::Handle<v8::Array>::Cast(result);
                   FC_ASSERT( array->Length() >= 4 );
                   play_code code;
                   code.from_account = v8_helper::ToCString( String::Utf8Value( array->Get( 0 )->ToString( isolate ) ) );
                   code.to_account = v8_helper::ToCString( String::Utf8Value( array->Get( 1 )->ToString( isolate ) ) );
                   code.amount = v8_helper::json_to_cpp<asset>(isolate, array->Get( 2 ) );
                   code.memo = v8_helper::ToCString( String::Utf8Value( array->Get( 3 )->ToString( isolate ) ) );
                   
                   codes.push_back( code );
                   if ( array->Length() >= 5 && array->Get( 4 )->IsArray() )
                   {
                       result = array->Get( 4 );
                   } else
                   {
                       break;
                   }
               }
           }
       }
       
       FC_ASSERT( codes.size() > 0, "The result codes of play should exist at least 1." );
       
       bool first = true;
       for ( auto code : codes )
       {
           if( ! blockchain->is_valid_account_name( code.from_account ) )
               FC_THROW_EXCEPTION( invalid_name, "Invalid account name!", ("game_account_name", code.from_account) );
           
           if( ! blockchain->is_valid_account_name( code.to_account ) )
               FC_THROW_EXCEPTION( invalid_name, "Invalid account name!", ("game_account_name", code.to_account) );
           
           auto play_account = blockchain->get_account_record(code.from_account);
           
           auto to_account = blockchain->get_account_record(code.to_account);
           
           
           // TO REVIEW: permission limitation to other assets
           auto game_asset = blockchain->get_asset_record( code.amount.asset_id );
           FC_ASSERT( game_asset.valid() );
           FC_ASSERT( game_asset->is_game_issued() );
           FC_ASSERT( game_asset->issuer.issuer_id == game_id, "Spending the game asset must get the permission, with the condition that the issuer is this game." );
           
           // TODO make sure it is using account active key
           w->withdraw_to_transaction(code.amount,
                                      code.from_account,
                                      trx,
                                      required_signatures);
           
           // withdraw the transaction fee from the first account name
           if ( first )
           {
               w->withdraw_to_transaction( required_fees,
                                          code.from_account,
                                          trx,
                                          required_signatures );
               first = false;
           }
           
           auto entry = ledger_entry();
           entry.from_account = play_account->active_key();
           entry.to_account = to_account->active_key();
           entry.amount = code.amount;
           entry.memo = code.memo;
           record.ledger_entries.push_back( entry );
           
           required_signatures.insert( play_account->active_key() );
       }
       
       game_input input;
       input.game_id = game_id;
       input.data = var;
       trx.operations.push_back( game_operation(input) );
       
       record.fee = required_fees;
      
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
       
       auto source =  "PLAY.scan_result(scan_rtx, scan_result_block_num, scan_result_block_time, scan_result_received_time, scan_result_trx_index, scan_w);";
       
       v8::Handle<v8::Script> script = v8::Script::Compile( String::NewFromUtf8( my->GetIsolate(), source) );
       if ( script.IsEmpty() )
       {
           String::Utf8Value error(try_catch.Exception());
           FC_CAPTURE_AND_THROW(failed_compile_script, (source)(*error));
       } else
       {
           // Run the script to get the result.
           Handle<Value> result = script->Run();
           
           if ( result.IsEmpty() )
           {
               FC_CAPTURE_AND_THROW(failed_run_script, ( v8_helper::ReportException( my->GetIsolate(), &try_catch) ));
           } else
           {
               wlog("The result of the running of script is ${s}", ( "s",  v8_helper::ToCString(String::Utf8Value(result)) ));
               return result->ToBoolean(my->GetIsolate())->BooleanValue();
               // TODO: deal with the result to record
           }
       }
      
      return false;
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
      
       v8::TryCatch try_catch;
       auto source =  "PLAY.execute(execute_blockchain, execute_block_num, execute_pendingstate);";
       
       v8::Handle<v8::Script> script = v8::Script::Compile( String::NewFromUtf8( my->GetIsolate(), source) );
       if ( script.IsEmpty() )
       {
           String::Utf8Value error(try_catch.Exception());
           FC_CAPTURE_AND_THROW(failed_compile_script, (source)(*error));
       } else
       {
           // Run the script to get the result.
           Handle<Value> result = script->Run();
           
           if ( result.IsEmpty() )
           {
               FC_CAPTURE_AND_THROW(failed_run_script, ( v8_helper::ReportException( my->GetIsolate(), &try_catch) ));
           } else
           {
               wlog("The result of the running of script is ${s}", ( "s",  v8_helper::ToCString(String::Utf8Value(result)) ));
               // TODO: deal with the result to record
           }
       }
      
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
