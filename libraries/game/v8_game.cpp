#include <bts/game/v8_game.hpp>

#include <bts/blockchain/time.hpp>

namespace bts { namespace game {
    using namespace v8;
    
    static bool first_engine = []()->bool{
        
        
        return true;
    }();
    
    v8_game_engine& v8_game_engine::instance()
    {
        static std::unique_ptr<v8_game_engine> inst( new v8_game_engine() );
        inst->init_class_template(v8::Isolate::GetCurrent());
        return *inst;
    }
    
    void v8_game_engine::evaluate( transaction_evaluation_state& eval_state)
    {
        
        // TODO
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
    
    bool scan( wallet_transaction_record& trx_rec, bts::wallet::wallet_ptr w )
    {
        
    }
    
    bool scan_result( const rule_result_transaction& rtrx,
                     uint32_t block_num,
                     const time_point_sec& block_time,
                     const time_point_sec& received_time,
                     const uint32_t trx_index, bts::wallet::wallet_ptr w)
    {
        
    }
    
    int v8_game_engine::execute( chain_database_ptr blockchain, uint32_t block_num, const pending_chain_state_ptr& pending_state )
    {
        v8::V8::InitializeICU();
        v8::Platform* platform = v8::platform::CreateDefaultPlatform();
        v8::V8::InitializePlatform(platform);
        v8::V8::Initialize();
        v8::Isolate* isolate = v8::Isolate::New();
        
        // initialize the context pointer and blocknum
        V8_Blockchain* v8_blockchain = new V8_Blockchain(blockchain, block_num);
        V8_ChainState* v8_pendingstate = new V8_ChainState(pending_state);
        int result = 0;
        {
            v8::Isolate::Scope isolate_scope(isolate);
            v8::HandleScope handle_scope(isolate);
            v8::Handle<v8::Context> context = CreateShellContext(isolate);
            if (context.IsEmpty()) {
                fprintf(stderr, "Error creating context\n");
                return 1;
            }
            v8::Context::Scope context_scope(context);
            {
                //get class template
                Handle<Function> blockchain_ctor = blockchain_templ->GetFunction();
                
                //get class instance
                Local<Object> g_blockchain = blockchain_ctor->NewInstance();
                
                //build the "bridge" between c++ and javascript by associating the 'p' pointer to the first internal
                //field of the object
                g_blockchain->SetInternalField(0, External::New(isolate, v8_blockchain));
                
                //associates our internal field pointing to 'p' with the "point" name inside the context
                //this enable usage of point inside this context without need to create a new one
                context->Global()->Set(String::NewFromUtf8(isolate, "g_blockchain"), g_blockchain);
            }
            
            context->Global()->Set(String::NewFromUtf8(isolate, "g_block_num"), Integer::New(isolate, block_num));
            {
                Handle<Function> pendingstate_ctor = pendingstate_templ->GetFunction();
                Local<Object> g_pendingstate = pendingstate_ctor->NewInstance();
                g_pendingstate->SetInternalField(0, External::New(isolate, v8_pendingstate));
                context->Global()->Set(String::NewFromUtf8(isolate, "g_pendingstate"), g_pendingstate);
            }
            
            // TODO: get rule execute
            
            // begin script execution
            ExecuteString(isolate, String::NewFromUtf8(isolate, "'defined rule';execute(g_blockchain, g_block_num, g_pendingstate);"), String::NewFromUtf8(isolate, "rule.execute"), true, true);
            
            // result = RunMain(isolate, argc, argv);
        }
        
        v8::V8::Dispose();
        v8::V8::ShutdownPlatform();
        delete platform;
        
        delete v8_blockchain;
        delete v8_pendingstate;
        
        return result;
    }
    
    /*
    int main(int argc, char* argv[]) {
        v8::V8::InitializeICU();
        v8::Platform* platform = v8::platform::CreateDefaultPlatform();
        v8::V8::InitializePlatform(platform);
        v8::V8::Initialize();
        v8::V8::SetFlagsFromCommandLine(&argc, argv, true);
        ShellArrayBufferAllocator array_buffer_allocator;
        v8::V8::SetArrayBufferAllocator(&array_buffer_allocator);
        v8::Isolate* isolate = v8::Isolate::New();
        run_shell = (argc == 1);
        int result;
        {
            v8::Isolate::Scope isolate_scope(isolate);
            v8::HandleScope handle_scope(isolate);
            v8::Handle<v8::Context> context = CreateShellContext(isolate);
            if (context.IsEmpty()) {
                fprintf(stderr, "Error creating context\n");
                return 1;
            }
            v8::Context::Scope context_scope(context);
            result = RunMain(isolate, argc, argv);
            if (run_shell) RunShell(context);
        }
        v8::V8::Dispose();
        v8::V8::ShutdownPlatform();
        delete platform;
        return result;
    }
     */
    
    bool v8_game_engine::init_class_template(v8::Isolate* isolate)
    {
        {
            //create a pointer to a class template
            blockchain_templ = FunctionTemplate::New(isolate);
            
            //assign the "BlockchainContext" name to the new class template
            blockchain_templ->SetClassName(String::NewFromUtf8(isolate, "Blockchain"));
            
            //access the class template
            Handle<ObjectTemplate> blockchain_proto = blockchain_templ->PrototypeTemplate();
            
            //associates the "method" string to the callback PointMethod in the class template
            //enabling point.method_a() constructions inside the javascript
            blockchain_proto->Set(isolate, "get_current_random_seed", FunctionTemplate::New(isolate, v8_game_engine::V8_Blockchain_Get_Current_Random_Seed));
            
            //access the instance pointer of our new class template
            Handle<ObjectTemplate> blockchain_inst = blockchain_templ->InstanceTemplate();
            
            //set the internal fields of the class as we have the Point class internally
            blockchain_inst->SetInternalFieldCount(1);
            
            //associates the name "x" with its Get/Set functions
            blockchain_inst->SetAccessor(String::NewFromUtf8(isolate, "block_num"), V8_Blockchain_Get_Block_Number);
        }
        
        {
            block_templ = FunctionTemplate::New(isolate);
            block_templ->SetClassName(String::NewFromUtf8(isolate, "Block"));
            
            //access the class template
            Handle<ObjectTemplate> block_proto = block_templ->PrototypeTemplate();
            block_proto->Set(isolate, "get_transactions", FunctionTemplate::New(isolate, V8_Block_Get_Transactions));
        }
        
        {
            pendingstate_templ = FunctionTemplate::New(isolate);
            pendingstate_templ->SetClassName(String::NewFromUtf8(isolate, "PendingState"));
            
            //access the class template
            Handle<ObjectTemplate> pendingstate_proto = pendingstate_templ->PrototypeTemplate();
            pendingstate_proto->Set(isolate, "get_balance_record", FunctionTemplate::New(isolate, V8_Chain_State_Get_Blance_Record));
            pendingstate_proto->Set(isolate, "get_asset_record", FunctionTemplate::New(isolate, V8_Chain_State_Get_Asset_Record));
            pendingstate_proto->Set(isolate, "get_rule_data_record", FunctionTemplate::New(isolate, V8_Chain_State_Get_Rule_Data_Record));
            
            pendingstate_proto->Set(isolate, "set_balance_record", FunctionTemplate::New(isolate, V8_Chain_State_Store_Blance_Record));
            pendingstate_proto->Set(isolate, "set_asset_record", FunctionTemplate::New(isolate, V8_Chain_State_Store_Asset_Record));
            pendingstate_proto->Set(isolate, "set_rule_data_record", FunctionTemplate::New(isolate, V8_Chain_State_Store_Rule_Data_Record));
        }
        
        {
            eval_state_templ = FunctionTemplate::New(isolate);
            eval_state_templ->SetClassName(String::NewFromUtf8(isolate, "EvalState"));
            
            Handle<ObjectTemplate> eval_state_proto = eval_state_templ->PrototypeTemplate();
            
            eval_state_proto->Set(isolate, "sub_balance", FunctionTemplate::New(isolate, V8_Eval_State_Sub_Balance));
        }
        
        return true;
    }
    
} } // bts::game