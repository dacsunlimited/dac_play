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
        inst->init_class_template();
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
    
    void v8_game_engine::execute( chain_database_ptr blockchain, uint32_t block_num, const pending_chain_state_ptr& pending_state )
    {
        Persistent<Context> context = Context::New();
        
        // initialize the context pointer and blocknum
        V8_Blockchain* v8_blockchain = new V8_Blockchain(blockchain, block_num);
        V8_ChainState* v8_pendingstate = new V8_ChainState(pending_state);
        {
            //context "lock"
            Context::Scope context_scope(context);
            
            {
                //get class template
                Handle<Function> blockchain_ctor = blockchain_templ->GetFunction();
                
                //get class instance
                Local<Object> g_blockchain = blockchain_ctor->NewInstance();
                
                //build the "bridge" between c++ and javascript by associating the 'p' pointer to the first internal
                //field of the object
                g_blockchain->SetInternalField(0, External::New(v8_blockchain));
                
                //associates our internal field pointing to 'p' with the "point" name inside the context
                //this enable usage of point inside this context without need to create a new one
                context->Global()->Set(String::New("g_blockchain"), g_blockchain);
            }
            
            context->Global()->Set(String::New("g_block_num"), Integer::New(block_num));
            {
                Handle<Function> pendingstate_ctor = pendingstate_templ->GetFunction();
                Local<Object> g_pendingstate = pendingstate_ctor->NewInstance();
                g_pendingstate->SetInternalField(0, External::New(v8_pendingstate));
                context->Global()->Set(String::New("g_pendingstate"), g_pendingstate);
            }
            
            // TODO: get rule execute
            
            // begin script execution
            ExecuteString(String::New("'defined rule';execute(g_blockchain, g_block_num, g_pendingstate);"), String::New("rule.execute"), true);
        }
        
        context.Dispose();
        delete v8_blockchain;
    }
    
    bool v8_game_engine::init_class_template()
    {
        {
            //create a pointer to a class template
            blockchain_templ = FunctionTemplate::New();
            
            //assign the "BlockchainContext" name to the new class template
            blockchain_templ->SetClassName(String::New("Blockchain"));
            
            //access the class template
            Handle<ObjectTemplate> blockchain_proto = blockchain_templ->PrototypeTemplate();
            
            //associates the "method" string to the callback PointMethod in the class template
            //enabling point.method_a() constructions inside the javascript
            blockchain_proto->Set("get_current_random_seed", FunctionTemplate::New(v8_game_engine::V8_Blockchain_Get_Current_Random_Seed));
            
            //access the instance pointer of our new class template
            Handle<ObjectTemplate> blockchain_inst = blockchain_templ->InstanceTemplate();
            
            //set the internal fields of the class as we have the Point class internally
            blockchain_inst->SetInternalFieldCount(1);
            
            //associates the name "x" with its Get/Set functions
            blockchain_inst->SetAccessor(String::New("block_num"), V8_Blockchain_Get_Block_Number);
        }
        
        {
            block_templ = FunctionTemplate::New();
            block_templ->SetClassName(String::New("Block"));
            
            //access the class template
            Handle<ObjectTemplate> block_proto = block_templ->PrototypeTemplate();
            block_proto->Set("get_transactions", FunctionTemplate::New(V8_Block_Get_Transactions));
        }
        
        {
            pendingstate_templ = FunctionTemplate::New();
            pendingstate_templ->SetClassName(String::New("PendingState"));
            
            //access the class template
            Handle<ObjectTemplate> pendingstate_proto = pendingstate_templ->PrototypeTemplate();
            pendingstate_proto->Set("get_balance_record", FunctionTemplate::New(V8_Chain_State_Get_Blance_Record));
            pendingstate_proto->Set("get_asset_record", FunctionTemplate::New(V8_Chain_State_Get_Asset_Record));
            pendingstate_proto->Set("get_rule_data_record", FunctionTemplate::New(V8_Chain_State_Get_Rule_Data_Record));
            
            pendingstate_proto->Set("set_balance_record", FunctionTemplate::New(V8_Chain_State_Store_Blance_Record));
            pendingstate_proto->Set("set_asset_record", FunctionTemplate::New(V8_Chain_State_Store_Asset_Record));
            pendingstate_proto->Set("set_rule_data_record", FunctionTemplate::New(V8_Chain_State_Store_Rule_Data_Record));
        }
        
        {
            eval_state_templ = FunctionTemplate::New();
            eval_state_templ->SetClassName(String::New("EvalState"));
            
            Handle<ObjectTemplate> eval_state_proto = eval_state_templ->PrototypeTemplate();
            
            eval_state_proto->Set("sub_balance", FunctionTemplate::New(V8_Eval_State_Sub_Balance));
        }
        
        return true;
    }
    
    int test()
    {
        auto v8_engine = v8_game_engine::instance();
        
        Persistent<Context> context = Context::New();
        
        // initialize the context pointer and blocknum
        v8_game_engine::V8_Blockchain* b_context = new v8_game_engine::V8_Blockchain(nullptr, 0);
        
        // printf("update x with 777 inside C++\n");
        // p->x_ = 777;
        {
            //context "lock"
            Context::Scope context_scope(context);
            
            //get class template
            Handle<Function> blockchain_context_ctor = v8_engine.blockchain_templ->GetFunction();
            
            //get class instance
            Local<Object> obj = blockchain_context_ctor->NewInstance();
            
            //build the "bridge" between c++ and javascript by associating the 'p' pointer to the first internal
            //field of the object
            obj->SetInternalField(0, External::New(b_context));
            
            //associates our internal field pointing to 'p' with the "point" name inside the context
            //this enable usage of point inside this context without need to create a new one
            context->Global()->Set(String::New("blockchain_context"), obj);
            
            // begin script execution
            v8_game_engine::ExecuteString(String::New("blockchain_context.get_current_random_seed()"), String::New("rule.sample"), true);
        }
        
        printf("Now inside C++\n");
        
        context.Dispose();
        
        delete b_context;
        
        return 0;
    }
    
} } // bts::game