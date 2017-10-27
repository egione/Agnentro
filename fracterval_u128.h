/*
Fracterval U128
Copyright 2017 Russell Leidich

This collection of files constitutes the Fracterval U128 Library. (This is a
library in the abstact sense; it's not intended to compile to a ".lib"
file.)

The Fracterval U128 Library is free software: you can redistribute it and/or
modify it under the terms of the GNU Limited General Public License as
published by the Free Software Foundation, version 3.

The Fracterval U128 Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
Limited General Public License version 3 for more details.

You should have received a copy of the GNU Limited General Public
License version 3 along with the Fracterval U128 Library (filename
"COPYING"). If not, see http://www.gnu.org/licenses/ .
*/
/*
128-bit Unsigned Fracterval and Fractoid Macros

ASSUMPTIONS

The following assumptions apply to the entire Fracterval U128 codebase:

* {X, Y} means "a value at least (X/(2^128)) and at most ((Y+1)/(2^128))", where X and Y are taken as nonnegative. In other words, the "fracterval" (FRU128) {X, Y} is equivalent to the interval [X/K, (Y/K)+ULP], where K is 128 in this case, and "ULP" refers to a "unit of the last place", which is (2^(-K)). The fracterval {X, X} is known as the "fractoid" (FTD128) X, denoted "{X}", because it's a fraction on [X/K, (X/K)+ULP]. By contrast, the "mantissa" X is exactly equal to (X/K). Finally, the 128-bit whole number X is just a "u128" (U128).

* X never exceeds Y, which implies that the minimum uncertainty is (1/(2^128)). This constraint also applies to all outputs automatically. Otherwise there are no restrictions whatsoever on input values; all macros behave in a well-defined platform-independent manner without generating machine exceptions.

* All outputs {X, Y} shall be clipped to {0, ((2^128)-1)}, corresponding to the floating-point interval [0.0, 1.0]. Division by zero, including zero divided by zero, shall return ((2^128)-1). However, the low limit of a fracterval after division by zero may still be some valid and lesser value, depending on the divisor.

* When using these macros, terminate them with a semicolon, as with any C statement. When defining them, omit the semicolon.

* Inputs may be one and the same, e.g. _p and _q may be the same. Other than _z outputs, however, outputs may not be directed to an input variable. Inputs and outputs must be disjoint; if they overlap, use a macro ending in "_SELF". This partitioning is required in order to facilitate future optimization.

* In macro parameter lists, outputs shall appear to the left of inputs; within this constraint, numerators, minuends, and dividends appear to the left of denominators, subtrahends, and divisors, respectively.

* These macros should be used instead of calling C functions directly, so that they can be transparently optimized as needed for the best tradeoff between speed and instruction cache footprint.

* Shift counts must not exceed the MSB of the type in question (but _can_ result in loss of the MSB on left shift, which shall result in all ones); "U64_SHIFTED" macros shall convert from u64 to u128 prior to shifting left, but the shift count shall be limited to U64_BIT_MAX.

* Local variables inside U128 macros are preceded by 2 underscores in order to prevent symbol conflicts due to namespace merging with other macros which use them.

* By default, the "U128" macros don't perform overflow checking; those that end in "CHECKED" do.

VARIABLE NAMING CONVENTIONS

The code in the entire Fracterval U128 codebase mostly follows the guideline below, but as a practical matter, some exceptions are necessary.

* _a is the primary 128(x2)-bit output, which is a u128 fracterval, u128 fractoid, u128 mantissa, or u128 whole number. In "_SELF" macros, it's also an input.

* _b is a bit count, which should be a u8 for optimal performance but need not be.

* _c is a u8, such as a borrow or a carry.

* _d is analogous but secondary to _a.

* _i is an index private to a macro.

* _j is an index private to a macro.

* _l is the maximum ("limit") index of a list.

* _m is the base of a data structure, such as a list.

* _n is a ULONG, which may be an index into a list.

* _p is the primary 128(x2)-bit input, which is a u128 fracterval, u128 fractoid, u128 mantissa, or u128 whole number.

* _q is analogous but secondary to _p.

* _s is a status private to a macro.

* _t is the primary 64(x2)-bit output, which is a u64 fracterval, u64 fractoid, u64 mantissa, or u64 whole number. Note that this differs from the Fracterval U64 definition.

* _u is analogous but secondary to _t.

* _v is the primary 64(x2)-bit input, which is a u64 fracterval, u64 fractoid, u64 mantissa, or u64 whole number. Note that this differs from the Fracterval U64 definition.

* _w is analogous but secondary to _v.

* _z is an overflow status which is set to one when a result wholly or partially outside of [0.0, 1.0] is clipped to that interval. _z is otherwise unchanged, so that exceptions can occur in a silent and well-defined manner, to be discovered in some future context after the computation. (Merely equalling 0.0 or 1.0 should not set _z, but in theory that could occur due to precision limitations, in particular with transcendental functions.)
*/
TYPEDEF_START
  u128 a;
  u128 b;
TYPEDEF_END(fru128);

#define FTD128_2LOG2_RECIP_FLOOR_HI 0xB8AA3B295C17F0BBULL
#define FTD128_2LOG2_RECIP_FLOOR_LO 0xBE87FED0691D3E88ULL
#define FTD128_LOG2_FLOOR_HI 0xB17217F7D1CF79ABULL
#define FTD128_LOG2_FLOOR_LO 0xC9E3B39803F2F6AFULL

