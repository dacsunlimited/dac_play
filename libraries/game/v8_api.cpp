#include <bts/game/v8_api.hpp>

namespace bts { namespace game {
   Persistent<ObjectTemplate> v8_api::global;
   
   Persistent<FunctionTemplate> v8_api::blockchain_templ;
    
   Persistent<FunctionTemplate> v8_api::wallet_templ;
   
   Persistent<FunctionTemplate> v8_api::block_templ;
   
   Persistent<FunctionTemplate> v8_api::pendingstate_templ;
   
   Persistent<FunctionTemplate> v8_api::eval_state_templ;
   
   Persistent<FunctionTemplate> v8_api::transaction_templ;
   
   Handle<FunctionTemplate> MakeBlockChainTemplate( Isolate* isolate) {
      EscapableHandleScope handle_scope(isolate);
      
      Local<FunctionTemplate> result = FunctionTemplate::New(isolate);
      
      //assign the "BlockchainContext" name to the new class template
      result->SetClassName(String::NewFromUtf8(isolate, "Blockchain"));
      
      //access the class template
      Handle<ObjectTemplate> proto = result->PrototypeTemplate();
      
      //associates the "method" string to the callback PointMethod in the class template
      //enabling point.method_a() constructions inside the javascript
      proto->Set(isolate, "get_current_random_seed", FunctionTemplate::New(isolate, v8_blockchain::Get_Current_Random_Seed));
      
      //access the instance pointer of our new class template
      Handle<ObjectTemplate> inst = result->InstanceTemplate();
      
      //set the internal fields of the class as we have the Point class internally
      inst->SetInternalFieldCount(1);
      
      //associates the name "x" with its Get/Set functions
      inst->SetAccessor(String::NewFromUtf8(isolate, "block_num"), v8_blockchain::Get_Block_Number);
      
      // Again, return the result through the current handle scope.
      return handle_scope.Escape(result);
   }
    
    Handle<FunctionTemplate> MakeWalletTemplate( Isolate* isolate) {
        EscapableHandleScope handle_scope(isolate);
        
        Local<FunctionTemplate> result = FunctionTemplate::New(isolate);
        
        //assign the "BlockchainContext" name to the new class template
        result->SetClassName(String::NewFromUtf8(isolate, "Wallet"));
        
        //access the class template
        Handle<ObjectTemplate> proto = result->PrototypeTemplate();
        
        Handle<ObjectTemplate> inst = result->InstanceTemplate();
        
        inst->SetInternalFieldCount(1);
        
        //associates the "method" string to the callback PointMethod in the class template
        //enabling point.method_a() constructions inside the javascript
        proto->Set(isolate, "get_transaction_fee", FunctionTemplate::New(isolate, v8_wallet::Get_Transaction_Fee));
        
        // Again, return the result through the current handle scope.
        return handle_scope.Escape(result);
    }
   
   Handle<FunctionTemplate> MakeBlockTemplate( Isolate* isolate) {
      EscapableHandleScope handle_scope(isolate);
      
      Local<FunctionTemplate> result = FunctionTemplate::New(isolate);
      result->SetClassName(String::NewFromUtf8(isolate, "Block"));
      
      //access the class template
      Handle<ObjectTemplate> block_proto = result->PrototypeTemplate();
      block_proto->Set(isolate, "get_transactions", FunctionTemplate::New(isolate, v8_api::V8_Block_Get_Transactions));
      
      // Again, return the result through the current handle scope.
      return handle_scope.Escape(result);
   }
   
   Handle<FunctionTemplate> MakeChainStateTemplate( Isolate* isolate) {
      EscapableHandleScope handle_scope(isolate);
      
      Local<FunctionTemplate> result = FunctionTemplate::New(isolate);
      result->SetClassName(String::NewFromUtf8(isolate, "PendingState"));
      
      //access the class template
      Handle<ObjectTemplate> pendingstate_proto = result->PrototypeTemplate();
      pendingstate_proto->Set(isolate, "get_balance_record", FunctionTemplate::New(isolate, v8_chainstate::Get_Blance_Record));
      pendingstate_proto->Set(isolate, "get_asset_record", FunctionTemplate::New(isolate, v8_chainstate::Get_Asset_Record));
      pendingstate_proto->Set(isolate, "get_game_data_record", FunctionTemplate::New(isolate, v8_chainstate::Get_Game_Data_Record));
      
      pendingstate_proto->Set(isolate, "set_balance_record", FunctionTemplate::New(isolate, v8_chainstate::Store_Blance_Record));
      pendingstate_proto->Set(isolate, "set_asset_record", FunctionTemplate::New(isolate, v8_chainstate::Store_Asset_Record));
      pendingstate_proto->Set(isolate, "set_game_data_record", FunctionTemplate::New(isolate, v8_chainstate::Store_Game_Data_Record));
      
       
       //access the instance pointer of our new class template
       Handle<ObjectTemplate> inst = result->InstanceTemplate();
       
       //set the internal fields of the class as we have the Point class internally
       inst->SetInternalFieldCount(1);
       
       //associates the name "x" with its Get/Set functions
       //inst->SetAccessor();
       
      // Again, return the result through the current handle scope.
      return handle_scope.Escape(result);
   }
   
