cmake_minimum_required(VERSION 3.16)
project(PandaBlur)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Network Svg)

# Enable Qt auto-generation
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Create resources.qrc file if it doesn't exist
if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/resources.qrc")
    file(WRITE "${CMAKE_CURRENT_SOURCE_DIR}/resources.qrc"
        "<RCC>\n"
        "    <qresource prefix=\"/\">\n"
        "        <file>panda.svg</file>\n"
        "        <file>check.svg</file>\n"
        "    </qresource>\n"
        "</RCC>\n"
    )
endif()

# Add executable with resources
add_executable(PandaBlur
    main.cpp
    mainwindow.cpp
    mainwindow.h
    config.h
    resources.qrc
)

# Copy SVG files to build directory as fallback
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/panda.svg")
    configure_file(panda.svg ${CMAKE_CURRENT_BINARY_DIR}/panda.svg COPYONLY)
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/check.svg")
    configure_file(check.svg ${CMAKE_CURRENT_BINARY_DIR}/check.svg COPYONLY)
endif()

# Link libraries
target_link_libraries(PandaBlur
    PRIVATE
        Qt6::Core
        Qt6::Widgets
        Qt6::Network
        Qt6::Svg
)

# Set output directory for better organization
set_target_properties(PandaBlur PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# Copy resources to build directory for debugging
add_custom_command(TARGET PandaBlur POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/panda.svg"
        "$<TARGET_FILE_DIR:PandaBlur>/panda.svg"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/check.svg"
        "$<TARGET_FILE_DIR:PandaBlur>/check.svg"
    COMMENT "Copying SVG files to output directory"
)

# Platform-specific configurations
if(WIN32)
    set_target_properties(PandaBlur PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
endif()

if(APPLE)
    set_target_properties(PandaBlur PROPERTIES
        MACOSX_BUNDLE TRUE
    )
endif()

# Debug configuration
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(PandaBlur PRIVATE DEBUG_BUILD)
endif()

# Compiler-specific optimizations
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(PandaBlur PRIVATE -Wall -Wextra -O2)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(PandaBlur PRIVATE /W4 /O2)
endif()

# Install targets
install(TARGETS PandaBlur
    RUNTIME DESTINATION bin
    BUNDLE DESTINATION .
)

# Install resources
install(FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/panda.svg"
    "${CMAKE_CURRENT_SOURCE_DIR}/check.svg"
    DESTINATION bin
    OPTIONAL
)
