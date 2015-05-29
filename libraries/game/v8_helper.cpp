#include <bts/blockchain/types.hpp>

#include <bts/game/v8_helper.hpp>

#include <boost/format.hpp>

namespace bts { namespace game {
    Local<Value> v8_helper::parseJson(Isolate* isolate, Handle<String> jsonString) {
        EscapableHandleScope handle_scope(isolate);
        
        v8::Local<v8::Value> result = v8::JSON::Parse( jsonString );
        
        return handle_scope.Escape( result );
    }
    
    /* old implement, see issue #125
     Local<Context> context = isolate->GetCurrentContext();
     v8::Context::Scope context_scope(context);
     
     Local<Object> global = context->Global();
     
     Local<Object> JSON = global->Get(String::NewFromUtf8(isolate, "JSON"))->ToObject();
     Local<Function> JSON_parse = Handle<Function>::Cast(JSON->Get(String::NewFromUtf8(isolate, "parse")));
     
     // return JSON.parse.apply(JSON, jsonString);
     return handle_scope.Escape(JSON_parse->Call(JSON, 1, &jsonString));
     */
    
    Local<String> v8_helper::toJson(Isolate* isolate, Handle<Value> object)
    {
        v8::Locker locker(isolate);
        Isolate::Scope isolate_scope(isolate);
        EscapableHandleScope handle_scope(isolate);
        
        Local<Context> context = isolate->GetCurrentContext();
        Local<Object> global = context->Global();
        
        Local<Object> JSON = global->Get(String::NewFromUtf8(isolate, "JSON"))->ToObject();
        Local<Function> JSON_stringify = Handle<Function>::Cast(JSON->Get(String::NewFromUtf8(isolate, "stringify")));
        
        return handle_scope.Escape(JSON_stringify->Call(JSON, 1, &object)->ToString());
    }
    
    void v8_helper::fc_ripemd160_hash(const v8::FunctionCallbackInfo<v8::Value>& args )
    {
        try {
            EscapableHandleScope handle_scope(args.GetIsolate());
            
            std::string str_to_hash(v8_helper::ToCString(String::Utf8Value(args[0]->ToString(args.GetIsolate()))));
            auto hash = fc::ripemd160::hash( str_to_hash );
            
            args.GetReturnValue().Set( v8_helper::cpp_to_json( args.GetIsolate(), hash ) );
        } catch ( ... )
        {
            args.GetReturnValue().Set( v8::Null( args.GetIsolate() ) );
        }
    }
    
    void v8_helper::trx_id_to_hash_array(const v8::FunctionCallbackInfo<v8::Value>& args )
    {
        try {
            HandleScope handle_scope(args.GetIsolate());
            
            // Create a new empty array.
            Handle<Array> array = Array::New(args.GetIsolate(), 5);
            
            // Return an empty result if there was an error creating the array.
            if (array.IsEmpty())
                args.GetReturnValue().Set(Handle<Array>());
            
            auto trx_id = json_to_cpp<bts::blockchain::transaction_id_type>(args.GetIsolate(), args[0]);
            
            // Fill out the values
            array->Set( 0, Integer::New(args.GetIsolate(), trx_id._hash[0]) );
            array->Set( 1, Integer::New(args.GetIsolate(), trx_id._hash[1]) );
            array->Set( 2, Integer::New(args.GetIsolate(), trx_id._hash[2]) );
            array->Set( 3, Integer::New(args.GetIsolate(), trx_id._hash[3]) );
            array->Set( 4, Integer::New(args.GetIsolate(), trx_id._hash[4]) );
            
            // Return the value through Close.
            
            args.GetReturnValue().Set( array );
        } catch ( ... )
        {
            args.GetReturnValue().Set( v8::Null( args.GetIsolate() ) );
        }
    }
    
    
   // Creates a new execution environment containing the built-in
   // functions.
   v8::Handle<v8::Context> v8_helper::CreateShellContext(v8::Isolate* isolate) {
      // Create a template for the global object.
      v8::Handle<v8::ObjectTemplate> global = v8::ObjectTemplate::New(isolate);
      // Bind the global 'print' function to the C++ Print callback.
      global->Set(v8::String::NewFromUtf8(isolate, "print"), v8::FunctionTemplate::New(isolate, Print));
      
      // Bind the global 'read' function to the C++ Read callback.
      global->Set(v8::String::NewFromUtf8(isolate, "read"), v8::FunctionTemplate::New(isolate, Read));
      
       // Bind the global 'load' function to the C++ Load callback.
      global->Set(v8::String::NewFromUtf8(isolate, "load"), v8::FunctionTemplate::New(isolate, Load));
      
       // Bind the 'quit' function
      // global->Set(v8::String::NewFromUtf8(isolate, "quit"), v8::FunctionTemplate::New(isolate, Quit));
      
       // Bind the 'version' function, TODO might conflict with game version
      // global->Set(v8::String::NewFromUtf8(isolate, "version"), v8::FunctionTemplate::New(isolate, Version));
      
      // TODO Reset
      // TODO Transaction Template
       
      global->Set(v8::String::NewFromUtf8(isolate, "trx_id_to_hash_array"), v8::FunctionTemplate::New(isolate, trx_id_to_hash_array) );
      global->Set(v8::String::NewFromUtf8(isolate, "fc_ripemd160_hash"), v8::FunctionTemplate::New(isolate, fc_ripemd160_hash) );
      
      return v8::Context::New(isolate, NULL, global);
   }
   
