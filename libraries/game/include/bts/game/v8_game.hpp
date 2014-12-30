#include <bts/blockchain/exceptions.hpp>
#include <bts/game/rule_record.hpp>
#include <bts/blockchain/transaction_evaluation_state.hpp>
#include <bts/blockchain/chain_interface.hpp>
#include <bts/blockchain/game_executors.hpp>

#include <v8.h>

namespace bts { namespace game {
    
    using namespace v8;
    using namespace bts::blockchain;
    
    v8::Handle<v8::ObjectTemplate> global;
    
    Handle<FunctionTemplate> blockchain_templ;
    
    Handle<FunctionTemplate> block_templ;
    
    Handle<FunctionTemplate> transaction_templ;
    
    // PlayContext mapped to v8
    class BlockchainContext
    {
    public:
        //constructor
        BlockchainContext(chain_database_ptr blockchain, int block_num, int data_record):_blockchain(blockchain), _block_num(block_num),_data_record(data_record){}
        
        //variables
        chain_database_ptr _blockchain;
        uint32_t _block_num;
        uint32_t _data_record;
    };
    
    // binding to blockchain to get block
    Handle<Value> Get_Block(const Arguments& args)
    {
        Local<Object> self = args.Holder();
        Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
        void* ptr = wrap->Value();
        //get member variable value
        auto block = static_cast<BlockchainContext*>(ptr)->_blockchain->get_block(args[0]->Uint32Value());
        //return the value
        
        //get class template
        Handle<Function> block_ctor = block_templ->GetFunction();
        
        //get class instance
        Local<Object> obj = block_ctor->NewInstance();
        
        //build the "bridge" between c++ and javascript by associating the 'p' pointer to the first internal
        //field of the object
        // TODO: Is it ok to return address of block here?
        obj->SetInternalField(0, External::New(&block));
        
        return obj;
    }
    
    // Taken from the embedder's guide
    Handle<Value> Get_Block_Number(Local<String> property, const AccessorInfo& info)
    {
        /*
         //this only shows information on what object is being used... just for fun
         {
         String::AsciiValue holder(info.Holder()->ToString()), self(info.This()->ToString());
         printf("getter: holder(%s), self(%s)\n", *holder, *self);
         }
         */
        //get object holder
        Local<Object> self = info.Holder();
        //get the holder's external object
        Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
        //get the pointer for Point
        void* ptr = wrap->Value();
        //get member variable value
        uint32_t value = static_cast<BlockchainContext*>(ptr)->_block_num;
        //return the value
        return Integer::New(value);
    }
    
    Handle<Value> Get_Current_Random_Seed(const Arguments& args)
    {
        Local<Object> self = args.Holder();
        Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
        void* ptr = wrap->Value();
        //get member variable value
        uint32_t value = static_cast<BlockchainContext*>(ptr)->_blockchain->get_current_random_seed()._hash[0];
        //return the value
        return Integer::New(value);
    }
    
    // Taken from the embedder's guide
    void Set_Data_Record(Local<String> property, Local<Value> value, const AccessorInfo& info)
    {
        Local<Object> self = info.Holder();
        Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
        void* ptr = wrap->Value();
        static_cast<BlockchainContext*>(ptr)->_data_record = value->Uint32Value();
    }
    
    class PendingChainState
    {
    public:
        PendingChainState(pending_chain_state_ptr pending_chain_state): _pending_chain_state(pending_chain_state){}
        
        pending_chain_state_ptr _pending_chain_state;
    };
    
    Handle<Value> Get_Blance_Record(const Arguments& args)
    {
        Local<Object> self = args.Holder();
        Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
        void* ptr = wrap->Value();
        
        Local<External> wrap_addr = Local<External>::Cast(args[0]);
        
        auto balance_record = static_cast<PendingChainState*>(ptr)->_pending_chain_state->get_balance_record(* static_cast<address*>(wrap_addr->Value()));
        
        return External::New(&balance_record);
    }
    
    Handle<Value> Get_Balance_ID_From_Address(const Arguments& args)
    {
        auto owner = * static_cast<address*> (Local<External>::Cast(args[0])->Value());
        
        int asset_id = args[1]->Int32Value();
        
        auto addr = withdraw_condition( withdraw_with_signature(owner), asset_id ).get_address();
        
        return External::New(&addr);
    }
    
    // binding to blockchain to get block
    Handle<Value> Get_Transactions(const Arguments& args)
    {
        Local<Object> self = args.Holder();
        Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
        void* ptr = wrap->Value();
        //get member variable value
        auto transactions = static_cast<full_block*>(ptr)->user_transactions;
        
        // We will be creating temporary handles so we use a handle scope.
        HandleScope handle_scope;
        
        Local<Array> array = Array::New(transactions.size());
        
        if (array.IsEmpty())
            return Handle<Array>();
        
        // Fill out the values
        int i = 0;
        for ( auto trx : transactions )
        {
            //get class template
            Handle<Function> trx_ctor = transaction_templ->GetFunction();
            
            //get class instance
            Local<Object> obj = trx_ctor->NewInstance();
            // TODO: Is it ok to return address of block here?
            obj->SetInternalField(0, External::New(&trx));
            
            
            array->Set(i, obj);
            
            i ++;
        }
        
        // Return the value through Close.
        return handle_scope.Close(array);
    }
    
    bool ExecuteString(v8::Handle<v8::String> source,v8::Handle<v8::Value> name,bool print_result)
    {
        v8::HandleScope handle_scope;
        v8::TryCatch try_catch;
        v8::Handle<v8::Script> script = v8::Script::Compile(source, name);
        if (script.IsEmpty())
        {
            v8::String::AsciiValue error(try_catch.Exception());
            printf("%s\n", *error);
            return false;
        }
        else
        {
            v8::Handle<v8::Value> result = script->Run();
            if (result.IsEmpty())
            {
                v8::String::AsciiValue error(try_catch.Exception());
                printf("%s\n", *error);
                return false;
            }
            else
            {
                if (print_result && !result->IsUndefined())
                {
                    v8::String::AsciiValue str(result);
                    printf("%s\n", *str);
                }
                return true;
            }
        }
    }
} } // bts::game
