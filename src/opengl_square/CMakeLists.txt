cmake_minimum_required(VERSION 3.12)

project(
    opengl_square
        LANGUAGES CXX
)

add_executable(${PROJECT_NAME} ${PROJECT_NAME}.cpp)

find_package(glfw3 REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
if( NOT GLEW_FOUND )
    message(FATAL_ERROR ${GLEW_LIBRARIES})
endif()

include_directories(${OPENGL_INCLUDE_DIRS};)

set_target_properties(
    ${PROJECT_NAME}
        PROPERTIES
            CXX_STANDARD 17
            CXX_EXTENSIONS OFF
            CXX_STANDARD_REQUIRED ON
            COMPILE_OPTIONS "-Wpedantic;-Wall;-Wextra;-Werror;"
            LINK_LIBRARIES "${CMAKE_THREAD_LIBS_INIT};${OPENGL_LIBRARIES};glfw;${GLEW_LIBRARIES}"
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
