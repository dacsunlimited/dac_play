#pragma once

#include <bts/blockchain/exceptions.hpp>
#include <bts/blockchain/games.hpp>
#include <bts/blockchain/transaction_evaluation_state.hpp>

namespace bts { namespace blockchain {

   /**
    * @class game_factory
    *
    *  Enables polymorphic creation and serialization of operation objects in
    *  an manner that can be extended by derived chains.
    */
   class game_factory
   {
       public:
          static game_factory& instance();
          class game_converter_base
          {
             public:
                  virtual ~game_converter_base(){};
                virtual void to_variant( const bts::blockchain::game& in, fc::variant& out ) = 0;
                virtual void from_variant( const fc::variant& in, bts::blockchain::game& out ) = 0;
                  virtual void evaluate( transaction_evaluation_state& eval_state, const game& game ) = 0;
          };

          template<typename GameType>
          class game_converter : public game_converter_base
          {
             public:
              virtual void to_variant( const bts::blockchain::game& in, fc::variant& output )
                  { try {
                     FC_ASSERT( in.type == GameType::type );
                     fc::mutable_variant_object obj( "type", in.type );

                     obj[ "data" ] = fc::raw::unpack<GameType>(in.data);

                     output = std::move(obj);
                  } FC_RETHROW_EXCEPTIONS( warn, "" ) }

              virtual void from_variant( const fc::variant& in, bts::blockchain::game& output )
                  { try {
                     auto obj = in.get_object();

                     FC_ASSERT( output.type == GameType::type );
                     output.data = fc::raw::pack( obj["data"].as<GameType>() );
                  } FC_RETHROW_EXCEPTIONS( warn, "type: ${type}", ("type",fc::get_typename<GameType>::name()) ) }

                  virtual void evaluate( transaction_evaluation_state& eval_state, const game& g )
                  { try {
                     g.as<GameType>().evaluate( eval_state );
                  } FC_CAPTURE_AND_RETHROW( (g) ) }
          };

          template<typename GameType>
          void   register_game()
          {
             FC_ASSERT( _converters.find( GameType::type ) == _converters.end(),
                        "Game ID already Registered ${id}", ("id",GameType::type) );
            _converters[GameType::type] = std::make_shared< game_converter<GameType> >();
          }

          void evaluate( transaction_evaluation_state& eval_state, const game& g )
          {
             auto itr = _converters.find( uint8_t(g.type) );
             if( itr == _converters.end() )
                FC_THROW_EXCEPTION( bts::blockchain::unsupported_chain_operation, "", ("game",g) );
             itr->second->evaluate( eval_state, g );
          }

          /// defined in games.cpp
       void to_variant( const bts::blockchain::game& in, fc::variant& output );
          /// defined in games.cpp
       void from_variant( const fc::variant& in, bts::blockchain::game& output );

       private:
          std::unordered_map<int, std::shared_ptr<game_converter_base> > _converters;
   };

} } // bts:blockchain

