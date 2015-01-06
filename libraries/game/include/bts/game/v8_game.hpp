#include <bts/blockchain/exceptions.hpp>
#include <bts/game/rule_record.hpp>
#include <bts/blockchain/transaction_evaluation_state.hpp>
#include <bts/blockchain/chain_interface.hpp>
#include <bts/blockchain/game_executors.hpp>

#include <bts/wallet/wallet.hpp>
#include <bts/wallet/wallet_records.hpp>

#include <include/v8.h>
#include <include/libplatform/libplatform.h>

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
        bool init_class_template(v8::Isolate* isolate);
        
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
        int execute( chain_database_ptr blockchain, uint32_t block_num, const pending_chain_state_ptr& pending_state );
        
        
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
        static void V8_Blockchain_Get_Block_Number(Local<String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
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
            info.GetReturnValue().Set( Integer::New(info.GetIsolate(), value) );
        }
        
        /**
         * @brief Method for V8_Blockchain
         *
         */
        static void V8_Blockchain_Get_Block(const v8::FunctionCallbackInfo<Value>& args)
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
            obj->SetInternalField(0, External::New(args.GetIsolate(), &block));
            
            args.GetReturnValue().Set( obj );
        }
        
        /**
         * @brief Method for V8_Blockchain
         *
         */
        static void V8_Blockchain_Get_Current_Random_Seed(const v8::FunctionCallbackInfo<Value>& args)
        {
            Local<Object> self = args.Holder();
            Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
            void* ptr = wrap->Value();
            //get member variable value
            uint32_t value = static_cast<V8_Blockchain*>(ptr)->_blockchain->get_current_random_seed()._hash[0];
            //return the value
            args.GetReturnValue().Set( Integer::New(args.GetIsolate(), value) );
        }
        
        /**
         * @brief Method for V8_ChainState
         *
         */
        static void V8_Chain_State_Get_Blance_Record(const v8::FunctionCallbackInfo<Value>& args)
        {
            Local<Object> self = args.Holder();
            Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
            void* ptr = wrap->Value();
            
            Local<External> wrap_addr = Local<External>::Cast(args[0]);
            
            auto balance_record = static_cast<V8_ChainState*>(ptr)->_chain_state->get_balance_record(* static_cast<address*>(wrap_addr->Value()));
            
            args.GetReturnValue().Set( External::New(args.GetIsolate(), &balance_record) );
        }
        
        /**
         * @brief Method for V8_ChainState
         * @return TODO JS Object
         */
        static void V8_Chain_State_Get_Asset_Record(const v8::FunctionCallbackInfo<Value>& args)
        {
            Local<Object> self = args.Holder();
            Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
            void* ptr = wrap->Value();
            
            Local<Integer> wrapper_asset_id = Local<Integer>::Cast(args[0]);
            
            auto asset_record = static_cast<V8_ChainState*>(ptr)->_chain_state->get_asset_record(wrapper_asset_id->Int32Value());
            
            args.GetReturnValue().Set( External::New(args.GetIsolate(), &asset_record) );
        }
        
        /**
         * @brief Method for V8_ChainState
         * @return TODO JS Object
         */
        static void V8_Chain_State_Get_Rule_Data_Record(const v8::FunctionCallbackInfo<Value>& args)
        {
            Local<Object> self = args.Holder();
            Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
            void* ptr = wrap->Value();
            
            Local<Integer> wrapper_type = Local<Integer>::Cast(args[0]);
            Local<Integer> wrapper_id = Local<Integer>::Cast(args[1]);
            
            auto rule_data_record = static_cast<V8_ChainState*>(ptr)->_chain_state->get_rule_data_record(wrapper_type->Int32Value(), wrapper_id->Int32Value() );
            
            args.GetReturnValue().Set( External::New(args.GetIsolate(), &rule_data_record) );
        }
        
        /**
         * @brief Method for V8_ChainState
         * @return undefine
         */
        static void V8_Chain_State_Store_Blance_Record(const v8::FunctionCallbackInfo<Value>& args)
        {
            Local<Object> self = args.Holder();
            Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
            void* ptr = wrap->Value();
            
            Local<External> wrap_addr = Local<External>::Cast(args[0]);
            
            // TODO: parse json to C++ struct, from variant
            static_cast<V8_ChainState*>(ptr)->_chain_state->store_balance_record(* static_cast<blockchain::balance_record*>(wrap_addr->Value()));
        }
        
        /**
         * @brief Method for V8_ChainState
         * @return TODO JS Object
         */
        static void V8_Chain_State_Store_Asset_Record(const v8::FunctionCallbackInfo<Value>& args)
        {
            Local<Object> self = args.Holder();
            Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
            void* ptr = wrap->Value();
            
            Local<External> wrapper_asset = Local<External>::Cast(args[0]);
            
            // TODO: parse json to C++ struct, from variant
            static_cast<V8_ChainState*>(ptr)->_chain_state->store_asset_record(* static_cast<blockchain::asset_record*>(wrapper_asset->Value()));
        }
        
        /**
         * @brief Method for V8_ChainState
         * @return undefine
         */
        static void V8_Chain_State_Store_Rule_Data_Record(const v8::FunctionCallbackInfo<Value>& args)
        {
            Local<Object> self = args.Holder();
            Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
            void* ptr = wrap->Value();
            
            Local<Integer> wrapper_type = Local<Integer>::Cast(args[0]);
            Local<Integer> wrapper_id = Local<Integer>::Cast(args[1]);
            Local<External> wrap_rule_data = Local<External>::Cast(args[2]);
            
            // TODO: parse json to C++ struct, from variant
            static_cast<V8_ChainState*>(ptr)->_chain_state->store_rule_data_record(wrapper_type->Int32Value(), wrapper_id->Int32Value(), * static_cast<blockchain::rule_data_record*>(wrap_rule_data->Value()) );
        }
        
        /**
         * @brief Method for V8_EvalState
         * @return TODO JS Object
         */
        static void V8_Eval_State_Sub_Balance(const v8::FunctionCallbackInfo<Value>& args)
        {
            Local<Object> self = args.Holder();
            Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
            void* ptr = wrap->Value();
            
            Local<External> wrapper_balance_id = Local<External>::Cast(args[0]);
            Local<External> wrap_asset = Local<External>::Cast(args[1]);
            
            // TODO: parse json to C++ struct, from variant
            static_cast<V8_EvalState*>(ptr)->_eval_state->sub_balance( * static_cast<address*>(wrapper_balance_id->Value()), * static_cast<asset*>(wrap_asset->Value() ) );
        }
        
        /**
         * @brief Global method for create balance id for the owner of balance
         *
         */
        static void V8_Global_Get_Balance_ID_For_Owner(const v8::FunctionCallbackInfo<Value>& args)
        {
            auto owner = * static_cast<address*> (Local<External>::Cast(args[0])->Value());
            
            int asset_id = args[1]->Int32Value();
            
            auto addr = withdraw_condition( withdraw_with_signature(owner), asset_id ).get_address();
            
            args.GetReturnValue().Set( External::New(args.GetIsolate(), &addr) );
        }
        
        /**
         * @brief Method for getting transactions from full block
         *
         */
        static void V8_Block_Get_Transactions(const v8::FunctionCallbackInfo<Value>& args)
        {
            Local<Object> self = args.Holder();
            Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
            void* ptr = wrap->Value();
            //get member variable value
            auto transactions = static_cast<full_block*>(ptr)->user_transactions;
            
            // We will be creating temporary handles so we use a handle scope.
            HandleScope handle_scope(args.GetIsolate());
            
            Local<Array> array = Array::New(args.GetIsolate(), transactions.size());
            
            if (array.IsEmpty())
                args.GetReturnValue().Set( Handle<Array>() );
                return;
            
            // Fill out the values
            int i = 0;
            for ( auto trx : transactions )
            {
                //get class template
                Handle<Function> trx_ctor = v8_game_engine::instance().transaction_templ->GetFunction();
                
                //get class instance
                Local<Object> obj = trx_ctor->NewInstance();
                // TODO: Is it ok to return address of block here?
                obj->SetInternalField(0, External::New(args.GetIsolate(), &trx));
                
                
                array->Set(i, obj);
                
                i ++;
            }
            
            // TODO: Return the value through Close. handle_scope.Close(array ) ?
            args.GetReturnValue().Set( array );
        }
        
        // Extracts a C string from a V8 Utf8Value.
        static const char* ToCString(const v8::String::Utf8Value& value) {
            return *value ? *value : "<string conversion failed>";
        }
        
        // Creates a new execution environment containing the built-in
        // functions.
        static v8::Handle<v8::Context> CreateShellContext(v8::Isolate* isolate) {
            // Create a template for the global object.
            v8::Handle<v8::ObjectTemplate> global = v8::ObjectTemplate::New(isolate);
            // Bind the global 'print' function to the C++ Print callback.
            global->Set(v8::String::NewFromUtf8(isolate, "print"),
                        v8::FunctionTemplate::New(isolate, Print));
            // Bind the global 'read' function to the C++ Read callback.
            global->Set(v8::String::NewFromUtf8(isolate, "read"),
                        v8::FunctionTemplate::New(isolate, Read));
            // Bind the global 'load' function to the C++ Load callback.
            global->Set(v8::String::NewFromUtf8(isolate, "load"),
                        v8::FunctionTemplate::New(isolate, Load));
            // Bind the 'quit' function
            global->Set(v8::String::NewFromUtf8(isolate, "quit"),
                        v8::FunctionTemplate::New(isolate, Quit));
            // Bind the 'version' function
            global->Set(v8::String::NewFromUtf8(isolate, "version"),
                        v8::FunctionTemplate::New(isolate, Version));
            
            return v8::Context::New(isolate, NULL, global);
        }
        
        // The callback that is invoked by v8 whenever the JavaScript 'print'
        // function is called.  Prints its arguments on stdout separated by
        // spaces and ending with a newline.
        static void Print(const v8::FunctionCallbackInfo<v8::Value>& args) {
            bool first = true;
            for (int i = 0; i < args.Length(); i++) {
                v8::HandleScope handle_scope(args.GetIsolate());
                if (first) {
                    first = false;
                } else {
                    printf(" ");
                }
                v8::String::Utf8Value str(args[i]);
                const char* cstr = ToCString(str);
                printf("%s", cstr);
            }
            printf("\n");
            fflush(stdout);
        }
        
        
        // The callback that is invoked by v8 whenever the JavaScript 'read'
        // function is called.  This function loads the content of the file named in
        // the argument into a JavaScript string.
        static void Read(const v8::FunctionCallbackInfo<v8::Value>& args) {
            if (args.Length() != 1) {
                args.GetIsolate()->ThrowException(
                                                  v8::String::NewFromUtf8(args.GetIsolate(), "Bad parameters"));
                return;
            }
            v8::String::Utf8Value file(args[0]);
            if (*file == NULL) {
                args.GetIsolate()->ThrowException(
                                                  v8::String::NewFromUtf8(args.GetIsolate(), "Error loading file"));
                return;
            }
            v8::Handle<v8::String> source = ReadFile(args.GetIsolate(), *file);
            if (source.IsEmpty()) {
                args.GetIsolate()->ThrowException(
                                                  v8::String::NewFromUtf8(args.GetIsolate(), "Error loading file"));
                return;
            }
            args.GetReturnValue().Set(source);
        }
        
        
        // The callback that is invoked by v8 whenever the JavaScript 'load'
        // function is called.  Loads, compiles and executes its argument
        // JavaScript file.
        static void Load(const v8::FunctionCallbackInfo<v8::Value>& args) {
            for (int i = 0; i < args.Length(); i++) {
                v8::HandleScope handle_scope(args.GetIsolate());
                v8::String::Utf8Value file(args[i]);
                if (*file == NULL) {
                    args.GetIsolate()->ThrowException(
                                                      v8::String::NewFromUtf8(args.GetIsolate(), "Error loading file"));
                    return;
                }
                v8::Handle<v8::String> source = ReadFile(args.GetIsolate(), *file);
                if (source.IsEmpty()) {
                    args.GetIsolate()->ThrowException(
                                                      v8::String::NewFromUtf8(args.GetIsolate(), "Error loading file"));
                    return;
                }
                if (!ExecuteString(args.GetIsolate(),
                                   source,
                                   v8::String::NewFromUtf8(args.GetIsolate(), *file),
                                   false,
                                   false)) {
                    args.GetIsolate()->ThrowException(
                                                      v8::String::NewFromUtf8(args.GetIsolate(), "Error executing file"));
                    return;
                }
            }
        }
        
        
        // The callback that is invoked by v8 whenever the JavaScript 'quit'
        // function is called.  Quits.
        static void Quit(const v8::FunctionCallbackInfo<v8::Value>& args) {
            // If not arguments are given args[0] will yield undefined which
            // converts to the integer value 0.
            int exit_code = args[0]->Int32Value();
            fflush(stdout);
            fflush(stderr);
            exit(exit_code);
        }
        
        
        static void Version(const v8::FunctionCallbackInfo<v8::Value>& args) {
            args.GetReturnValue().Set(
                                      v8::String::NewFromUtf8(args.GetIsolate(), v8::V8::GetVersion()));
        }
        
        // Reads a file into a v8 string.
        static v8::Handle<v8::String> ReadFile(v8::Isolate* isolate, const char* name) {
            FILE* file = fopen(name, "rb");
            if (file == NULL) return v8::Handle<v8::String>();
            
            fseek(file, 0, SEEK_END);
            int size = ftell(file);
            rewind(file);
            
            char* chars = new char[size + 1];
            chars[size] = '\0';
            for (int i = 0; i < size;) {
                int read = static_cast<int>(fread(&chars[i], 1, size - i, file));
                i += read;
            }
            fclose(file);
            v8::Handle<v8::String> result =
            v8::String::NewFromUtf8(isolate, chars, v8::String::kNormalString, size);
            delete[] chars;
            return result;
        }
        
        static bool ExecuteString(v8::Isolate* isolate, v8::Handle<v8::String> source, v8::Handle<v8::String> name, bool print_result, bool report_exceptions)
        {
            v8::HandleScope handle_scope(isolate);
            v8::TryCatch try_catch;
            v8::Handle<v8::Script> script = v8::Script::Compile(source, name);
            if (script.IsEmpty())
            {
                // print compile error
                if (report_exceptions)
                    ReportException(isolate, &try_catch);
                return false;
            }
            else
            {
                v8::Handle<v8::Value> result = script->Run();
                if (result.IsEmpty())
                {
                    assert(try_catch.HasCaught());
                    // print errors during run
                    if (report_exceptions)
                        ReportException(isolate, &try_catch);
                    return false;
                }
                else
                {
                    assert(!try_catch.HasCaught());
                    if (print_result && !result->IsUndefined()) {
                        // If no errors and the result is not undefined, then print the return value
                        v8::String::Utf8Value str(result);
                        const char* cstr = ToCString(str);
                        printf("%s\n", cstr);
                    }
                    return true;
                }
            }
        }
        
        static void ReportException(v8::Isolate* isolate, v8::TryCatch* try_catch) {
            v8::HandleScope handle_scope(isolate);
            v8::String::Utf8Value exception(try_catch->Exception());
            const char* exception_string = ToCString(exception);
            v8::Handle<v8::Message> message = try_catch->Message();
            if (message.IsEmpty()) {
                // V8 didn't provide any extra information about this error; just
                // print the exception.
                fprintf(stderr, "%s\n", exception_string);
            } else {
                // Print (filename):(line number): (message).
                v8::String::Utf8Value filename(message->GetScriptOrigin().ResourceName());
                const char* filename_string = ToCString(filename);
                int linenum = message->GetLineNumber();
                fprintf(stderr, "%s:%i: %s\n", filename_string, linenum, exception_string);
                // Print line of source code.
                v8::String::Utf8Value sourceline(message->GetSourceLine());
                const char* sourceline_string = ToCString(sourceline);
                fprintf(stderr, "%s\n", sourceline_string);
                // Print wavy underline (GetUnderline is deprecated).
                int start = message->GetStartColumn();
                for (int i = 0; i < start; i++) {
                    fprintf(stderr, " ");
                }
                int end = message->GetEndColumn();
                for (int i = start; i < end; i++) {
                    fprintf(stderr, "^");
                }
                fprintf(stderr, "\n");
                v8::String::Utf8Value stack_trace(try_catch->StackTrace());
                if (stack_trace.length() > 0) {
                    const char* stack_trace_string = ToCString(stack_trace);
                    fprintf(stderr, "%s\n", stack_trace_string);
                }
            }
        }
    };
} } // bts::game
