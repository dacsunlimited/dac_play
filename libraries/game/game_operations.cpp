#include <bts/game/game_operations.hpp>
#include <bts/blockchain/chain_interface.hpp>
#include <bts/blockchain/exceptions.hpp>

namespace bts { namespace game {
    using namespace bts::blockchain;

    /**
     *  @note in this method we are using 'this->' to refer to member variables for
     *  clarity.
     */
    void game_operation::evaluate( transaction_evaluation_state& eval_state ) const
    { try {
        
        //bts::game::client::evaluate( eval_state, input );

    } FC_CAPTURE_AND_RETHROW( (*this) ) }

} } // bts::game
