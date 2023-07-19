cmake_minimum_required(VERSION 3.14.7)

project(mxchip_client)

option(MXCHIP_CLIENT_GUI "Whether or not to build the UI." OFF)

add_subdirectory(client)

if(MXCHIP_CLIENT_GUI)
  add_subdirectory(gui)
endif(MXCHIP_CLIENT_GUI)
