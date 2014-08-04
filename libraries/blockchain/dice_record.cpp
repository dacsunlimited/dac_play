#include <bts/blockchain/dice_record.hpp>

namespace bts { namespace blockchain {
    
    address dice_record::owner()const
    {
        if( condition.type == withdraw_signature_type )
            return condition.as<withdraw_with_signature>().owner;
        return address();
    }
    
    bool dice_record::is_null()const
    {
        return id == dice_id_type();
    }
    
    dice_record dice_record::make_null()const
    {
        dice_record cpy(*this);
        cpy.id = dice_id_type();
        return cpy;
    }
    
}} // bts::blockchain