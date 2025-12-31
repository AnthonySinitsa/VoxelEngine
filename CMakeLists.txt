# 1. Force CMake to accept older sub-projects (Fixes tinyobjloader error)
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)

cmake_minimum_required(VERSION 3.14)
project(VgeEngine VERSION 0.1.0)

# 2. Modern CMake Policy for FindVulkan (Silences some warnings)
if(POLICY CMP0147)
  cmake_policy(SET CMP0147 NEW)
endif()

# 3. Basic Settings
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)

# --- 2. Automated Dependencies (FetchContent) ---

# GLFW
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG        3.4 
)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(glfw)

# GLM (Math library)
FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG        1.0.1
)
FetchContent_MakeAvailable(glm)

# TinyObjLoader
FetchContent_Declare(
    tinyobjloader
    GIT_REPOSITORY https://github.com/tinyobjloader/tinyobjloader.git
    GIT_TAG        v1.0.6
)
FetchContent_MakeAvailable(tinyobjloader)

# ImGui (Pinned to version v1.89.9 to match old code)
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG        v1.89.9-docking
)
FetchContent_MakeAvailable(imgui)
if(NOT imgui_POPULATED)
    FetchContent_Populate(imgui)
endif()

# --- 3. System Dependencies (Must be installed on OS) ---
find_package(Vulkan REQUIRED)

# --- 4. Source Files ---

# Get your engine source code
file(GLOB_RECURSE SOURCES ${PROJECT_SOURCE_DIR}/src/*.cpp)

# Define ImGui source files from the automatically downloaded path
set(IMGUI_SOURCES
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_vulkan.cpp
)

# --- 5. Build Executable ---

add_executable(${PROJECT_NAME} ${SOURCES} ${IMGUI_SOURCES})

# Define ENGINE_DIR for finding shaders/assets at runtime
target_compile_definitions(${PROJECT_NAME} PRIVATE ENGINE_DIR="${CMAKE_SOURCE_DIR}/")

# Include Directories
target_include_directories(${PROJECT_NAME} PUBLIC 
    ${PROJECT_SOURCE_DIR}/src
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
    ${tinyobjloader_SOURCE_DIR}
    ${Vulkan_INCLUDE_DIRS}
)

# Link Libraries
# Note: 'glfw' and 'glm::glm' are provided by FetchContent
# 'Vulkan::Vulkan' is provided by find_package
target_link_libraries(${PROJECT_NAME} 
    PRIVATE 
    glfw 
    glm::glm 
    Vulkan::Vulkan
)

# --- 6. Shader Compilation Logic (Keep your existing logic) ---

find_program(GLSL_VALIDATOR glslangValidator HINTS
    ${Vulkan_GLSLANG_VALIDATOR_EXECUTABLE}
    /usr/bin
    /usr/local/bin
    $ENV{VULKAN_SDK}/Bin/
)

set(SHADER_SOURCE_DIR ${PROJECT_SOURCE_DIR}/shaders)
file(GLOB_RECURSE GLSL_SOURCE_FILES
    "${SHADER_SOURCE_DIR}/*.frag"
    "${SHADER_SOURCE_DIR}/*.vert"
    "${SHADER_SOURCE_DIR}/*.comp"
)

foreach(GLSL ${GLSL_SOURCE_FILES})
    get_filename_component(FILE_NAME ${GLSL} NAME)
    get_filename_component(FILE_PATH ${GLSL} PATH)
    set(SPIRV "${FILE_PATH}/${FILE_NAME}.spv")

    add_custom_command(
        OUTPUT ${SPIRV}
        COMMAND ${GLSL_VALIDATOR} -V ${GLSL} -o ${SPIRV}
        DEPENDS ${GLSL}
        COMMENT "Compiling shader: ${FILE_NAME}"
    )
    list(APPEND SPIRV_BINARY_FILES ${SPIRV})
endforeach(GLSL)

add_custom_target(Shaders DEPENDS ${SPIRV_BINARY_FILES})
