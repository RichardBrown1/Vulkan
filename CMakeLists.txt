# CMakeList.txt : CMake project for VulkanFromScratch, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.11)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("VulkanFromScratch")

include(FetchContent)

FetchContent_Declare(
    SDL2
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG release-2.30.7
    GIT_SHALLOW TRUE
    GIT_PROGRESS TRUE
)

FetchContent_Declare(
    glm
 	GIT_REPOSITORY	https://github.com/g-truc/glm.git
    GIT_TAG 1.0.1
)

FetchContent_MakeAvailable(SDL2)
FetchContent_MakeAvailable(glm)

find_package(Vulkan REQUIRED)

# Add source to this project's executable.
add_executable (VulkanFromScratch "src/VulkanFromScratch.cpp"  "src/Utilities.cpp" "src/VulkanEngine.cpp" "include/VulkanEngine.hpp")

# Define the source and destination directories
set(SHADERS_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/shaders)
set(SHADERS_DEST_DIR ${CMAKE_CURRENT_BINARY_DIR}/shaders)

# Add a custom command to copy the folder
add_custom_command(
    TARGET VulkanFromScratch
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${SHADERS_SOURCE_DIR} ${SHADERS_DEST_DIR}
)


if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET VulkanFromScratch PROPERTY CXX_STANDARD 20)
endif()

if (MSVC)
    target_compile_options(VulkanFromScratch PRIVATE /W4 /WX /std:c++20)
else()
    target_compile_options(VulkanFromScratch PRIVATE -Wall -Wextra -Wpedantic -Werror -std=c++20 -fsanitize=undefined -fsanitize=address)
endif()

# TODO: Add tests and install targets if needed
target_link_libraries(VulkanFromScratch PUBLIC Vulkan::Vulkan SDL2::SDL2main SDL2::SDL2-static glm::glm)

target_precompile_headers(VulkanFromScratch PUBLIC <optional> <vector> <memory> <string> <vector> <unordered_map> <vulkan/vulkan.h>)