#ifdef _64_
  #define U128_ADD_U128(__a, __p, __q) \
    __a=(__p)+(__q)

  #define U128_ADD_U128_CHECK(__a, __p, __q, __z) \
    __a=(__p)+(__q); \
    __z=(u8)(__z|(__a<(__p)))

  #define U128_ADD_U128_SELF(__a, __p) \
    __a+=(__p)

  #define U128_ADD_U128_SELF_CHECK(__a, __p, __z) \
    __a+=(__p); \
    __z=(u8)(__z|(__a<(__p)))

  #define U128_ADD_U64_HI(__a, __p, __v) \
    __a=(__p)+((u128)(__v)<<64)

  #define U128_ADD_U64_HI_CHECK(__a, __p, __v, __z) \
    __a=(__p)+((u128)(__v)<<64); \
    __z=(u8)(__z|(__a<(__p)))

  #define U128_ADD_U64_HI_SELF(__a, __v) \
    __a+=((u128)(__v)<<64)

  #define U128_ADD_U64_HI_SELF_CHECK(__a, __v, __z) \
    do{ \
      u128 __p; \
      \
      __p=__a; \
      U128_ADD_U64_HI_CHECK(__a, __p, __v, __z); \
    }while(0)

  #define U128_ADD_U64_LO(__a, __p, __v) \
    __a=(__p)+(__v)

  #define U128_ADD_U64_LO_CHECK(__a, __p, __v, __z) \
    __a=(__p)+(__v); \
    __z=(u8)(__z|(__a<(__v)))

  #define U128_ADD_U64_LO_SELF(__a, __v) \
    __a+=(__v)

  #define U128_ADD_U64_LO_SELF_CHECK(__a, __v, __z) \
    __a+=(__v); \
    __z=(u8)(__z|(__a<(__v)))

  #define U128_ADD_U64_SHIFTED(__a, __b, __p, __v) \
    __a=(__p)+((u128)(__v)<<(__b))

  #define U128_ADD_U64_SHIFTED_SELF(__a, __b, __v) \
    __a+=((u128)(__v)<<(__b))

  #define U128_ADD_U8(__a, __c, __p); \
    __a=(__c)+(__p)

  #define U128_ADD_U8_HI(__a, __c, __p); \
    __a=((u128)(__c)<<64)+(__p)

  #define U128_ADD_U8_HI_SELF(__a, __c); \
    __a+=((u128)(__c)<<64)

  #define U128_ADD_U8_SELF(__a, __c); \
    __a+=(__c)

  #define U128_BIT_CLEAR(__a, __b, __p); \
    __a=(u128)(__p)&(~((u128)(1)<<(__b)))

  #define U128_BIT_CLEAR_SELF(__a, __b); \
    U128_BIT_CLEAR(__a, __b, __a)

  #define U128_BIT_FLIP(__a, __b, __p); \
    __a=(u128)(__p)^((u128)(1)<<(__b))

  #define U128_BIT_FLIP_SELF(__a, __b); \
    U128_BIT_FLIP(__a, __b, __a)

  #define U128_BIT_GET(__c, __b, __p); \
    __c=(u8)(((__p)>>(__b))&1)

  #define U128_BIT_SET(__a, __b, __p); \
    __a=(u128)(__p)|((u128)(1)<<(__b))

  #define U128_BIT_SET_SELF(__a, __b); \
    U128_BIT_SET(__a, __b, __a)

  #define U128_CHECKSUM_TO_U64(__t, __p, __v) \
    __t=(u64)((__p)>>64)+(u64)(__p)+(__v)

  #define U128_DECREMENT(__a, __p) \
    __a=(__p)-1

  #define U128_DECREMENT_SATURATE(__a, __p) \
    __a=(__p)-!!(__p)

  #define U128_DECREMENT_SATURATE_SELF(__a) \
    __a=(__a)-!!(__a)

  #define U128_DECREMENT_SELF(__a) \
    __a--

  #define U128_DECREMENT_U64_HI(__a, __p) \
    __a=(__p.b)-U64_MAX; \
    __a--

  #define U128_DECREMENT_U64_HI_SELF(__a) \
    __a-=U64_MAX; \
    __a--

  #define U128_DIVIDE_U128_SATURATE(__a, __p, __q, __z) \
    __z=(u8)(__z|u128_divide_u128_saturate(&__a, __p, __q))

  #define U128_DIVIDE_U128_SATURATE_SELF(__a, __p, __z) \
    __z=(u8)(__z|u128_divide_u128_saturate(&__a, __a, __p))

  #define U128_DIVIDE_U64_TO_U128_SATURATE(__a, __p, __v, __z) \
    __z=(u8)(__z|u128_divide_u64_to_u128_saturate(&__a, __p, __v))

  #define U128_DIVIDE_U64_TO_U128_SATURATE_SELF(__a, __v, __z) \
    __z=(u8)(__z|u128_divide_u64_to_u128_saturate(&__a, __a, __v))

  #define U128_DIVIDE_U64_TO_U64_SATURATE(__t, __p, __v, __z) \
    __z=(u8)(__z|u128_divide_u64_to_u64_saturate(&__t, __p, __v))

  #define U128_FROM_BOOL(__a, __c) \
    __a=0; \
    __a-=!!__c

  #define U128_FROM_U128_PAIR_BIT_IDX(__a, __b, __p, __q) \
    __a=(__p)>>(__b); \
    if(__b){ \
      __a+=(__q)<<(128-(__b)); \
    }

  #define U128_FROM_U64_HI(__a, __w) \
    __a=(u128)(__w)<<64

  #define U128_FROM_U64_HI_SATURATE(__a, __w) \
    __a=((u128)(__w)<<64)|U64_MAX;

  #define U128_FROM_U64_LO(__a, __v) \
    __a=(__v)

  #define U128_FROM_U64_PAIR(__a, __v, __w) \
    __a=((u128)(__w)<<64)|(__v)

  #define U128_FROM_U64_PRODUCT(__a, __v, __w) \
    __a=(u128)(__v)*(__w)

  #define U128_FROM_U64_SHIFTED(__a, __b, __v) \
    __a=(u128)(__v)<<(__b)

  #define U128_INCREMENT(__a, __p) \
    __a=(__p)+1

  #define U128_INCREMENT_SATURATE(__a, __p) \
    __a=(__p)+!!(~(__p))

  #define U128_INCREMENT_SATURATE_SELF(__a) \
    __a=(__a)+!!(~(__a))

  #define U128_INCREMENT_SELF(__a) \
    __a++

  #define U128_INCREMENT_U64_HI(__a, __p) \
    __a=(__p.b)+U64_MAX; \
    __a++

  #define U128_INCREMENT_U64_HI_SELF(__a) \
    __a+=U64_MAX; \
    __a++

  #define U128_IS_EQUAL(__p, __q) \
    ((__p)==(__q))

  #define U128_IS_LESS(__p, __q) \
    ((__p)<(__q))

  #define U128_IS_LESS_EQUAL(__p, __q) \
    ((__p)<=(__q))

  #define U128_IS_LESS_EQUAL_U64(__p, __q) \
    ((__p)<=(__q))

  #define U128_IS_LESS_U64(__p, __q) \
    ((__p)<(__q))

  #define U128_IS_NOT_EQUAL(__p, __q) \
    ((__p)!=(__q))

  #define U128_IS_NOT_EQUAL_U64(__p, __q) \
    ((__p)!=(__q))

  #define U128_IS_NOT_ONES(__p) \
    (!!(~(__p)))

  #define U128_IS_NOT_POWER_OF_2(__p) \
    (!!(p&(p-1)))

  #define U128_IS_NOT_SIGNED(__p) \
    (!((__p)>>127))

  #define U128_IS_NOT_ZERO(__p) \
    (!!(__p))

  #define U128_IS_ONES(__p) \
    (!(~(__p)))

  #define U128_IS_POWER_OF_2(__p) \
    (!(p&(p-1)))

  #define U128_IS_SIGNED(__p) \
    (!!((__p)>>127))

  #define U128_IS_ZERO(__p) \
    (!(__p))

  #define U128_MAX(__a, __p, __q) \
    __a=((__p)<=(__q))?(__q):(__p)

  #define U128_MAX_SELF(__a, __p) \
    __a=((__a)<=(__p))?(__p):(__a)

  #define U128_MEAN_FLOOR(__a, __p, __q) \
    __a=((__p)>>1)+((__q)>>1)+(1&(__p)&(__q))

  #define U128_MIN(__a, __p, __q) \
    __a=((__p)<=(__q))?(__p):(__q)

  #define U128_MIN_SELF(__a, __p) \
    __a=((__a)<=(__p))?(__a):(__p)

  #define U128_MSB_GET(__b, __p) \
    __b=u128_msb__get(__p)

  #define U128_MULTIPLY_U64_SATURATE(__a, __p, __v, __z) \
    __z=(u8)(__z|u128_multiply_u64_saturate(&__a, __p, __v))

  #define U128_MULTIPLY_U64_SATURATE_SELF(__a, __v, __z) \
    __z=(u8)(__z|u128_multiply_u64_saturate(&__a, __a, __v))

  #define U128_NEGATE(__a, __p) \
    __a=0U-(__p);

  #define U128_NEGATE_SELF(__a) \
    __a=0U-__a;

  #define U128_NOT(__a, __p) \
    __a=~(__p);

  #define U128_NOT_SELF(__a) \
    __a=~__a;

  #define U128_SET_ONES(__a) \
    __a=0; \
    __a=~__a

  #define U128_SET_SPAN_HALF(__a) \
    __a=(u128)(1)<<127

  #define U128_SET_SPAN_HALF_MINUS_1(__a) \
    __a=~((u128)(1)<<127)

  #define U128_SET_ULP(__a) \
    __a=1

  #define U128_SET_ZERO(__a) \
    __a=0

  #define U128_SHIFT_LEFT(__a, __b, __p) \
    __a=(__p)<<(__b)

  #define U128_SHIFT_LEFT_CHECK(__a, __b, __p, __z) \
    __a=(__p)<<(__b); \
    __z=(u8)(__z|((__a>>(__b))!=(__p)))

  #define U128_SHIFT_LEFT_SELF(__a, __b) \
    __a<<=(__b)

  #define U128_SHIFT_LEFT_SELF_CHECK(__a, __b, __z) \
    do{ \
      u128 __p; \
      \
      __p=__a; \
      U128_SHIFT_LEFT_CHECK(__a, __b, __p, __z); \
    }while(0)

  #define U128_SHIFT_RIGHT(__a, __b, __p) \
    __a=(__p)>>(__b)

  #define U128_SHIFT_RIGHT_SELF(__a, __b) \
    __a>>=(__b)

  #define U128_SHIFTED_TO_U64(__t, __b, __p) \
    __t=(u64)((__p)>>(__b))

  #define U128_SUBTRACT_FROM_U128_SELF(__a, __p) \
    __a=(__p)-__a;

  #define U128_SUBTRACT_U128(__a, __p, __q) \
    __a=(__p)-(__q)

  #define U128_SUBTRACT_U128_SELF(__a, __p) \
    __a-=(__p)

  #define U128_SUBTRACT_U64_HI(__a, __p, __v) \
    __a=(__p)-((u128)(__v)<<64)

  #define U128_SUBTRACT_U64_HI_CHECK(__a, __p, __v, __z) \
    __a=((u128)(__v)<<64); \
    __z=(u8)(__z|((__p)<__a)); \
    __a=(__p)-__a

  #define U128_SUBTRACT_U64_HI_SELF(__a, __v) \
    __a-=((u128)(__v)<<64)

  #define U128_SUBTRACT_U64_HI_SELF_CHECK(__a, __v, __z) \
    do{ \
      u128 __p; \
      \
      __p=__a; \
      U128_SUBTRACT_U64_HI_CHECK(__a, __p, __v, __z); \
    }while(0)

  #define U128_SUBTRACT_U64_LO(__a, __p, __v) \
    __a=(__p)-(__v)

  #define U128_SUBTRACT_U64_LO_CHECK(__a, __p, __v, __z) \
    __z=(u8)(__z|((__p)<(__v))); \
    __a=(__p)-(__v)

  #define U128_SUBTRACT_U64_LO_SELF(__a, __v) \
    __a-=(__v)

  #define U128_SUBTRACT_U64_LO_SELF_CHECK(__a, __v, __z) \
    do{ \
      u128 __p; \
      \
      __p=__a; \
      U128_SUBTRACT_U64_LO_CHECK(__a, __p, __v, __z); \
    }while(0)

  #define U128_SUBTRACT_U64_SHIFTED(__a, __b, __p, __v) \
    __a=(__p)-((u128)(__v)<<(__b))

  #define U128_SUBTRACT_U64_SHIFTED_SELF(__a, __b, __v) \
    __a-=((u128)(__v)<<(__b))

  #define U128_SWAP(__p, __q) \
    (__p)^=(__q); \
    (__q)^=(__p); \
    (__p)^=(__q)

  #define U128_TO_OR_U64_LO(__t, __p) \
    __t|=(u64)(__p)

  #define U128_TO_U64_HI(__u, __p) \
    __u=(u64)((__p)>>64)

  #define U128_TO_U64_LO(__t, __p) \
    __t=(u64)(__p)

  #define U128_TO_U64_PAIR(__t, __u, __p) \
    __t=(u64)(__p); \
    __u=(u64)((__p)>>64)
