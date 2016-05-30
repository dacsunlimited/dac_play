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
         std::string                        _game_name;
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
            
            //_isolate = v8::Isolate::GetCurrent();
            _isolate = (Isolate*)_client->get_isolate();
             
            v8::Locker locker(_isolate);
            Isolate::Scope isolate_scope(_isolate);
            HandleScope handle_scope(_isolate);
            v8::TryCatch try_catch( _isolate );
            v8::Handle<v8::Context> context = v8_helper::CreateShellContext(_isolate);
            if (context.IsEmpty()) {
                String::Utf8Value error(try_catch.Exception());
                // wlog("Error creating context in game ${name}, error is ${e}", ("name", _game_name)("e", *error));
                FC_CAPTURE_AND_THROW(failed_game_engine_init);
            }
            _context.Reset(_isolate, context);
            
            Context::Scope context_scope(context);
            
            //ilog("The game is ${s}", ("s", _game_name ));
            
             auto ogame_rec = _client->get_chain_database()->get_game_record( _game_name );
             FC_ASSERT( ogame_rec.valid() );
             
             v8::Handle<v8::String> source = v8::String::NewFromUtf8( GetIsolate(), ogame_rec->script_code.c_str() );
            
             if (ogame_rec->script_code.empty()) {
                 //wlog("The souce is empty, error loading script code");
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
                    wlog("Successfull to init the game engine.");
                    wlog("Script init result is ${s}", ( "s",  v8_helper::ToCString(String::Utf8Value(result)) ));
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
       
      auto ogame_rec = my->_client->get_chain_database()->get_game_record( game_name );
      FC_ASSERT( ogame_rec.valid() );
       
      auto game_assets = my->_client->get_chain_database()->get_assets_by_issuer( asset_record::game_issuer_id, ogame_rec->id);
       
      global(ogame_rec->id, game_assets);
   }
    
   bool v8_game_engine::global( game_id_type game_id, vector<asset_record> game_assets)
   {
       auto isolate = my->GetIsolate();
       v8::Locker locker( isolate );
       Isolate::Scope isolate_scope(my->GetIsolate());
       v8::HandleScope handle_scope( isolate );
       v8::Local<v8::Context> context = v8::Local<v8::Context>::New(my->GetIsolate(), my->_context);
       
       // Entering the context
       Context::Scope context_scope(context);
       
       v8::TryCatch try_catch(my->GetIsolate());
       
       Local<Function> evaluate_func;
       Local<Value>  argv[2] ;
       
       auto play = context->Global()->Get( String::NewFromUtf8( my->GetIsolate(), "PLAY") );
       
       auto evaluate = play->ToObject()->Get( String::NewFromUtf8( my->GetIsolate(), "global") );
       
       if(!evaluate->IsFunction()) {
           FC_CAPTURE_AND_THROW( failed_compile_script );
       } else {
           evaluate_func = Handle<Function>::Cast(evaluate);
           argv[0] = v8_helper::cpp_to_json(isolate, game_id);
           argv[1] = v8_helper::cpp_to_json( isolate, game_assets );
           
           Local<Value> result = evaluate_func->Call(context->Global(), 2, argv);
           
           if ( result.IsEmpty() )
           {
               FC_CAPTURE_AND_THROW(failed_run_script, (v8_helper::ReportException(my->GetIsolate(), &try_catch)));
           } else
           {
               variant v = v8_helper::json_to_cpp<variant>(isolate, result);
               //wlog("The result of the running of script is ${s}", ( "s",  v) );
               return v.as_bool();
           }
       }
   }
    
   void v8_game_engine::evaluate( transaction_evaluation_state& eval_state, game_id_type game_id, const variant& var)
   {
       auto isolate = my->GetIsolate();
       v8::Locker locker( isolate );
       Isolate::Scope isolate_scope(my->GetIsolate());
       v8::HandleScope handle_scope( isolate );
       v8::Local<v8::Context> context = v8::Local<v8::Context>::New(my->GetIsolate(), my->_context);
      
       // Entering the context
       Context::Scope context_scope(context);
       
       v8::TryCatch try_catch(my->GetIsolate());
       
       Local<Function> evaluate_func;
       Local<Value>  argv[3] ;
       
       auto play = context->Global()->Get( String::NewFromUtf8( my->GetIsolate(), "PLAY") );
       
       auto evaluate = play->ToObject()->Get( String::NewFromUtf8( my->GetIsolate(), "evaluate") );
       
       if(!evaluate->IsFunction()) {
           FC_CAPTURE_AND_THROW( failed_compile_script );
       } else {
           evaluate_func = Handle<Function>::Cast(evaluate);
           argv[0] = v8_evalstate::New( isolate, &eval_state );
           argv[1] = v8_chainstate::New( isolate, eval_state.pending_state()->shared_from_this() );
           auto _input = var; // TODO: convert/parse it to a v8 javascript object
           argv[2] = v8_helper::cpp_to_json(isolate, _input);
           
           Local<Value> result = evaluate_func->Call(context->Global(), 3, argv);
       
           //wlog("Start evaluating the game.. with var ${v}", ("v", var));
           
           if ( result.IsEmpty() )
           {
               FC_CAPTURE_AND_THROW(failed_run_script, (v8_helper::ReportException(my->GetIsolate(), &try_catch)));
           } else
           {
               auto result_obj = v8_helper::json_to_cpp<variant>( isolate, result );
               //wlog("The result of the running of script is ${s}", ( "s",  result_obj ));
               
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
       vector<play_code> codes;
      
       auto isolate = my->GetIsolate();
       v8::Locker locker(isolate);
       
       // crash when using game_play #162
       // https://github.com/bitsuperlab/cpp-play/issues/162#issuecomment-14764010
       //
       // below is a temp fix for #162
       // play() is in a new fiber,which has its own stack, though it's in a same thread as execute()
       // so we need to adjust the stacklimit to make V8 believe that there is no stackoverfow happens
       //
       // basically, we should set a non-default stack limit separately for each thread, and i think this
       // also apply to fibers.
       //
       // final solution should be adjust the stacklimit on fiber_switch(), which need more research.
       //
       // https://codereview.chromium.org/572243002/diff/1/Source/bindings/core/v8/V8Initializer.cpp
       
//       uint32_t here;
//       static const int kWorkerMaxStackSize = 500 * 1024;
//       isolate->SetStackLimit(reinterpret_cast<uintptr_t>(&here - kWorkerMaxStackSize / sizeof(uint32_t*)));

       Isolate::Scope isolate_scope(my->GetIsolate());
       v8::HandleScope handle_scope( isolate );
       
       v8::Local<v8::Context> context = v8::Local<v8::Context>::New(isolate, my->_context);
       
       
       // Entering the context
       Context::Scope context_scope(context);
       
       v8::TryCatch try_catch(my->GetIsolate());
       
       Local<Function> play_func;
       Local<Value>  argv[3] ;
       
       auto play = context->Global()->Get( String::NewFromUtf8( my->GetIsolate(), "PLAY") );
       
       auto play_f = play->ToObject()->Get( String::NewFromUtf8( my->GetIsolate(), "play") );
       
       if(!play_f->IsFunction()) {
           FC_CAPTURE_AND_THROW( failed_compile_script );
       } else {
           play_func = Handle<Function>::Cast(play_f);
           argv[0] = v8_blockchain::New(isolate, blockchain, blockchain->get_head_block_num());
           argv[1] = v8_wallet::New(isolate, w);
           auto _input = var; // TODO: convert/parse it to a v8 javascript object
           argv[2] = v8_helper::cpp_to_json(isolate, _input);
           
           // wlog("Start game play script.. with var ${v}", ("v", var));
           Local<Value> result = play_func->Call(context->Global(), 3, argv);
           
           if ( result.IsEmpty() )
           {
               FC_CAPTURE_AND_THROW(failed_run_script, ( v8_helper::ReportException(isolate, &try_catch) ));
           } else
           {
               auto v = v8_helper::json_to_cpp<variant>(isolate, result);
               // wlog("The result of the running of script is ${s}", ( "s",  v ));
               FC_ASSERT( v.is_array(), "The script result should be array!");
               
               while ( v.is_array() )
               {
                   auto array = v.get_array();
                   FC_ASSERT( array.size() >= 4 );
                   play_code code;
                   code.from_account = array[0].as_string();
                   code.to_account = array[1].as_string();
                   code.amount = array[2].as<asset>();
                   code.memo = array[3].as_string();
                   
                   codes.push_back( code );
                   if ( array.size() >= 5 && array[4].is_array() )
                   {
                       v = array[4];
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
       trx.operations.push_back( game_play_operation(input) );
       
       record.fee = required_fees;
      
      if( sign )
         w->sign_transaction( trx, required_signatures );
      
      record.trx = trx;
      return record;
   }
   
   bool v8_game_engine::scan_ledger( chain_database_ptr blockchain, bts::wallet::wallet_ptr w, wallet_transaction_record& trx_rec,  const variant& var )
   {
       auto isolate = my->GetIsolate();
       v8::Locker locker(isolate);
       Isolate::Scope isolate_scope(my->GetIsolate());
       v8::HandleScope handle_scope( isolate );
       v8::Local<v8::Context> context = v8::Local<v8::Context>::New(isolate, my->_context);
       // Entering the context
       Context::Scope context_scope(context);
       
       v8::TryCatch try_catch(my->GetIsolate());
       
       Local<Function> scan_ledger_func;
       Local<Value>  argv[4] ;
       
       auto play = context->Global()->Get( String::NewFromUtf8( my->GetIsolate(), "PLAY") );
       
       auto scan_ledger = play->ToObject()->Get( String::NewFromUtf8( my->GetIsolate(), "scan_ledger") );
       
       if(!scan_ledger->IsFunction()) {
           FC_CAPTURE_AND_THROW( failed_compile_script );
       } else {
           scan_ledger_func = Handle<Function>::Cast(scan_ledger);
           argv[0] = v8_blockchain::New(my->GetIsolate(), blockchain, blockchain->get_head_block_num());
           argv[1] = v8_helper::cpp_to_json(isolate, trx_rec);
           argv[2] = v8_wallet::New(isolate, w);
           auto _input = var; // TODO: convert/parse it to a v8 javascript object
           argv[3] = v8_helper::cpp_to_json(isolate, _input);
           
           // Run the script to get the result.
           Local<Value> result = scan_ledger_func->Call(context->Global(), 4, argv);
           
           if ( result.IsEmpty() )
           {
               FC_CAPTURE_AND_THROW(failed_run_script, ( v8_helper::ReportException(isolate, &try_catch) ));
           } else
           {
               auto v = v8_helper::json_to_cpp<variant>(isolate, result);
               // wlog("The result of the running of script is ${s}", ( "s", v ));
               
               FC_ASSERT( v.is_object() && v.get_object().contains("wallet_trx_record") && v.get_object().contains( "has_deposit" ));
               
               auto w_trx_rec = v.get_object()["wallet_trx_record"].as<wallet_transaction_record>();
               
               // TODO: Check How to modify and update the reference trx_rec, current only the ledger_entries is possible to be changed.
               trx_rec = w_trx_rec;
               
               bool has_deposit = v.get_object()["has_deposit"].as_bool();
               return has_deposit;
           }
       }
       
       return false;
   }
   
   bool v8_game_engine::scan_result( const game_result_transaction& rtrx,
                    uint32_t block_num,
                    const time_point_sec& block_time,
                    const uint32_t trx_index, bts::wallet::wallet_ptr w)
   {
       auto isolate = my->GetIsolate();
       v8::Locker locker(isolate);
       Isolate::Scope isolate_scope(isolate);
       v8::HandleScope handle_scope(isolate);
       v8::Local<v8::Context> context = v8::Local<v8::Context>::New(isolate, my->_context);
       v8::Context::Scope context_scope(context);
       
       v8::TryCatch try_catch(my->GetIsolate());
       
       Local<Function> scan_result_func;
       Local<Value>  argv[5] ;
       
       auto play = context->Global()->Get( String::NewFromUtf8( my->GetIsolate(), "PLAY") );
       
       auto scan_result = play->ToObject()->Get( String::NewFromUtf8( my->GetIsolate(), "scan_result") );
       
       if(!scan_result->IsFunction()) {
           FC_CAPTURE_AND_THROW( failed_compile_script );
       } else {
           scan_result_func = Handle<Function>::Cast(scan_result);
           argv[0] = v8_helper::cpp_to_json(isolate, rtrx);
           argv[1] = Integer::New(my->GetIsolate(), block_num);
           argv[2] = String::NewFromUtf8(my->GetIsolate(), fc::variant(block_time).as_string().c_str() );
           argv[3] = Integer::New(my->GetIsolate(), trx_index);
           argv[4] = v8_wallet::New(isolate, w);
           
           Local<Value> result = scan_result_func->Call(context->Global(), 5, argv);
           
           if ( result.IsEmpty() )
           {
               FC_CAPTURE_AND_THROW(failed_run_script, ( v8_helper::ReportException( my->GetIsolate(), &try_catch) ));
           } else
           {
               variant v = v8_helper::json_to_cpp<variant>(isolate, result);
               //wlog("The result of the running of script is ${s}", ( "s",  v) );
               return v.as_bool();
           }
       }
      
      return false;
   }
   
   void v8_game_engine::execute( game_id_type game_id, chain_database_ptr blockchain, uint32_t block_num, const pending_chain_state_ptr& pending_state )
   {
       try {
           //wlog("Start execute in game engine...");
           v8::Locker locker(my->GetIsolate());
           Isolate::Scope isolate_scope(my->GetIsolate());
           v8::HandleScope handle_scope(my->GetIsolate());
           v8::Local<v8::Context> context = v8::Local<v8::Context>::New(my->GetIsolate(), my->_context);
           v8::Context::Scope context_scope(context);
           v8::TryCatch try_catch( my->GetIsolate() );
           
           Local<Function> execute_func;
           Local<Value>  argv[3] ;
           
           auto play = context->Global()->Get( String::NewFromUtf8( my->GetIsolate(), "PLAY") );
           
           auto execute = play->ToObject()->Get( String::NewFromUtf8( my->GetIsolate(), "execute") );
           
           if(!execute->IsFunction()) {
               FC_CAPTURE_AND_THROW( failed_compile_script );
           } else {
               execute_func = Handle<Function>::Cast(execute);
               
               argv[0] = v8_blockchain::New(my->GetIsolate(), blockchain, block_num);
               argv[1] = Integer::New(my->GetIsolate(), block_num);
               argv[2] = v8_chainstate::New(my->GetIsolate(), pending_state);
               
               // Run the script to get the result.
               // wlog("Run the script to get the result...");
               Local<Value> result = execute_func->Call(context->Global(), 3, argv);
               
               if ( result.IsEmpty() )
               {
                   FC_CAPTURE_AND_THROW(failed_run_script, ( v8_helper::ReportException( my->GetIsolate(), &try_catch) ));
               } else
               {
                   //assert(!try_catch.HasCaught());
                    //if (!result->IsUndefined()) {
                    // TOOD: return the result
                   //}
                   auto v = v8_helper::json_to_cpp<variant>(my->GetIsolate(), result);
                   // wlog("The result of the running of script is ${s}", ( "s",  v ));
                   if ( v.is_numeric() && v.as_int64() == 0 )
                   {
                       // wlog("Nothing is done...");
                   } else
                   {
                       FC_ASSERT( v.is_object() );
                       auto execute_results = v.get_object()["execute_results"];
                       auto game_datas = v.get_object()["game_datas"];
                       auto diff_balances = v.get_object()["diff_balances"];
                       auto diff_supply = v.get_object()["diff_supply"];
                   
                       FC_ASSERT( execute_results.is_array() );

                       if (execute_results.get_array().size())
                       {
                           vector<game_result_transaction> game_result_transactions;

                           game_result_transactions.reserve(execute_results.get_array().size());
                           for (auto result : execute_results.get_array())
                           {
                               game_result_transaction g_trx;
                               g_trx.game_id = game_id;
                               g_trx.data = result;
                               game_result_transactions.push_back(std::move(g_trx));
                           }

                           // execute() could be re-enterred for mutiple games execution in one block
                           // so here we need to append the game transactions instend of re-set
                           pending_state->game_result_transactions.insert(pending_state->game_result_transactions.end(),
                               game_result_transactions.begin(), game_result_transactions.end());
                       }

                   
                       FC_ASSERT( game_datas.is_array() );
                       for ( auto d : game_datas.get_array() ) {
                           if ( d.is_numeric() )    // game_data must be a object and include a property called index
                           {
                               game_data_record null_rec;
                               pending_state->store_game_data_record(game_id, d.as<data_id_type>(), null_rec.make_null() );
                           } else {
                               FC_ASSERT( d.is_object() && d.get_object().contains( "index" ) );
                               game_data_record g_rec;
                               g_rec.game_id = game_id;
                               g_rec.data = d;
                           
                               pending_state->store_game_data_record(game_id, g_rec.get_game_data_index(), std::move( g_rec ) );
                           }
                       }
                   
                       FC_ASSERT( diff_balances.is_array() );
                       for ( auto b : diff_balances.get_array() )
                       {
                           FC_ASSERT( b.is_object() && b.get_object().contains( "owner" ) );
                           FC_ASSERT( b.is_object() && b.get_object().contains( "asset" ) );
                           auto owner = b.get_object()["owner"].as<address>();
                           auto diff_asset = b.get_object()["asset"].as<asset>();
                       
                           auto game_asset = blockchain->get_asset_record( diff_asset.asset_id );
                           FC_ASSERT( game_asset.valid() );
                           FC_ASSERT( game_asset->is_game_issued() );
                           FC_ASSERT( game_asset->issuer.issuer_id == game_id, "Updating the game asset must get the permission, with the condition that the issuer is this game." );
                       
                           withdraw_condition balance_condition(withdraw_with_signature(owner), asset_id_type(diff_asset.asset_id));
                           obalance_record balance_rec = pending_state->get_balance_record( balance_condition.get_address() );
                           if( !balance_rec.valid() )
                           {
                               balance_rec = balance_record( balance_condition );
                           }
                           if( balance_rec->balance == 0 || diff_asset.amount <= 0 ) // negetive deposit
                           {
                               balance_rec->deposit_date = pending_state->now();
                           }
                           else
                           {
                               fc::uint128 old_sec_since_epoch( balance_rec->deposit_date.sec_since_epoch() );
                               fc::uint128 new_sec_since_epoch( pending_state->now().sec_since_epoch() );
                           
                               fc::uint128 avg = (old_sec_since_epoch * balance_rec->balance) + (new_sec_since_epoch * diff_asset.amount );
                               avg /= (balance_rec->balance + diff_asset.amount);
                           
                               balance_rec->deposit_date = time_point_sec( avg.to_integer() );
                           }
                       
                           balance_rec->balance += diff_asset.amount;
                           FC_ASSERT( balance_rec->balance >= 0);
                           pending_state->store_balance_record( *balance_rec );
                       }
                   
                       FC_ASSERT( diff_supply.is_array() );
                       for ( auto s : diff_supply.get_array() )
                       {
                           FC_ASSERT( s.is_object() );
                           auto supply_change = s.as<asset>();
                           auto game_base_asset_record = pending_state->get_asset_record( supply_change.asset_id );
                           FC_ASSERT( game_base_asset_record.valid() );
                           FC_ASSERT( game_base_asset_record->is_game_issued() );
                           FC_ASSERT( game_base_asset_record->issuer.issuer_id == game_id, "Updating the game asset must get the permission, with the condition that the issuer is this game." );
                       
                           game_base_asset_record->current_supply += supply_change.amount;
                           pending_state->store_asset_record( *game_base_asset_record );
                       }
                   
                       // TODO: check supply diff
                   }
               }
           }
           // wlog("End running the script in game engine...");
           
       }
       catch( const fc::exception& e )
       {
           // wlog( "error executing game contract  ${game_id}\n ${e}", ("game_id", game_id)("e",e.to_detail_string()) );
           
           ogame_status game_stat = pending_state->get_game_status( game_id );
           if( !game_stat.valid() ) game_stat = game_status( game_id );
           game_stat->block_number = block_num;
           game_stat->last_error = e;
           pending_state->store_game_status( *game_stat );
       }
   }
} } // bts::game
