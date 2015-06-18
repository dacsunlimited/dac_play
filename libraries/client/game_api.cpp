#include <bts/client/client_impl.hpp>

using namespace bts::client;
using namespace bts::client::detail;

wallet_transaction_record client_impl::game_buy_chips(
        const std::string& from_account_name, const std::string& quantity, const std::string& quantity_symbol )
{
    auto record = _wallet->buy_chips(from_account_name, _chain_db->to_ugly_asset(quantity, quantity_symbol), true);
    _wallet->cache_transaction(record);
    network_broadcast_transaction(record.trx);
    return record;
}

wallet_transaction_record client_impl::game_create( const std::string& game_name,
                                                                  const std::string& issuer_name,
                                                                  const std::string& script_url,
                                                                  const std::string& script_hash,
                                                                  const std::string& description,
                                                                  const fc::variant& public_data /* = fc::json::from_string("null").as<fc::variant>() */ )
{
   auto record = _wallet->create_game( game_name, description, public_data, issuer_name,
                                            script_url, script_hash, true );
   _wallet->cache_transaction( record );
   network_broadcast_transaction( record.trx );
   return record;
}

wallet_transaction_record client_impl::game_update(const string& paying_account, const std::string& game_name, const std::string& script_url, const std::string& script_hash, const std::string& description, const fc::variant& public_data )
{
    auto record = _wallet->update_game( paying_account, game_name, description, public_data,
                                       script_url, script_hash, true );
    _wallet->cache_transaction( record );
    network_broadcast_transaction( record.trx );
    return record;
}

wallet_transaction_record client_impl::game_play(const std::string& game_name, const fc::variant& param )
{
    auto record = _wallet->play_game(game_name, param, true);
    _wallet->cache_transaction(record);
    network_broadcast_transaction(record.trx);
    return record;
}

vector<game_data_record> client_impl::game_list_datas( const string& game_name, uint32_t limit )const
{ try {
    FC_ASSERT( limit > 0 );
    return _chain_db->fetch_game_data_records( game_name, limit );
} FC_CAPTURE_AND_RETHROW( (game_name)(limit) ) }

bts::blockchain::game_status client_impl::game_status(const std::string& game_name) const
{
    auto game_rec = _chain_db->get_game_record( game_name );
    
    FC_ASSERT( game_rec.valid() );
    
    auto s = _chain_db->get_game_status( game_rec->id );
    
    FC_ASSERT( s, "The ${n} game has not yet been initialized.", ("n", game_name));
    
    return *s;
}


std::vector<bts::blockchain::game_status> client_impl::game_list_status() const
{
    return _chain_db->list_game_statuses();
}
