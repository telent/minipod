#ifdef CPU_ARM

/*Sign-15.16 format */
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

#elif defined(CPU_COLDFIRE)

static inline int32_t fixmul32b(int32_t x, int32_t y)
{
    asm (
        "mac.l   %[x], %[y], %%acc0  \n" /* multiply */
        "movclr.l %%acc0, %[x]  \n"     /* get higher half */
        : [x] "+d" (x)
        : [y] "d"  (y)
    );
    return x;
}

#else

static inline fixed32 fixmul32b(fixed32 x, fixed32 y)
{
    fixed64 temp;

    temp = x;
    temp *= y;

    temp >>= 31;        //16+31-16 = 31 bits

    return (fixed32)temp;
}
#endif
