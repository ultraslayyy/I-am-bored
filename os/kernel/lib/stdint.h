#ifndef MY_STDINT
#define MY_STDINT

typedef unsigned char      uint8_t;
typedef unsigned short     uint16_t;
typedef int                int32_t;
typedef unsigned int       uint32_t;
typedef unsigned long long uint64_t;

#if defined(__SIZEOF_POINTER__)
    #if __SIZEOF_POINTER__ == 8
        typedef unsigned long long uintptr_t;
    #elif __SIZEOF_POINTER__ == 4
        typedef unsigned long uintptr_t;
    #else
        #error "Unsupported pointer size"
    #endif
#elif defined(_WIN64)
    typedef unsigned long long uintptr_t;
#elif defined(_WIN32)
    typedef unsigned long uintptr_t;
#else
    #error "Cannot determine pointer size"
#endif

#endif