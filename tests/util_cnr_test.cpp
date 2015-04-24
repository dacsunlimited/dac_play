#define BOOST_TEST_MODULE CNRTests

#include <boost/test/unit_test.hpp>

#include <bts/utilities/combinatorics.hpp>
#include <fc/exception/exception.hpp>
#include <fc/log/logger.hpp>
#include <fc/thread/thread.hpp>
#include <fc/filesystem.hpp>
#include <fc/network/ip.hpp>
#include <fc/io/json.hpp>

#include <iostream>

BOOST_AUTO_TEST_CASE( util_cnr )
{
    BOOST_CHECK_EQUAL(bts::utilities::cnr(5, 2), 10);
    
    auto res = bts::utilities::unranking( bts::utilities::cnr(5, 2) - 1, 2, 5);
    
    std::cout << res.size() << " "<< res[0] << " " << res[1] << "\n";
}