#else
  #define U128_ADD_U128(__a, __p, __q) \
    __a.a=(__p.a)+(__q.a); \
    __a.b=(__p.b)+(__q.b); \
    __a.b+=(__a.a<(__p.a))

  #define U128_ADD_U128_CHECK(__a, __p, __q, __z) \
    __a.a=(__p.a)+(__q.a); \
    __a.b=(__p.b)+(__q.b); \
    __z=(u8)(__z|(__a.b<(__p.b))); \
    if(__a.a<(__p.a)){ \
      __a.b++; \
      __z=(u8)(__z|!__a.b); \
    }

  #define U128_ADD_U128_SELF(__a, __p) \
    __a.a+=(__p.a); \
    __a.b+=(__p.b)+(__a.a<(__p.a))

  #define U128_ADD_U128_SELF_CHECK(__a, __p, __z) \
    __a.a+=(__p.a); \
    __a.b+=(__p.b); \
    __z=(u8)(__z|(__a.b<(__p.b))); \
    if(__a.a<(__p.a)){ \
      __a.b++; \
      __z=(u8)(__z|!__a.b); \
    }

  #define U128_ADD_U64_HI(__a, __p, __v) \
    __a.a=__p.a; \
    __a.b=(__p.b)+(__v)

  #define U128_ADD_U64_HI_CHECK(__a, __p, __v, __z) \
    __a.a=__p.a; \
    __a.b=(__p.b)+(__v); \
    __z=(u8)(__z|(__a.b<(__v)))

  #define U128_ADD_U64_HI_SELF(__a, __v) \
    __a.b+=(__v)

  #define U128_ADD_U64_HI_SELF_CHECK(__a, __v, __z) \
    __a.b+=(__v); \
    __z=(u8)(__z|(__a.b<(__v)))

  #define U128_ADD_U64_LO(__a, __p, __v) \
    __a.a=(__p.a)+(__v); \
    __a.b=(__p.b)+(__a.a<(__v))

  #define U128_ADD_U64_LO_CHECK(__a, __p, __v, __z) \
    __a.a=(__p.a)+(__v); \
    __a.b=(__p.b); \
    if(__a.a<(__v)){ \
      __a.b++; \
      __z=(u8)(__z|!__a.b); \
    }

  #define U128_ADD_U64_LO_SELF(__a, __v) \
    __a.a+=(__v); \
    __a.b+=(__a.a<(__v))

  #define U128_ADD_U64_LO_SELF_CHECK(__a, __v, __z) \
    __a.a+=(__v); \
    if(__a.a<(__v)){ \
      __a.b++; \
      __z=(u8)(__z|!__a.b); \
    }

  #define U128_ADD_U64_SHIFTED(__a, __b, __p, __v) \
    __a.a=(__p.a)+((__v)<<(__b)); \
    __a.b=(__p.b)+(__a.a<(__p.a)); \
    if(__b){ \
      __a.b+=(__v)>>(64-(__b)); \
    }

  #define U128_ADD_U64_SHIFTED_SELF(__a, __b, __v) \
    do{ \
      u64 __w; \
      \
      __w=(__v)<<(__b); \
      __a.a+=__w; \
      __a.b+=(__a.a<__w); \
      if(__b){ \
        __a.b+=(__v)>>(64-(__b)); \
      } \
    }while(0)

  #define U128_ADD_U8(__a, __c, __p); \
    __a.a=(__c)+(__p.a); \
    __a.b=(__a.a<(__p.a))+(__p.b)

  #define U128_ADD_U8_HI(__a, __c, __p); \
    __a.a=(__p.a); \
    __a.b=(__c)+(__p.b)

  #define U128_ADD_U8_HI_SELF(__a, __c); \
    __a.b+=(__c)

  #define U128_ADD_U8_SELF(__a, __c); \
    __a.a+=(__c); \
    __a.b+=(__a.a<(__c))

  #define U128_BIT_CLEAR(__a, __b, __p); \
    do{ \
      u64 __q; \
      \
      __a.a=__p.a; \
      __a.b=__p.b; \
      __q=~(1ULL<<((__b)&63)); \
      if((__b)<=63){ \
        __a.a&=__q; \
      }else{ \
        __a.b&=__q; \
      } \
    }while(0)

  #define U128_BIT_CLEAR_SELF(__a, __b); \
    do{ \
      u64 __q; \
      \
      __q=~(1ULL<<((__b)&63)); \
      if((__b)<=63){ \
        __a.a&=__q; \
      }else{ \
        __a.b&=__q; \
      } \
    }while(0)

  #define U128_BIT_FLIP(__a, __b, __p); \
    do{ \
      u64 __q; \
      \
      __q=1ULL<<((__b)&63); \
      if((__b)<=63){ \
        __a.a=__p.a^__q; \
        __a.b=__p.b; \
      }else{ \
        __a.a=__p.a; \
        __a.b=__p.b^__q; \
      } \
    }while(0)

  #define U128_BIT_FLIP_SELF(__a, __b); \
    U128_BIT_FLIP(__a, __b, __a)

  #define U128_BIT_GET(__c, __b, __p); \
    if((__b)<=63){ \
      __c=(u8)((__p.a)>>(__b)); \
    }else{ \
      __c=(u8)((__p.b)>>((__b)&63)); \
    } \
    __c=(u8)(__c&1)

  #define U128_BIT_SET(__a, __b, __p); \
    do{ \
      u64 __q; \
      \
      __a.a=__p.a; \
      __a.b=__p.b; \
      __q=~(1ULL<<((__b)&63)); \
      if((__b)<=63){ \
        __a.a|=__q; \
      }else{ \
        __a.b|=__q; \
      } \
    }while(0)

  #define U128_BIT_SET_SELF(__a, __b); \
    do{ \
      u64 __q; \
      \
      __q=~(1ULL<<((__b)&63)); \
      if((__b)<=63){ \
        __a.a|=__q; \
      }else{ \
        __a.b|=__q; \
      } \
    }while(0)

  #define U128_CHECKSUM_TO_U64(__t, __p, __v) \
    __t=(__p.a)+(__p.b)+(__v)

  #define U128_DECREMENT(__a, __p) \
    __a.b=(__p.b)-!(__p.a); \
    __a.a=(__p.a)-1

  #define U128_DECREMENT_SATURATE(__a, __p) \
    __a.a=0; \
    __a.b=0; \
    if((__p.a)|(__p.b)){ \
      __a.b=(__p.b)-!(__p.a); \
      __a.a=(__p.a)-1; \
    }

  #define U128_DECREMENT_SATURATE_SELF(__a) \
    if(__a.a|__a.b){ \
      __a.b-=!__a.a; \
      __a.a--; \
    }

  #define U128_DECREMENT_SELF(__a) \
    __a.b-=!__a.a; \
    __a.a--

  #define U128_DECREMENT_U64_HI(__a, __p) \
    __a.a=(__p.a); \
    __a.b=(__p.b)-1

  #define U128_DECREMENT_U64_HI_SELF(__a) \
    __a.b--

  #define U128_DIVIDE_U128_SATURATE(__a, __p, __q, __z) \
    __z=(u8)(__z|u128_divide_u128_saturate(&__a, __p, __q))

  #define U128_DIVIDE_U128_SATURATE_SELF(__a, __p, __z) \
    __z=(u8)(__z|u128_divide_u128_saturate(&__a, __a, __p))

  #define U128_DIVIDE_U64_TO_U128_SATURATE(__a, __p, __v, __z) \
    __z=(u8)(__z|u128_divide_u64_to_u128_saturate(&__a, __p, __v))

  #define U128_DIVIDE_U64_TO_U128_SATURATE_SELF(__a, __v, __z) \
    __z=(u8)(__z|u128_divide_u64_to_u128_saturate(&__a, __a, __v))

  #define U128_DIVIDE_U64_TO_U64_SATURATE(__t, __p, __v, __z) \
    __z=(u8)(__z|u128_divide_u64_to_u64_saturate(&__t, __p, __v))

  #define U128_FROM_BOOL(__a, __c) \
    __a.a=!__c; \
    __a.a--; \
    __a.b=__a.a

  #define U128_FROM_U128_PAIR_BIT_IDX(__a, __b, __p, __q) \
    __a=u128_from_u128_pair_bit_idx(__b, __p, __q)

  #define U128_FROM_U64_HI(__a, __w) \
    __a.a=0; \
    __a.b=(__w)

  #define U128_FROM_U64_HI_SATURATE(__a, __w) \
    __a.a=0; \
    __a.a=~__a.a; \
    __a.b=(__w)

  #define U128_FROM_U64_LO(__a, __v) \
    __a.a=(__v); \
    __a.b=0

  #define U128_FROM_U64_PAIR(__a, __v, __w) \
    __a.a=(__v); \
    __a.b=(__w)

  #define U128_FROM_U64_PRODUCT(__a, __v, __w) \
    __a=u128_from_u64_product(__v, __w)

  #define U128_FROM_U64_SHIFTED(__a, __b, __v) \
    __a.a=(__v)<<(__b); \
    __a.b=0; \
    if(__b){ \
      __a.b=(__v)>>(64-(__b)); \
    }

  #define U128_INCREMENT(__a, __p) \
    __a.a=(__p.a)+1; \
    __a.b=(__p.b)+!__a.a

  #define U128_INCREMENT_SATURATE(__a, __p) \
    __a.a=(__p.a)+1; \
    __a.b=(__p.b); \
    if(!__a.a){ \
      __a.b++; \
      if(!__a.b){ \
        __a.a=~__a.a; \
        __a.b=~__a.b; \
      } \
    }

  #define U128_INCREMENT_SATURATE_SELF(__a) \
    if(~(__a.a&__a.b)){ \
      __a.a++; \
      __a.b+=!__a.a; \
    }

  #define U128_INCREMENT_SELF(__a) \
    __a.a++; \
    __a.b+=!__a.a

  #define U128_INCREMENT_U64_HI(__a, __p) \
    __a.a=(__p.a); \
    __a.b=(__p.b)+1

  #define U128_INCREMENT_U64_HI_SELF(__a) \
    __a.b++

  #define U128_IS_EQUAL(__p, __q) \
    (((__p.a)==(__q.a))&&((__p.b)==(__q.b)))

  #define U128_IS_EQUAL_U64(__p, __q) \
    ((__p.a)==(__q))

  #define U128_IS_LESS(__p, __q) \
    (((__p.b)<(__q.b))||(((__p.b)==(__q.b))&&((__p.a)<(__q.a))))

  #define U128_IS_LESS_EQUAL(__p, __q) \
    (((__p.b)<(__q.b))||(((__p.b)==(__q.b))&&((__p.a)<=(__q.a))))

  #define U128_IS_LESS_EQUAL_U64(__p, __q) \
    ((!(__p.b))&&((__p.a)<=(__q)))

  #define U128_IS_LESS_U64(__p, __q) \
    ((!(__p.b))&&((__p.a)<(__q)))

  #define U128_IS_NOT_EQUAL(__p, __q) \
    (((__p.a)!=(__q.a))||((__p.b)!=(__q.b)))

  #define U128_IS_NOT_EQUAL_U64(__p, __q) \
    ((__p.b)||((__p.a)!=(__q)))

  #define U128_IS_NOT_ONES(__p) \
    ((~(__p.a))||(~(__p.b)))

  #define U128_IS_NOT_POWER_OF_2(__p) \
    ((p.a&(p.a-1))||(p.b&(p.b-1)))

  #define U128_IS_NOT_SIGNED(__p) \
    (!((__p.b)>>63))

  #define U128_IS_NOT_ZERO(__p) \
    ((__p.a)||(__p.b))

  #define U128_IS_ONES(__p) \
    (!(~((__p.a)&(__p.b))))

  #define U128_IS_POWER_OF_2(__p) \
    (!((p.a&(p.a-1))||(p.b&(p.b-1))))

  #define U128_IS_SIGNED(__p) \
    (!!((__p.b)>>63))

  #define U128_IS_ZERO(__p) \
    (!((__p.a)|(__p.b)))

  #define U128_MAX(__a, __p, __q) \
    __a.a=(__p.a); \
    __a.b=(__p.b); \
    if(((__p.b)<(__q.b))||(((__p.b)==(__q.b))&&((__p.a)<(__q.a)))){ \
      __a.a=(__q.a); \
      __a.b=(__q.b); \
    }

  #define U128_MAX_SELF(__a, __p) \
    if(((__a.b)<(__p.b))||(((__a.b)==(__p.b))&&((__a.a)<(__p.a)))){ \
      __a.a=(__p.a); \
      __a.b=(__p.b); \
    }

  #define U128_MEAN_FLOOR(__a, __p, __q) \
    do{ \
      u64 __r; \
      \
      __a.a=((__p.a)>>1)+((__q.a)>>1); \
      __a.b=((__p.b)>>1)+((__q.b)>>1); \
      __r=(__p.b)<<63; \
      __a.a+=__r; \
      __a.b+=(__a.a<__r); \
      __r=(__q.b)<<63; \
      __a.a+=__r; \
      __a.b+=(__a.a<__r); \
      __r=1&(__p.a)&(__q.a); \
      __a.a+=__r; \
      __a.b+=(__a.a<__r); \
    }while(0)

  #define U128_MIN(__a, __p, __q) \
    __a.a=(__p.a); \
    __a.b=(__p.b); \
    if(((__q.b)<(__p.b))||(((__p.b)==(__q.b))&&((__q.a)<(__p.a)))){ \
      __a.a=(__q.a); \
      __a.b=(__q.b); \
    }

  #define U128_MIN_SELF(__a, __p) \
    if(((__a.b)<(__p.b))||(((__a.b)==(__p.b))&&((__a.a)<(__p.a)))){ \
      __a.a=(__p.a); \
      __a.b=(__p.b); \
    }

  #define U128_MSB_GET(__b, __p) \
    __b=u128_msb__get(__p)

  #define U128_MULTIPLY_U64_SATURATE(__a, __p, __v, __z) \
    __z=(u8)(__z|u128_multiply_u64_saturate(&__a, __p, __v))

  #define U128_MULTIPLY_U64_SATURATE_SELF(__a, __v, __z) \
    __z=(u8)(__z|u128_multiply_u64_saturate(&__a, __a, __v))

  #define U128_NEGATE(__a, __p) \
    __a.a=0U-(__p.a); \
    __a.b=0U-(__p.b)-!!__a.a

  #define U128_NEGATE_SELF(__a) \
    __a.a=0U-__a.a; \
    __a.b=0U-__a.b-!!__a.a

  #define U128_NOT(__a, __p) \
    __a.a=~(__p.a); \
    __a.b=~(__p.b)

  #define U128_NOT_SELF(__a) \
    __a.a=~__a.a; \
    __a.b=~__a.b

  #define U128_SET_ONES(__a) \
    __a.a=0; \
    __a.a=~__a.a; \
    __a.b=__a.a

  #define U128_SET_SPAN_HALF(__a) \
    __a.a=0; \
    __a.b=U64_SPAN_HALF

  #define U128_SET_SPAN_HALF_MINUS_1(__a) \
    __a.a=U64_MAX; \
    __a.b=U64_SPAN_HALF-1

  #define U128_SET_ULP(__a) \
    __a.b=0; \
    __a.a=__a.b+1

  #define U128_SET_ZERO(__a) \
    __a.a=0; \
    __a.b=__a.a

  #define U128_SHIFT_LEFT(__a, __b, __p) \
    if((__b)<=63){ \
      __a.b=(__p.b)<<((__b)&63); \
      if(((__b)&63)){ \
        __a.b+=(__p.a)>>(u8)((64-(__b))&63); \
      } \
      __a.a=(__p.a)<<((__b)&63); \
    }else{ \
      __a.b=(__p.a)<<((__b)&63); \
      __a.a=0; \
    }

  #define U128_SHIFT_LEFT_CHECK(__a, __b, __p, __z) \
    if((__b)<=63){ \
      __a.b=(__p.b)<<((__b)&63); \
      if(((__b)&63)){ \
        __a.b+=(__p.a)>>(u8)((64-(__b))&63); \
      } \
      __a.a=(__p.a)<<((__b)&63); \
      __z=(u8)(__z|((__a.b>>((__b)&63))!=(__p.b))); \
    }else{ \
      (__a.b)=(__p.a)<<(u8)((__b)&63); \
      (__a.a)=0; \
      __z=(u8)(__z|((__p.b)||(((__a.b)>>(u8)((__b)&63))!=(__p.a)))); \
    }

  #define U128_SHIFT_LEFT_SELF(__a, __b) \
    if((__b)<=63){ \
      __a.b<<=(__b)&63; \
      if(((__b)&63)){ \
        __a.b+=__a.a>>(u8)((64-(__b))&63); \
      } \
      __a.a<<=(__b)&63; \
    }else{ \
      __a.b=__a.a<<((__b)&63); \
      __a.a=0; \
    }

  #define U128_SHIFT_LEFT_SELF_CHECK(__a, __b, __z) \
    do{ \
      u128 __p; \
      \
      __p.a=__a.a; \
      __p.b=__a.b; \
      U128_SHIFT_LEFT_CHECK(__a, __b, __p, __z); \
    }while(0)

  #define U128_SHIFT_RIGHT(__a, __b, __p) \
    if((__b)<=63){ \
      __a.a=(__p.a)>>((__b)&63); \
      if((__b)&63){ \
        __a.a+=(__p.b)<<(u8)((64-(__b))&63); \
      } \
      __a.b=(__p.b)>>((__b)&63); \
    }else{ \
      __a.a=(__p.b)>>(u8)((__b)&63); \
      __a.b=0; \
    }

  #define U128_SHIFT_RIGHT_SELF(__a, __b) \
    if((__b)<=63){ \
      __a.a>>=(__b)&63; \
      if((__b)&63){ \
        __a.a+=__a.b<<(u8)((64-(__b))&63); \
      } \
      __a.b>>=(__b)&63; \
    }else{ \
      __a.a=__a.b>>(u8)((__b)&63); \
      __a.b=0; \
    }

  #define U128_SHIFTED_TO_U64(__t, __b, __p) \
    __t=((__p.a)>>(__b))|((__p.b)<<(64-(__b)))

  #define U128_SUBTRACT_FROM_U128_SELF(__a, __p) \
    __a.b=(__p.b)-__a.b-((__p.a)<__a.a); \
    __a.a=(__p.a)-__a.a

  #define U128_SUBTRACT_U128(__a, __p, __q) \
    __a.b=(__p.b)-((__p.a)<(__q.a)); \
    __a.a=(__p.a)-(__q.a); \
    __a.b-=(__q.b)

  #define U128_SUBTRACT_U128_SELF(__a, __p) \
    __a.b-=(__p.b)+(__a.a<(__p.a)); \
    __a.a-=(__p.a)

  #define U128_SUBTRACT_U64_HI(__a, __p, __v) \
    __a.b=(__p.b)-(__v)

  #define U128_SUBTRACT_U64_HI_CHECK(__a, __p, __v, __z) \
    __z=(u8)(__z|((__p.b)<(__v))); \
    __a.b=(__p.b)-(__v)

  #define U128_SUBTRACT_U64_HI_SELF(__a, __v) \
    __a.b-=(__v)

  #define U128_SUBTRACT_U64_HI_SELF_CHECK(__a, __v, __z) \
    __z=(u8)(__z|(__a.b<(__v))); \
    __a.b-=(__v)

  #define U128_SUBTRACT_U64_LO(__a, __p, __v) \
    __a.b-=((__p.a)<(__v)); \
    __a.a=(__p.a)-(__v)

  #define U128_SUBTRACT_U64_LO_CHECK(__a, __p, __v, __z) \
    __z=(u8)(__z|((!__p.b)&&((__p.a)<(__v)))); \
    __a.b-=((__p.a)<(__v)); \
    __a.a=(__p.a)-(__v)

  #define U128_SUBTRACT_U64_LO_SELF(__a, __v) \
    __a.b-=((__a.a)<(__v)); \
    __a.a-=(__v)

  #define U128_SUBTRACT_U64_LO_SELF_CHECK(__a, __v, __z) \
    __z=(u8)(__z|((!__a.b)&&((__a.a)<(__v)))); \
    __a.b-=((__a.a)<(__v)); \
    __a.a-=(__v)

  #define U128_SUBTRACT_U64_SHIFTED(__a, __b, __p, __v) \
    if(__b){ \
      __a.b=(__p.b)-((__v)>>(64-(__b))); \
    } \
    __a.a=(__p.a)-((__v)<<(__b)); \
    __a.b-=((__p.a)<__a.a)

  #define U128_SUBTRACT_U64_SHIFTED_SELF(__a, __b, __v) \
    do{ \
      u64 __w; \
      \
      if(__b){ \
        __a.b-=(__v)>>(64-(__b)); \
      } \
      __w=__a.a; \
      __a.a-=((__v)<<(__b)); \
      __a.b-=(__w<__a.a); \
    }while(0)

  #define U128_SWAP(__p, __q) \
    (__p.a)^=(__q.a); \
    (__p.b)^=(__q.b); \
    (__q.a)^=(__p.a); \
    (__q.b)^=(__p.b); \
    (__p.a)^=(__q.a); \
    (__p.b)^=(__q.b)

  #define U128_TO_OR_U64_LO(__t, __p) \
    __t|=(__p.a)

  #define U128_TO_U64_HI(__u, __p) \
    __u=(__p.b)

  #define U128_TO_U64_LO(__t, __p) \
    __t=(__p.a)

  #define U128_TO_U64_PAIR(__t, __u, __p) \
    __t=(__p.a); \
    __u=(__p.b)
