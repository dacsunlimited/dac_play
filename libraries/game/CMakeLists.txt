file(GLOB HEADERS "include/bts/game/*.hpp")

set(SOURCES game_operations.cpp rule_factory.cpp client.cpp v8_api.cpp v8_game.cpp)

add_library( bts_game ${SOURCES} ${HEADERS} )

target_link_libraries( bts_game
PUBLIC fc bts_wallet bts_db bts_utilities leveldb bts_api bts_rpc_stubs upnpc-static exlib v8)

target_include_directories( bts_game 
  PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include"
# currently depends on client for definitions, remove this
  PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../client/include"
# Looks like bts_mail and bts_rpc also have cycle deps
          "${CMAKE_CURRENT_SOURCE_DIR}/../rpc/include"
          "${CMAKE_CURRENT_SOURCE_DIR}/../wallet/include"
# later, move upnp include into target_include_directories for upnp
"${CMAKE_CURRENT_SOURCE_DIR}/../../vendor/miniupnp"
"${CMAKE_CURRENT_SOURCE_DIR}/../../vendor/v8-fibjs/v8"
)
