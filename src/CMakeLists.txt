if(NOT "${CUDA_ARCH}" MATCHES "^[0-9][0-9]$")
    message(FATAL_ERROR "CUDA_ARCH not set, run e.g. `cmake -DCUDA_ARCH=61 ..` based on your GPU")
else()
    message(STATUS "Building for CUDA ${CUDA_ARCH}")
endif()

# set c++/cuda standards for all targets
# `target_compile_features({target} PUBLIC cxx_std_17 cuda_std_14)` does not work (https://gitlab.kitware.com/cmake/cmake/-/issues/21883)
# by removing `set(CMAKE_CUDA_STANDARD XX)` nvcc compiles with -std=c++CXX_STANDARD no matter the `cuda_std_14`
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CUDA_STANDARD 17)

find_package(GLviz REQUIRED CONFIG)
find_package(CUDAToolkit REQUIRED)
find_package(CLI11 CONFIG REQUIRED)
find_package(Boost REQUIRED system serialization)
find_package(Eigen3 3.3 REQUIRED NO_MODULE)
find_package(CLI11 CONFIG REQUIRED)
find_package(OpenGL REQUIRED COMPONENTS OpenGL EGL GLX)
find_package(glfw3 3.3 REQUIRED)
find_package(nlohmann_json 3.10 REQUIRED)
find_package(OpenCV REQUIRED)

file(TO_NATIVE_PATH "${PROJECT_SOURCE_DIR}/resources/" GLVIZ_RESOURCES_DIR)
configure_file(config.hpp.in "${CMAKE_CURRENT_BINARY_DIR}/config.hpp")

# Surface splatting shader.
set(SHADER_GLSL
    shader/attribute_fs.glsl
    shader/attribute_vs.glsl
    shader/finalization_fs.glsl
    shader/finalization_vs.glsl
    shader/lighting.glsl
)

include(GLvizShaderWrapCpp)
glviz_shader_wrap_cpp(SHADER_CPP ${SHADER_GLSL})

add_library(shader STATIC)

target_sources(shader PRIVATE
    ${SHADER_GLSL}
    ${SHADER_CPP}
)

source_group("Shader Files" FILES ${SHADER_GLSL})

# Surface splatting executable.
add_executable(surface_splatting
    main.cpp
    framebuffer.hpp
    framebuffer.cpp
    program_finalization.hpp
    program_finalization.cpp
    program_attribute.hpp
    program_attribute.cpp
    splat_renderer.cpp
    splat_renderer.hpp
    stb_image_write.cpp
    egl.cpp
    ply_loader.hpp
    utils.cpp
    npy.hpp
)

set_source_files_properties(stb_image_write.cpp
    PROPERTIES
    COMPILE_FLAGS "-w"  # Ignore warnings
)

add_library(HAPPLY INTERFACE)

target_include_directories(HAPPLY
    SYSTEM INTERFACE
        happly
)

# Suppress warnings from glm, that's why there's not used:
# find_package(glm REQUIRED PATHS include/glm/cmake/glm)
add_subdirectory("glm")

target_compile_definitions(glm
    INTERFACE
        GLM_FORCE_SILENT_WARNINGS
        GLM_SILENT_WARNINGS=GLM_ENABLE
)

target_include_directories(surface_splatting
    PRIVATE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
        ${Boost_INCLUDE_DIRS}
        ${OpenCV_INCLUDE_DIRS}
)

target_link_libraries(surface_splatting
    PRIVATE shader
            GLviz::glviz
            HAPPLY
            dl
            ${Boost_LIBRARIES}
            CLI11::CLI11
            OpenGL::GL
            OpenGL::OpenGL
            OpenGL::EGL
            OpenGL::GLX
            glfw
            nlohmann_json::nlohmann_json
            glm::glm
            ${OpenCV_LIBS}
)

file(COPY ../resources DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")

# Surface splatting executable.
add_executable(serializer
    serializer.cu
    ply_loader.hpp
)

set_property(TARGET serializer
    PROPERTY
        CUDA_SEPARABLE_COMPILATION ON
)

target_include_directories(serializer
    PRIVATE
        ${Boost_INCLUDE_DIRS}
)

target_link_libraries(serializer
    CLI11::CLI11
    HAPPLY
    dl
    CUDA::toolkit
    Boost::boost  # Header-only target for interprocess.
    ${Boost_LIBRARIES}
    Eigen3::Eigen
    glm::glm
    #  pthread
)

target_compile_options(serializer
    PRIVATE
        $<$<AND:$<COMPILE_LANG_AND_ID:CXX,CUDA,GNU>,$<CONFIG:DEBUG>>:-Wall -Wextra -Wextra -Wunreachable-code -Wunused -Wunused-function -Wunused-label -Wunused-parameter -Wunused-value -Wunused-variable>
        $<$<AND:$<COMPILE_LANG_AND_ID:CXX,CUDA,GNU>,$<CONFIG:RELEASE>>:-O2>
        $<$<COMPILE_LANGUAGE:CUDA>:--extended-lambda --relocatable-device-code=true --compile>
)
