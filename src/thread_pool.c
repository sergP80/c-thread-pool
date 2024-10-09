#include <thread_pool.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <pthread.h>
#endif

#include <stdlib.h>
#include <stdio.h>

// Create a new thread pool
threadpool *threadpool_create(int num_threads) {
    if (num_threads <= 0) num_threads = 1;

    threadpool *pool = (threadpool *)malloc(sizeof(threadpool));
    if (!pool) return NULL;

    pool->thread_count = num_threads;
    pool->stop = false;
    pool->task_queue = NULL;
    pool->started = false;

#ifdef _WIN32
    pool->threads = (HANDLE *)malloc(sizeof(HANDLE) * num_threads);
    pool->task_mutex = CreateMutex(NULL, FALSE, NULL);
    pool->task_available = CreateEvent(NULL, FALSE, FALSE, NULL);
#else
    pool->threads = (pthread_t *)malloc(sizeof(pthread_t) * num_threads);
    pthread_mutex_init(&pool->task_mutex, NULL);
    pthread_cond_init(&pool->task_available, NULL);
#endif

    if (!pool->threads) {
        free(pool);
        return NULL;
    }

    for (int i = 0; i < num_threads; i++) {
#ifdef _WIN32
        pool->threads[i] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)thread_worker, (void *)pool, 0, NULL);
#else
        pthread_create(&(pool->threads[i]), NULL, thread_worker, (void *)pool);
#endif
    }

    pool->started = true;
    return pool;
}

// Destroy the thread pool and free resources
void threadpool_destroy(threadpool *pool) {
    if (!pool || !pool->started) return;

    pool->stop = true;

#ifdef _WIN32
    WaitForMultipleObjects(pool->thread_count, pool->threads, TRUE, INFINITE);
    for (int i = 0; i < pool->thread_count; i++) {
        CloseHandle(pool->threads[i]);
    }
    CloseHandle(pool->task_mutex);
    CloseHandle(pool->task_available);
#else
    pthread_mutex_lock(&pool->task_mutex);
    pthread_cond_broadcast(&pool->task_available);
    pthread_mutex_unlock(&pool->task_mutex);

    for (int i = 0; i < pool->thread_count; i++) {
        pthread_join(pool->threads[i], NULL);
    }
    pthread_mutex_destroy(&pool->task_mutex);
    pthread_cond_destroy(&pool->task_available);
#endif

    free(pool->threads);
    while (pool->task_queue) {
        threadpool_task *task = pool->task_queue;
        pool->task_queue = task->next;
        free(task);
    }

    free(pool);
}

// Add a new task to the thread pool
bool threadpool_add_task(threadpool *pool, void (*function)(void*), void *arg) {
    if (!pool || !function) return false;

    threadpool_task *task = (threadpool_task *)malloc(sizeof(threadpool_task));
    if (!task) return false;

    task->function = function;
    task->arg = arg;
    task->next = NULL;

#ifdef _WIN32
    WaitForSingleObject(pool->task_mutex, INFINITE);
#else
    pthread_mutex_lock(&pool->task_mutex);
#endif

    threadpool_task *last = pool->task_queue;
    if (!last) {
        pool->task_queue = task;
    } else {
        while (last->next) last = last->next;
        last->next = task;
    }

#ifdef _WIN32
    ReleaseMutex(pool->task_mutex);
    SetEvent(pool->task_available);
#else
    pthread_mutex_unlock(&pool->task_mutex);
    pthread_cond_signal(&pool->task_available);
#endif

    return true;
}

// Worker thread function
void *thread_worker(void *arg) {
    threadpool *pool = (threadpool *)arg;

    while (1) {
#ifdef _WIN32
        WaitForSingleObject(pool->task_mutex, INFINITE);
        while (!pool->task_queue && !pool->stop) {
            ReleaseMutex(pool->task_mutex);
            WaitForSingleObject(pool->task_available, INFINITE);
            WaitForSingleObject(pool->task_mutex, INFINITE);
        }
#else
        pthread_mutex_lock(&pool->task_mutex);
        while (!pool->task_queue && !pool->stop) {
            pthread_cond_wait(&pool->task_available, &pool->task_mutex);
        }
#endif

        if (pool->stop && !pool->task_queue) {
#ifdef _WIN32
            ReleaseMutex(pool->task_mutex);
#else
            pthread_mutex_unlock(&pool->task_mutex);
#endif
            break;
        }

        threadpool_task *task = pool->task_queue;
        if (task) {
            pool->task_queue = task->next;
        }

#ifdef _WIN32
        ReleaseMutex(pool->task_mutex);
#else
        pthread_mutex_unlock(&pool->task_mutex);
#endif

        if (task) {
            task->function(task->arg);
            free(task);
        }
    }

    return NULL;
}
