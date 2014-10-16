#include <bts/blockchain/games.hpp>
#include <bts/blockchain/game_factory.hpp>
#include <bts/game/dice_game.hpp>


namespace bts { namespace game {
    
    const game_type_enum dice_game::type = dice_game_type;

    static bool first_chain = []()->bool{
        bts::game::game_factory::instance().register_game<dice_game>();
    
        return true;
    }();

} } // bts:game