#include <bts/game/v8_api.hpp>

namespace bts { namespace game {
   Persistent<ObjectTemplate> v8_api::global;
   
   Persistent<FunctionTemplate> v8_api::blockchain_templ;
    
   Persistent<FunctionTemplate> v8_api::wallet_templ;
   
   Persistent<FunctionTemplate> v8_api::pendingstate_templ;
   
   Persistent<FunctionTemplate> v8_api::eval_state_templ;
   
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
       
       proto->Set( isolate, "get_block_digest", FunctionTemplate::New( isolate, v8_blockchain::Get_Block_Digest));
       
       proto->Set( isolate, "get_block", FunctionTemplate::New( isolate, v8_blockchain::Get_Block));
       
       proto->Set( isolate, "get_transaction", FunctionTemplate::New( isolate, v8_blockchain::Get_Transaction));
       
       proto->Set( isolate, "get_asset_record", FunctionTemplate::New(isolate, v8_blockchain::Get_Asset_Record) );
       
       proto->Set(isolate, "get_game_data_record", FunctionTemplate::New(isolate, v8_blockchain::Get_Game_Data_Record));
       
       proto->Set(isolate, "get_account_record_by_name", FunctionTemplate::New(isolate, v8_blockchain::Get_Account_Record_By_Name));
      
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
        
        //associates the "method" string to the callback PointMethod in the class template
        //enabling point.method_a() constructions inside the javascript
        proto->Set(isolate, "get_transaction_fee", FunctionTemplate::New(isolate, v8_wallet::Get_Transaction_Fee));
        
        proto->Set(isolate, "get_wallet_key_for_address", FunctionTemplate::New(isolate, v8_wallet::Get_Wallet_Key_For_Address) );
        proto->Set(isolate, "store_transaction", FunctionTemplate::New(isolate, v8_wallet::Store_Transaction) );
        
        Handle<ObjectTemplate> inst = result->InstanceTemplate();
        
        inst->SetInternalFieldCount(1);
        
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
      pendingstate_proto->Set(isolate, "get_account_record_by_name", FunctionTemplate::New(isolate, v8_chainstate::Get_Account_Record_By_Name));
      
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
       
       eval_state_proto->Set(isolate, "get_transaction_id", FunctionTemplate::New(isolate, v8_evalstate::Get_Transaction_Id));
       
       //access the instance pointer of our new class template
       Handle<ObjectTemplate> inst = result->InstanceTemplate();
       
       //set the internal fields of the class as we have the Point class internally
       inst->SetInternalFieldCount(1);
      
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
       EscapableHandleScope handle_scope(args.GetIsolate());
       
      auto owner = * static_cast<address*> (Local<External>::Cast(args[0])->Value());
      
      int asset_id = args[1]->Int32Value();
      
      auto addr = withdraw_condition( withdraw_with_signature(owner), asset_id ).get_address();
      
      args.GetReturnValue().Set( External::New(args.GetIsolate(), &addr) );
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
       EscapableHandleScope handle_scope(info.GetIsolate());
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
    
    void v8_blockchain::Get_Block_Digest(const v8::FunctionCallbackInfo<Value>& args)
    {
        EscapableHandleScope handle_scope(args.GetIsolate());
        Local<Object> self = args.Holder();
        Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
        void* ptr = wrap->Value();
        //get member variable value
        
        wlog( "The block number is ${d}", ("d", args[0]->Uint32Value()) );
        
        auto block_digest = static_cast<v8_blockchain*>(ptr)->_blockchain->get_block_digest(args[0]->Uint32Value());
        //return the value
        
        // args.GetReturnValue().Set( v8::Null( args.GetIsolate() ) );
        
        wlog( "The block digest is ${d}", ("d", block_digest) );
        
        auto v = variant(block_digest);
        
        if ( v.is_object() )
        {
            fc::mutable_variant_object obj(v);
            
            obj("id", block_digest.id() );
            
            v = obj;
        }
        
        auto result = v8_helper::cpp_to_json(args.GetIsolate(), v );
        
        wlog( "get the result after cpp to json" );
        
        args.GetReturnValue().Set( result );
    }
   
