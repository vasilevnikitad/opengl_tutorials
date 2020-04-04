cmake_minimum_required(VERSION 3.12 FATAL_ERROR)

if(POLICY CMP0076)
    cmake_policy(SET CMP0076 NEW)
endif()

project(
    opengl_lib
)


set(LIB_FILES gl_wrappers.hpp gl_helpers.hpp)

add_library(${PROJECT_NAME} INTERFACE)
if(POLICY CMP0076)
    target_sources(${PROJECT_NAME} INTERFACE ${LIB_FILES})
else()
    target_sources(${PROJECT_NAME}
                    INTERFACE "${CMAKE_SOURCE_DIR}/lib/${LIB_FILES}")
endif()

target_include_directories(${PROJECT_NAME} INTERFACE "${CMAKE_SOURCE_DIR}/lib")

set_property(
    TARGET ${PROJECT_NAME}
        APPEND PROPERTY
            INTERFACE_INCLUDE_DIRECTORIES
                "${CMAKE_BINARY_DIR}"
                "${CMAKE_SOURCE_DIR}/lib"
)

install(
    TARGETS ${PROJECT_NAME}
        LIBRARY DESTINATION lib
            COMPONENT ${PROJECT_NAME}
)
