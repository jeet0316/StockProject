cmake_minimum_required(VERSION 3.15)
project(StockSense VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Build options
option(USE_EIGEN "Use Eigen library for PCA" OFF)
option(USE_MATPLOTLIB_CPP "Use matplotlib-cpp for plotting" OFF)
option(USE_QT "Use Qt for GUI" OFF)

# Find required libraries
find_package(CURL REQUIRED)
find_package(PkgConfig QUIET)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/src)
include_directories(${CMAKE_SOURCE_DIR}/include)

# Add rapidcsv (header-only, if you have it in include/)
# If rapidcsv.h is in a subdirectory, adjust path accordingly

# Optional: Eigen library for PCA
if(USE_EIGEN)
    find_package(Eigen3 QUIET)
    if(Eigen3_FOUND)
        message(STATUS "Eigen3 found: ${EIGEN3_INCLUDE_DIR}")
        include_directories(${EIGEN3_INCLUDE_DIR})
        add_definitions(-DUSE_EIGEN)
    else()
        message(WARNING "Eigen3 not found. PCA will use simplified implementation.")
    endif()
endif()

# Optional: matplotlib-cpp
if(USE_MATPLOTLIB_CPP)
    find_path(MATPLOTLIB_CPP_INCLUDE_DIR matplotlibcpp.h)
    if(MATPLOTLIB_CPP_INCLUDE_DIR)
        include_directories(${MATPLOTLIB_CPP_INCLUDE_DIR})
        add_definitions(-DUSE_MATPLOTLIB_CPP)
        # matplotlib-cpp requires Python
        find_package(Python3 COMPONENTS Interpreter Development REQUIRED)
        target_include_directories(${PROJECT_NAME} PRIVATE ${Python3_INCLUDE_DIRS})
    else()
        message(WARNING "matplotlib-cpp not found. Using Python scripts for plotting.")
    endif()
endif()

# Optional: Qt
if(USE_QT)
    find_package(Qt5 COMPONENTS Core Widgets REQUIRED)
    add_definitions(-DUSE_QT)
endif()

# Source files
set(SOURCES
    src/main.cpp
    src/StockDataLoader.cpp
    src/DataProcessor.cpp
    src/Visualizer.cpp
    src/Config.cpp
)

# Header files
set(HEADERS
    src/StockData.h
    src/StockDataLoader.h
    src/DataProcessor.h
    src/Visualizer.h
    src/Config.h
)

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# Link libraries
target_link_libraries(${PROJECT_NAME} 
    ${CURL_LIBRARIES}
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CURL_INCLUDE_DIRS}
)

# Optional: Link Qt
if(USE_QT AND Qt5_FOUND)
    target_link_libraries(${PROJECT_NAME} Qt5::Core Qt5::Widgets)
endif()

# Compiler-specific options
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -pedantic)
endif()

# Copy data directory to build directory
file(COPY ${CMAKE_SOURCE_DIR}/data DESTINATION ${CMAKE_BINARY_DIR})

# Print configuration summary
message(STATUS "")
message(STATUS "=== StockSense Configuration ===")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Eigen: ${USE_EIGEN}")
message(STATUS "matplotlib-cpp: ${USE_MATPLOTLIB_CPP}")
message(STATUS "Qt: ${USE_QT}")
message(STATUS "=================================")
message(STATUS "")