   void v8_blockchain::Get_Block(const v8::FunctionCallbackInfo<Value>& args)
   {
      EscapableHandleScope handle_scope(args.GetIsolate());
      Local<Object> self = args.Holder();
      Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
      void* ptr = wrap->Value();
      //get member variable value
       
      auto block = static_cast<v8_blockchain*>(ptr)->_blockchain->get_block(args[0]->Uint32Value());
      //return the value
       
       auto block_obj = v8_helper::cpp_to_json(args.GetIsolate(), block );
       
       if ( block_obj->IsObject() )
       {
           block_obj->ToObject()->Set(String::NewFromUtf8( args.GetIsolate(), "id") , v8_helper::cpp_to_json(args.GetIsolate(), block.id()) );
       }
      
      args.GetReturnValue().Set( block_obj );
   }
    
    void v8_blockchain::Get_Transaction(const v8::FunctionCallbackInfo<Value>& args)
    {
        EscapableHandleScope handle_scope(args.GetIsolate());
        Local<Object> self = args.Holder();
        Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
        void* ptr = wrap->Value();
        //get member variable value
        
        auto trx = static_cast<v8_blockchain*>(ptr)->_blockchain->get_transaction( v8_helper::json_to_cpp<transaction_id_type>(args.GetIsolate(), args[0]));
        //return the value
        
        if ( trx.valid() )
        {
            auto trx_obj = v8_helper::cpp_to_json(args.GetIsolate(), *trx );
            
            if ( trx_obj->IsObject() )
            {
                trx_obj->ToObject()->Set(String::NewFromUtf8( args.GetIsolate(), "id") , v8_helper::cpp_to_json(args.GetIsolate(), trx->trx.id()) );
            }
            
            args.GetReturnValue().Set( trx_obj );
            
        } else
        {
            args.GetReturnValue().Set( v8::Null( args.GetIsolate() ) );
        }
        
    }
   
   void v8_blockchain::Get_Current_Random_Seed(const v8::FunctionCallbackInfo<Value>& args)
   {
       EscapableHandleScope handle_scope(args.GetIsolate());
       
      Local<Object> self = args.Holder();
      Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
      void* ptr = wrap->Value();
      //get member variable value
      auto random_seed = static_cast<v8_blockchain*>(ptr)->_blockchain->get_current_random_seed();
      
       args.GetReturnValue().Set( String::NewFromUtf8(args.GetIsolate(), random_seed.str().c_str()) );
   }
    
    void v8_blockchain::Get_Asset_Record(const v8::FunctionCallbackInfo<Value>& args)
    {
        EscapableHandleScope handle_scope(args.GetIsolate());
        Local<Object> self = args.Holder();
        Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
        void* ptr = wrap->Value();
        //get member variable value
        oasset_record asset_rec;
        if ( args[0]->IsString() )
        {
            asset_rec = static_cast<v8_blockchain*>(ptr)->_blockchain->get_asset_record( v8_helper::ToCString(String::Utf8Value(args[0]->ToString())) );
        } else
        {
            asset_rec = static_cast<v8_blockchain*>(ptr)->_blockchain->get_asset_record( v8_helper::ToCString(String::Utf8Value( args[0]->ToInt32() )) );
        }
        
        if ( asset_rec.valid() )
        {
            args.GetReturnValue().Set( v8_helper::cpp_to_json(args.GetIsolate(), *asset_rec) );
        } else
        {
            args.GetReturnValue().Set( v8::Null(args.GetIsolate() ) );
        }
    }
    