   // The callback that is invoked by v8 whenever the JavaScript 'print'
   // function is called.  Prints its arguments on stdout separated by
   // spaces and ending with a newline.
   void v8_helper::Print(const v8::FunctionCallbackInfo<v8::Value>& args) {
      bool first = true;
      for (int i = 0; i < args.Length(); i++) {
         v8::HandleScope handle_scope(args.GetIsolate());
         if (first) {
            first = false;
         } else {
             wlog(" ");
         }
         v8::String::Utf8Value str( toJson( args.GetIsolate(), args[i] ) );
         const char* cstr = v8_helper::ToCString(str);
         wlog( "The ${i}th parameter is ${s}", ("i", i)("s", cstr) );
      }
   }
   
   
   // The callback that is invoked by v8 whenever the JavaScript 'read'
   // function is called.  This function loads the content of the file named in
   // the argument into a JavaScript string.
   void v8_helper::Read(const v8::FunctionCallbackInfo<v8::Value>& args) {
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
   void v8_helper::Load(const v8::FunctionCallbackInfo<v8::Value>& args) {
      for (int i = 0; i < args.Length(); i++) {
         v8::HandleScope handle_scope(args.GetIsolate());
         v8::String::Utf8Value file(args[i]);
         if (*file == NULL) {
            args.GetIsolate()->ThrowException(
                                              v8::String::NewFromUtf8(args.GetIsolate(), "Error loading file"));
            return;
         }
         v8::Handle<v8::String> source = v8_helper::ReadFile(args.GetIsolate(), *file);
         if (source.IsEmpty()) {
            args.GetIsolate()->ThrowException(
                                              v8::String::NewFromUtf8(args.GetIsolate(), "Error loading file"));
            return;
         }
         if (!v8_helper::ExecuteString(args.GetIsolate(),
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
   void v8_helper::Quit(const v8::FunctionCallbackInfo<v8::Value>& args) {
      // If not arguments are given args[0] will yield undefined which
      // converts to the integer value 0.
      int exit_code = args[0]->Int32Value();
      fflush(stdout);
      fflush(stderr);
      exit(exit_code);
   }
   
   
   void v8_helper::Version(const v8::FunctionCallbackInfo<v8::Value>& args) {
      args.GetReturnValue().Set(
                                v8::String::NewFromUtf8(args.GetIsolate(), v8::V8::GetVersion()));
   }
   
   // Reads a file into a v8 string.
   v8::Handle<v8::String> v8_helper::ReadFile(v8::Isolate* isolate, const char* name) {
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
      v8::Handle<v8::String> result = v8::String::NewFromUtf8(isolate, chars, v8::String::kNormalString, size);
      delete[] chars;
      return result;
   }
   
   bool v8_helper::ExecuteString(v8::Isolate* isolate, v8::Handle<v8::String> source, v8::Handle<v8::String> name, bool print_result, bool report_exceptions)
   {
      v8::HandleScope handle_scope(isolate);
      v8::TryCatch try_catch(isolate);
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
               const char* cstr = v8_helper::ToCString(str);
               printf("%s\n", cstr);
            }
            return true;
         }
      }
   }
   
    std::string v8_helper::ReportException(v8::Isolate* isolate, v8::TryCatch* try_catch) {
       std::stringstream ss;
       
      v8::HandleScope handle_scope(isolate);
      v8::String::Utf8Value exception(try_catch->Exception());
      const char* exception_string = ToCString(exception);
      v8::Handle<v8::Message> message = try_catch->Message();
      if (message.IsEmpty()) {
         // V8 didn't provide any extra information about this error; just
         // print the exception.
          ss << exception_string << "\n";
      } else {
         // Print (filename):(line number): (message).
         v8::String::Utf8Value filename(message->GetScriptOrigin().ResourceName());
         const char* filename_string = ToCString(filename);
         int linenum = message->GetLineNumber();
         
          ss << boost::format("%s:%i: %s\n")%filename_string%linenum%exception_string;
          
         // Print line of source code.
         v8::String::Utf8Value sourceline(message->GetSourceLine());
         const char* sourceline_string = ToCString(sourceline);
          
          ss << boost::format("%s\n")%sourceline_string;
          
         // Print wavy underline (GetUnderline is deprecated).
         int start = message->GetStartColumn();
         for (int i = 0; i < start; i++) {
             ss << " ";
         }
         int end = message->GetEndColumn();
         for (int i = start; i < end; i++) {
             ss << "^";
         }
          ss << "\n";
          
         v8::String::Utf8Value stack_trace(try_catch->StackTrace());
         if (stack_trace.length() > 0) {
            const char* stack_trace_string = ToCString(stack_trace);
             ss << boost::format("%s\n")%stack_trace_string;
         }
      }
       
       return ss.str();
   }
   
   // Extracts a C string from a V8 Utf8Value.
   const char* v8_helper::ToCString(const v8::String::Utf8Value& value) {
      return *value ? *value : "<string conversion failed>";
   }
}}