#pragma once

#include <fc/io/enum_type.hpp>
#include <fc/io/raw.hpp>
#include <fc/reflect/reflect.hpp>

#include <bts/blockchain/types.hpp>
#include <bts/blockchain/withdraw_types.hpp>
#include <bts/blockchain/transaction.hpp>

/**
 *  The C keyword 'not' is NOT friendly on VC++ but we still want to use
 *  it for readability, so we will have the pre-processor convert it to the
 *  more traditional form.  The goal here is to make the understanding of
 *  the validation logic as english-like as possible.
 */
#define NOT !

namespace bts { namespace game {
    using namespace bts::blockchain;
    
    enum game_record_type_enum
    {
        null_record_type     = 0,
        dice_record_type     = 1
    };
    
    template<game_record_type_enum RecordType>
    struct game_base_record
    {
        enum { type  = RecordType };
        
        game_base_record( int32_t idx = 0 ):game_record_index(idx){}
        int32_t game_record_index;
    };
    
    template<typename RecordTypeName, game_record_type_enum RecordTypeNumber>
    struct game_record : public game_base_record<RecordTypeNumber>, public RecordTypeName
    {
        game_record(){}
        game_record( const RecordTypeName& rec, int32_t game_record_index = 0 )
        :game_base_record<RecordTypeNumber>(game_record_index),RecordTypeName(rec){}
    };
    
    struct dice_data
    {
        dice_data(){}
        
        dice_id_type        id = dice_id_type();
        address             owner;
        share_type          amount;
        uint32_t            odds;
        uint32_t            guess;
    };
    
    typedef game_record< dice_data,                dice_record_type >  game_dice_record;
    
} } // bts::game

FC_REFLECT_ENUM( bts::game::game_record_type_enum,
                (null_record_type)
                (dice_record_type)
                )

FC_REFLECT( bts::game::dice_data,
           (id)(owner)(amount)(odds)(guess)
           )

/**
 *  Implement generic reflection for wallet record types
 */
namespace fc {
    
    template<typename T, bts::game::game_record_type_enum N>
    struct get_typename< bts::game::game_record<T,N> >
    {
        static const char* name()
        {
            static std::string _name =  get_typename<T>::name() + std::string("_record");
            return _name.c_str();
        }
    };
    
    template<typename Type, bts::game::game_record_type_enum N>
    struct reflector<bts::game::game_record<Type,N>>
    {
        typedef bts::game::game_record<Type,N>  type;
        typedef fc::true_type is_defined;
        typedef fc::false_type is_enum;
        enum member_count_enum {
            local_member_count = 1,
            total_member_count = local_member_count + reflector<Type>::total_member_count
        };
        
        template<typename Visitor>
        static void visit( const Visitor& visitor )
        {
            {
                typedef decltype(((bts::game::game_base_record<N>*)nullptr)->game_record_index) member_type;
                visitor.TEMPLATE operator()<member_type,bts::game::game_base_record<N>,&bts::game::game_base_record<N>::game_record_index>( "index" );
            }
            
            fc::reflector<Type>::visit( visitor );
        }
    };
} // namespace fc