    void v8_blockchain::Get_Account_Record_By_Name(const v8::FunctionCallbackInfo<Value>& args)
    {
        EscapableHandleScope handle_scope(args.GetIsolate());
        
        Local<Object> self = args.Holder();
        Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
        void* ptr = wrap->Value();
        
        try {
            string account_name = v8_helper::ToCString( String::Utf8Value( args[0]->ToString() ) );
            wlog("the account name is ${n}", ("n", account_name) );
            auto account_record = static_cast<v8_blockchain*>(ptr)->_blockchain->get_account_record( account_name );
            
            if ( account_record.valid() )
            {
                wlog("the account record is ${a}", ("a", *account_record) );
                Local<Value> res = v8_helper::cpp_to_json(args.GetIsolate(), *account_record );
                
                if ( res->IsObject() )
                {
                    res->ToObject()->Set(String::NewFromUtf8( args.GetIsolate(), "active_key") , v8_helper::cpp_to_json(args.GetIsolate(), account_record->active_key() ) );
                }
                
                args.GetReturnValue().Set( handle_scope.Escape( res ) );
            } else {
                args.GetReturnValue().Set( v8::Null( args.GetIsolate() ) );
            }
        } catch ( const fc::exception& e )
        {
            wlog("Failed to Get_Account_Record_By_Name: ${e}", ("e", e.to_detail_string()));
            args.GetReturnValue().Set( v8::Null( args.GetIsolate() ) );
            
        }
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
        EscapableHandleScope handle_scope(args.GetIsolate());
        
        Local<Object> self = args.Holder();
        Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
        void* ptr = wrap->Value();
        //get member variable value
        auto value = static_cast<v8_wallet*>(ptr)->_wallet->get_transaction_fee();
        
        //return the value
        args.GetReturnValue().Set( v8_helper::cpp_to_json(args.GetIsolate(), value) );
    }
    
    void v8_wallet::Get_Wallet_Key_For_Address(const v8::FunctionCallbackInfo<Value>& args)
    {
        EscapableHandleScope handle_scope(args.GetIsolate());
        
        Local<Object> self = args.Holder();
        Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
        void* ptr = wrap->Value();
        //get member variable value
        try {
            auto addr = v8_helper::json_to_cpp<bts::blockchain::address>(args.GetIsolate(), args[0] );
            auto key = static_cast<v8_wallet*>(ptr)->_wallet->get_wallet_key_for_address( addr );
            if ( key.valid() )
            {
                // return the value
                args.GetReturnValue().Set( v8_helper::cpp_to_json(args.GetIsolate(), key) );
            } else
            {
                args.GetReturnValue().Set( v8::Null(args.GetIsolate() ) );
            }
        } catch ( ... ) {
            args.GetReturnValue().Set( v8::Null(args.GetIsolate() ) );
        }
        
    }
    
    void v8_wallet::Store_Transaction(const v8::FunctionCallbackInfo<Value>& args)
    {
        EscapableHandleScope handle_scope(args.GetIsolate());
        
        Local<Object> self = args.Holder();
        Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
        void* ptr = wrap->Value();
        
        Local<String> v8_string = v8_helper::toJson( args.GetIsolate(), args[0] );
        std::string str = *v8::String::Utf8Value(v8_string);
        wlog( "Starting store transaction ${v}", ("v", str) );
        auto trx_data = v8_helper::json_to_cpp<transaction_info>(args.GetIsolate(), args[0] );
        wlog("The transaction data is ${x}", ("x", trx_data));
        
        static_cast<v8_wallet*>(ptr)->_wallet->store_transaction( trx_data );
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
       EscapableHandleScope handle_scope(args.GetIsolate());
       
      Local<Object> self = args.Holder();
      Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
      void* ptr = wrap->Value();
      
      Local<External> wrap_addr = Local<External>::Cast(args[0]);
      
      auto balance_record = static_cast<v8_chainstate*>(ptr)->_chain_state->get_balance_record(* static_cast<address*>(wrap_addr->Value()));
      
      args.GetReturnValue().Set( External::New(args.GetIsolate(), &balance_record) );
   }
   
