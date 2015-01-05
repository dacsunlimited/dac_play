#include <bts/game/v8_game.hpp>

namespace bts { namespace game {
    using namespace v8;
    
    static bool first_engine = []()->bool{
        
        
        return true;
    }();
    
    v8_game_engine& v8_game_engine::instance()
    {
        static std::unique_ptr<v8_game_engine> inst( new v8_game_engine() );
        return *inst;
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
            pendingchainstate_templ = FunctionTemplate::New();
            pendingchainstate_templ->SetClassName(String::New("PendingState"));
            
            //access the class template
            Handle<ObjectTemplate> pendingchainstate_proto = pendingchainstate_templ->PrototypeTemplate();
            pendingchainstate_proto->Set("get_balance_record", FunctionTemplate::New(V8_Pending_State_Get_Blance_Record));
        }
        return true;
    }
    
    void v8_game_engine::execute( chain_database_ptr blockchain, uint32_t block_num, const pending_chain_state_ptr& pending_state )
    {
        Persistent<Context> context = Context::New();
        
        // initialize the context pointer and blocknum
        V8_Blockchain* v8_blockchain = new V8_Blockchain(blockchain, block_num);
        V8_PendingState* v8_pendingchainstate = new V8_PendingState(pending_state);
        {
            //context "lock"
            Context::Scope context_scope(context);
            
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
            
            context->Global()->Set(String::New("g_block_num"), Integer::New(block_num));
            
            Handle<Function> pendingstate_ctor = pendingchainstate_templ->GetFunction();
            Local<Object> g_pendingstate = pendingstate_ctor->NewInstance();
            g_pendingstate->SetInternalField(0, External::New(v8_pendingchainstate));
            context->Global()->Set(String::New("g_pendingstate"), g_blockchain);
            
            // TODO: get rule execute
            
            // begin script execution
            ExecuteString(String::New("'defined rule';execute(g_blockchain, g_block_num, g_pendingstate);"), String::New("rule.execute"), true);
        }
        
        context.Dispose();
        delete v8_blockchain;
    }
    
    int test()
    {
        auto v8_engine = v8_game_engine::instance();
        v8_engine.init_class_template();
        
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