#endif

#define FRU128_ADD_FRU128(_a, _p, _q, _z) \
  U128_ADD_U128(_a.a, _p.a, _q.a); \
  U128_ADD_U128(_a.b, _p.b, _q.b); \
  U128_INCREMENT_SELF(_a.b); \
  if(U128_IS_LESS_EQUAL(_a.b, _q.b)){ \
    U128_SET_ONES(_a.b); \
    _z=1; \
    if(U128_IS_LESS(_a.a, _q.a)){ \
      _a.a=_a.b; \
    } \
  }

#define FRU128_ADD_FRU128_SELF(_a, _p, _z) \
  U128_ADD_U128_SELF(_a.a, _p.a); \
  U128_ADD_U128_SELF(_a.b, _p.b); \
  U128_INCREMENT_SELF(_a.b); \
  if(U128_IS_LESS_EQUAL(_a.b, _p.b)){ \
    U128_SET_ONES(_a.b); \
    _z=1; \
    if(U128_IS_LESS(_a.a, _p.a)){ \
      _a.a=_a.b; \
    } \
  }

#define FRU128_ADD_FRU64_HI(_a, _p, _v, _z) \
  U128_ADD_U64_HI(_a.a, _p.a, _v.a); \
  U128_ADD_U64_HI(_a.b, _p.b, _v.b); \
  U128_INCREMENT_U64_HI_SELF(_a.b); \
  if(U128_IS_LESS_EQUAL(_a.b, _p.b)){ \
    U128_SET_ONES(_a.b); \
    _z=1; \
    if(U128_IS_LESS(_a.a, _p.a)){ \
      _a.a=_a.b; \
    } \
  }

