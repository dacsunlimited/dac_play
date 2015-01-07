#include <bts/game/v8_game.hpp>
#include <bts/game/v8_api.hpp>
#include <bts/blockchain/time.hpp>

#include <include/v8.h>
#include <include/libplatform/libplatform.h>

namespace bts { namespace game {
    using namespace v8;
    
    static bool first_engine = []()->bool{
        init_class_template(v8::Isolate::GetCurrent());
        
        return true;
    }();
    
    v8_game_engine& v8_game_engine::instance()
    {
        static std::unique_ptr<v8_game_engine> inst( new v8_game_engine() );
        return *inst;
    }
    
    void v8_game_engine::evaluate( transaction_evaluation_state& eval_state)
    {
        
        // TODO
    }
    
    wallet_transaction_record v8_game_engine::play( chain_database_ptr blockchain, bts::wallet::wallet_ptr w, const variant& var, bool sign )
    {
        // TODO: Now we have the assumption that the asset id equals the game id.
        signed_transaction     trx;
        unordered_set<address> required_signatures;
        
        trx.expiration = now() + w->get_transaction_expiration();
        
        // TODO: adjust fee based upon blockchain price per byte and
        // the size of trx... 'recursively'
        auto required_fees = w->get_transaction_fee();
        
        {
            // TODO
        }
        
        auto record = wallet_transaction_record();
        // create the operations according to rule_type and input types
        // then how to map the structs?
        //trx.operations.push_back( game_operation(bts::game::dice_rule(address( play_account->active_key() ), amount_to_play, d_input.odds, d_input.guess ))//slate_id 0 );
        
        auto entry = bts::wallet::ledger_entry();
        //entry.from_account = play_account->active_key();
        //entry.to_account = play_account->active_key();
        //entry.memo = "play dice";
        
        record.ledger_entries.push_back( entry );
        record.fee = required_fees;
        
        if( sign ) w->sign_transaction( trx, required_signatures );
        
        record.trx = trx;
        
        return record;
    }
    
    bool scan( wallet_transaction_record& trx_rec, bts::wallet::wallet_ptr w )
    {
        
    }
    
    bool scan_result( const rule_result_transaction& rtrx,
                     uint32_t block_num,
                     const time_point_sec& block_time,
                     const time_point_sec& received_time,
                     const uint32_t trx_index, bts::wallet::wallet_ptr w)
    {
        
    }
    
    int v8_game_engine::execute( chain_database_ptr blockchain, uint32_t block_num, const pending_chain_state_ptr& pending_state )
    {
        v8::V8::InitializeICU();
        v8::Platform* platform = v8::platform::CreateDefaultPlatform();
        v8::V8::InitializePlatform(platform);
        v8::V8::Initialize();
        v8::Isolate* isolate = v8::Isolate::New();
        
        int result = 0;
        {
            v8::Isolate::Scope isolate_scope(isolate);
            v8::HandleScope handle_scope(isolate);
            v8::Handle<v8::Context> context = CreateShellContext(isolate);
            if (context.IsEmpty()) {
                fprintf(stderr, "Error creating context\n");
                return 1;
            }
            v8::Context::Scope context_scope(context);
            
            //associates our internal field pointing to 'p' with the "point" name inside the context
            //this enable usage of point inside this context without need to create a new one
            context->Global()->Set(String::NewFromUtf8(isolate, "g_blockchain"), v8_blockchain::New(isolate, blockchain, block_num));
            
            context->Global()->Set(String::NewFromUtf8(isolate, "g_block_num"), Integer::New(isolate, block_num));
            
            context->Global()->Set(String::NewFromUtf8(isolate, "g_pendingstate"), v8_chainstate::New(isolate, pending_state));
            
            // TODO: get rule execute
            
            // begin script execution
            ExecuteString(isolate, String::NewFromUtf8(isolate, "'TODO, defined rule';execute(g_blockchain, g_block_num, g_pendingstate);"), String::NewFromUtf8(isolate, "rule.execute"), true, true);
            
            // result = RunMain(isolate, argc, argv);
        }
        
        v8::V8::Dispose();
        v8::V8::ShutdownPlatform();
        delete platform;
        return result;
    }
    
} } // bts::game