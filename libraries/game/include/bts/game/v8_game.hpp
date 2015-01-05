#include <bts/blockchain/exceptions.hpp>
#include <bts/game/rule_record.hpp>
#include <bts/blockchain/transaction_evaluation_state.hpp>
#include <bts/blockchain/chain_interface.hpp>
#include <bts/blockchain/game_executors.hpp>

#include <bts/wallet/wallet.hpp>
#include <bts/wallet/wallet_records.hpp>

#include <v8.h>

namespace bts { namespace game {
    
    using namespace v8;
    using namespace bts::blockchain;
    using namespace bts::wallet;
    
    /**
     * @class v8_game_engine
     *
     *  script context for javascript running
     */
    class v8_game_engine
    {
    public:
        static v8_game_engine& instance();
        
        /**
         * init the javascript classes
         */
        bool init_class_template();
        
        void evaluate( transaction_evaluation_state& eval_state);
        
        wallet_transaction_record play( chain_database_ptr blockchain, bts::wallet::wallet_ptr w, const variant& var, bool sign );
        
        bool scan( wallet_transaction_record& trx_rec, bts::wallet::wallet_ptr w );
        
        bool scan_result( const rule_result_transaction& rtrx,
                                 uint32_t block_num,
                                 const time_point_sec& block_time,
                                 const time_point_sec& received_time,
                                 const uint32_t trx_index, bts::wallet::wallet_ptr w);
        
        /**
         * wrapper to call the javascript stub defined by game developers
         */
        void execute( chain_database_ptr blockchain, uint32_t block_num, const pending_chain_state_ptr& pending_state );
        
        
        v8::Handle<v8::ObjectTemplate> global;
        
        Handle<FunctionTemplate> blockchain_templ;
        
        Handle<FunctionTemplate> pendingstate_templ;
        
        Handle<FunctionTemplate> eval_state_templ;
        
        Handle<FunctionTemplate> block_templ;
        
        Handle<FunctionTemplate> transaction_templ;
        
        /**
         *  @class V8_Blockchain
         *  @brief wrappers blockchain pointer to js object, blockchain
         */
        class V8_Blockchain
        {
        public:
            //constructor
            V8_Blockchain(chain_database_ptr blockchain, int block_num):_blockchain(blockchain), _block_num(block_num){}
            
            //variables
            chain_database_ptr _blockchain;
            uint32_t _block_num;
        };
        
        /**
         *  @class V8_PendingState
         *  @brief wrappers pendingstate pointer to js object
         */
        class V8_ChainState
        {
        public:
            V8_ChainState(chain_interface_ptr chain_state): _chain_state(chain_state){}
            
            chain_interface_ptr _chain_state;
        };
        
        /**
         *  @class V8_PendingState
         *  @brief wrappers pendingstate pointer to js object
         */
        class V8_EvalState
        {
        public:
            V8_EvalState(transaction_evaluation_state_ptr eval_state): _eval_state(eval_state){}
            
            transaction_evaluation_state_ptr _eval_state;
        };
        
        /**
         *  @method V8_Get_Block_Number
         *  @brief Getter method for V8_Blockchain
         */
        static Handle<Value> V8_Blockchain_Get_Block_Number(Local<String> property, const AccessorInfo& info)
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
            uint32_t value = static_cast<V8_Blockchain*>(ptr)->_block_num;
            //return the value
            return Integer::New(value);
        }
        
        /**
         * @brief Method for V8_Blockchain
         *
         */
        static Handle<Value> V8_Blockchain_Get_Block(const Arguments& args)
        {
            Local<Object> self = args.Holder();
            Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
            void* ptr = wrap->Value();
            //get member variable value
            auto block = static_cast<V8_Blockchain*>(ptr)->_blockchain->get_block(args[0]->Uint32Value());
            //return the value
            
            //get class template
            Handle<Function> block_ctor = v8_game_engine::instance().block_templ->GetFunction();
            
            //get class instance
            Local<Object> obj = block_ctor->NewInstance();
            
            //build the "bridge" between c++ and javascript by associating the 'p' pointer to the first internal
            //field of the object
            // TODO: Is it ok to return address of block here?
            obj->SetInternalField(0, External::New(&block));
            
            return obj;
        }
        
        /**
         * @brief Method for V8_Blockchain
         *
         */
        static Handle<Value> V8_Blockchain_Get_Current_Random_Seed(const Arguments& args)
        {
            Local<Object> self = args.Holder();
            Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
            void* ptr = wrap->Value();
            //get member variable value
            uint32_t value = static_cast<V8_Blockchain*>(ptr)->_blockchain->get_current_random_seed()._hash[0];
            //return the value
            return Integer::New(value);
        }
        
        /**
         * @brief Method for V8_ChainState
         *
         */
        static Handle<Value> V8_Chain_State_Get_Blance_Record(const Arguments& args)
        {
            Local<Object> self = args.Holder();
            Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
            void* ptr = wrap->Value();
            
            Local<External> wrap_addr = Local<External>::Cast(args[0]);
            
            auto balance_record = static_cast<V8_ChainState*>(ptr)->_chain_state->get_balance_record(* static_cast<address*>(wrap_addr->Value()));
            
            return External::New(&balance_record);
        }
        
