#include <bts/game/rule_record.hpp>
#include <bts/game/rule_factory.hpp>
#include <bts/game/dice_rule.hpp>
#include <bts/game/game_operations.hpp>
#include <bts/blockchain/operation_factory.hpp>


namespace bts { namespace game {
    using namespace bts::blockchain;
    
    const uint8_t dice_rule::type = dice_rule_type;
    
    const operation_type_enum game_operation::type              = game_op_type;

    static bool first_chain = []()->bool{
        bts::game::rule_factory::instance().register_rule<dice_rule>();
        
        bts::blockchain::operation_factory::instance().register_operation<game_operation>();
        
        game_executors::instance().register_game_executor(
                                                          std::function<void( chain_database_ptr, uint32_t, const pending_chain_state_ptr&)>(std::bind(&rule_factory::execute, rule_factory::instance(), std::placeholders::_1, std::placeholders::_2, std::placeholders::_3))
                                                          );
    
        return true;
    }();

} } // bts:game