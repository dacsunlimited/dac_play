#pragma once

#include <algorithm>
#include <bitset>
#include <functional>
#include <vector>
#include <iostream>
#include <cstdint>

namespace bts { namespace utilities {
    typedef std::vector<uint16_t>                  combination;
    
    /*
     * http://my.oschina.net/psaux0/blog/214013
     */
    uint64_t cnr(uint16_t,uint16_t);
    
    uint64_t ranking(const combination& c);
    
    combination unranking(uint64_t num, uint16_t k, uint16_t n);

} } // end namespace bts::utilities
