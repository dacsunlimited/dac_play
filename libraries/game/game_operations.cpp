#include <bts/game/game_operations.hpp>
#include <bts/blockchain/chain_interface.hpp>
#include <bts/blockchain/exceptions.hpp>
#include <bts/game/rule_factory.hpp>

namespace bts { namespace game {
    using namespace bts::blockchain;

    /**
     *  @note in this method we are using 'this->' to refer to member variables for
     *  clarity.
     */
    void game_operation::evaluate( transaction_evaluation_state& eval_state )
    { try {
        
        rule_factory::instance().evaluate( eval_state, rule );

    } FC_CAPTURE_AND_RETHROW( (*this) ) }

} } // bts::game
