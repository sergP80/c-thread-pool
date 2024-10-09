#include "unity.h"
#include "thread_pool.h"
#include <stdatomic.h>
#include <stdlib.h>

// Platform-specific headers for sleep functionality
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

// Global counter for task function
atomic_int counter;

// Task function to increment the counter
void increment_task(void *arg) {
    atomic_fetch_add(&counter, 1);
}

// Test creating and destroying a thread pool
void test_create_and_destroy(void) {
    threadpool *pool = threadpool_create(4);
    TEST_ASSERT_NOT_NULL(pool);
    threadpool_destroy(pool);
}

// Test adding tasks to the thread pool
void test_add_tasks(void) {
    counter = 0;
    threadpool *pool = threadpool_create(4);

    // Add 10 tasks to the pool
    for (int i = 0; i < 10; i++) {
        TEST_ASSERT_TRUE(threadpool_add_task(pool, increment_task, NULL));
    }

    // Allow some time for the tasks to complete
#ifdef _WIN32
    Sleep(1000);  // Windows uses milliseconds
#else
    sleep(1);     // Unix uses seconds
#endif

    // Verify all tasks were processed
    TEST_ASSERT_EQUAL(10, counter);

    threadpool_destroy(pool);
}

// Test concurrent execution of tasks in multiple threads
void test_concurrent_tasks(void) {
    counter = 0;
    threadpool *pool = threadpool_create(8);

    // Add 100 tasks to the pool
    for (int i = 0; i < 100; i++) {
        TEST_ASSERT_TRUE(threadpool_add_task(pool, increment_task, NULL));
    }

    // Allow some time for the tasks to complete
#ifdef _WIN32
    Sleep(2000);
#else
    sleep(2);
#endif

    // Verify all tasks were processed
    TEST_ASSERT_EQUAL(100, counter);

    threadpool_destroy(pool);
}

// Unity's main test runner function
int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_create_and_destroy);
    RUN_TEST(test_add_tasks);
    RUN_TEST(test_concurrent_tasks);
    return UNITY_END();
}
