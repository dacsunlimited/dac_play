#include <bts/game/v8_helper.hpp>

namespace bts { namespace game {
   // Creates a new execution environment containing the built-in
   // functions.
   v8::Handle<v8::Context> v8_helper::CreateShellContext(v8::Isolate* isolate) {
      // Create a template for the global object.
      v8::Handle<v8::ObjectTemplate> global = v8::ObjectTemplate::New(isolate);
      // Bind the global 'print' function to the C++ Print callback.
      //global->Set(v8::String::NewFromUtf8(isolate, "print"), v8::FunctionTemplate::New(isolate, Print));
      
      // Bind the global 'read' function to the C++ Read callback.
      //global->Set(v8::String::NewFromUtf8(isolate, "read"), v8::FunctionTemplate::New(isolate, Read));
      
       // Bind the global 'load' function to the C++ Load callback.
      //global->Set(v8::String::NewFromUtf8(isolate, "load"), v8::FunctionTemplate::New(isolate, Load));
      
       // Bind the 'quit' function
      // global->Set(v8::String::NewFromUtf8(isolate, "quit"), v8::FunctionTemplate::New(isolate, Quit));
      
       // Bind the 'version' function
      //global->Set(v8::String::NewFromUtf8(isolate, "version"), v8::FunctionTemplate::New(isolate, Version));
      
      // TODO Reset
      // TODO Transaction Template
      
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
            printf(" ");
         }
         v8::String::Utf8Value str(args[i]);
         const char* cstr = v8_helper::ToCString(str);
         printf("%s", cstr);
      }
      printf("\n");
      fflush(stdout);
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
               const char* cstr = v8_helper::ToCString(str);
               printf("%s\n", cstr);
            }
            return true;
         }
      }
   }
   
   void v8_helper::ReportException(v8::Isolate* isolate, v8::TryCatch* try_catch) {
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
   
   // Extracts a C string from a V8 Utf8Value.
   const char* v8_helper::ToCString(const v8::String::Utf8Value& value) {
      return *value ? *value : "<string conversion failed>";
   }
   
   /*
    int main(int argc, char* argv[]) {
    v8::V8::InitializeICU();
    v8::Platform* platform = v8::platform::CreateDefaultPlatform();
    v8::V8::InitializePlatform(platform);
    v8::V8::Initialize();
    v8::V8::SetFlagsFromCommandLine(&argc, argv, true);
    ShellArrayBufferAllocator array_buffer_allocator;
    v8::V8::SetArrayBufferAllocator(&array_buffer_allocator);
    v8::Isolate* isolate = v8::Isolate::New();
    run_shell = (argc == 1);
    int result;
    {
    v8::Isolate::Scope isolate_scope(isolate);
    v8::HandleScope handle_scope(isolate);
    v8::Handle<v8::Context> context = CreateShellContext(isolate);
    if (context.IsEmpty()) {
    fprintf(stderr, "Error creating context\n");
    return 1;
    }
    v8::Context::Scope context_scope(context);
    result = RunMain(isolate, argc, argv);
    if (run_shell) RunShell(context);
    }
    v8::V8::Dispose();
    v8::V8::ShutdownPlatform();
    delete platform;
    return result;
    }
    */
}}