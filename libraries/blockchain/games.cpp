#include <bts/blockchain/games.hpp>
#include <bts/blockchain/game_factory.hpp>

#include <fc/io/raw_variant.hpp>
#include <fc/reflect/variant.hpp>

namespace bts { namespace blockchain {

   game_factory& game_factory::instance()
   {
      static std::unique_ptr<game_factory> inst( new game_factory() );
      return *inst;
   }

   void game_factory::to_variant( const bts::blockchain::game& in, fc::variant& output )
   { try {
      auto converter_itr = _converters.find( in.type.value );
      FC_ASSERT( converter_itr != _converters.end() );
      converter_itr->second->to_variant( in, output );
   } FC_RETHROW_EXCEPTIONS( warn, "" ) }

   void game_factory::from_variant( const fc::variant& in, bts::blockchain::game& output )
   { try {
      auto obj = in.get_object();
      output.type = obj["type"].as<game_type_enum>();

      auto converter_itr = _converters.find( output.type.value );
      FC_ASSERT( converter_itr != _converters.end() );
      converter_itr->second->from_variant( in, output );
   } FC_RETHROW_EXCEPTIONS( warn, "", ("in",in) ) }

} } // bts::blockchain

namespace fc {
   void to_variant( const bts::blockchain::game& var,  variant& vo )
   {
      bts::blockchain::game_factory::instance().to_variant( var, vo );
   }

   void from_variant( const variant& var,  bts::blockchain::game& vo )
   {
      bts::blockchain::game_factory::instance().from_variant( var, vo );
   }
}