#define FRU128_ADD_FRU64_HI_SELF(_a, _v, _z) \
  do{ \
    fru128 _q; \
    \
    _q=_a; \
    FRU128_ADD_FRU64_HI(_a, _q, _v, _z); \
  }while(0)

#define FRU128_ADD_FRU64_LO(_a, _p, _v, _z) \
  U128_ADD_U64_LO(_a.a, _p.a, _v.a); \
  U128_ADD_U64_LO(_a.b, _p.b, _v.b); \
  U128_INCREMENT_SELF(_a.b); \
  if(U128_IS_LESS_EQUAL_U64(_a.b, _v.b)){ \
    U128_SET_ONES(_a.b); \
    _z=1; \
    if(U128_IS_LESS_U64(_a.a, _v.a)){ \
      _a.a=_a.b; \
    } \
  }

#define FRU128_ADD_FRU64_LO_SELF(_a, _v, _z) \
  U128_ADD_U64_LO_SELF(_a.a, _v.a); \
  U128_ADD_U64_LO_SELF(_a.b, _v.b); \
  U128_INCREMENT_SELF(_a.b); \
  if(U128_IS_LESS_EQUAL_U64(_a.b, _v.b)){ \
    U128_SET_ONES(_a.b); \
    _z=1; \
    if(U128_IS_LESS_U64(_a.a, _v.a)){ \
      _a.a=_a.b; \
    } \
  }

