#include <bts/blockchain/dice_record.hpp>

namespace bts { namespace blockchain {
    
    bool dice_record::is_null()const
    {
        return account_id == -1;
    }
    
    dice_record dice_record::make_null()const
    {
        dice_record cpy(*this);
        cpy.account_id = -1;
        return cpy;
    }
    
}} // bts::blockchain