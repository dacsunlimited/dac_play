/** @file bts/blockchain/fork_blocks.hpp
 *  @brief Defines global block number constants for when hardforks take effect
 */

#pragma once

#include <stdint.h>
#include <vector>

#define PLS_EXPECTED_CHAIN_ID       digest_type( "85d334301fea511328af381349e357e4c5b628859af98de08fbb8639262c29f4" )
#define PLS_DESIRED_CHAIN_ID        digest_type( "1b7e602523026b6a51acb884d055e04ab4e89ab864060a63bcf6f664df2fcb19" )

#define PLS_V0_0_3_FORK_BLOCK_NUM     4000
#define PLS_V0_1_0_FORK_BLOCK_NUM   574000
#define PLS_V0_1_2_FORK_BLOCK_NUM   640000
#define PLS_V0_1_4_FORK_BLOCK_NUM   829000
