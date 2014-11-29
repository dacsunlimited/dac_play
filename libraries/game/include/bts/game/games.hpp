#pragma once

#include <fc/io/enum_type.hpp>
#include <fc/io/raw.hpp>
#include <fc/reflect/reflect.hpp>
#include <bts/blockchain/address.hpp>
#include <bts/blockchain/types.hpp>
#include <bts/blockchain/withdraw_types.hpp>

/**
 *  The C keyword 'not' is NOT friendly on VC++ but we still want to use
 *  it for readability, so we will have the pre-processor convert it to the
 *  more traditional form.  The goal here is to make the understanding of
 *  the validation logic as english-like as possible.
 */
#define NOT !

namespace bts { namespace game {
    
    enum game_type_enum
    {
        null_game_type                  = 0,
        
        dice_game_type                  = 1
    };
    
    /**
     *  A poly-morphic operator that modifies the blockchain database
     *  is some manner.
     */
    struct game
    {
        game():type(null_game_type){}
        
        game( const game& o )
        :type(o.type),data(o.data){}
        
        game( game&& o )
        :type(o.type),data(std::move(o.data)){}
        
        template<typename GameType>
        game( const GameType& t )
        {
            type = GameType::type;
            data = fc::raw::pack( t );
        }
        
        template<typename GameType>
        GameType as()const
        {
            FC_ASSERT( (game_type_enum)type == GameType::type, "", ("type",type)("GameType",GameType::type) );
            return fc::raw::unpack<GameType>(data);
        }
        
        game& operator=( const game& o )
        {
            if( this == &o ) return *this;
            type = o.type;
            data = o.data;
            return *this;
        }
        
        game& operator=( game&& o )
        {
            if( this == &o ) return *this;
            type = o.type;
            data = std::move(o.data);
            return *this;
        }
        
        fc::enum_type<uint8_t,game_type_enum> type;
        std::vector<char> data;
    };
} } // bts::game

FC_REFLECT_ENUM( bts::game::game_type_enum,
                (null_game_type)
                (dice_game_type)
                )

FC_REFLECT( bts::game::game, (type)(data) )

namespace fc {
    void to_variant( const bts::game::game& var,  variant& vo );
    void from_variant( const variant& var,  bts::game::game& vo );
}
