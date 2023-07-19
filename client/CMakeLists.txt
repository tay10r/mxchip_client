cmake_minimum_required(VERSION 3.14.7)

if(POLICY CMP0135)
  cmake_policy(SET CMP0135 NEW)
endif()

include(FetchContent)

set(LIBUV_BUILD_SHARED OFF CACHE BOOL "" FORCE)
FetchContent_Declare(uv URL "https://github.com/libuv/libuv/archive/refs/tags/v1.46.0.zip")
FetchContent_MakeAvailable(uv)

add_library(mxchip_client
  include/mxchip_client/client.h
  src/client.c)

target_compile_definitions(mxchip_client PRIVATE c_std_99)

if(CMAKE_COMPILER_IS_GNUCC AND NOT MXCHIP_CLIENT_NO_WARNINGS)
  target_compile_options(mxchip_client PRIVATE -Wall -Wextra -Werror -Wfatal-errors)
endif()

target_include_directories(mxchip_client
  PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/include")

target_link_libraries(mxchip_client PUBLIC uv_a)

if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
  add_executable(mxchip_client_example example/main.c)
  target_link_libraries(mxchip_client_example PRIVATE mxchip_client)
  set_target_properties(mxchip_client_example PROPERTIES OUTPUT_NAME example)
endif()
