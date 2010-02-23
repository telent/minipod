/* #include <codecs/lib/codeclib.h> */

#define fixed32         int32_t
#define fixed64         int64_t
#define IBSS_ATTR
#define DEBUGF

#define fixmul32b(x, y)  \
    ({ int32_t __hi;  \
       uint32_t __lo;  \
       int32_t __result;  \
       asm ("smull   %0, %1, %3, %4\n\t"  \
            "mov     %2, %1, lsl #1"  \
            : "=&r" (__lo), "=&r" (__hi), "=r" (__result)  \
            : "%r" (x), "r" (y)  \
            : "cc" );  \
       __result;  \
    })


