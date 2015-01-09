#pragma once

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
     *  @class v8_blockchain
     *  @brief wrappers blockchain pointer to js object, blockchain
     */
    class v8_blockchain
    {
    public:
        v8_blockchain(chain_database_ptr blockchain, int block_num):_blockchain(blockchain), _block_num(block_num){}
        
        static Local<Object> New(v8::Isolate* isolate, chain_database_ptr blockchain, uint32_t block_num);
        
        static void Get_Block_Number(Local<String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
        
        static void Get_Block(const v8::FunctionCallbackInfo<Value>& args);
        
        static void Get_Current_Random_Seed(const v8::FunctionCallbackInfo<Value>& args);
        
        //variables
        chain_database_ptr _blockchain;
        uint32_t _block_num;
    };
    
    /**
     *  @class V8_PendingState
     *  @brief wrappers pendingstate pointer to js object
     */
    class v8_chainstate
    {
    public:
        v8_chainstate(chain_interface_ptr chain_state): _chain_state(chain_state){}
        
        chain_interface_ptr _chain_state;
        
        static Local<Object> New(v8::Isolate* isolate, const pending_chain_state_ptr& pending_state);
        
        static void Get_Blance_Record(const v8::FunctionCallbackInfo<Value>& args);
        
        static void Get_Asset_Record(const v8::FunctionCallbackInfo<Value>& args);
        
        static void Get_Rule_Data_Record(const v8::FunctionCallbackInfo<Value>& args);
        
        static void Store_Blance_Record(const v8::FunctionCallbackInfo<Value>& args);
        
        static void Store_Asset_Record(const v8::FunctionCallbackInfo<Value>& args);
        
        static void Store_Rule_Data_Record(const v8::FunctionCallbackInfo<Value>& args);
    };
    
    /**
     *  @class V8_PendingState
     *  @brief wrappers pendingstate pointer to js object
     */
    class v8_evalstate
    {
    public:
        v8_evalstate(transaction_evaluation_state_ptr eval_state): _eval_state(eval_state){}
        
        transaction_evaluation_state_ptr _eval_state;
        
        static Local<Object> New(v8::Isolate* isolate, transaction_evaluation_state_ptr eval_state);
        
        /**
         * @brief Method for v8_evalstate
         * @return TODO JS Object
         */
        static void Sub_Balance(const v8::FunctionCallbackInfo<Value>& args);
    };
    /**
     * init the javascript classes
     */
    bool init_class_template(v8::Isolate* isolate);
    
    /**
     * @brief Global method for create balance id for the owner of balance
     *
     */
    void V8_Global_Get_Balance_ID_For_Owner(const v8::FunctionCallbackInfo<Value>& args);
    
    /**
     * @brief Method for getting transactions from full block
     *
     */
    void V8_Block_Get_Transactions(const v8::FunctionCallbackInfo<Value>& args);
    
    // Extracts a C string from a V8 Utf8Value.
    const char* ToCString(const v8::String::Utf8Value& value) {
        return *value ? *value : "<string conversion failed>";
    }
    
    // Creates a new execution environment containing the built-in
    // functions.
    v8::Handle<v8::Context> CreateShellContext(v8::Isolate* isolate);
    
    // The callback that is invoked by v8 whenever the JavaScript 'print'
    // function is called.  Prints its arguments on stdout separated by
    // spaces and ending with a newline.
    void Print(const v8::FunctionCallbackInfo<v8::Value>& args);
    
    
    // The callback that is invoked by v8 whenever the JavaScript 'read'
    // function is called.  This function loads the content of the file named in
    // the argument into a JavaScript string.
    void Read(const v8::FunctionCallbackInfo<v8::Value>& args);
    
    
    // The callback that is invoked by v8 whenever the JavaScript 'load'
    // function is called.  Loads, compiles and executes its argument
    // JavaScript file.
    void Load(const v8::FunctionCallbackInfo<v8::Value>& args);
    
    
    // The callback that is invoked by v8 whenever the JavaScript 'quit'
    // function is called.  Quits.
    void Quit(const v8::FunctionCallbackInfo<v8::Value>& args);
    
    
    void Version(const v8::FunctionCallbackInfo<v8::Value>& args);
    
    // Reads a file into a v8 string.
    v8::Handle<v8::String> ReadFile(v8::Isolate* isolate, const char* name);
    
    bool ExecuteString(v8::Isolate* isolate, v8::Handle<v8::String> source, v8::Handle<v8::String> name, bool print_result, bool report_exceptions);
    
    void ReportException(v8::Isolate* isolate, v8::TryCatch* try_catch);
} } // bts::game