   Handle<FunctionTemplate> MakeEvalStateTemplate( Isolate* isolate) {
      EscapableHandleScope handle_scope(isolate);
      
      Local<FunctionTemplate> result = FunctionTemplate::New(isolate);
      result->SetClassName(String::NewFromUtf8(isolate, "EvalState"));
      
      Handle<ObjectTemplate> eval_state_proto = result->PrototypeTemplate();
      
      eval_state_proto->Set(isolate, "sub_balance", FunctionTemplate::New(isolate, v8_evalstate::Sub_Balance));
      
      // Again, return the result through the current handle scope.
      return handle_scope.Escape(result);
   }
   
   bool v8_api::init_class_template(v8::Isolate* isolate)
   {
      HandleScope handle_scope(isolate);
      if ( blockchain_templ.IsEmpty() )
      {
         Handle<FunctionTemplate> raw_template = MakeBlockChainTemplate(isolate);
         blockchain_templ.Reset(isolate, raw_template);
      }
       
       if ( wallet_templ.IsEmpty() )
       {
           Handle<FunctionTemplate> raw_template = MakeWalletTemplate(isolate);
           wallet_templ.Reset(isolate, raw_template);
       }
      
      if ( block_templ.IsEmpty() )
      {
         Handle<FunctionTemplate> raw_template = MakeBlockTemplate(isolate);
         block_templ.Reset(isolate, raw_template);
      }

      if ( pendingstate_templ.IsEmpty() )
      {
         Handle<FunctionTemplate> raw_template = MakeChainStateTemplate(isolate);
         pendingstate_templ.Reset(isolate, raw_template);
      }
      
      if ( eval_state_templ.IsEmpty() )
      {
         Handle<FunctionTemplate> raw_template = MakeEvalStateTemplate(isolate);
         eval_state_templ.Reset(isolate, raw_template);
      }
      
      return true;
   }
   
   /**
    * @brief Global method for create balance id for the owner of balance
    *
    */
   void v8_api::V8_Global_Get_Balance_ID_For_Owner(const v8::FunctionCallbackInfo<Value>& args)
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
   void v8_api::V8_Block_Get_Transactions(const v8::FunctionCallbackInfo<Value>& args)
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
         Handle<FunctionTemplate> templ = Local<FunctionTemplate>::New(args.GetIsolate(), transaction_templ);
         Handle<Function> trx_ctor = templ->GetFunction();
         
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
   
   Local<Object> v8_blockchain::New(v8::Isolate* isolate, chain_database_ptr blockchain, uint32_t block_num)
   {
      EscapableHandleScope handle_scope(isolate);
      // FIXME TODO: Delete this.
      v8_blockchain* local_v8_blockchain = new v8_blockchain(blockchain, block_num);
      //get class template
      Handle<FunctionTemplate> templ = Local<FunctionTemplate>::New(isolate, v8_api::blockchain_templ);
      Handle<Function> blockchain_ctor = templ->GetFunction();
      
      //get class instance
      Local<Object> g_blockchain = blockchain_ctor->NewInstance();
      
      //build the "bridge" between c++ and javascript by associating the 'p' pointer to the first internal
      //field of the object
      g_blockchain->SetInternalField(0, External::New(isolate, local_v8_blockchain));
      
      // delete v8_blockchain;
      
      return handle_scope.Escape(g_blockchain);
   }
   
   void v8_blockchain::Get_Block_Number(Local<String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
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
      uint32_t value = static_cast<v8_blockchain*>(ptr)->_block_num;
      //return the value
      info.GetReturnValue().Set( Integer::New(info.GetIsolate(), value) );
   }
   
