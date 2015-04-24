#include <bts/utilities/combinatorics.hpp>

#include <fc/io/raw.hpp>

namespace bts { namespace utilities {
    
    uint64_t cnr(uint16_t n,uint16_t r)
    {
        if ((n - r) < r) r = n - r;
        
        uint64_t c[r+1];
        uint16_t i,j;
        
        for( i=0; i<=r; i++ ) c[i] = 1; /*Initialize*/
        
        for( i=1; i<=n-r; i++)
            for( j=1; j<=r; j++ )
            {
                c[j] += c[j-1];
            }
        
        return c[r];
    }

    uint64_t ranking(const combination& c)
    {
        std::vector<uint16_t> v(c);
        std::sort(v.begin(), v.end());
        uint64_t n = 0;
        // sum of C(v[k - 1], k)
        for (size_t i = 1; i <= v.size(); i ++) {
            n += cnr(v[i - 1], i);
        }
        return n;
    }
    
    combination unranking(uint64_t num, uint16_t k, uint16_t n)
    {
        std::vector<uint16_t> c;
        uint16_t max = n;
        
        for (uint16_t i = k; i >= 1; i--)
        {
            if (num <= 0) {
                c.push_back(i-1);
            } else {
                for (; max >= 1;)
                {
                    uint64_t c_max_i = cnr(max, i);
                    if (num >= c_max_i) {
                        c.push_back(max);
                        
                        num -=  c_max_i;
                        max --;
                        break;
                    } else {
                        max --;
                    }
                }
            }
        }
        
        std::sort(c.begin(), c.end());
        
        return c;
    }

} } // end namespace bts::utilities

