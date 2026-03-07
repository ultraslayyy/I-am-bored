#ifndef MY_STDDEF
#define MY_STDDEF

#ifndef NULL
#define NULL ((void *)0)
#endif

#if defined(__SIZEOF_POINTER__)
    #if __SIZEOF_POINTER__ == 8
        typedef unsigned long long size_t;
    #elif __SIZEOF_POINTER__ == 4
        typedef unsigned long size_t;
    #else
        #error "Unsupported pointer size"
    #endif
#elif defined(_WIN64)
    typedef unsigned long long size_t;
#elif defined(_WIN32)
    typedef unsigned long size_t;
#else
    #error "Cannot determine pointer size"
#endif

#define offsetof(type, member) ((size_t)((char *)&(((type *)0)->member) - (char *)0))

#endif