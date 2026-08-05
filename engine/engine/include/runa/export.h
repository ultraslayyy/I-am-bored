#ifndef RUNA_EXPORT_H
#define RUNA_EXPORT_H

/**
 * When building as shared library, define RUNA_BUILD_SHARED
 * When compiling inside the engine, define RUNA_EXPORTS
 */

#if defined(_WIN32) || defined(_WIN64)
    #if defined(RUNA_BUILD_SHARED)
        #if defined(RUNA_EXPORTS)
            #define RUNA_API __declspec(dllexport)
        #else
            #define RUNA_API __declspec(dllimport)
        #endif
    #else
        #define RUNA_API
    #endif

#elif defined(__GNUC__) || defined(__clang__)
    #if defined(RUNA_BUILD_SHARED)
        #define RUNA_API __attribute__((visibility("default")))
    #else
        #define RUNA_API
    #endif
#else
    #define RUNA_API
#endif

#endif // RUNA_EXPORT_H