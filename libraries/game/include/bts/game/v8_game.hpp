#pragma once

#include <bts/blockchain/exceptions.hpp>
#include <bts/blockchain/transaction_evaluation_state.hpp>
#include <bts/blockchain/chain_interface.hpp>
#include <bts/blockchain/game_executors.hpp>

#include <bts/wallet/wallet.hpp>
#include <bts/wallet/wallet_records.hpp>
#include <bts/wallet/exceptions.hpp>

#include <bts/game/rule_record.hpp>

namespace bts { namespace game {
   using namespace bts::blockchain;
   using namespace bts::wallet;
   
   class client;
   namespace detail { class v8_game_engine_impl; }
    
    struct play_code
    {
        string from_account;
        string to_account;
        asset  amount;
        string memo;
    };
   
   /**
    * @class v8_game_engine
    *
    *  script context for javascript running
    */
   class v8_game_engine
   {
   public:
       v8_game_engine(std::string game_name, bts::game::client* client);
      
      ~v8_game_engine(){};
      
      void evaluate( transaction_evaluation_state& eval_state, game_id_type game_id, const variant& var);
      
      wallet_transaction_record play( game_id_type game_id, chain_database_ptr blockchain, bts::wallet::wallet_ptr w, const variant& var, bool sign );
      
      bool scan( wallet_transaction_record& trx_rec, bts::wallet::wallet_ptr w );
      
      bool scan_result( const game_result_transaction& rtrx,
                               uint32_t block_num,
                               const time_point_sec& block_time,
                               const uint32_t trx_index, bts::wallet::wallet_ptr w);
      
      /**
       * wrapper to call the javascript stub defined by game developers
       */
      void execute( chain_database_ptr blockchain, uint32_t block_num, const pending_chain_state_ptr& pending_state );
   private:
      std::shared_ptr<detail::v8_game_engine_impl> my;
   };
   
   typedef std::shared_ptr<v8_game_engine> v8_game_engine_ptr;
} } // bts::game
