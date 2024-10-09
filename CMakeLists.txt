# Minimum CMake version required
cmake_minimum_required(VERSION 3.10)

project(c_thread_pool VERSION 1.0 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../dist/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../dist/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../dist/bin)

set(INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include)
set(SRC_DIR ${PROJECT_SOURCE_DIR}/src)

include_directories(${INCLUDE_DIR})

file(GLOB SRC_FILES ${SRC_DIR}/*.c ${SRC_DIR}/*.cc ${SRC_DIR}/*.cpp ${SRC_DIR}/*.cxx)

set(STATIC_THREAD_POOL ${PROJECT_NAME}_static)
set(SHARED_THREAD_POOL ${PROJECT_NAME}_shared)

# Create a static library target
add_library(${STATIC_THREAD_POOL} STATIC ${SRC_FILES})
set_target_properties(${STATIC_THREAD_POOL} PROPERTIES
        OUTPUT_NAME  ${PROJECT_NAME}
        VERSION ${PROJECT_VERSION}
        SOVERSION 1
)

# Create a shared library target
add_library(${SHARED_THREAD_POOL} SHARED ${SRC_FILES})
set_target_properties(${SHARED_THREAD_POOL} PROPERTIES
        OUTPUT_NAME "threadpool"
        VERSION ${PROJECT_VERSION}
        SOVERSION 1
)

# Set include directories for both static and shared libraries
target_include_directories(${STATIC_THREAD_POOL} PUBLIC ${INCLUDE_DIR})
target_include_directories(${SHARED_THREAD_POOL} PUBLIC ${INCLUDE_DIR})

# Link pthread library for Unix-like systems
if(UNIX)
    target_link_libraries(${STATIC_THREAD_POOL} pthread)
    target_link_libraries(${SHARED_THREAD_POOL} pthread)
endif()

# Install targets (optional)
install(TARGETS ${STATIC_THREAD_POOL} ${SHARED_THREAD_POOL}
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        RUNTIME DESTINATION bin
        INCLUDES DESTINATION include
)

# Include header file during installation
install(DIRECTORY ${INCLUDE_DIR}/ DESTINATION include)

# Enable testing and include the `tests` subdirectory
enable_testing()
add_subdirectory(tests)
