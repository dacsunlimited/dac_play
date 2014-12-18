#pragma once

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
    * @class rule_factory
    *
    *  Enables polymorphic creation and serialization of rule objects in
    *  an manner that can be extended by derived chains.
    */
   class rule_factory
   {
       public:
          static rule_factory& instance();
          class rule_converter_base
          {
             public:
                virtual ~rule_converter_base(){};
                virtual void to_variant( const bts::game::rule& in, fc::variant& out ) = 0;
                virtual void from_variant( const fc::variant& in, bts::game::rule& out ) = 0;
                virtual void evaluate( transaction_evaluation_state& eval_state, const rule& game ) = 0;
              
                virtual wallet_transaction_record play( chain_database_ptr blockchain, bts::wallet::wallet_ptr w, const variant& var, bool sign ) = 0;
              
                virtual bool scan( const rule& game, wallet_transaction_record& trx_rec, bts::wallet::wallet_ptr w ) = 0;
          };

          template<typename RuleType>
          class rule_converter : public rule_converter_base
          {
             public:
              virtual void to_variant( const bts::game::rule& in, fc::variant& output )
                  { try {
                     FC_ASSERT( in.type == RuleType::type );
                     fc::mutable_variant_object obj( "type", in.type );

                     obj[ "data" ] = fc::raw::unpack<RuleType>(in.data);

                     output = std::move(obj);
                  } FC_RETHROW_EXCEPTIONS( warn, "" ) }

              virtual void from_variant( const fc::variant& in, bts::game::rule& output )
                  { try {
                     auto obj = in.get_object();

                     FC_ASSERT( output.type == RuleType::type );
                     output.data = fc::raw::pack( obj["data"].as<RuleType>() );
                  } FC_RETHROW_EXCEPTIONS( warn, "type: ${type}", ("type",fc::get_typename<RuleType>::name()) ) }

                  virtual void evaluate( transaction_evaluation_state& eval_state, const rule& g )
                  { try {
                     g.as<RuleType>().evaluate( eval_state );
                  } FC_CAPTURE_AND_RETHROW( (g) ) }
              
              virtual wallet_transaction_record play( chain_database_ptr blockchain, bts::wallet::wallet_ptr w, const variant& var, bool sign )
              { try {
                  return RuleType::play(blockchain, w, var, sign);
              } FC_CAPTURE_AND_RETHROW( (var) ) }
              
              virtual bool scan( const rule& g, wallet_transaction_record& trx_rec, bts::wallet::wallet_ptr w )
              { try {
                  return g.as<RuleType>().scan(trx_rec, w);
              } FC_CAPTURE_AND_RETHROW( (g) ) }
          };

          template<typename RuleType>
          void   register_rule()
          {
             FC_ASSERT( _converters.find( RuleType::type ) == _converters.end(),
                        "Rule ID already Registered ${id}", ("id", RuleType::type) );
            _converters[RuleType::type] = std::make_shared< rule_converter<RuleType> >();

            game_executors::instance().register_game_executor(
                std::function<void( chain_database_ptr, uint32_t, const pending_chain_state_ptr&)>(RuleType::execute)
            );
          }

          void evaluate( transaction_evaluation_state& eval_state, const rule& g )
          {
             auto itr = _converters.find( uint8_t(g.type) );
             if( itr == _converters.end() )
                FC_THROW_EXCEPTION( bts::blockchain::unsupported_chain_operation, "", ("rule",g) );
             itr->second->evaluate( eval_state, g );
          }
       
          wallet_transaction_record play(const int& game_id, chain_database_ptr blockchain, bts::wallet::wallet_ptr w, const variant& var, bool sign)
          {
              auto itr = _converters.find( uint8_t(game_id) );
              if( itr == _converters.end() )
                  FC_THROW_EXCEPTION( bts::blockchain::unsupported_chain_operation, "", ("game_id", game_id) );
              
              return itr->second->play(blockchain, w, var, sign);
          }

          /// defined in games.cpp
          void to_variant( const bts::game::rule& in, fc::variant& output );
          /// defined in games.cpp
          void from_variant( const fc::variant& in, bts::game::rule& output );
       
          bool scan( const rule& g, wallet_transaction_record& trx_rec, bts::wallet::wallet_ptr w );

       private:
          std::unordered_map<int, std::shared_ptr<rule_converter_base> > _converters;
   };

} } // bts:game

