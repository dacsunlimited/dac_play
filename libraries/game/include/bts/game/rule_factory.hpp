#pragma once

#include <bts/blockchain/exceptions.hpp>
#include <bts/blockchain/transaction_evaluation_state.hpp>
#include <bts/blockchain/chain_interface.hpp>
#include <bts/blockchain/game_executors.hpp>
#include <bts/wallet/wallet.hpp>
#include <bts/wallet/wallet_records.hpp>

#include <bts/game/rule_record.hpp>
#include <bts/game/v8_game.hpp>

namespace bts { namespace game {
   using namespace bts::blockchain;
   using namespace bts::wallet;

   class rule_factory
   {
   public:
      static rule_factory& instance();

      void   register_rule(uint8_t game_id, v8_game_engine_ptr engine_ptr )
      {
         FC_ASSERT( _engines.find( game_id ) == _engines.end(),
                        "Game ID already Registered ${id}", ("id", game_id) );
         _engines[game_id] = engine_ptr;
      }

      void evaluate( transaction_evaluation_state& eval_state, const rule& g )
      {
         auto itr = _engines.find( uint8_t(g.type) );
         if( itr == _engines.end() )
            FC_THROW_EXCEPTION( bts::blockchain::unsupported_chain_operation, "", ("rule",g) );
         itr->second->evaluate( eval_state);
      }
       
      wallet_transaction_record play(const int& game_id, chain_database_ptr blockchain, bts::wallet::wallet_ptr w, const variant& var, bool sign)
      {
          auto itr = _engines.find( uint8_t(game_id) );
          if( itr == _engines.end() )
              FC_THROW_EXCEPTION( bts::blockchain::unsupported_chain_operation, "", ("game_id", game_id) );
          
          return itr->second->play(blockchain, w, var, sign);
      }

      /// defined in games.cpp
      void to_variant( const bts::game::rule& in, fc::variant& output );
         /// defined in games.cpp
      void from_variant( const fc::variant& in, bts::game::rule& output );
       
      bool scan( const rule& g, wallet_transaction_record& trx_rec, bts::wallet::wallet_ptr w );
      
      bool scan_result( const game_result_transaction& rtrx,
                    uint32_t block_num,
                    const time_point_sec& block_time,
                    const uint32_t trx_index, bts::wallet::wallet_ptr w);
      
      void execute( chain_database_ptr blockchain, uint32_t block_num, const pending_chain_state_ptr& pending_state );

   private:
      std::unordered_map<int, v8_game_engine_ptr > _engines;
   };
} } // bts:game
