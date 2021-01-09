cmake_minimum_required(VERSION 3.12)

project(
    opengl_coord_systems
        LANGUAGES CXX
)

add_executable(${PROJECT_NAME} ${PROJECT_NAME}.cpp)

find_package(GLEW REQUIRED)
find_package(glm REQUIRED)
find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)

include_directories(${OPENGL_INCLUDE_DIRS};)

set_target_properties(
    ${PROJECT_NAME}
        PROPERTIES
            CXX_STANDARD 17
            CXX_EXTENSIONS OFF
            CXX_STANDARD_REQUIRED ON
            COMPILE_OPTIONS "-Wpedantic;-Wall;-Wextra;-Werror;"
            LINK_LIBRARIES "opengl_lib;glfw;${CMAKE_THREAD_LIBS_INIT};${OPENGL_LIBRARIES};${GLEW_LIBRARIES}"
            BUILD_RPATH "${CMAKE_BINARY_DIR}/lib"
            INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib"
)


install(
    TARGETS ${PROJECT_NAME}
        RUNTIME DESTINATION bin
)

install(
    DIRECTORY shaders/
        DESTINATION bin
)
