# CMake configuration for Unity tests
project(threadpool_unity_tests)

# Use FetchContent to download and add Unity
include(FetchContent)
FetchContent_Declare(
        unity
        GIT_REPOSITORY https://github.com/ThrowTheSwitch/Unity.git
        GIT_TAG v2.5.2
)
FetchContent_MakeAvailable(unity)

# Define the test source files
file(GLOB TEST_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.c)

# Create a test executable
add_executable(${PROJECT_NAME} ${TEST_FILES})

# Link Unity and the threadpool library to the test executable
target_link_libraries(${PROJECT_NAME} unity ${STATIC_THREAD_POOL})

if(UNIX)
    target_compile_options(${PROJECT_NAME} PRIVATE -std=c11)
endif()

# Include the Unity and threadpool header files in the test executable
target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/include ${unity_SOURCE_DIR}/src)

# Register the Unity tests with CTest
add_test(NAME ThreadPoolUnityTests COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIG>/${PROJECT_NAME})