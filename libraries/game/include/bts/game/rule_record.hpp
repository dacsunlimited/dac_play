#pragma once

#include <fc/io/enum_type.hpp>
#include <fc/io/raw.hpp>
#include <fc/reflect/reflect.hpp>

/**
 *  The C keyword 'not' is NOT friendly on VC++ but we still want to use
 *  it for readability, so we will have the pre-processor convert it to the
 *  more traditional form.  The goal here is to make the understanding of
 *  the validation logic as english-like as possible.
 */
#define NOT !

#define null_rule_type  0
#define dice_rule_type  1

namespace bts { namespace game {
    
    /**
     *  A poly-morphic operator that modifies the blockchain database
     *  is some manner.
     */
    struct rule
    {
        rule():type(null_rule_type){}
        
        rule( const rule& o )
        :type(o.type),data(o.data){}
        
        rule( rule&& o )
        :type(o.type),data(std::move(o.data)){}
        
        template<typename RuleType>
        rule( const RuleType& t )
        {
            type = RuleType::type;
            data = fc::raw::pack( t );
        }
        
        template<typename RuleType>
        RuleType as()const
        {
            FC_ASSERT( type == RuleType::type, "", ("type",type)("RuleType",RuleType::type) );
            return fc::raw::unpack<RuleType>(data);
        }
        
        rule& operator=( const rule& o )
        {
            if( this == &o ) return *this;
            type = o.type;
            data = o.data;
            return *this;
        }
        
        rule& operator=( rule&& o )
        {
            if( this == &o ) return *this;
            type = o.type;
            data = std::move(o.data);
            return *this;
        }
        
        uint8_t type;
        std::vector<char> data;
    };
    
    template<uint8_t RuleType>
    struct rule_base_record
    {
        enum { type  = RuleType };
        
        rule_base_record( int32_t idx = 0 ):rule_index(idx){}
        int32_t rule_index;
    };
    
    template<typename RuleTypeName, uint8_t RuleTypeNumber>
    struct rule_record : public rule_base_record<RuleTypeNumber>, public RuleTypeName
    {
        rule_record(){}
        rule_record( const RuleTypeName& rec, int32_t rule_index = 0 )
        :rule_base_record<RuleTypeNumber>(rule_index),RuleTypeName(rec){}
    };
    
} } // bts::game

FC_REFLECT( bts::game::rule, (type)(data) )

/**
 *  Implement generic reflection for wallet record types
 */
namespace fc {
    void to_variant( const bts::game::rule& var,  variant& vo );
    void from_variant( const variant& var,  bts::game::rule& vo );
    
    template<typename T, uint8_t N>
    struct get_typename< bts::game::rule_record<T,N> >
    {
        static const char* name()
        {
            static std::string _name =  get_typename<T>::name() + std::string("_record");
            return _name.c_str();
        }
    };
    
    template<typename Type, uint8_t N>
    struct reflector<bts::game::rule_record<Type,N>>
    {
        typedef bts::game::rule_record<Type,N>  type;
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
                typedef decltype(((bts::game::rule_base_record<N>*)nullptr)->rule_index) member_type;
                visitor.TEMPLATE operator()<member_type,bts::game::rule_base_record<N>,&bts::game::rule_base_record<N>::rule_index>( "index" );
            }
            
            fc::reflector<Type>::visit( visitor );
        }
    };
} // namespace fc
