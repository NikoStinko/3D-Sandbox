cmake_minimum_required(VERSION 3.10)
project(3D-Sandbox)

set(CMAKE_CXX_STANDARD 17)

find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(GLEW REQUIRED)
find_package(assimp REQUIRED)

# FetchContent for Dear ImGui
include(FetchContent)
FetchContent_Declare(
  imgui
  GIT_REPOSITORY https://github.com/ocornut/imgui.git
  GIT_TAG v1.91.0
)

FetchContent_Declare(
  nlohmann_json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG v3.11.2
)

FetchContent_MakeAvailable(imgui nlohmann_json)

add_executable(${PROJECT_NAME} 
    src/main.cpp
    src/Shader.cpp
    src/Camera.cpp
    src/Model.cpp
    src/Mesh.cpp
    src/Log.cpp
    src/Texture.cpp
    src/ModelManager.cpp
    src/Grid.cpp
    src/UiOverlay.cpp
    src/ModelBrowserPanel.cpp
    src/CustomButtonsPanel.cpp
    src/MapPanel.cpp
    src/ScenePanel.cpp
    src/SceneState.cpp
    src/SceneSerializer.cpp
    src/MenuRenderer.cpp
    src/SandBoxUI.cpp
    src/Window.cpp
    src/InputHandler.cpp
    src/Renderer.cpp
    src/Scene.cpp
    src/App.cpp
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
    ${imgui_SOURCE_DIR}/misc/cpp/imgui_stdlib.cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${ASSIMP_INCLUDE_DIRS}
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    ${OPENGL_gl_LIBRARY}
    glfw
    GLEW::GLEW
    ${ASSIMP_LIBRARIES}
    nlohmann_json::nlohmann_json
    dl
    pthread
)

file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/shaders)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/resources/models)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/resources/maps)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/logs)

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