   void v8_chainstate::Get_Asset_Record(const v8::FunctionCallbackInfo<Value>& args)
   {
       EscapableHandleScope handle_scope(args.GetIsolate());
       
       Local<Object> self = args.Holder();
       Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
       void* ptr = wrap->Value();
      
       oasset_record asset_rec;
       if ( args[0]->IsString() )
       {
           asset_rec = static_cast<v8_chainstate*>(ptr)->_chain_state->get_asset_record( v8_helper::ToCString(String::Utf8Value(args[0]->ToString())) );
       } else
       {
           asset_rec = static_cast<v8_chainstate*>(ptr)->_chain_state->get_asset_record( v8_helper::ToCString(String::Utf8Value( args[0]->ToInt32() )) );
       }
      
      if ( asset_rec.valid() )
      {
           args.GetReturnValue().Set( v8_helper::cpp_to_json(args.GetIsolate(), *asset_rec) );
      } else
      {
          args.GetReturnValue().Set( v8::Null(args.GetIsolate() ) );
      }
   }
   
   void v8_chainstate::Get_Game_Data_Record(const v8::FunctionCallbackInfo<Value>& args)
   {
       EscapableHandleScope handle_scope(args.GetIsolate());
       
      Local<Object> self = args.Holder();
      Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
      void* ptr = wrap->Value();
      
      Local<Integer> wrapper_type = Local<Integer>::Cast(args[0]);
      Local<Integer> wrapper_id = Local<Integer>::Cast(args[1]);
      
      auto game_data_record = static_cast<v8_chainstate*>(ptr)->_chain_state->get_game_data_record(wrapper_type->Int32Value(), wrapper_id->Int32Value() );
       
       if ( game_data_record.valid() )
       {
           Local<Value> res = v8_helper::cpp_to_json(args.GetIsolate(), *game_data_record );
           args.GetReturnValue().Set( handle_scope.Escape( res ) );
       } else {
           args.GetReturnValue().Set( v8::Null( args.GetIsolate() ) );
       }
   }
    
    void v8_chainstate::Get_Account_Record_By_Name(const v8::FunctionCallbackInfo<Value>& args)
    {
        EscapableHandleScope handle_scope(args.GetIsolate());
        
        Local<Object> self = args.Holder();
        Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
        void* ptr = wrap->Value();
        
        try {
            string account_name = v8_helper::ToCString( String::Utf8Value( args[0]->ToString() ) );
            wlog("the account name is ${n}", ("n", account_name) );
            auto account_record = static_cast<v8_chainstate*>(ptr)->_chain_state->get_account_record( account_name );
            
            if ( account_record.valid() )
            {
                wlog("the account record is ${a}", ("a", *account_record) );
                Local<Value> res = v8_helper::cpp_to_json(args.GetIsolate(), *account_record );
                
                if ( res->IsObject() )
                {
                    res->ToObject()->Set(String::NewFromUtf8( args.GetIsolate(), "active_key") , v8_helper::cpp_to_json(args.GetIsolate(), account_record->active_key() ) );
                }
                
                args.GetReturnValue().Set( handle_scope.Escape( res ) );
            } else {
                args.GetReturnValue().Set( v8::Null( args.GetIsolate() ) );
            }
        } catch ( const fc::exception& e )
        {
            wlog("Failed to Get_Account_Record_By_Name: ${e}", ("e", e.to_detail_string()));
            args.GetReturnValue().Set( v8::Null( args.GetIsolate() ) );
            
        }
    }
    
    void v8_blockchain::Get_Game_Data_Record(const v8::FunctionCallbackInfo<Value>& args)
    {
        EscapableHandleScope handle_scope(args.GetIsolate());
        
        Local<Object> self = args.Holder();
        Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
        void* ptr = wrap->Value();
        
        Local<Integer> wrapper_game_id = Local<Integer>::Cast(args[0]);
        Local<Integer> wrapper_data_id = Local<Integer>::Cast(args[1]);
        
        int32_t game_id = wrapper_game_id->Int32Value();
        int32_t data_id = wrapper_data_id->Int32Value();
        
        wlog("Starting Get_Game_Data_Record with game_id:${g} and data_id:${d}", ("g", game_id)("d", data_id));
        auto game_data_record = static_cast<v8_blockchain*>(ptr)->_blockchain->get_game_data_record(game_id, data_id );
        
        if ( game_data_record.valid() )
        {
            wlog("game_data_record:${d}", ("d", *game_data_record));
            Local<Value> res = v8_helper::cpp_to_json(args.GetIsolate(), *game_data_record );
            args.GetReturnValue().Set( handle_scope.Escape( res ) );
        } else {
            args.GetReturnValue().Set( v8::Null( args.GetIsolate() ) );
        }
    }
   
