#include "unity.h"
#include "thread_pool.h"
#include <stdlib.h>
#include <stdio.h>

#ifdef _WIN32
#include <windows.h>  // For Sleep on Windows
#define sleep(x) Sleep(1000 * (x))  // Redefine sleep to work on Windows
#else
#include <unistd.h>   // For sleep on Unix-based systems
#endif

#ifdef _WIN32
#include <windows.h>
#define atomic_int LONG
#define atomic_fetch_add(x, y) InterlockedExchangeAdd(x, y)
#else
#include <stdatomic.h>
#endif

// Global counter for testing task execution
atomic_int counter;

// Test Setup and Teardown Functions
// Called before each test
void setUp(void) {
    counter = 0;  // Initialize global counter to zero before each test
}

// Called after each test
void tearDown(void) {
    // No special cleanup is required
}

// Task function to increment the global counter
void increment_task(void *arg) {
    atomic_fetch_add(&counter, 1);
}

// Task function to decrement the global counter
void decrement_task(void *arg) {
    atomic_fetch_add(&counter, -1);
}

// Task function to perform a dummy calculation
void dummy_calculation(void *arg) {
    int *input = (int *)arg;
    int result = 0;
    for (int i = 0; i < *input; i++) {
        result += i;
    }
}

// Test Case 1: Creating and Destroying a Thread Pool
void test_create_and_destroy(void) {
    threadpool *pool = threadpool_create(4);  // Create a thread pool with 4 threads
    TEST_ASSERT_NOT_NULL(pool);  // Check that the thread pool was successfully created

    threadpool_destroy(pool);  // Destroy the thread pool
}

// Test Case 2: Adding a Single Task
void test_add_single_task(void) {
    threadpool *pool = threadpool_create(4);
    TEST_ASSERT_TRUE(threadpool_add_task(pool, increment_task, NULL));  // Add a single task

    sleep(1);  // Allow some time for the task to complete
    TEST_ASSERT_EQUAL(1, counter);  // Verify that the counter was incremented

    threadpool_destroy(pool);
}

// Test Case 3: Adding Multiple Tasks
void test_add_multiple_tasks(void) {
    threadpool *pool = threadpool_create(4);
    int num_tasks = 10;

    for (int i = 0; i < num_tasks; i++) {
        TEST_ASSERT_TRUE(threadpool_add_task(pool, increment_task, NULL));  // Add multiple tasks
    }

    sleep(1);  // Allow time for all tasks to complete
    TEST_ASSERT_EQUAL(num_tasks, counter);  // Verify that all tasks were processed

    threadpool_destroy(pool);
}

// Test Case 4: Concurrent Task Execution
void test_concurrent_tasks(void) {
    threadpool *pool = threadpool_create(8);
    int num_tasks = 100;

    for (int i = 0; i < num_tasks; i++) {
        TEST_ASSERT_TRUE(threadpool_add_task(pool, increment_task, NULL));  // Add a large number of tasks
    }

    sleep(2);  // Allow more time for all tasks to complete concurrently
    TEST_ASSERT_EQUAL(num_tasks, counter);  // Verify that all tasks were processed correctly

    threadpool_destroy(pool);
}

// Test Case 5: Adding Tasks of Different Types
void test_mixed_tasks(void) {
    threadpool *pool = threadpool_create(4);
    int num_increment = 10;
    int num_decrement = 5;

    // Add increment tasks
    for (int i = 0; i < num_increment; i++) {
        TEST_ASSERT_TRUE(threadpool_add_task(pool, increment_task, NULL));
    }

    // Add decrement tasks
    for (int i = 0; i < num_decrement; i++) {
        TEST_ASSERT_TRUE(threadpool_add_task(pool, decrement_task, NULL));
    }

    sleep(1);  // Allow time for tasks to complete
    TEST_ASSERT_EQUAL(num_increment - num_decrement, counter);  // Verify mixed task results

    threadpool_destroy(pool);
}

// Test Case 6: Dummy Calculation Task
void test_dummy_calculation(void) {
    threadpool *pool = threadpool_create(4);
    int calculation_value = 1000;

    TEST_ASSERT_TRUE(threadpool_add_task(pool, dummy_calculation, &calculation_value));  // Add a dummy calculation task

    sleep(1);  // Allow time for the task to complete

    // No counter check here; just ensure the task runs without error
    threadpool_destroy(pool);
}

// Main function to run all tests
int main(void) {
    UNITY_BEGIN();  // Initialize Unity test framework

    // Register test cases
    RUN_TEST(test_create_and_destroy);
    // RUN_TEST(test_add_single_task);
    // RUN_TEST(test_add_multiple_tasks);
    // RUN_TEST(test_concurrent_tasks);
    // RUN_TEST(test_mixed_tasks);
    RUN_TEST(test_dummy_calculation);

    return UNITY_END();  // Report test results
}
