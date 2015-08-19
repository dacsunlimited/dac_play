#include "include/v8.h"
#include "include/libplatform/libplatform.h"

#include <iostream>
#include <fstream>
#include <string>

using namespace v8;

Local<Value> parseJson(Isolate* isolate, Handle<Value> jsonString) {
    EscapableHandleScope handle_scope(isolate);
    
    Local<Context> context = isolate->GetCurrentContext();
    Local<Object> global = context->Global();
    
    Local<Object> JSON = global->Get(String::NewFromUtf8(isolate, "JSON"))->ToObject();
    Local<Function> JSON_parse = Handle<Function>::Cast(JSON->Get(String::NewFromUtf8(isolate, "parse")));
    
    // return JSON.parse.apply(JSON, jsonString);
    return handle_scope.Escape(JSON_parse->Call(JSON, 1, &jsonString));
}

class ArrayBufferAllocator : public v8::ArrayBuffer::Allocator {
public:
    virtual void* Allocate(size_t length) {
        void* data = AllocateUninitialized(length);
        return data == NULL ? data : memset(data, 0, length);
    }
    virtual void* AllocateUninitialized(size_t length) { return malloc(length); }
    virtual void Free(void* data, size_t) { free(data); }
};

int main(int argc, char* argv[]) {
  // Initialize V8.
  V8::InitializeICU();
  Platform* platform = platform::CreateDefaultPlatform();
  V8::InitializePlatform(platform);
  V8::Initialize();
    
    ArrayBufferAllocator* _allocator = new ArrayBufferAllocator();
    Isolate::CreateParams create_params;
    create_params.array_buffer_allocator = _allocator;
    
    ResourceConstraints rc;
    rc.set_max_old_space_size(40); //MB
    rc.set_max_executable_size(40); //MB
    static const int stack_breathing_room = 1024 * 1024;
    //uint32_t* set_limit = ComputeStackLimit(stack_breathing_room);
    rc.set_stack_limit(reinterpret_cast<uint32_t*>((char*)&rc - stack_breathing_room));

  // Create a new Isolate and make it the current one.
  Isolate* isolate = Isolate::New(create_params);
  {
    Isolate::Scope isolate_scope(isolate);

    // Create a stack-allocated handle scope.
    HandleScope handle_scope(isolate);

    // Create a new context.
    Local<Context> context = Context::New(isolate);
      for ( int i = 1; i < 1000; i ++ )
      {
          Context::Scope context_scope(context);
          Local<Value> v = parseJson( isolate, String::NewFromUtf8(isolate, "\"8350f2c8e37168e9186c3fc04da8534574b1e474\""));
          String::Utf8Value utf8(v);
          printf("%s\n", *utf8);
      }
      
      for ( int i = 1; i < 100; i ++ )
      {
          v8::Local<v8::Context> new_context = v8::Local<v8::Context>::New(isolate, context);
          
          // Enter the context for compiling and running the hello world script.
          Context::Scope context_scope(new_context);

          // Create a string containing the JavaScript source code.
          Local<String> source = String::NewFromUtf8(isolate, "var PLAY={};PLAY.execute = function(){};PLAY.execute(1, 1);");
          
          // Compile the source code.
          Local<Script> script = Script::Compile(source);
          
          if (script.IsEmpty())
          {
              printf("%s\n", "error....");
          }
          
          // Run the script to get the result.
          Local<Value> result = script->Run();
          
          // Convert the result to an UTF8 string and print it.
          String::Utf8Value utf8(result);
          printf("%s\n", *utf8);
      }

  }
  
  // Dispose the isolate and tear down V8.
  isolate->Dispose();
  V8::Dispose();
  V8::ShutdownPlatform();
  delete platform;
  delete _allocator;
  return 0;
}
