# stb target
add_library(stb INTERFACE)
target_include_directories(stb INTERFACE ${CMAKE_CURRENT_LIST_DIR}/stb/include)

add_library(glad STATIC
    ${CMAKE_CURRENT_LIST_DIR}/glad/src/glad.c
    ${CMAKE_CURRENT_LIST_DIR}/glad/include/glad/glad.h)
target_include_directories(glad PRIVATE ${CMAKE_CURRENT_LIST_DIR}/glad/include/glad)
target_include_directories(glad INTERFACE ${CMAKE_CURRENT_LIST_DIR}/glad/include/)

# GLFW target.
add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/glfw-3.2.1-modified")

add_library(imgui STATIC 
    ${CMAKE_CURRENT_LIST_DIR}/imgui/imgui.cpp
    ${CMAKE_CURRENT_LIST_DIR}/imgui/imgui_demo.cpp
    ${CMAKE_CURRENT_LIST_DIR}/imgui/imgui_impl_glfw.cpp
    ${CMAKE_CURRENT_LIST_DIR}/imgui/imgui_impl_opengl3.cpp
    ${CMAKE_CURRENT_LIST_DIR}/imgui/imgui_draw.cpp
    ${CMAKE_CURRENT_LIST_DIR}/imgui/include/imgui/imgui.h
    ${CMAKE_CURRENT_LIST_DIR}/imgui/include/imgui/imconfig.h
    ${CMAKE_CURRENT_LIST_DIR}/imgui/include/imgui/imgui_internal.h
    ${CMAKE_CURRENT_LIST_DIR}/imgui/include/imgui/imgui_impl_glfw.h
    ${CMAKE_CURRENT_LIST_DIR}/imgui/include/imgui/imgui_impl_opengl3.h)
target_include_directories(imgui PRIVATE ${CMAKE_CURRENT_LIST_DIR}/imgui/include/imgui)
target_include_directories(imgui INTERFACE ${CMAKE_CURRENT_LIST_DIR}/imgui/include)
target_link_libraries(imgui PRIVATE stb)
target_link_libraries(imgui PRIVATE glad)
target_link_libraries(imgui PRIVATE glfw)

