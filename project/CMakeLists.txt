# Source files
set(SOURCES 
    "src/main.cpp"
)

# Create the executable
add_executable(${PROJECT_NAME} ${SOURCES})

# Find Vulkan
find_package(Vulkan REQUIRED)

# Set the path to glslc (Adjust if Vulkan SDK is installed elsewhere)
set(GLSLC "C:/VulkanSDK/1.3.296.0/Bin/glslc.exe")

# Fetch used libraries
include(FetchContent)
Set(FETCHCONTENT_QUIET FALSE)

FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 1.0.1
    GIT_PROGRESS TRUE
    GIT_SHALLOW TRUE
)
set(GLM_TEST_ENABLE OFF CACHE BOOL "" FORCE) # Disable GLM tests
FetchContent_MakeAvailable(glm)

FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.4
    GIT_PROGRESS TRUE
    GIT_SHALLOW TRUE
)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)       # Disable GLFW documentation
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)   # Disable GLFW examples
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)      # Disable GLFW tests
FetchContent_MakeAvailable(glfw)

# Fetch stb_image
FetchContent_Declare(
    stb
    GIT_REPOSITORY https://github.com/nothings/stb.git
    GIT_TAG master
    GIT_PROGRESS TRUE
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(stb)

# Add the stb_image.h directory to your include paths
target_include_directories(${PROJECT_NAME} PRIVATE ${stb_SOURCE_DIR})

# Link libraries
target_link_libraries(${PROJECT_NAME} PRIVATE glfw Vulkan::Vulkan glm::glm)

# Copy resources to output folder
set(TEXTURES_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/resources/textures")
file(GLOB_RECURSE TEXTURE_FILES
    "${TEXTURES_SOURCE_DIR}/*"
)

set(SHADER_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/resources/shaders")
file(GLOB_RECURSE SHADER_FILES
    "${SHADER_SOURCE_DIR}/*"
)

set(TEXTURES_OUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/resources/textures")
file(MAKE_DIRECTORY ${TEXTURES_OUT_DIR})
foreach(TEXTURE ${TEXTURE_FILES})
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${TEXTURE}
    ${TEXTURES_OUT_DIR})
endforeach(TEXTURE)

set(SHADER_OUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/resources/shaders/")
file(MAKE_DIRECTORY ${SHADER_OUT_DIR})
foreach(RESOURCE ${SHADER_FILES})
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${RESOURCE}
    ${SHADER_OUT_DIR})
endforeach(RESOURCE)

foreach(SHADER ${SHADER_FILES})
    set(SHADER_SOURCE "${SHADER}")
    get_filename_component(SHADER_NAME ${SHADER} NAME)  # Extract file name
    get_filename_component(SHADER_EXT ${SHADER} EXT)    # Extract extension (e.g., .vert, .frag)

    # Remove the leading dot from the extension and construct the new file name
    string(REPLACE "." "" SHADER_EXT_CLEAN ${SHADER_EXT})
    set(SHADER_BINARY "${SHADER_OUT_DIR}/${SHADER_EXT_CLEAN}.spv")

    add_custom_command(
        OUTPUT ${SHADER_BINARY}
        COMMAND ${GLSLC} ${SHADER_SOURCE} -o ${SHADER_BINARY}
        DEPENDS ${SHADER_SOURCE}
        COMMENT "Compiling shader: ${SHADER}"
        VERBATIM
    )

    list(APPEND SPV_SHADERS ${SHADER_BINARY})
endforeach()

# Create a custom target to compile shaders
add_custom_target(Shaders ALL DEPENDS ${SPV_SHADERS})
