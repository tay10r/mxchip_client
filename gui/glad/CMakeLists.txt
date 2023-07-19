cmake_minimum_required(VERSION 3.14.7)

add_library(glad
  include/glad/glad.h
  include/GLES2/gl2.h
  include/KHR/khrplatform.h
  src/glad.c)

target_include_directories(glad PUBLIC include)