   void v8_blockchain::Get_Block(const v8::FunctionCallbackInfo<Value>& args)
   {
      Local<Object> self = args.Holder();
      Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
      void* ptr = wrap->Value();
      //get member variable value
      auto block = static_cast<v8_blockchain*>(ptr)->_blockchain->get_block(args[0]->Uint32Value());
      //return the value
      
      //get class template
      EscapableHandleScope handle_scope(args.GetIsolate());
      
      Handle<FunctionTemplate> templ = Local<FunctionTemplate>::New(args.GetIsolate(), v8_api::block_templ);
      Handle<Function> block_ctor = templ->GetFunction();
      
      //get class instance
      Local<Object> obj = block_ctor->NewInstance();
      
      //build the "bridge" between c++ and javascript by associating the 'p' pointer to the first internal
      //field of the object
      // TODO: Is it ok to return address of block here?
      obj->SetInternalField(0, External::New(args.GetIsolate(), &block));
      
      args.GetReturnValue().Set( handle_scope.Escape(obj) );
   }
   
   void v8_blockchain::Get_Current_Random_Seed(const v8::FunctionCallbackInfo<Value>& args)
   {
      Local<Object> self = args.Holder();
      Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
      void* ptr = wrap->Value();
      //get member variable value
      uint32_t value = static_cast<v8_blockchain*>(ptr)->_blockchain->get_current_random_seed()._hash[0];
      //return the value
      args.GetReturnValue().Set( Integer::New(args.GetIsolate(), value) );
   }
    
    Local<Object> v8_wallet::New(v8::Isolate* isolate, wallet_ptr wallet)
    {
        EscapableHandleScope handle_scope(isolate);
        // FIXME TODO: Delete this.
        v8_wallet* local_v8_wallet = new v8_wallet(wallet);
        //get class template
        Handle<FunctionTemplate> templ = Local<FunctionTemplate>::New(isolate, v8_api::wallet_templ);
        Handle<Function> wallet_ctor = templ->GetFunction();
        
        //get class instance
        Local<Object> g_wallet = wallet_ctor->NewInstance();
        
        //build the "bridge" between c++ and javascript by associating the 'p' pointer to the first internal
        //field of the object
        g_wallet->SetInternalField(0, External::New(isolate, local_v8_wallet));
        
        // delete v8_blockchain;
        
        return handle_scope.Escape(g_wallet);
    }
    
    void v8_wallet::Get_Transaction_Fee( const v8::FunctionCallbackInfo<Value> &args )
    {
        Local<Object> self = args.Holder();
        Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
        void* ptr = wrap->Value();
        //get member variable value
        auto value = static_cast<v8_wallet*>(ptr)->_wallet->get_transaction_fee();
        v8::Isolate* isolate = v8::Isolate::GetCurrent(); // TODO: parse isolate in?
        
        //return the value
        args.GetReturnValue().Set( v8_helper::cpp_to_json(isolate, value) );
    }
   
   Local<Object> v8_chainstate::New(v8::Isolate* isolate, const pending_chain_state_ptr& pending_state)
   {
       
      EscapableHandleScope handle_scope(isolate);
      
      v8_chainstate* v8_pendingstate = new v8_chainstate(pending_state);
      
      Handle<FunctionTemplate> templ = Local<FunctionTemplate>::New(isolate, v8_api::pendingstate_templ);
      
      Handle<Function> pendingstate_ctor = templ->GetFunction();
      Local<Object> g_pendingstate = pendingstate_ctor->NewInstance();
      g_pendingstate->SetInternalField(0, External::New(isolate, v8_pendingstate));
      
      // TODO: Fixme
      //delete v8_pendingstate;
      
      return handle_scope.Escape(g_pendingstate);
   }
   
   void v8_chainstate::Get_Blance_Record(const v8::FunctionCallbackInfo<Value>& args)
   {
      Local<Object> self = args.Holder();
      Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
      void* ptr = wrap->Value();
      
      Local<External> wrap_addr = Local<External>::Cast(args[0]);
      
      auto balance_record = static_cast<v8_chainstate*>(ptr)->_chain_state->get_balance_record(* static_cast<address*>(wrap_addr->Value()));
      
      args.GetReturnValue().Set( External::New(args.GetIsolate(), &balance_record) );
   }
   
