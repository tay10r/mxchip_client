cmake_minimum_required(VERSION 3.14.7)

if(NOT TARGET mxchip_client)
  add_subdirectory(../client client)
endif()

if(POLICY CMP0135)
  cmake_policy(SET CMP0135 NEW)
endif()

add_subdirectory(glad)

include(FetchContent)

set(GLFW_INSTALL OFF)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

FetchContent_Declare(glfw URL "https://github.com/glfw/glfw/archive/refs/heads/master.zip")
FetchContent_MakeAvailable(glfw)

FetchContent_Declare(imgui URL "https://github.com/ocornut/imgui/archive/refs/tags/v1.89.7.zip")
FetchContent_MakeAvailable(imgui)
FetchContent_GetProperties(imgui)

add_library(imgui
  "${imgui_SOURCE_DIR}/imgui.h"
  "${imgui_SOURCE_DIR}/imgui_internal.h"
  "${imgui_SOURCE_DIR}/imstb_rectpack.h"
  "${imgui_SOURCE_DIR}/imstb_textedit.h"
  "${imgui_SOURCE_DIR}/imstb_truetype.h"
  "${imgui_SOURCE_DIR}/imgui.cpp"
  "${imgui_SOURCE_DIR}/imgui_draw.cpp"
  "${imgui_SOURCE_DIR}/imgui_tables.cpp"
  "${imgui_SOURCE_DIR}/imgui_widgets.cpp"
  "${imgui_SOURCE_DIR}/misc/cpp/imgui_stdlib.h"
  "${imgui_SOURCE_DIR}/misc/cpp/imgui_stdlib.cpp"
  "${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.h"
  "${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp"
  "${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.h"
  "${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp")

target_include_directories(imgui
  PUBLIC
    "${imgui_SOURCE_DIR}"
    "${imgui_SOURCE_DIR}/misc/cpp"
    "${imgui_SOURCE_DIR}/backends")

target_compile_definitions(imgui
  PUBLIC
    IMGUI_DEFINE_MATH_OPERATORS=1
  PRIVATE
    IMGUI_IMPL_OPENGL_ES2=1)

target_link_libraries(imgui
  PUBLIC
    glfw
    glad)

FetchContent_Declare(implot URL "https://github.com/epezent/implot/archive/refs/tags/v0.15.zip")
FetchContent_MakeAvailable(implot)
FetchContent_GetProperties(implot)

add_library(implot
  "${implot_SOURCE_DIR}/implot.h"
  "${implot_SOURCE_DIR}/implot.cpp"
  "${implot_SOURCE_DIR}/implot_items.cpp")

target_include_directories(implot PUBLIC "${implot_SOURCE_DIR}")

target_link_libraries(implot PUBLIC imgui)

add_executable(mxchip_client_ui
  main.cpp)

target_link_libraries(mxchip_client_ui
  PRIVATE
    mxchip_client
    glfw
    imgui
    implot)

set_target_properties(mxchip_client_ui
  PROPERTIES
    OUTPUT_NAME mxclient)
