cmake_minimum_required(VERSION 3.10)
project(3D-Sandbox)

set(CMAKE_CXX_STANDARD 17)

find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(GLEW REQUIRED)
find_package(assimp REQUIRED)

add_executable(${PROJECT_NAME} 
    src/main.cpp
    src/Shader.cpp
    src/Camera.cpp
    src/Model.cpp
    src/Mesh.cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${ASSIMP_INCLUDE_DIRS}
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    ${OPENGL_gl_LIBRARY}
    glfw
    GLEW::GLEW
    ${ASSIMP_LIBRARIES}
    dl
    pthread
)

file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/shaders)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/resources/models)

execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/shaders)
execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/resources)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}/shaders
    $<TARGET_FILE_DIR:${PROJECT_NAME}>/shaders
)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}/resources
    $<TARGET_FILE_DIR:${PROJECT_NAME}>/resources
)