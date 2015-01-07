#include <bts/blockchain/exceptions.hpp>
#include <bts/game/rule_record.hpp>
#include <bts/blockchain/transaction_evaluation_state.hpp>
#include <bts/blockchain/chain_interface.hpp>
#include <bts/blockchain/game_executors.hpp>

#include <bts/wallet/wallet.hpp>
#include <bts/wallet/wallet_records.hpp>

namespace bts { namespace game {
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
        
     };
} } // bts::game
