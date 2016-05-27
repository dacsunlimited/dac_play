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
            god_owner_id     =  0,
            null_owner_id    = -1
        };
        
        game_record make_null()const;
        bool is_null()const               { return owner_account_id == null_owner_id; };
        
        game_id_type        id;
        std::string         name;
        std::string         description;
        fc::variant         public_data;
        account_id_type     owner_account_id;
        std::string         script_code;
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
      virtual ogame_record game_lookup_by_name( const string& )const = 0;
      
      virtual void game_insert_into_id_map( const game_id_type, const game_record& ) = 0;
      virtual void game_insert_into_name_map( const string&, const game_id_type ) = 0;
      
      virtual void game_erase_from_id_map( const game_id_type ) = 0;
      virtual void game_erase_from_name_map( const string& ) = 0;
   };
    
    struct game_data_index
    {
        game_id_type        game_id;
        data_id_type        data_id;
        
        friend bool operator < ( const game_data_index& a, const game_data_index& b )
        {
            return std::tie( a.game_id, a.data_id ) < std::tie( b.game_id, b.data_id );
        }
        
        friend bool operator == ( const game_data_index& a, const game_data_index& b )
        {
            return std::tie( a.game_id, a.data_id ) == std::tie( b.game_id, b.data_id );
        }
    };
    
    struct game_data_record
    {
        game_data_record():game_id(0){}
        
        data_id_type get_game_data_index()const
        { try {
            FC_ASSERT( data.is_object() );
            FC_ASSERT( data.get_object().contains( "index" ) );
            return data.get_object()["index"].as<data_id_type>();
        } FC_RETHROW_EXCEPTIONS( warn, "" ) }
        
        bool is_null()const
        {
            return game_id == 0;
        }
        
        game_data_record make_null()const
        {
            game_data_record cpy(*this);
            cpy.game_id = 0;
            return cpy;
        }
        
        game_id_type                                     game_id;
        fc::variant                                      data;
    };
    
    struct game_result_transaction
    {
        game_result_transaction():game_id(0){}
        
        game_id_type                                     game_id;
        fc::variant                                      data;
    };
    
    typedef fc::optional<game_data_record> ogame_data_record;
    
    struct game_status
    {
        game_status(){} // Null case
        game_status( game_id_type id )
        :game_id(id)
        {
        }
        
        bool is_null()const { return !last_error.valid(); }
        
        game_id_type             game_id;
        uint32_t                 block_number;
        optional<fc::exception>  last_error;
    };
    typedef optional<game_status> ogame_status;
    
} } // bts::blockchain

FC_REFLECT( bts::blockchain::game_record,
           (id)
           (name)
           (description)
           (public_data)
           (owner_account_id)
           (script_code)
           (registration_date)
           (last_update)
           )
FC_REFLECT( bts::blockchain::game_data_index, (game_id)(data_id) )
FC_REFLECT( bts::blockchain::game_data_record,
           (game_id)
           (data)
           )
FC_REFLECT_TYPENAME( std::vector<bts::blockchain::game_result_transaction> )
FC_REFLECT( bts::blockchain::game_result_transaction,
           (game_id)(data)
           )
FC_REFLECT( bts::blockchain::game_status, (game_id)(block_number)(last_error) )
