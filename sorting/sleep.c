#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "sort.h"

#ifdef _WIN32
    #include <windows.h>
    #include <process.h>
    #define THREAD_TYPE HANDLE
    #define THREAD_FUNC unsigned __stdcall
    #define THREAD_CREATE(thr, func, arg) thr = (HANDLE)_beginthreadex(NULL, 0, func, arg, 0, NULL)
    #define THREAD_JOIN(thr) WaitForSingleObject(thr, INFINITE); CloseHandle(thr)
    #define SLEEP_MS(ms) Sleep(ms)
#else
    #include <unistd.h>
    #include <pthread.h>
    #define THREAD_TYPE pthread_t
    #define THREAD_FUNC void *
    #define THREAD_CREATE(thr, func, arg) pthread_create(&thr, NULL, func, arg)
    #define THREAD_JOIN(thr) pthread_join(thr, NULL)
    #define SLEEP_MS(ms) usleep((ms) * 1000)
#endif

typedef struct {
    int value;
    int scale;
} sleep_arg_t;

THREAD_FUNC print_after_sleep(void *arg) {
    sleep_arg_t *sa = arg;
    SLEEP_MS(sa->value * sa->scale);
    printf("%d ", sa->value);
    fflush(stdout);
    free(sa);
#ifdef _WIN32
    return 0;
#else
    return NULL;
#endif
}

void sleep_sort(int *arr, size_t n) {
    const int scale = 10;
    THREAD_TYPE *threads = malloc(n * sizeof(THREAD_TYPE));

    for (size_t i = 0; i < n; i++) {
        sleep_arg_t *sa = malloc(sizeof(sleep_arg_t));
        sa->value = arr[i];
        sa->scale = scale;

        THREAD_CREATE(threads[i], print_after_sleep, sa);
    }

    for (size_t i = 0; i < n; i++) {
        THREAD_JOIN(threads[i]);
    }

    printf("\n");
    free(threads);
}