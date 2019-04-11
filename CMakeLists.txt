cmake_minimum_required(VERSION 3.9)

project(hairdemo)

add_subdirectory(thirdparty)

# main target.
add_executable(hairdemo 
    src/hair_renderer.h
    src/hair_renderer.cc
    src/main.cc
    src/render.h
    src/render.cc
    src/xy_calc.h
    src/xy_rt.h
    src/xy_ext.h
    src/xy_opengl.h
    src/xy_opengl.cc
    src/xy_glfw.h
    src/xy_glfw.cc
    src/tiny_obj_loader.h
    src/tiny_obj_loader.cc)

target_include_directories(hairdemo
PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(hairdemo glad)
target_link_libraries(hairdemo glfw)
target_link_libraries(hairdemo stb)
target_link_libraries(hairdemo imgui)

install(TARGETS hairdemo
    DESTINATION bin)
