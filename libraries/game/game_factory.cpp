#include <bts/game/rule_record.hpp>
#include <bts/game/game_factory.hpp>

#include <fc/io/raw_variant.hpp>
#include <fc/reflect/variant.hpp>

namespace bts { namespace game {

   game_factory& game_factory::instance()
   {
      static std::unique_ptr<game_factory> inst( new game_factory() );
      return *inst;
   }

   void game_factory::to_variant( const bts::game::rule& in, fc::variant& output )
   { try {
      auto converter_itr = _converters.find( in.type );
      FC_ASSERT( converter_itr != _converters.end() );
      converter_itr->second->to_variant( in, output );
   } FC_RETHROW_EXCEPTIONS( warn, "" ) }

   void game_factory::from_variant( const fc::variant& in, bts::game::rule& output )
   { try {
      auto obj = in.get_object();
      output.type = obj["type"].as<uint8_t>();

      auto converter_itr = _converters.find( output.type );
      FC_ASSERT( converter_itr != _converters.end() );
      converter_itr->second->from_variant( in, output );
   } FC_RETHROW_EXCEPTIONS( warn, "", ("in",in) ) }
    
    bool game_factory::scan( const rule& g, wallet_transaction_record& trx_rec, bts::wallet::wallet_ptr w )
    {
        auto converter_itr = _converters.find( g.type );
        FC_ASSERT( converter_itr != _converters.end() );
        return converter_itr->second->scan( g, trx_rec, w );
    }

} } // bts::blockchain

namespace fc {
   void to_variant( const bts::game::rule& var,  variant& vo )
   {
      bts::game::game_factory::instance().to_variant( var, vo );
   }

   void from_variant( const variant& var,  bts::game::rule& vo )
   {
      bts::game::game_factory::instance().from_variant( var, vo );
   }
}
