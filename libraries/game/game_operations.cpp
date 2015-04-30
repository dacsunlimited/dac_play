#include <bts/game/game_operations.hpp>
#include <bts/blockchain/chain_interface.hpp>
#include <bts/blockchain/exceptions.hpp>
#include <bts/game/client.hpp>
#include <bts/game/v8_game.hpp>

namespace bts { namespace game {
    using namespace bts::blockchain;

    /**
     *  @note in this method we are using 'this->' to refer to member variables for
     *  clarity.
     */
    void game_operation::evaluate( transaction_evaluation_state& eval_state ) const
    { try {
        // TODO: shoud provide with game_input
        bts::game::client::get_current().get_v8_engine(input.game_id)->evaluate( eval_state );

    } FC_CAPTURE_AND_RETHROW( (*this) ) }

} } // bts::game
