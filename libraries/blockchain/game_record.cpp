#include <bts/blockchain/game_record.hpp>

namespace bts { namespace blockchain {

game_record game_record::make_null()const
{
    game_record cpy(*this);
    cpy.issuer_account_id = -1;
    return cpy;
}

}} // bts::blockchain