#define FRU128_ADD_FRU64_SHIFTED(_a, _b, _p, _v, _z) \
  do{ \
    u128 _r; \
    \
    U128_FROM_U64_LO(_r, _v.b); \
    U128_INCREMENT_SELF(_r); \
    U128_SHIFT_LEFT_SELF(_r, _b); \
    U128_ADD_U128(_a.b, _p.b, _r); \
    U128_ADD_U64_SHIFTED(_a.a, _b, _p.a, _v.a); \
    if(U128_IS_LESS(_a.b, _p.b)){ \
      U128_SET_ONES(_a.b); \
      _z=1; \
      if(U128_IS_LESS(_a.a, _p.a)){ \
        _a.a=_a.b; \
      } \
    } \
  }while(0)

#define FRU128_ADD_FRU64_SHIFTED_SELF(_a, _b, _v, _z) \
  do{ \
    fru128 _q; \
    \
    _q=_a; \
    FRU128_ADD_FRU64_SHIFTED(_a, _b, _q, _v, _z); \
  }while(0)

#define FRU128_ADD_FTD128(_a, _p, _q, _z) \
  U128_ADD_U128(_a.a, _p.a, _q); \
  U128_ADD_U128(_a.b, _p.b, _q); \
  U128_INCREMENT_SELF(_a.b); \
  if(U128_IS_LESS_EQUAL(_a.b, _q)){ \
    U128_SET_ONES(_a.b); \
    _z=1; \
    if(U128_IS_LESS(_a.a, _q)){ \
      _a.a=_a.b; \
    } \
  }

#define FRU128_ADD_FTD128_SELF(_a, _p, _z) \
  U128_ADD_U128_SELF(_a.a, _p); \
  U128_ADD_U128_SELF(_a.b, _p); \
  U128_INCREMENT_SELF(_a.b); \
  if(U128_IS_LESS_EQUAL(_a.b, _p)){ \
    U128_SET_ONES(_a.b); \
    _z=1; \
    if(U128_IS_LESS(_a.a, _p)){ \
      _a.a=_a.b; \
    } \
  }

#define FRU128_ADD_U64(_a, _p, _v, _z) \
  U128_ADD_U64_LO(_a.a, _p.a, _v); \
  U128_ADD_U64_LO(_a.b, _p.b, _v); \
  if(U128_IS_LESS_U64(_a.b, _v)){ \
    U128_SET_ONES(_a.b); \
    _z=1; \
    if(U128_IS_LESS_U64(_a.a, _v)){ \
      _a.a=_a.b; \
    } \
  }

#define FRU128_ADD_U64_HI(_a, _p, _v, _z) \
  U128_ADD_U64_HI(_a.a, _p.a, _v); \
  U128_ADD_U64_HI(_a.b, _p.b, _v); \
  if(U128_IS_LESS(_a.b, _p.b)){ \
    U128_SET_ONES(_a.b); \
    _z=1; \
    if(U128_IS_LESS(_a.a, _p.a)){ \
      _a.a=_a.b; \
    } \
  }

#define FRU128_ADD_U64_HI_SELF(_a, _v, _z) \
  do{ \
    fru128 _p; \
    \
    _p.a=_a.a; \
    U128_ADD_U64_HI_SELF(_a.a, _v); \
    _p.b=_a.b; \
    U128_ADD_U64_HI_SELF(_a.b, _v); \
    if(U128_IS_LESS(_a.b, _p.b)){ \
      U128_SET_ONES(_a.b); \
      _z=1; \
      if(U128_IS_LESS(_a.a, _p.a)){ \
        _a.a=_a.b; \
      } \
    } \
  }while(0)

#define FRU128_ADD_U64_SELF(_a, _v, _z) \
  U128_ADD_U64_LO_SELF(_a.a, _v); \
  U128_ADD_U64_LO_SELF(_a.b, _v); \
  if(U128_IS_LESS_U64(_a.b, _v)){ \
    U128_SET_ONES(_a.b); \
    _z=1; \
    if(U128_IS_LESS_U64(_a.a, _v)){ \
      _a.a=_a.b; \
    } \
  }

#define FRU128_DIVIDE_FRU128(_a, _p, _q, _z) \
  _z=(u8)(_z|fracterval_u128_divide_fracterval_u128(&_a, _p, _q))

#define FRU128_DIVIDE_FRU128_SELF(_a, _p, _z) \
  _z=(u8)(_z|fracterval_u128_divide_fracterval_u128(&_a, _a, _p))

#define FRU128_DIVIDE_U128(_a, _p, _q, _z) \
  _z=(u8)(_z|fracterval_u128_divide_u128(&_a, _p, _q))

#define FRU128_DIVIDE_U128_SELF(_a, _v, _z) \
  _z=(u8)(_z|fracterval_u128_divide_u128(&_a, _a, _v))

#define FRU128_DIVIDE_U64(_a, _p, _v, _z) \
  _z=(u8)(_z|fracterval_u128_divide_u64(&_a, _p, _v))

#define FRU128_DIVIDE_U64_SELF(_a, _v, _z) \
  _z=(u8)(_z|fracterval_u128_divide_u64(&_a, _a, _v))

#define FRU128_EXPAND_DOWN(_a, _p, _z) \
  U128_DECREMENT(_a.a, _p.a); \
  if(U128_IS_ZERO(_p.a)){ \
    U128_INCREMENT_SELF(_a.a); \
    _z=1; \
  }

#define FRU128_EXPAND_DOWN_SELF(_a, _z) \
  if(U128_IS_NOT_ZERO(_a.a)){ \
    U128_DECREMENT_SELF(_a.a); \
  }else{ \
    _z=1; \
  }

#define FRU128_EXPAND_UP(_a, _p, _z) \
  U128_INCREMENT(_a.b, _p.b); \
  if(U128_IS_ZERO(_a.b)){ \
    U128_DECREMENT_SELF(_a.b); \
    _z=1; \
  }

#define FRU128_EXPAND_UP_SELF(_a, _z) \
  U128_INCREMENT_SELF(_a.b); \
  if(U128_IS_ZERO(_a.b)){ \
    U128_DECREMENT_SELF(_a.b); \
    _z=1; \
  }

#define FRU128_FROM_FRU64_HI(_a, _v) \
  U128_FROM_U64_HI(_a.a, _v.a); \
  U128_FROM_U64_HI(_a.b, _v.b)

#define FRU128_FROM_FRU64_LO(_a, _v) \
  U128_FROM_U64_LO(_a.a, _v.a); \
  U128_FROM_U64_LO(_a.b, _v.b)

#define FRU128_FROM_FRU64_MULTIPLY_U64(_a, _p, _v) \
  U128_FROM_U64_PRODUCT(_a.a, _p.a, _v); \
  U128_FROM_U64_PRODUCT(_a.b, _p.b, _v); \
  U128_ADD_U64_LO_SELF(_a.b, _v); \
  U128_DECREMENT_SATURATE_SELF(_a.b)

#define FRU128_FROM_FRU64_SHIFTED(_a, _b, _v) \
  U128_FROM_U64_SHIFTED(_a.a, _b, _v.a); \
  U128_FROM_U64_LO(_a.b, _v.b); \
  U128_INCREMENT_SELF(_a.b); \
  U128_SHIFT_LEFT_SELF(_a.b, _b); \
  U128_DECREMENT_SELF(_a.b)

#define FRU128_FROM_FTD128(_a, _p) \
  _a.a=(_p); \
  _a.b=(_p)

