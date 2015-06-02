#include <bts/utilities/combinatorics.hpp>

#include <fc/io/raw.hpp>

namespace bts { namespace utilities {
    
    uint64_t cnr(uint16_t n,uint16_t r)
    {
        assert(r>=0 && n>= r);
        
        if ((n - r) < r) r = n - r; // for less space
        
        uint64_t c[r+1];
        uint16_t i,j;

        
        /*
         Below method is really hard to understand at first glance,
         so I added an example(n=5,r=2) here. Personally I prefer to use a two-dimension matrix which need more space.
         
         
         [1,0]     [1,1]      [2,2]
         [2,0]     [2,1]      [3,2]
         [3,0]     [3,1]      [4,2]
         [4,0]     [4,1]      [5,2]
           
           |        |          |
          
         c[0]       c[1]       c[2]
         
         Note that each column(i) is stored in array element c[i].
         
         
         In init, we init for [1->4,0] ,[1,1], [2,2] which equals 1.                    (1)
         In the first loop when i=1 and j= (1->r), we compute [2,1] , [3,2] in turn.    (2)
         And we need to repeat step(2) for (n-r) times to reach [5,2].
        
         
         The algo behind this is C[n,r] = C[n-1, r] + C[n, r-1],
         which means we can split the choose-r-in-n problem into 2 smaller problems
            1) choose a specific one, and choose-(r-1)-in-(n-1)
            2) not choose a specific one, and choose-r-in-(n-1)
         
         */
        
 
        for( i=0; i<=r; i++ ) c[i] = 1; /*Initialize*/
        
        for( i=1; i<=n-r; i++)
        {
            for( j=1; j<=r; j++ )
            {
                c[j] += c[j-1];
            }
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

