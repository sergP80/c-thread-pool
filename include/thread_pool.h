#ifndef THREADPOOL_H
#define THREADPOOL_H

#ifdef _WIN32
#include <windows.h>
#else
#include <pthread.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

typedef struct threadpool_task {
    void (*function)(void* arg);
    void *arg;
    struct threadpool_task *next;
} threadpool_task;

typedef struct threadpool {
    threadpool_task *task_queue;
    int thread_count;
    bool stop;
    bool started;

#ifdef _WIN32
    HANDLE *threads;
    HANDLE task_mutex;
    HANDLE task_available;
#else
    pthread_t *threads;
    pthread_mutex_t task_mutex;
    pthread_cond_t task_available;
#endif

} threadpool;

threadpool *threadpool_create(int num_threads);
void threadpool_destroy(threadpool *pool);
bool threadpool_add_task(threadpool *pool, void (*function)(void*), void *arg);
void *thread_worker(void *pool);

#endif  // THREADPOOL_H