#define FRU128_FROM_FTD64_HI(_a, _v) \
  U128_FROM_U64_HI(_a.a, _v); \
  U128_FROM_U64_HI_SATURATE(_a.b, _v)

#define FRU128_FROM_FTD64_LO(_a, _v) \
  U128_FROM_U64_LO(_a.a, _v); \
  U128_FROM_U64_LO(_a.b, _v)

#define FRU128_FROM_FTD128_MANTISSA_U128_PRODUCT(_a, _p, _q) \
  fracterval_u128_from_fractoid_u128_mantissa_u128_product(&_a, _p, _q)

#define FRU128_FROM_FTD128_MANTISSA_U64_PRODUCT(_a, _p, _v) \
  fracterval_u128_from_fractoid_u128_mantissa_u64_product(&_a, _p, _v)

#define FRU128_FROM_FTD128_U64_NONZERO_PRODUCT(_a, _p, _v, _z) \
  _z=(u8)(_z|fracterval_u128_from_fractoid_u128_u64_product(&_a, _p, _v))

#define FRU128_FROM_FTD128_U64_PRODUCT(_a, _p, _v, _z) \
  _z=(u8)(_z|fracterval_u128_from_fractoid_u128_u64_product(&_a, _p, _v));

#define FRU128_FROM_U128_PAIR(_a, _p, _q) \
  _a.a=(_p); \
  _a.b=(_q)

#define FRU128_LOG_DELTA_U64(_a, _v, _z) \
  _z=(u8)(_z|fracterval_u128_log_delta_u64(&_a, _v))

#define FRU128_LOG_DELTA_U64_CACHED(_a, _l, _m0, _m1, _v, _z) \
  _z=(u8)(_z|fracterval_u128_log_delta_u64_cached(&_a, _l, _m0, _m1, _v))

#define FRU128_LOG_DELTA_U64_NONZERO_CACHED(_a, _l, _m0, _m1, _v) \
  do{ \
    ULONG _i; \
    \
    _i=(_l)&(ULONG)(_v); \
    if((_m1)[_i]==(_v)){ \
      _a=(_m0)[_i]; \
    }else{ \
      fracterval_u128_log_delta_u64_cached(&_a, _l, _m0, _m1, _v); \
    } \
  }while(0)

#define FRU128_LOG_MANTISSA_DELTA_U64(_a, _p, _q, _z) \
  _z=(u8)(_z|fracterval_u128_log_mantissa_delta_u64(&_a, _p, _q))

#define FRU128_LOG_MANTISSA_U128(_a, _p, _z) \
  _z=(u8)(_z|fracterval_u128_log_mantissa_u128(&_a, _p))

#define FRU128_LOG_U128(_a, _p, _z) \
  _z=(u8)(_z|fracterval_u128_log_u128(&_a, _p))

#define FRU128_LOG_U128_CACHED(_a, _l, _m0, _m1, _p, _z) \
  _z=(u8)(_z|fracterval_u128_log_u128_cached(&_a, _l, _m0, _m1, _p))

#define FRU128_LOG_U128_NONZERO_CACHED(_a, _l, _m0, _m1, _p) \
  do{ \
    ULONG _i; \
    u64 _j; \
    \
    U128_TO_U64_LO(_j, _p); \
    _i=(_l)&(ULONG)(_j); \
    if(U128_IS_EQUAL((_m1)[_i], _p)){ \
      _a=(_m0)[_i]; \
    }else{ \
      fracterval_u128_log_u128_cached(&_a, _l, _m0, _m1, _p); \
    } \
  }while(0)

#define FRU128_LOG_U64(_a, _v, _z) \
  _z=(u8)(_z|fracterval_u128_log_u64(&_a, _v))

#define FRU128_LOG_U64_CACHED(_a, _l, _m0, _m1, _v, _z) \
  _z=(u8)(_z|fracterval_u128_log_u64_cached(&_a, _l, _m0, _m1, _v))

#define FRU128_LOG_U64_NONZERO_CACHED(_a, _l, _m0, _m1, _v) \
  do{ \
    ULONG _i; \
    \
    _i=(_l)&(ULONG)(_v); \
    if((_m1)[_i]==(_v)){ \
      _a=(_m0)[_i]; \
    }else{ \
      fracterval_u128_log_u64_cached(&_a, _l, _m0, _m1, _v); \
    } \
  }while(0)
/*
See comments for FRU64_MEAN_TO_FTD64(), which operates in exactly the same manner, but for u64 fractervals.
*/
#define FRU128_MEAN_TO_FTD128(_a, _p) \
  U128_MEAN_FLOOR(_a, _p.a, _p.b)

#define FRU128_MULTIPLY_FRU128(_a, _p, _q) \
  fracterval_u128_multiply_fracterval_u128(&_a, _p, _q)

#define FRU128_MULTIPLY_FRU128_SELF(_a, _p) \
  fracterval_u128_multiply_fracterval_u128(&_a, _a, _p)

#define FRU128_MULTIPLY_FTD128(_a, _p, _q) \
  fracterval_u128_multiply_fractoid_u128(&_a, _p, _q)

#define FRU128_MULTIPLY_FTD128_SELF(_a, _p) \
  fracterval_u128_multiply_fractoid_u128(&_a, _a, _p)

#define FRU128_MULTIPLY_MANTISSA_U128(_a, _p, _q) \
  fracterval_u128_multiply_mantissa_u128(&_a, _p, _q)

#define FRU128_MULTIPLY_MANTISSA_U128_SELF(_a, _p) \
  fracterval_u128_multiply_mantissa_u128(&_a, _a, _p)

#define FRU128_MULTIPLY_MANTISSA_U64(_a, _p, _v) \
  fracterval_u128_multiply_mantissa_u64(&_a, _p, _v)

#define FRU128_MULTIPLY_MANTISSA_U64_SELF(_a, _v) \
  fracterval_u128_multiply_mantissa_u64(&_a, _a, _v)

#define FRU128_MULTIPLY_U64(_a, _p, _v, _z) \
  _z=(u8)(_z|fracterval_u128_multiply_u64(&_a, _p, _v))

#define FRU128_MULTIPLY_U64_SELF(_a, _v, _z) \
  _z=(u8)(_z|fracterval_u128_multiply_u64(&_a, _a, _v))

#define FRU128_NATS_FROM_BITS(_a, _p) \
  fracterval_u128_nats_from_bits(&_a, _p)

#define FRU128_NATS_TO_BITS(_a, _p, _z) \
  _z=(u8)(_z|fracterval_u128_nats_to_bits(&_a, _p))

#define FRU128_NOT_FRU128(_a, _p) \
  U128_NOT(_a.b, _p.a); \
  U128_NOT(_a.a, _p.b)

#define FRU128_NOT_SELF(_a) \
  do{ \
    u128 _p; \
    \
    U128_NOT(_p, _a.a); \
    U128_NOT(_a.a, _a.b); \
    _a.b=_p; \
  }while(0)

#define FRU128_RATIO_U128_SATURATE(_a, _p, _q, _z) \
  _z=(u8)(_z|fractoid_u128_ratio_u128_saturate(&_a.a, _p, _q)); \
  _a.b=_a.a

#define FRU128_RATIO_U128_SATURATE_SELF(_a, _p, _z) \
  _z=(u8)(_z|fractoid_u128_ratio_u128_saturate(&_a.a, _a.a, _p)); \
  _a.b=_a.a

#define FRU128_RATIO_U64_SATURATE(_a, _v, _w, _z) \
  _z=(u8)(_z|fractoid_u128_ratio_u64_saturate(&_a.a, _v, _w)); \
  _a.b=_a.a

#define FRU128_RECIPROCAL_U128_SATURATE(_a, _p, _z) \
  _z=(u8)(_z|fractoid_u128_reciprocal_u128_saturate(&_a.a, _p)); \
  _a.b=_a.a

#define FRU128_RECIPROCAL_U64_SATURATE(_a, _v, _z) \
  _z=(u8)(_z|fractoid_u128_reciprocal_u64_saturate(&_a.a, _v)); \
  _a.b=_a.a

#define FRU128_ROOT_FRACTOID_U128(_a, _p) \
  fracterval_u128_root_fractoid_u128(&_a, _p);

#define FRU128_SET_AMBIGUOUS(_a) \
  U128_SET_ZERO(_a.a); \
  U128_NOT(_a.b, _a.a)

#define FRU128_SET_HALF_BIASED_HI(_a) \
  U128_SET_SPAN_HALF(_a.a); \
  _a.b=_a.a

