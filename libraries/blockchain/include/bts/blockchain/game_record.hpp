#pragma once
#include <bts/blockchain/types.hpp>
#include <bts/blockchain/withdraw_types.hpp>
#include <bts/blockchain/transaction.hpp>

namespace bts { namespace blockchain {
   struct game_record;
   typedef fc::optional<game_record> ogame_record;
   
    class chain_interface;
    struct game_record
    {
        enum
        {
            god_issuer_id     =  0,
            null_issuer_id    = -1
        };
        
        game_record make_null()const;
        bool is_null()const               { return issuer_account_id == null_issuer_id; };
        
        bool is_user_issued()const        { return issuer_account_id > god_issuer_id;};
        
        game_id_type        id;
        std::string         symbol;
        std::string         name;
        std::string         description;
        fc::variant         public_data;
        account_id_type     issuer_account_id;
        asset_id_type       asset_id;
        rule_id_type        rule_id;
        fc::time_point_sec  registration_date;
        fc::time_point_sec  last_update;
        
        /** reserved for future extensions */
        vector<char>        reserved;
       
        void sanity_check( const chain_interface& )const;
        static ogame_record lookup( const chain_interface&, const game_id_type );
        static ogame_record lookup( const chain_interface&, const string& );
        static void store( chain_interface&, const game_id_type, const game_record& );
        static void remove( chain_interface&, const game_id_type );
   };
   
   class game_db_interface
   {
      friend struct game_record;
      
      virtual ogame_record game_lookup_by_id( const game_id_type )const = 0;
      virtual ogame_record game_lookup_by_symbol( const string& )const = 0;
      
      virtual void game_insert_into_id_map( const game_id_type, const game_record& ) = 0;
      virtual void game_insert_into_symbol_map( const string&, const game_id_type ) = 0;
      
      virtual void game_erase_from_id_map( const game_id_type ) = 0;
      virtual void game_erase_from_symbol_map( const string& ) = 0;
   };
    
    struct rule_data_record
    {
        rule_data_record():type(0){}
        
        template<typename RecordType>
        rule_data_record( const RecordType& rec )
        :type( int(RecordType::type) ),data(rec)
        { }
        
        template<typename RecordType>
        RecordType as()const;
        
        // TODO: Figure out what following index used for
        int32_t get_rule_data_index()const
        { try {
            FC_ASSERT( data.is_object() );
            FC_ASSERT( data.get_object().contains( "index" ) );
            return data.get_object()["index"].as<int32_t>();
        } FC_RETHROW_EXCEPTIONS( warn, "" ) }
        
        bool is_null()const
        {
            return type == 0;
        }
        
        rule_data_record make_null()const
        {
            rule_data_record cpy(*this);
            cpy.type = 0;
            return cpy;
        }
        
        uint8_t                                          type;
        fc::variant                                      data;
    };
    
    struct rule_result_transaction
    {
        rule_result_transaction():type(0){}
        
        template<typename RecordType>
        rule_result_transaction( const RecordType& rec )
        :type( int(RecordType::type) ),data(rec)
        { }
        
        template<typename RecordType>
        RecordType as()const;
        
        uint8_t                                          type;
        fc::variant                                      data;
    };
    
    typedef fc::optional<rule_data_record> orule_data_record;
    
} } // bts::blockchain

FC_REFLECT( bts::blockchain::game_record,
           (id)
           (symbol)
           (name)
           (description)
           (public_data)
           (issuer_account_id)
           (asset_id)
           (rule_id)
           (registration_date)
           (last_update)
           )

FC_REFLECT( bts::blockchain::rule_data_record,
           (type)
           (data)
           )
FC_REFLECT_TYPENAME( std::vector<bts::blockchain::rule_result_transaction> )
FC_REFLECT( bts::blockchain::rule_result_transaction,
           (type)(data)
           )

namespace bts { namespace blockchain {
    template<typename RecordType>
    RecordType rule_data_record::as()const
    {
        FC_ASSERT( type == RecordType::type, "",
                  ("type",type));
        
        return data.as<RecordType>();
    }
    
    template<typename RecordType>
    RecordType rule_result_transaction::as()const
    {
        FC_ASSERT( type == RecordType::type, "",
                  ("type",type));
        
        return data.as<RecordType>();
    }
} }
