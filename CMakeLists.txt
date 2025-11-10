cmake_minimum_required(VERSION 3.20)

project(OnlyTheDead VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Unreal Engine 5 paths (user must set UE5_ROOT)
if(NOT DEFINED ENV{UE5_ROOT})
    message(FATAL_ERROR "UE5_ROOT environment variable not set. Please set it to your Unreal Engine 5 installation directory.")
endif()

set(UE5_ROOT $ENV{UE5_ROOT})

# Project source files
file(GLOB_RECURSE SOURCE_FILES
    "${PROJECT_SOURCE_DIR}/src/**/*.cpp"
    "${PROJECT_SOURCE_DIR}/src/**/*.h"
)

# Include directories
include_directories(
    "${PROJECT_SOURCE_DIR}/src"
    "${UE5_ROOT}/Engine/Source/Runtime/Core/Public"
    "${UE5_ROOT}/Engine/Source/Runtime/CoreUObject/Public"
    "${UE5_ROOT}/Engine/Source/Runtime/Engine/Public"
    "${UE5_ROOT}/Engine/Source/Runtime/AIModule/Public"
)

# Create library target (for IDE integration)
add_library(OnlyTheDeadLib STATIC ${SOURCE_FILES})

# Set compiler flags
if(MSVC)
    target_compile_options(OnlyTheDeadLib PRIVATE /W4 /WX)
else()
    target_compile_options(OnlyTheDeadLib PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()

# Custom targets for Unreal Build Tool
add_custom_target(GenerateProjectFiles
    COMMAND "${UE5_ROOT}/Engine/Build/BatchFiles/GenerateProjectFiles.bat" "${PROJECT_SOURCE_DIR}/OnlyTheDead.uproject"
    WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
    COMMENT "Generating UE5 project files..."
)

add_custom_target(BuildEditor
    COMMAND "${UE5_ROOT}/Engine/Build/BatchFiles/Build.bat" OnlyTheDeadEditor Win64 Development "${PROJECT_SOURCE_DIR}/OnlyTheDead.uproject"
    WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
    COMMENT "Building UE5 Editor..."
    DEPENDS GenerateProjectFiles
)

add_custom_target(BuildGame
    COMMAND "${UE5_ROOT}/Engine/Build/BatchFiles/Build.bat" OnlyTheDead Win64 Shipping "${PROJECT_SOURCE_DIR}/OnlyTheDead.uproject"
    WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
    COMMENT "Building Game (Shipping)..."
    DEPENDS GenerateProjectFiles
)

# Testing
enable_testing()

# Add test target (placeholder - actual tests would use UE5's automation system)
add_test(NAME SurvivalSystemTests COMMAND echo "Tests would run via UE5 Automation Framework")

# Installation
install(TARGETS OnlyTheDeadLib
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

# Documentation
find_package(Doxygen)
if(DOXYGEN_FOUND)
    set(DOXYGEN_IN ${PROJECT_SOURCE_DIR}/docs/Doxyfile.in)
    set(DOXYGEN_OUT ${PROJECT_BINARY_DIR}/Doxyfile)

    configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)

    add_custom_target(docs
        COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
        WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM
    )
endif()

# Print configuration summary
message(STATUS "")
message(STATUS "Only The Dead - Configuration Summary")
message(STATUS "======================================")
message(STATUS "Version: ${PROJECT_VERSION}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "UE5 Root: ${UE5_ROOT}")
message(STATUS "Source files: ${CMAKE_CURRENT_SOURCE_DIR}/src")
message(STATUS "")