#define FRU128_SET_HALF_BIASED_LO(_a) \
  U128_SET_SPAN_HALF_MINUS_1(_a.a); \
  _a.b=_a.a

#define FRU128_SET_HALF_UNBIASED(_a) \
  U128_SET_SPAN_HALF(_a.b); \
  U128_NOT(_a.a, _a.b)

#define FRU128_SET_ONES(_a) \
  U128_SET_ONES(_a.a); \
  _a.b=_a.a

#define FRU128_SET_ZERO(_a) \
  U128_SET_ZERO(_a.a); \
  _a.b=_a.a

#define FRU128_SHIFT_LEFT(_a, _b, _p, _z) \
  _z=(u8)(_z|fracterval_u128_shift_left(&_a, _b, _p))

#define FRU128_SHIFT_LEFT_SELF(_a, _b, _z) \
  _z=(u8)(_z|fracterval_u128_shift_left(&_a, _b, _a))

#define FRU128_SHIFT_RIGHT(_a, _b, _p) \
  U128_SHIFT_RIGHT(_a.a, _b, _p.a); \
  U128_SHIFT_RIGHT(_a.b, _b, _p.b)

#define FRU128_SHIFT_RIGHT_SELF(_a, _b) \
  U128_SHIFT_RIGHT(_a.a, _b, _a.a); \
  U128_SHIFT_RIGHT(_a.b, _b, _a.b)

#define FRU128_SUBTRACT_FRU128(_a, _p, _q, _z) \
  U128_SUBTRACT_U128(_a.a, _p.a, _q.b); \
  U128_DECREMENT_SELF(_a.a); \
  U128_SUBTRACT_U128(_a.b, _p.b, _q.a); \
  if(U128_IS_LESS_EQUAL(_p.a, _a.a)){ \
    U128_SET_ZERO(_a.a); \
    _z=1; \
    if(U128_IS_LESS(_p.b, _a.b)){ \
      _a.b=_a.a; \
    } \
  }

#define FRU128_SUBTRACT_FRU128_SELF(_a, _p, _z) \
  do{ \
    fru128 _q; \
    \
    _q=_a; \
    FRU128_SUBTRACT_FRU128(_a, _q, _p, _z); \
  }while(0)

#define FRU128_SUBTRACT_FRU64_HI(_a, _p, _v, _z) \
  U128_SUBTRACT_U64_HI(_a.a, _p.a, _v.b); \
  U128_DECREMENT_U64_HI_SELF(_a.a); \
  U128_SUBTRACT_U64_HI(_a.b, _p.b, _v.a); \
  if(U128_IS_LESS_EQUAL(_p.a, _a.a)){ \
    U128_SET_ZERO(_a.a); \
    _z=1; \
    if(U128_IS_LESS(_p.b, _a.b)){ \
      _a.b=_a.a; \
    } \
  }

#define FRU128_SUBTRACT_FRU64_HI_SELF(_a, _v, _z) \
  do{ \
    fru128 _q; \
    \
    _q=_a; \
    FRU128_SUBTRACT_FRU64_HI(_a, _q, _v, _z); \
  }while(0)

#define FRU128_SUBTRACT_FRU64_LO(_a, _p, _v, _z) \
  U128_SUBTRACT_U64_LO(_a.a, _p.a, _v.b); \
  U128_DECREMENT_SELF(_a.a); \
  U128_SUBTRACT_U64_LO(_a.b, _p.b, _v.a); \
  if(U128_IS_LESS_EQUAL(_p.a, _a.a)){ \
    U128_SET_ZERO(_a.a); \
    _z=1; \
    if(U128_IS_LESS(_p.b, _a.b)){ \
      _a.b=_a.a; \
    } \
  }

#define FRU128_SUBTRACT_FRU64_LO_SELF(_a, _v, _z) \
  do{ \
    fru128 _q; \
    \
    _q=_a; \
    FRU128_SUBTRACT_FRU64_LO(_a, _q, _v, _z); \
  }while(0)

#define FRU128_SUBTRACT_FRU64_SHIFTED(_a, _b, _p, _v, _z) \
  do{ \
    u128 _r; \
    \
    U128_FROM_U64_LO(_r, _v.b); \
    U128_INCREMENT_SELF(_r); \
    U128_SHIFT_LEFT_SELF(_r, _b); \
    U128_SUBTRACT_U128(_a.a, _p.a, _r); \
    U128_SUBTRACT_U64_SHIFTED(_a.b, _b, _p.b, _v.a); \
    if(U128_IS_LESS_EQUAL(_p.a, _a.a)){ \
      U128_SET_ZERO(_a.a); \
      _z=1; \
      if(U128_IS_LESS(_p.b, _a.b)){ \
        _a.b=_a.a; \
      } \
    } \
  }while(0)

#define FRU128_SUBTRACT_FRU64_SHIFTED_SELF(_a, _b, _v, _z) \
  do{ \
    fru128 _q; \
    \
    _q=_a; \
    FRU128_SUBTRACT_FRU64_SHIFTED(_a, _b, _q, _v, _z); \
  }while(0)

#define FRU128_SUBTRACT_FTD128(_a, _p, _q, _z) \
  U128_SUBTRACT_U128(_a.a, _p.a, _q); \
  U128_DECREMENT_SELF(_a.a); \
  U128_SUBTRACT_U128(_a.b, _p.b, _q); \
  if(U128_IS_LESS_EQUAL(_p.a, _a.a)){ \
    U128_SET_ZERO(_a.a); \
    _z=1; \
    if(U128_IS_LESS(_p.b, _a.b)){ \
      _a.b=_a.a; \
    } \
  }

#define FRU128_SUBTRACT_FTD128_SELF(_a, _p, _z) \
  do{ \
    fru128 _q; \
    \
    _q=_a; \
    FRU128_SUBTRACT_FTD128(_a, _q, _p, _z); \
  }while(0)

#define FRU128_SUBTRACT_FROM_FRU128_SELF(_a, _p, _z) \
  do{ \
    fru128 _q; \
    \
    _q=_a; \
    FRU128_SUBTRACT_FRU128(_a, _p, _q, _z); \
  }while(0)

#define FRU128_TO_DIAMETER_MINUS_1_U128(_a, _p) \
  U128_SUBTRACT_U128(_a, _p.b, _p.a)

#define FRU128_TO_FRU64(_t, _p) \
  U128_TO_U64_HI(_t.a, _p.a); \
  U128_TO_U64_HI(_t.b, _p.b)

#define FRU128_TO_U128_PAIR(_a, _d, _p) \
  _a=_p.a; \
  _d=_p.b

#define FRU128_UNION(_a, _p, _q) \
  if(U128_IS_LESS_EQUAL(_q.a, _p.a)){ \
    _a.a=_q.a; \
  }else{ \
    _a.a=_p.a; \
  } \
  if(U128_IS_LESS_EQUAL(_p.b, _q.b)){ \
    _a.b=_q.b; \
  }else{ \
    _a.b=_p.b; \
  }

#define FRU128_UNION_SELF(_a, _p) \
  if(U128_IS_LESS_EQUAL(_p.a, _a.a)){ \
    _a.a=_p.a; \
  } \
  if(U128_IS_LESS_EQUAL(_a.b, _p.b)){ \
    _a.b=_p.b; \
  }

#define FTD128_FROM_MANTISSA_U128_PRODUCT(_a, _p, _q) \
  _a=fractoid_u128_from_mantissa_u128_product(_p, _q)

#define FTD128_RATIO_U128_SATURATE(_a, _p, _q, _z) \
  _z=(u8)(_z|fractoid_u128_ratio_u128_saturate(&_a, _p, _q))

#define FTD128_RATIO_U128_SATURATE_SELF(_a, _p, _z) \
  _z=(u8)(_z|fractoid_u128_ratio_u128_saturate(&_a, _a, _p))

#define FTD128_RATIO_U64_SATURATE(_a, _v, _w, _z) \
  _z=(u8)(_z|fractoid_u128_ratio_u64_saturate(&_a, _v, _w))

#define FTD128_RECIPROCAL_U128_SATURATE(_a, _p, _z) \
  _z=(u8)(_z|fractoid_u128_reciprocal_u128_saturate(&_a, _p))

#define FTD128_RECIPROCAL_U128_SATURATE_SELF(_a, _z) \
  _z=(u8)(_z|fractoid_u128_reciprocal_u128_saturate(&_a, _a))

#define FTD128_RECIPROCAL_U64_SATURATE(_a, _v, _z) \
  _z=(u8)(_z|fractoid_u128_reciprocal_u64_saturate(&_a, _v))