   void v8_chainstate::Store_Blance_Record(const v8::FunctionCallbackInfo<Value>& args)
   {
       EscapableHandleScope handle_scope(args.GetIsolate());
      Local<Object> self = args.Holder();
      Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
      void* ptr = wrap->Value();
      
      Local<External> wrap_addr = Local<External>::Cast(args[0]);
      
      // TODO: parse json to C++ struct, from variant
      static_cast<v8_chainstate*>(ptr)->_chain_state->store_balance_record(* static_cast<blockchain::balance_record*>(wrap_addr->Value()));
   }
   
   void v8_chainstate::Store_Asset_Record(const v8::FunctionCallbackInfo<Value>& args)
   {
       EscapableHandleScope handle_scope(args.GetIsolate());
      Local<Object> self = args.Holder();
      Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
      void* ptr = wrap->Value();
      
      Local<External> wrapper_asset = Local<External>::Cast(args[0]);
      
      // TODO: parse json to C++ struct, from variant
      static_cast<v8_chainstate*>(ptr)->_chain_state->store_asset_record(* static_cast<blockchain::asset_record*>(wrapper_asset->Value()));
   }
   
   /**
    * @brief Method for v8_chainstate
    * @deprecated
    * TODO
    */
   void v8_chainstate::Store_Game_Data_Record(const v8::FunctionCallbackInfo<Value>& args)
   {
       EscapableHandleScope handle_scope(args.GetIsolate());
       
      Local<Object> self = args.Holder();
      Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
      void* ptr = wrap->Value();
      
      Local<Integer> wrapper_type = Local<Integer>::Cast(args[0]);
      Local<Integer> wrapper_id = Local<Integer>::Cast(args[1]);
      Local<Object> wrap_game_data = Local<Object>::Cast(args[2]);
      
      // TODO: parse json to C++ struct, from variant
       static_cast<v8_chainstate*>(ptr)->_chain_state->store_game_data_record(wrapper_type->Int32Value(), wrapper_id->Int32Value(), v8_helper::json_to_cpp<game_data_record>(args.GetIsolate(), wrap_game_data ) );
   }
   
   Local<Object> v8_evalstate::New(v8::Isolate* isolate, transaction_evaluation_state* eval_state_ptr)
   {
      EscapableHandleScope handle_scope(isolate);
      
      v8_evalstate* local_v8_evalstate = new v8_evalstate(eval_state_ptr);
      
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
       EscapableHandleScope handle_scope(args.GetIsolate());
       
      Local<Object> self = args.Holder();
      Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
      void* ptr = wrap->Value();
      
      Local<External> wrap_asset = Local<External>::Cast(args[1]);
      
      // TODO: parse json to C++ struct, from variant
      static_cast<v8_evalstate*>(ptr)->_eval_state->sub_balance( * static_cast<asset*>(wrap_asset->Value() ) );
   }
    
    void v8_evalstate::Get_Transaction_Id(const v8::FunctionCallbackInfo<Value>& args)
    {
        EscapableHandleScope handle_scope(args.GetIsolate());
        
        Local<Object> self = args.Holder();
        Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
        void* ptr = wrap->Value();
        
        auto trx_id = static_cast<v8_evalstate*>(ptr)->_eval_state->trx.id();
        
        args.GetReturnValue().Set( v8_helper::cpp_to_json( args.GetIsolate(), trx_id ) );
    }
} } // bts::game
