#ifndef MACROS_HPP
#define MACROS_HPP

#if defined(__GNUC__) || defined(__clang__)
    #include <xmmintrin.h> // _mm_prefetch
    #define PREFETCH(addr) _mm_prefetch(reinterpret_cast<const char*>(addr), _MM_HINT_T0)
    // branch predictor
    // __builtin_expect tells compiler which branch is more common
    // https://stackoverflow.com/questions/109710/likely-unlikely-macros-in-the-linux-kernel
    // This is easier in 20 [[likely]] and [[unlikely]] attributes
    #define likely(x)   __builtin_expect(!!(x), 1)
    #define unlikely(x) __builtin_expect(!!(x), 0)
#else
    #define PREFETCH(addr)
    #define likely(x)   (x)
    #define unlikely(x) (x)
#endif

#endif // MACROS_HPP