        /**
         * @brief Method for V8_ChainState
         * @return TODO JS Object
         */
        static Handle<Value> V8_Chain_State_Get_Asset_Record(const Arguments& args)
        {
            Local<Object> self = args.Holder();
            Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
            void* ptr = wrap->Value();
            
            Local<Integer> wrapper_asset_id = Local<Integer>::Cast(args[0]);
            
            auto asset_record = static_cast<V8_ChainState*>(ptr)->_chain_state->get_asset_record(wrapper_asset_id->Int32Value());
            
            return External::New(&asset_record);
        }
        
        /**
         * @brief Method for V8_ChainState
         * @return TODO JS Object
         */
        static Handle<Value> V8_Chain_State_Get_Rule_Data_Record(const Arguments& args)
        {
            Local<Object> self = args.Holder();
            Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
            void* ptr = wrap->Value();
            
            Local<Integer> wrapper_type = Local<Integer>::Cast(args[0]);
            Local<Integer> wrapper_id = Local<Integer>::Cast(args[1]);
            
            auto rule_data_record = static_cast<V8_ChainState*>(ptr)->_chain_state->get_rule_data_record(wrapper_type->Int32Value(), wrapper_id->Int32Value() );
            
            return External::New(&rule_data_record);
        }
        
        /**
         * @brief Method for V8_ChainState
         * @return undefine
         */
        static Handle<Value> V8_Chain_State_Store_Blance_Record(const Arguments& args)
        {
            Local<Object> self = args.Holder();
            Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
            void* ptr = wrap->Value();
            
            Local<External> wrap_addr = Local<External>::Cast(args[0]);
            
            // TODO: parse json to C++ struct, from variant
            static_cast<V8_ChainState*>(ptr)->_chain_state->store_balance_record(* static_cast<blockchain::balance_record*>(wrap_addr->Value()));
            
            return Undefined();
        }
        
        /**
         * @brief Method for V8_ChainState
         * @return TODO JS Object
         */
        static Handle<Value> V8_Chain_State_Store_Asset_Record(const Arguments& args)
        {
            Local<Object> self = args.Holder();
            Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
            void* ptr = wrap->Value();
            
            Local<External> wrapper_asset = Local<External>::Cast(args[0]);
            
            // TODO: parse json to C++ struct, from variant
            static_cast<V8_ChainState*>(ptr)->_chain_state->store_asset_record(* static_cast<blockchain::asset_record*>(wrapper_asset->Value()));
            
            return Undefined();
        }
        
        /**
         * @brief Method for V8_ChainState
         * @return undefine
         */
        static Handle<Value> V8_Chain_State_Store_Rule_Data_Record(const Arguments& args)
        {
            Local<Object> self = args.Holder();
            Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
            void* ptr = wrap->Value();
            
            Local<Integer> wrapper_type = Local<Integer>::Cast(args[0]);
            Local<Integer> wrapper_id = Local<Integer>::Cast(args[1]);
            Local<External> wrap_rule_data = Local<External>::Cast(args[2]);
            
            // TODO: parse json to C++ struct, from variant
            static_cast<V8_ChainState*>(ptr)->_chain_state->store_rule_data_record(wrapper_type->Int32Value(), wrapper_id->Int32Value(), * static_cast<blockchain::rule_data_record*>(wrap_rule_data->Value()) );
            
            
            return Undefined();
        }
        
        /**
         * @brief Method for V8_EvalState
         * @return TODO JS Object
         */
        static Handle<Value> V8_Eval_State_Sub_Balance(const Arguments& args)
        {
            Local<Object> self = args.Holder();
            Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
            void* ptr = wrap->Value();
            
            Local<External> wrapper_balance_id = Local<External>::Cast(args[0]);
            Local<External> wrap_asset = Local<External>::Cast(args[1]);
            
            // TODO: parse json to C++ struct, from variant
            static_cast<V8_EvalState*>(ptr)->_eval_state->sub_balance( * static_cast<address*>(wrapper_balance_id->Value()), * static_cast<asset*>(wrap_asset->Value() ) );
            
            return Undefined();
        }
        
        /**
         * @brief Global method for create balance id for the owner of balance
         *
         */
        static Handle<Value> V8_Global_Get_Balance_ID_For_Owner(const Arguments& args)
        {
            auto owner = * static_cast<address*> (Local<External>::Cast(args[0])->Value());
            
            int asset_id = args[1]->Int32Value();
            
            auto addr = withdraw_condition( withdraw_with_signature(owner), asset_id ).get_address();
            
            return External::New(&addr);
        }
        
        /**
         * @brief Method for getting transactions from full block
         *
         */
        static Handle<Value> V8_Block_Get_Transactions(const Arguments& args)
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
                Handle<Function> trx_ctor = v8_game_engine::instance().transaction_templ->GetFunction();
                
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
        
        static bool ExecuteString(v8::Handle<v8::String> source,v8::Handle<v8::Value> name,bool print_result)
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
    };
} } // bts::game
