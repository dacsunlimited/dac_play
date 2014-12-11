#pragma once
#include <bts/blockchain/types.hpp>
#include <bts/blockchain/withdraw_types.hpp>
#include <bts/blockchain/transaction.hpp>

namespace bts { namespace blockchain {
    
    struct generic_game_record
    {
        generic_game_record():type(0){}
        
        template<typename RecordType>
        generic_game_record( const RecordType& rec )
        :type( int(RecordType::type) ),data(rec)
        { }
        
        template<typename RecordType>
        RecordType as()const;
        
        int32_t get_game_record_index()const
        { try {
            FC_ASSERT( data.is_object() );
            FC_ASSERT( data.get_object().contains( "index" ) );
            return data.get_object()["index"].as<int32_t>();
        } FC_RETHROW_EXCEPTIONS( warn, "" ) }
        
        bool is_null()const
        {
            return type == 0;
        }
        
        generic_game_record make_null()const
        {
            generic_game_record cpy(*this);
            cpy.type = 0;
            return cpy;
        }
        
        uint8_t                                          type;
        fc::variant                                      data;
    };
    
    struct game_transaction
    {
        game_transaction(){}
        
        address                                   play_owner;
        address                                   jackpot_owner;
        share_type                                play_amount;
        share_type                                jackpot_received;
        uint32_t                                  odds;
        uint32_t                                  lucky_number;
    };
    
    typedef fc::optional<generic_game_record> ogeneric_game_record;
    
} } // bts::blockchain

FC_REFLECT( bts::blockchain::generic_game_record,
           (type)
           (data)
           )
FC_REFLECT_TYPENAME( std::vector<bts::blockchain::game_transaction> )
FC_REFLECT( bts::blockchain::game_transaction,
           (play_owner)
           (jackpot_owner)
           (play_amount)
           (jackpot_received)
           (odds)
           (lucky_number)
           )

namespace bts { namespace blockchain {
    template<typename RecordType>
    RecordType generic_game_record::as()const
    {
        FC_ASSERT( type == RecordType::type, "",
                  ("type",type));
        
        return data.as<RecordType>();
    }
} }