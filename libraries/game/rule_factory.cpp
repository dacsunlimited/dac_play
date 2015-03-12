#include <bts/game/rule_record.hpp>
#include <bts/game/rule_factory.hpp>

#include <fc/io/raw_variant.hpp>
#include <fc/reflect/variant.hpp>

#include <bts/game/v8_game.hpp>

namespace bts { namespace game {

   rule_factory& rule_factory::instance()
   {
      static std::unique_ptr<rule_factory> inst( new rule_factory() );
      return *inst;
   }

   void rule_factory::to_variant( const bts::game::rule& in, fc::variant& output )
   {
      /*
       try {
       FC_ASSERT( in.type == RuleType::type );
       fc::mutable_variant_object obj( "type", in.type );
       
       obj[ "data" ] = fc::raw::unpack<RuleType>(in.data);
       
       output = std::move(obj);
       } FC_RETHROW_EXCEPTIONS( warn, "" )
       
      try {
      auto converter_itr = _converters.find( in.type );
      FC_ASSERT( converter_itr != _converters.end() );
      converter_itr->second->to_variant( in, output );
       
   } FC_RETHROW_EXCEPTIONS( warn, "" ) */}

   void rule_factory::from_variant( const fc::variant& in, bts::game::rule& output )
   {
      /*
       try {
       auto obj = in.get_object();
       
       FC_ASSERT( output.type == RuleType::type );
       output.data = fc::raw::pack( obj["data"].as<RuleType>() );
       } FC_RETHROW_EXCEPTIONS( warn, "type: ${type}", ("type",fc::get_typename<RuleType>::name()) )
       
      try {
      auto obj = in.get_object();
      output.type = obj["type"].as<uint8_t>();

      auto converter_itr = _converters.find( output.type );
      FC_ASSERT( converter_itr != _converters.end() );
      converter_itr->second->from_variant( in, output );
   } FC_RETHROW_EXCEPTIONS( warn, "", ("in",in) ) */}
    
    bool rule_factory::scan( const rule& g, wallet_transaction_record& trx_rec, bts::wallet::wallet_ptr w )
    {
        auto converter_itr = _engines.find( g.type );
        FC_ASSERT( converter_itr != _engines.end() );
        return converter_itr->second->scan( trx_rec, w );
    }
    
    bool rule_factory::scan_result( const game_result_transaction& rtrx,
                     uint32_t block_num,
                     const time_point_sec& block_time,
                     const uint32_t trx_index, bts::wallet::wallet_ptr w)
    {
        auto converter_itr = _engines.find( rtrx.type );
        FC_ASSERT( converter_itr != _engines.end() );
        return converter_itr->second->scan_result( rtrx, block_num, block_time, trx_index, w );
    }
    
    void rule_factory::execute( chain_database_ptr blockchain, uint32_t block_num, const pending_chain_state_ptr& pending_state )
    {
        // TODO: FIXME
        auto games = blockchain->get_games("", -1);
        
        for ( const auto& g : games)
        {
            auto converter_itr = _engines.find( g.id );
            
            if ( converter_itr != _engines.end() )
            {
                converter_itr->second->execute(blockchain, block_num, pending_state);
            }
        }
    }

} } // bts::game

namespace fc {
   void to_variant( const bts::game::rule& var,  variant& vo )
   {
      bts::game::rule_factory::instance().to_variant( var, vo );
   }

   void from_variant( const variant& var,  bts::game::rule& vo )
   {
      bts::game::rule_factory::instance().from_variant( var, vo );
   }
}
