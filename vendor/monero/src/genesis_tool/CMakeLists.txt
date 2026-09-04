# Copyright (c) 2026, The RandomLite (XRL) Project
#
# Build for the deterministic genesis block generator.

set(genesis_tool_sources
  genesis_tool.cpp)

monero_add_executable(xrl-genesis-tool
  ${genesis_tool_sources})

target_link_libraries(xrl-genesis-tool
  PRIVATE
    cryptonote_core
    cncrypto
    common
    ${Boost_PROGRAM_OPTIONS_LIBRARY}
    ${Boost_FILESYSTEM_LIBRARY}
    ${Boost_THREAD_LIBRARY}
    ${CMAKE_THREAD_LIBS_INIT}
    ${EXTRA_LIBRARIES})

set_property(TARGET xrl-genesis-tool
  PROPERTY
    OUTPUT_NAME "xrl-genesis-tool")
