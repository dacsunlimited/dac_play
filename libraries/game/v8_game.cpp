#include <bts/game/v8_game.hpp>

namespace bts { namespace game {
    using namespace v8;
    
    bool init()
    {
        // Blockchain
        //create a pointer to a class template
        blockchain_templ = FunctionTemplate::New();
        
        //assign the "BlockchainContext" name to the new class template
        blockchain_templ->SetClassName(String::New("BlockchainContext"));
        
        //access the class template
        Handle<ObjectTemplate> blockchain_proto = blockchain_templ->PrototypeTemplate();
        
        //associates the "method" string to the callback PointMethod in the class template
        //enabling point.method_a() constructions inside the javascript
        blockchain_proto->Set("get_current_random_seed", FunctionTemplate::New(Get_Current_Random_Seed));
        
        //access the instance pointer of our new class template
        Handle<ObjectTemplate> blockchain_inst = blockchain_templ->InstanceTemplate();
        
        //set the internal fields of the class as we have the Point class internally
        blockchain_inst->SetInternalFieldCount(1);
        
        //associates the name "x" with its Get/Set functions
        blockchain_inst->SetAccessor(String::New("block_num"), Get_Block_Number);
        
        blockchain_inst->SetAccessor(String::New("data_record"), 0, Set_Data_Record);
        
        
        ///------------
        block_templ = FunctionTemplate::New();
        block_templ->SetClassName(String::New("Block"));
        
        //access the class template
        Handle<ObjectTemplate> block_proto = block_templ->PrototypeTemplate();
        block_proto->Set("get_transactions", FunctionTemplate::New(Get_Transactions));
        
        return true;
    }
    
    int test()
    {
        
        Persistent<Context> context = Context::New();
        
        // initialize the context pointer and blocknum
        BlockchainContext* b_context = new BlockchainContext(nullptr, 0, 0);
        
        // printf("update x with 777 inside C++\n");
        // p->x_ = 777;
        {
            //context "lock"
            Context::Scope context_scope(context);
            
            //get class template
            Handle<Function> blockchain_context_ctor = blockchain_templ->GetFunction();
            
            //get class instance
            Local<Object> obj = blockchain_context_ctor->NewInstance();
            
            //build the "bridge" between c++ and javascript by associating the 'p' pointer to the first internal
            //field of the object
            obj->SetInternalField(0, External::New(b_context));
            
            //associates our internal field pointing to 'p' with the "point" name inside the context
            //this enable usage of point inside this context without need to create a new one
            context->Global()->Set(String::New("blockchain_context"), obj);
            
            // begin script execution
            ExecuteString(String::New("blockchain_context.get_current_random_seed()"), String::New("rule.sample"), true);
        }
        
        printf("Now inside C++\n");
        
        context.Dispose();
        
        delete b_context;
        
        return 0;
    }
    
} } // bts::game