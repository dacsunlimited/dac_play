#include <bts/blockchain/address.hpp>
#include <bts/blockchain/pts_address.hpp>
#include <bts/blockchain/types.hpp>

#include <fc/crypto/elliptic.hpp>
#include <bts/utilities/key_conversion.hpp>
#include <fc/io/json.hpp>
#include <iostream>
#include <fc/exception/exception.hpp>

int main( int argc, char** argv )
{
   if( argc < 2 ) 
      return -1;

   try{
     auto k = bts::blockchain::public_key_type(argv[1]);
     auto addr = bts::blockchain::address(k);
     std::cout << std::string(addr) << "\n";
   } catch (fc::exception e) {
     std::cout << e.to_detail_string();
   }

   return 0;
}