   void v8_chainstate::Get_Asset_Record(const v8::FunctionCallbackInfo<Value>& args)
   {
      Local<Object> self = args.Holder();
      Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
      void* ptr = wrap->Value();
      
      Local<Integer> wrapper_asset_id = Local<Integer>::Cast(args[0]);
      
      auto asset_record = static_cast<v8_chainstate*>(ptr)->_chain_state->get_asset_record(wrapper_asset_id->Int32Value());
      
      args.GetReturnValue().Set( External::New(args.GetIsolate(), &asset_record) );
   }
   
   void v8_chainstate::Get_Game_Data_Record(const v8::FunctionCallbackInfo<Value>& args)
   {
      Local<Object> self = args.Holder();
      Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
      void* ptr = wrap->Value();
      
      Local<Integer> wrapper_type = Local<Integer>::Cast(args[0]);
      Local<Integer> wrapper_id = Local<Integer>::Cast(args[1]);
      
      auto game_data_record = static_cast<v8_chainstate*>(ptr)->_chain_state->get_game_data_record(wrapper_type->Int32Value(), wrapper_id->Int32Value() );
      
      args.GetReturnValue().Set( External::New(args.GetIsolate(), &game_data_record) );
   }
   
   void v8_chainstate::Store_Blance_Record(const v8::FunctionCallbackInfo<Value>& args)
   {
      Local<Object> self = args.Holder();
      Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
      void* ptr = wrap->Value();
      
      Local<External> wrap_addr = Local<External>::Cast(args[0]);
      
      // TODO: parse json to C++ struct, from variant
      static_cast<v8_chainstate*>(ptr)->_chain_state->store_balance_record(* static_cast<blockchain::balance_record*>(wrap_addr->Value()));
   }
   
   void v8_chainstate::Store_Asset_Record(const v8::FunctionCallbackInfo<Value>& args)
   {
      Local<Object> self = args.Holder();
      Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
      void* ptr = wrap->Value();
      
      Local<External> wrapper_asset = Local<External>::Cast(args[0]);
      
      // TODO: parse json to C++ struct, from variant
      static_cast<v8_chainstate*>(ptr)->_chain_state->store_asset_record(* static_cast<blockchain::asset_record*>(wrapper_asset->Value()));
   }
   
   /**
    * @brief Method for v8_chainstate
    * @return undefine
    */
   void v8_chainstate::Store_Game_Data_Record(const v8::FunctionCallbackInfo<Value>& args)
   {
      Local<Object> self = args.Holder();
      Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
      void* ptr = wrap->Value();
      
      Local<Integer> wrapper_type = Local<Integer>::Cast(args[0]);
      Local<Integer> wrapper_id = Local<Integer>::Cast(args[1]);
      Local<External> wrap_rule_data = Local<External>::Cast(args[2]);
      
      // TODO: parse json to C++ struct, from variant
      static_cast<v8_chainstate*>(ptr)->_chain_state->store_game_data_record(wrapper_type->Int32Value(), wrapper_id->Int32Value(), * static_cast<blockchain::game_data_record*>(wrap_rule_data->Value()) );
   }
   
   Local<Object> v8_evalstate::New(v8::Isolate* isolate, transaction_evaluation_state_ptr eval_state)
   {
      EscapableHandleScope handle_scope(isolate);
      
      v8_evalstate* local_v8_evalstate = new v8_evalstate(eval_state);
      
      Handle<FunctionTemplate> templ = Local<FunctionTemplate>::New(isolate, v8_api::eval_state_templ);
      
      Handle<Function> evalstate_ctor = templ->GetFunction();
      Local<Object> g_evalstate = evalstate_ctor->NewInstance();
      g_evalstate->SetInternalField(0, External::New(isolate, local_v8_evalstate));
      return handle_scope.Escape(g_evalstate);
   }
   
   /**
    * @brief Method for v8_evalstate
    * @return TODO JS Object
    */
   void v8_evalstate::Sub_Balance(const v8::FunctionCallbackInfo<Value>& args)
   {
      Local<Object> self = args.Holder();
      Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
      void* ptr = wrap->Value();
      
      Local<External> wrap_asset = Local<External>::Cast(args[1]);
      
      // TODO: parse json to C++ struct, from variant
      static_cast<v8_evalstate*>(ptr)->_eval_state->sub_balance( * static_cast<asset*>(wrap_asset->Value() ) );
   }
} } // bts::game
