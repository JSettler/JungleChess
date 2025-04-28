cmake_minimum_required(VERSION 3.10)

project(JungleChess CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# --- Optimization Flags ---
# Add -O3 and -Ofast to the Release build type flags.
# The compiler will likely apply the effects of -Ofast which includes -O3.
# Appending ensures both are considered if needed.
# Optional: Add -march=native to optimize for the specific CPU building the code
# set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -Ofast -march=native")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -Ofast")

# Set default optimization for Debug builds (usually -O0 -g)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g") # -g adds debug symbols

# Set a default build type if none is specified by the user
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose build type: Debug Release RelWithDebInfo MinSizeRel" FORCE)
  message(STATUS "No build type specified, defaulting to ${CMAKE_BUILD_TYPE}")
endif()


# --- Find Packages ---
find_package(SFML 2.5 COMPONENTS system window graphics REQUIRED)


# --- Project Configuration ---
include_directories(include)

add_executable(jungle_chess
    src/main.cpp
    src/GameState.cpp
    src/Graphics.cpp
    src/AI.cpp
    src/Hashing.cpp
)

# Link SFML libraries
# Add standard libraries if needed (fstream is usually header-only or linked by default)
target_link_libraries(jungle_chess PRIVATE sfml-system sfml-window sfml-graphics)

# Copy assets directory to build directory
file(COPY assets DESTINATION ${CMAKE_BINARY_DIR})

message(STATUS "Configuring project for build type: ${CMAKE_BUILD_TYPE}")


