#include <bts/game/rule_record.hpp>
#include <bts/game/game_factory.hpp>
#include <bts/game/dice_game.hpp>
#include <bts/game/game_operations.hpp>
#include <bts/blockchain/operation_factory.hpp>


namespace bts { namespace game {
    using namespace bts::blockchain;
    
    const uint8_t dice_game::type = dice_rule_type;
    
    const operation_type_enum game_operation::type              = game_op_type;

    static bool first_chain = []()->bool{
        bts::game::game_factory::instance().register_rule<dice_game>();
        
        bts::blockchain::operation_factory::instance().register_operation<game_operation>();
    
        return true;
    }();

} } // bts:game