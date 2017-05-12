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

* By default, the "FRU128" macros perform overflow checking if overflow is possible; those that end in "UNSAFE" don't, and assume that the programmer is careful enough to verify that overflow either can't occur or would be innocuous. The advantage to "UNSAFE" macros is that they're optimized for speed, typically involving inline code.

VARIABLE NAMING CONVENTIONS

The code in the entire Fracterval U128 codebase mostly follows the guideline below, but as a practical matter, some exceptions are necessary.

* _a is the primary 128(x2)-bit output, which is a u128 fracterval, u128 fractoid, u128 mantissa, or u128 whole number. In "_SELF" macros, it's also an input.

* _b is a bit count, which should be a u8 for optimal performance but need not be.

* _c is a u8, such as a borrow or a carry.

* _d is analogous but secondary to _a.

* _i is an index private to a macro.

* _j is an index private to a macro.

* _k ("kache") is base of a list.

* _l is the maximum ("limit") index of a list. Specifically, _l0 denotes the current limit, whereas _l1 denotes the upper bound.

* _m is the base of a data structure other than a list, which is always _k.

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

#define FRU128_LOG2_FLOOR_HI 0xB17217F7D1CF79ABULL
#define FRU128_LOG2_FLOOR_LO 0xC9E3B39803F2F6AFULL

#ifdef _64_
  #define U128_ADD_U128(_a, _p, _q) \
    _a=(_p)+(_q)

  #define U128_ADD_U128_CHECK(_a, _p, _q, _z) \
    _a=(_p)+(_q); \
    _z=(u8)(_z|(_a<(_p)))

  #define U128_ADD_U128_SELF(_a, _p) \
    _a+=(_p)

  #define U128_ADD_U128_SELF_CHECK(_a, _p, _z) \
    _a+=(_p); \
    _z=(u8)(_z|(_a<(_p)))

  #define U128_ADD_U64_HI(_a, _p, _v) \
    _a=(_p)+((u128)(_v)<<64)

  #define U128_ADD_U64_HI_CHECK(_a, _p, _v, _z) \
    _a=(_p)+((u128)(_v)<<64); \
    _z=(u8)(_z|(_a<(_p)))

  #define U128_ADD_U64_HI_SELF(_a, _v) \
    _a+=((u128)(_v)<<64)

  #define U128_ADD_U64_HI_SELF_CHECK(_a, _v, _z) \
    do{ \
      u128 __p; \
      \
      __p=_a; \
      U128_ADD_U64_HI_CHECK(_a, __p, _v, _z); \
    }while(0)

  #define U128_ADD_U64_LO(_a, _p, _v) \
    _a=(_p)+(_v)

  #define U128_ADD_U64_LO_CHECK(_a, _p, _v, _z) \
    _a=(_p)+(_v); \
    _z=(u8)(_z|(_a<(_v)))

  #define U128_ADD_U64_LO_SELF(_a, _v) \
    _a+=(_v)

  #define U128_ADD_U64_LO_SELF_CHECK(_a, _v, _z) \
    _a+=(_v); \
    _z=(u8)(_z|(_a<(_v)))

  #define U128_ADD_U64_SHIFTED(_a, _b, _p, _v) \
    _a=(_p)+((u128)(_v)<<(_b))

  #define U128_ADD_U64_SHIFTED_SELF(_a, _b, _v) \
    _a+=((u128)(_v)<<(_b))

  #define U128_ADD_U8(_a, _c, _p); \
    _a=(_c)+(_p)

  #define U128_ADD_U8_HI(_a, _c, _p); \
    _a=((u128)(_c)<<64)+(_p)

  #define U128_ADD_U8_HI_SELF(_a, _c); \
    _a+=((u128)(_c)<<64)

  #define U128_ADD_U8_SELF(_a, _c); \
    _a+=(_c)

  #define U128_BIT_CLEAR(_a, _b, _p); \
    _a=(u128)(_p)&(~((u128)(1)<<(_b)))

  #define U128_BIT_CLEAR_SELF(_a, _b); \
    U128_BIT_CLEAR(_a, _b, _a)

  #define U128_BIT_FLIP(_a, _b, _p); \
    _a=(u128)(_p)^((u128)(1)<<(_b))

  #define U128_BIT_FLIP_SELF(_a, _b); \
    U128_BIT_FLIP(_a, _b, _a)

  #define U128_BIT_GET(_c, _b, _p); \
    _c=(u8)(((_p)>>(_b))&1)

  #define U128_BIT_SET(_a, _b, _p); \
    _a=(u128)(_p)|((u128)(1)<<(_b))

  #define U128_BIT_SET_SELF(_a, _b); \
    U128_BIT_SET(_a, _b, _a)

  #define U128_CHECKSUM_TO_U64(_t, _p, _v) \
    _t=(u64)((_p)>>64)+(u64)(_p)+(_v)

  #define U128_DECREMENT(_a, _p) \
    _a=(_p)-1

  #define U128_DECREMENT_SATURATE(_a, _p) \
    _a=(_p)-!!(_p)

  #define U128_DECREMENT_SATURATE_SELF(_a) \
    _a=(_a)-!!(_a)

  #define U128_DECREMENT_SELF(_a) \
    _a--

  #define U128_DECREMENT_U64_HI(_a, _p) \
    _a=(_p.b)-U64_MAX; \
    _a--

  #define U128_DECREMENT_U64_HI_SELF(_a) \
    _a-=U64_MAX; \
    _a--

  #define U128_DIVIDE_U64_TO_U128_SATURATE(_a, _p, _v, _z) \
    _z=(u8)(_z|u128_divide_u64_to_u128_saturate(&_a, _p, _v))

  #define U128_DIVIDE_U64_TO_U128_SATURATE_SELF(_a, _v, _z) \
    _z=(u8)(_z|u128_divide_u64_to_u128_saturate(&_a, _a, _v))

  #define U128_DIVIDE_U64_TO_U64_SATURATE(_t, _p, _v, _z) \
    _z=(u8)(_z|u128_divide_u64_to_u64_saturate(&_t, _p, _v))

  #define U128_FROM_BOOL(_a, _c) \
    _a=0; \
    _a-=!!_c

  #define U128_FROM_U128_PAIR_BIT_IDX(_a, _b, _p, _q) \
    _a=(_p)>>(_b); \
    if(_b){ \
      _a+=(_q)<<(128-(_b)); \
    }

  #define U128_FROM_U64_HI(_a, _w) \
    _a=(u128)(_w)<<64

  #define U128_FROM_U64_HI_SATURATE(_a, _w) \
    _a=((u128)(_w)<<64)|U64_MAX;

  #define U128_FROM_U64_LO(_a, _v) \
    _a=(_v)

  #define U128_FROM_U64_PAIR(_a, _v, _w) \
    _a=((u128)(_w)<<64)|(_v)

  #define U128_FROM_U64_PRODUCT(_a, _v, _w) \
    _a=(u128)(_v)*(_w)

  #define U128_FROM_U64_SHIFTED(_a, _b, _v) \
    _a=(u128)(_v)<<(_b)

  #define U128_INCREMENT(_a, _p) \
    _a=(_p)+1

  #define U128_INCREMENT_SATURATE(_a, _p) \
    _a=(_p)+!!(~(_p))

  #define U128_INCREMENT_SATURATE_SELF(_a) \
    _a=(_a)+!!(~(_a))

  #define U128_INCREMENT_SELF(_a) \
    _a++

  #define U128_INCREMENT_U64_HI(_a, _p) \
    _a=(_p.b)+U64_MAX; \
    _a++

  #define U128_INCREMENT_U64_HI_SELF(_a) \
    _a+=U64_MAX; \
    _a++

  #define U128_IS_EQUAL(_p, _q) \
    ((_p)==(_q))

  #define U128_IS_LESS(_p, _q) \
    ((_p)<(_q))

  #define U128_IS_LESS_EQUAL(_p, _q) \
    ((_p)<=(_q))

  #define U128_IS_LESS_EQUAL_U64(_p, _q) \
    ((_p)<=(_q))

  #define U128_IS_LESS_U64(_p, _q) \
    ((_p)<(_q))

  #define U128_IS_NOT_EQUAL(_p, _q) \
    ((_p)!=(_q))

  #define U128_IS_NOT_EQUAL_U64(_p, _q) \
    ((_p)!=(_q))

  #define U128_IS_NOT_ONES(_p) \
    (!!(~(_p)))

  #define U128_IS_NOT_POWER_OF_2(_p) \
    (!!(p&(p-1)))

  #define U128_IS_NOT_SIGNED(_p) \
    (!((_p)>>127))

  #define U128_IS_NOT_ZERO(_p) \
    (!!(_p))

  #define U128_IS_ONES(_p) \
    (!(~(_p)))

  #define U128_IS_POWER_OF_2(_p) \
    (!(p&(p-1)))

  #define U128_IS_SIGNED(_p) \
    (!!((_p)>>127))

  #define U128_IS_ZERO(_p) \
    (!(_p))

  #define U128_MAX(_a, _p, _q) \
    _a=((_p)<=(_q))?(_q):(_p)

  #define U128_MAX_SELF(_a, _p) \
    _a=((_a)<=(_p))?(_p):(_a)

  #define U128_MEAN_FLOOR(_a, _p, _q) \
    _a=((_p)>>1)+((_q)>>1)+(1&(_p)&(_q))

  #define U128_MIN(_a, _p, _q) \
    _a=((_p)<=(_q))?(_p):(_q)

  #define U128_MIN_SELF(_a, _p) \
    _a=((_a)<=(_p))?(_a):(_p)

  #define U128_MSB_GET(_b, _p) \
    _b=u128_msb_get(_p)

  #define U128_MULTIPLY_U64_SATURATE(_a, _p, _v, _z) \
    _z=(u8)(_z|u128_multiply_u64_saturate(&_a, _p, _v))

  #define U128_MULTIPLY_U64_SATURATE_SELF(_a, _v, _z) \
    _z=(u8)(_z|u128_multiply_u64_saturate(&_a, _a, _v))

  #define U128_NEGATE(_a, _p) \
    _a=0U-(_p);

  #define U128_NEGATE_SELF(_a) \
    _a=0U-_a;

  #define U128_NOT(_a, _p) \
    _a=~(_p);

  #define U128_NOT_SELF(_a) \
    _a=~_a;

  #define U128_SET_ONES(_a) \
    _a=0; \
    _a=~_a

  #define U128_SET_SPAN_HALF(_a) \
    _a=(u128)(1)<<127

  #define U128_SET_SPAN_HALF_MINUS_1(_a) \
    _a=~((u128)(1)<<127)

  #define U128_SET_ULP(_a) \
    _a=1

  #define U128_SET_ZERO(_a) \
    _a=0

  #define U128_SHIFT_LEFT(_a, _b, _p) \
    _a=(_p)<<(_b)

  #define U128_SHIFT_LEFT_CHECK(_a, _b, _p, _z) \
    _a=(_p)<<(_b); \
    _z=(u8)(_z|((_a>>(_b))!=(_p)))

  #define U128_SHIFT_LEFT_SELF(_a, _b) \
    _a<<=(_b)

  #define U128_SHIFT_LEFT_SELF_CHECK(_a, _b, _z) \
    do{ \
      u128 __p; \
      \
      __p=_a; \
      U128_SHIFT_LEFT_CHECK(_a, _b, __p, _z); \
    }while(0)

  #define U128_SHIFT_RIGHT(_a, _b, _p) \
    _a=(_p)>>(_b)

  #define U128_SHIFT_RIGHT_SELF(_a, _b) \
    _a>>=(_b)

  #define U128_SHIFTED_TO_U64(_t, _b, _p) \
    _t=(u64)((_p)>>(_b))

  #define U128_SUBTRACT_FROM_U128_SELF(_a, _p) \
    _a=(_p)-_a;

  #define U128_SUBTRACT_U128(_a, _p, _q) \
    _a=(_p)-(_q)

  #define U128_SUBTRACT_U128_SELF(_a, _p) \
    _a-=(_p)

  #define U128_SUBTRACT_U64_HI(_a, _p, _v) \
    _a=(_p)-((u128)(_v)<<64)

  #define U128_SUBTRACT_U64_HI_CHECK(_a, _p, _v, _z) \
    _a=((u128)(_v)<<64); \
    _z=(u8)(_z|((_p)<_a)); \
    _a=(_p)-_a

  #define U128_SUBTRACT_U64_HI_SELF(_a, _v) \
    _a-=((u128)(_v)<<64)

  #define U128_SUBTRACT_U64_HI_SELF_CHECK(_a, _v, _z) \
    do{ \
      u128 __p; \
      \
      __p=_a; \
      U128_SUBTRACT_U64_HI_CHECK(_a, __p, _v, _z); \
    }while(0)

  #define U128_SUBTRACT_U64_LO_CHECK(_a, _p, _v, _z) \
    _z=(u8)(_z|((_p)<(_v))); \
    _a=(_p)-(_v)

  #define U128_SUBTRACT_U64_LO_SELF(_a, _v) \
    _a-=(_v)

  #define U128_SUBTRACT_U64_LO_SELF_CHECK(_a, _v, _z) \
    do{ \
      u128 __p; \
      \
      __p=_a; \
      U128_SUBTRACT_U64_LO_CHECK(_a, __p, _v, _z); \
    }while(0)

  #define U128_SUBTRACT_U64_SHIFTED(_a, _b, _p, _v) \
    _a=(_p)-((u128)(_v)<<(_b))

  #define U128_SUBTRACT_U64_SHIFTED_SELF(_a, _b, _v) \
    _a-=((u128)(_v)<<(_b))

  #define U128_SWAP(_p, _q) \
    (_p)^=(_q); \
    (_q)^=(_p); \
    (_p)^=(_q)

  #define U128_TO_U64_HI(_u, _p) \
    _u=(u64)((_p)>>64)

  #define U128_TO_U64_LO(_t, _p) \
    _t=(u64)(_p)

  #define U128_TO_U64_PAIR(_t, _u, _p) \
    _t=(u64)(_p); \
    _u=(u64)((_p)>>64)
#else
  #define U128_ADD_U128(_a, _p, _q) \
    _a.a=(_p.a)+(_q.a); \
    _a.b=(_p.b)+(_q.b); \
    _a.b+=(_a.a<(_p.a))

  #define U128_ADD_U128_CHECK(_a, _p, _q, _z) \
    _a.a=(_p.a)+(_q.a); \
    _a.b=(_p.b)+(_q.b); \
    _z=(u8)(_z|(_a.b<(_p.b))); \
    if(_a.a<(_p.a)){ \
      _a.b++; \
      _z=(u8)(_z|!_a.b); \
    }

  #define U128_ADD_U128_SELF(_a, _p) \
    _a.a+=(_p.a); \
    _a.b+=(_p.b)+(_a.a<(_p.a))

  #define U128_ADD_U128_SELF_CHECK(_a, _p, _z) \
    _a.a+=(_p.a); \
    _a.b+=(_p.b); \
    _z=(u8)(_z|(_a.b<(_p.b))); \
    if(_a.a<(_p.a)){ \
      _a.b++; \
      _z=(u8)(_z|!_a.b); \
    }

  #define U128_ADD_U64_HI(_a, _p, _v) \
    _a.a=_p.a; \
    _a.b=(_p.b)+(_v)

  #define U128_ADD_U64_HI_CHECK(_a, _p, _v, _z) \
    _a.a=_p.a; \
    _a.b=(_p.b)+(_v); \
    _z=(u8)(_z|(_a.b<(_v)))

  #define U128_ADD_U64_HI_SELF(_a, _v) \
    _a.b+=(_v)

  #define U128_ADD_U64_HI_SELF_CHECK(_a, _v, _z) \
    _a.b+=(_v); \
    _z=(u8)(_z|(_a.b<(_v)))

  #define U128_ADD_U64_LO(_a, _p, _v) \
    _a.a=(_p.a)+(_v); \
    _a.b=(_p.b)+(_a.a<(_v))

  #define U128_ADD_U64_LO_CHECK(_a, _p, _v, _z) \
    _a.a=(_p.a)+(_v); \
    _a.b=(_p.b); \
    if(_a.a<(_v)){ \
      _a.b++; \
      _z=(u8)(_z|!_a.b); \
    }

  #define U128_ADD_U64_LO_SELF(_a, _v) \
    _a.a+=(_v); \
    _a.b+=(_a.a<(_v))

  #define U128_ADD_U64_LO_SELF_CHECK(_a, _v, _z) \
    _a.a+=(_v); \
    if(_a.a<(_v)){ \
      _a.b++; \
      _z=(u8)(_z|!_a.b); \
    }

  #define U128_ADD_U64_SHIFTED(_a, _b, _p, _v) \
    _a.a=(_p.a)+((_v)<<(_b)); \
    _a.b=(_p.b)+(_a.a<(_p.a)); \
    if(_b){ \
      _a.b+=(_v)>>(64-(_b)); \
    }

  #define U128_ADD_U64_SHIFTED_SELF(_a, _b, _v) \
    do{ \
      u64 __w; \
      \
      __w=(_v)<<(_b); \
      _a.a+=__w; \
      _a.b+=(_a.a<__w); \
      if(_b){ \
        _a.b+=(_v)>>(64-(_b)); \
      } \
    }while(0)

  #define U128_ADD_U8(_a, _c, _p); \
    _a.a=(_c)+(_p.a); \
    _a.b=(_a.a<(_p.a))+(_p.b)

  #define U128_ADD_U8_HI(_a, _c, _p); \
    _a.a=(_p.a); \
    _a.b=(_c)+(_p.b)

  #define U128_ADD_U8_HI_SELF(_a, _c); \
    _a.b+=(_c)

  #define U128_ADD_U8_SELF(_a, _c); \
    _a.a+=(_c); \
    _a.b+=(_a.a<(_c))

  #define U128_BIT_CLEAR(_a, _b, _p); \
    do{ \
      u64 __q; \
      \
      _a.a=_p.a; \
      _a.b=_p.b; \
      __q=~(1ULL<<((_b)&63)); \
      if((_b)<=63){ \
        _a.a&=__q; \
      }else{ \
        _a.b&=__q; \
      } \
    }while(0)

  #define U128_BIT_CLEAR_SELF(_a, _b); \
    do{ \
      u64 __q; \
      \
      __q=~(1ULL<<((_b)&63)); \
      if((_b)<=63){ \
        _a.a&=__q; \
      }else{ \
        _a.b&=__q; \
      } \
    }while(0)

  #define U128_BIT_FLIP(_a, _b, _p); \
    do{ \
      u64 __q; \
      \
      __q=1ULL<<((_b)&63); \
      if((_b)<=63){ \
        _a.a=_p.a^__q; \
        _a.b=_p.b; \
      }else{ \
        _a.a=_p.a; \
        _a.b=_p.b^__q; \
      } \
    }while(0)

  #define U128_BIT_FLIP_SELF(_a, _b); \
    U128_BIT_FLIP(_a, _b, _a)

  #define U128_BIT_GET(_c, _b, _p); \
    if((_b)<=63){ \
      _c=(u8)((_p.a)>>(_b)); \
    }else{ \
      _c=(u8)((_p.b)>>((_b)&63)); \
    } \
    _c=(u8)(_c&1)

  #define U128_BIT_SET(_a, _b, _p); \
    do{ \
      u64 __q; \
      \
      _a.a=_p.a; \
      _a.b=_p.b; \
      __q=~(1ULL<<((_b)&63)); \
      if((_b)<=63){ \
        _a.a|=__q; \
      }else{ \
        _a.b|=__q; \
      } \
    }while(0)

  #define U128_BIT_SET_SELF(_a, _b); \
    do{ \
      u64 __q; \
      \
      __q=~(1ULL<<((_b)&63)); \
      if((_b)<=63){ \
        _a.a|=__q; \
      }else{ \
        _a.b|=__q; \
      } \
    }while(0)

  #define U128_CHECKSUM_TO_U64(_t, _p, _v) \
    _t=(_p.a)+(_p.b)+(_v)

  #define U128_DECREMENT(_a, _p) \
    _a.b=(_p.b)-!(_p.a); \
    _a.a=(_p.a)-1

  #define U128_DECREMENT_SATURATE(_a, _p) \
    _a.a=0; \
    _a.b=0; \
    if((_p.a)|(_p.b)){ \
      _a.b=(_p.b)-!(_p.a); \
      _a.a=(_p.a)-1; \
    }

  #define U128_DECREMENT_SATURATE_SELF(_a) \
    if(_a.a|_a.b){ \
      _a.b-=!_a.a; \
      _a.a--; \
    }

  #define U128_DECREMENT_SELF(_a) \
    _a.b-=!_a.a; \
    _a.a--

  #define U128_DECREMENT_U64_HI(_a, _p) \
    _a.a=(_p.a); \
    _a.b=(_p.b)-1

  #define U128_DECREMENT_U64_HI_SELF(_a) \
    _a.b--

  #define U128_DIVIDE_U64_TO_U128_SATURATE(_a, _p, _v, _z) \
    _z=(u8)(_z|u128_divide_u64_to_u128_saturate(&_a, _p, _v))

  #define U128_DIVIDE_U64_TO_U128_SATURATE_SELF(_a, _v, _z) \
    _z=(u8)(_z|u128_divide_u64_to_u128_saturate(&_a, _a, _v))

  #define U128_DIVIDE_U64_TO_U64_SATURATE(_t, _p, _v, _z) \
    _z=(u8)(_z|u128_divide_u64_to_u64_saturate(&_t, _p, _v))

  #define U128_FROM_BOOL(_a, _c) \
    _a.a=!_c; \
    _a.a--; \
    _a.b=_a.a

  #define U128_FROM_U128_PAIR_BIT_IDX(_a, _b, _p, _q) \
    _a=u128_from_u128_pair_bit_idx(_b, _p, _q)

  #define U128_FROM_U64_HI(_a, _w) \
    _a.a=0; \
    _a.b=(_w)

  #define U128_FROM_U64_HI_SATURATE(_a, _w) \
    _a.a=0; \
    _a.a=~_a.a; \
    _a.b=(_w)

  #define U128_FROM_U64_LO(_a, _v) \
    _a.a=(_v); \
    _a.b=0

  #define U128_FROM_U64_PAIR(_a, _v, _w) \
    _a.a=(_v); \
    _a.b=(_w)

  #define U128_FROM_U64_PRODUCT(_a, _v, _w) \
    _a=u128_from_u64_product(_v, _w)

  #define U128_FROM_U64_SHIFTED(_a, _b, _v) \
    _a.a=(_v)<<(_b); \
    _a.b=0; \
    if(_b){ \
      _a.b=(_v)>>(64-(_b)); \
    }

  #define U128_INCREMENT(_a, _p) \
    _a.a=(_p.a)+1; \
    _a.b=(_p.b)+!_a.a

  #define U128_INCREMENT_SATURATE(_a, _p) \
    _a.a=(_p.a)+1; \
    _a.b=(_p.b); \
    if(!_a.a){ \
      _a.b++; \
      if(!_a.b){ \
        _a.a=~_a.a; \
        _a.b=~_a.b; \
      } \
    }

  #define U128_INCREMENT_SATURATE_SELF(_a) \
    if(~(_a.a&_a.b)){ \
      _a.a++; \
      _a.b+=!_a.a; \
    }

  #define U128_INCREMENT_SELF(_a) \
    _a.a++; \
    _a.b+=!_a.a

  #define U128_INCREMENT_U64_HI(_a, _p) \
    _a.a=(_p.a); \
    _a.b=(_p.b)+1

  #define U128_INCREMENT_U64_HI_SELF(_a) \
    _a.b++

  #define U128_IS_EQUAL(_p, _q) \
    (((_p.a)==(_q.a))&&((_p.b)==(_q.b)))

  #define U128_IS_EQUAL_U64(_p, _q) \
    ((_p.a)==(_q))

  #define U128_IS_LESS(_p, _q) \
    (((_p.b)<(_q.b))||(((_p.b)==(_q.b))&&((_p.a)<(_q.a))))

  #define U128_IS_LESS_EQUAL(_p, _q) \
    (((_p.b)<(_q.b))||(((_p.b)==(_q.b))&&((_p.a)<=(_q.a))))

  #define U128_IS_LESS_EQUAL_U64(_p, _q) \
    ((!(_p.b))&&((_p.a)<=(_q)))

  #define U128_IS_LESS_U64(_p, _q) \
    ((!(_p.b))&&((_p.a)<(_q)))

  #define U128_IS_NOT_EQUAL(_p, _q) \
    (((_p.a)!=(_q.a))||((_p.b)!=(_q.b)))

  #define U128_IS_NOT_EQUAL_U64(_p, _q) \
    ((_p.b)||((_p.a)!=(_q)))

  #define U128_IS_NOT_ONES(_p) \
    ((~(_p.a))||(~(_p.b)))

  #define U128_IS_NOT_POWER_OF_2(_p) \
    ((p.a&(p.a-1))||(p.b&(p.b-1)))

  #define U128_IS_NOT_SIGNED(_p) \
    (!((_p.b)>>63))

  #define U128_IS_NOT_ZERO(_p) \
    ((_p.a)||(_p.b))

  #define U128_IS_ONES(_p) \
    (!(~((_p.a)&(_p.b))))

  #define U128_IS_POWER_OF_2(_p) \
    (!((p.a&(p.a-1))||(p.b&(p.b-1))))

  #define U128_IS_SIGNED(_p) \
    (!!((_p.b)>>63))

  #define U128_IS_ZERO(_p) \
    (!((_p.a)|(_p.b)))

  #define U128_MAX(_a, _p, _q) \
    _a.a=(_p.a); \
    _a.b=(_p.b); \
    if(((_p.b)<(_q.b))||(((_p.b)==(_q.b))&&((_p.a)<(_q.a)))){ \
      _a.a=(_q.a); \
      _a.b=(_q.b); \
    }

  #define U128_MAX_SELF(_a, _p) \
    if(((_a.b)<(_p.b))||(((_a.b)==(_p.b))&&((_a.a)<(_p.a)))){ \
      _a.a=(_p.a); \
      _a.b=(_p.b); \
    }

  #define U128_MEAN_FLOOR(_a, _p, _q) \
    do{ \
      u64 __r; \
      \
      _a.a=((_p.a)>>1)+((_q.a)>>1); \
      _a.b=((_p.b)>>1)+((_q.b)>>1); \
      __r=(_p.b)<<63; \
      _a.a+=__r; \
      _a.b+=(_a.a<__r); \
      __r=(_q.b)<<63; \
      _a.a+=__r; \
      _a.b+=(_a.a<__r); \
      __r=1&(_p.a)&(_q.a); \
      _a.a+=__r; \
      _a.b+=(_a.a<__r); \
    }while(0)

  #define U128_MIN(_a, _p, _q) \
    _a.a=(_p.a); \
    _a.b=(_p.b); \
    if(((_q.b)<(_p.b))||(((_p.b)==(_q.b))&&((_q.a)<(_p.a)))){ \
      _a.a=(_q.a); \
      _a.b=(_q.b); \
    }

  #define U128_MIN_SELF(_a, _p) \
    if(((_a.b)<(_p.b))||(((_a.b)==(_p.b))&&((_a.a)<(_p.a)))){ \
      _a.a=(_p.a); \
      _a.b=(_p.b); \
    }

  #define U128_MSB_GET(_b, _p) \
    _b=u128_msb_get(_p)

  #define U128_MULTIPLY_U64_SATURATE(_a, _p, _v, _z) \
    _z=(u8)(_z|u128_multiply_u64_saturate(&_a, _p, _v))

  #define U128_MULTIPLY_U64_SATURATE_SELF(_a, _v, _z) \
    _z=(u8)(_z|u128_multiply_u64_saturate(&_a, _a, _v))

  #define U128_NEGATE(_a, _p) \
    _a.a=0U-(_p.a); \
    _a.b=0U-(_p.b)-!!_a.a

  #define U128_NEGATE_SELF(_a) \
    _a.a=0U-_a.a; \
    _a.b=0U-_a.b-!!_a.a

  #define U128_NOT(_a, _p) \
    _a.a=~(_p.a); \
    _a.b=~(_p.b)

  #define U128_NOT_SELF(_a) \
    _a.a=~_a.a; \
    _a.b=~_a.b

  #define U128_SET_ONES(_a) \
    _a.a=0; \
    _a.a=~_a.a; \
    _a.b=_a.a

  #define U128_SET_SPAN_HALF(_a) \
    _a.a=0; \
    _a.b=U64_SPAN_HALF

  #define U128_SET_SPAN_HALF_MINUS_1(_a) \
    _a.a=U64_MAX; \
    _a.b=U64_SPAN_HALF-1

  #define U128_SET_ULP(_a) \
    _a.b=0; \
    _a.a=_a.b+1

  #define U128_SET_ZERO(_a) \
    _a.a=0; \
    _a.b=_a.a

  #define U128_SHIFT_LEFT(_a, _b, _p) \
    if((_b)<=63){ \
      _a.b=(_p.b)<<((_b)&63); \
      if(((_b)&63)){ \
        _a.b+=(_p.a)>>(u8)((64-(_b))&63); \
      } \
      _a.a=(_p.a)<<((_b)&63); \
    }else{ \
      _a.b=(_p.a)<<((_b)&63); \
      _a.a=0; \
    }

  #define U128_SHIFT_LEFT_CHECK(_a, _b, _p, _z) \
    if((_b)<=63){ \
      _a.b=(_p.b)<<((_b)&63); \
      if(((_b)&63)){ \
        _a.b+=(_p.a)>>(u8)((64-(_b))&63); \
      } \
      _a.a=(_p.a)<<((_b)&63); \
      _z=(u8)(_z|((_a.b>>((_b)&63))!=(_p.b))); \
    }else{ \
      (_a.b)=(_p.a)<<(u8)((_b)&63); \
      (_a.a)=0; \
      _z=(u8)(_z|((_p.b)||(((_a.b)>>(u8)((_b)&63))!=(_p.a)))); \
    }

  #define U128_SHIFT_LEFT_SELF(_a, _b) \
    if((_b)<=63){ \
      _a.b<<=(_b)&63; \
      if(((_b)&63)){ \
        _a.b+=_a.a>>(u8)((64-(_b))&63); \
      } \
      _a.a<<=(_b)&63; \
    }else{ \
      _a.b=_a.a<<((_b)&63); \
      _a.a=0; \
    }

  #define U128_SHIFT_LEFT_SELF_CHECK(_a, _b, _z) \
    do{ \
      u128 __p; \
      \
      __p.a=_a.a; \
      __p.b=_a.b; \
      U128_SHIFT_LEFT_CHECK(_a, _b, __p, _z); \
    }while(0)

  #define U128_SHIFT_RIGHT(_a, _b, _p) \
    if((_b)<=63){ \
      _a.a=(_p.a)>>((_b)&63); \
      if((_b)&63){ \
        _a.a+=(_p.b)<<(u8)((64-(_b))&63); \
      } \
      _a.b=(_p.b)>>((_b)&63); \
    }else{ \
      _a.a=(_p.b)>>(u8)((_b)&63); \
      _a.b=0; \
    }

  #define U128_SHIFT_RIGHT_SELF(_a, _b) \
    if((_b)<=63){ \
      _a.a>>=(_b)&63; \
      if((_b)&63){ \
        _a.a+=_a.b<<(u8)((64-(_b))&63); \
      } \
      _a.b>>=(_b)&63; \
    }else{ \
      _a.a=_a.b>>(u8)((_b)&63); \
      _a.b=0; \
    }

  #define U128_SHIFTED_TO_U64(_t, _b, _p) \
    _t=((_p.a)>>(_b))|((_p.b)<<(64-(_b)))

  #define U128_SUBTRACT_FROM_U128_SELF(_a, _p) \
    _a.b=(_p.b)-_a.b-((_p.a)<_a.a); \
    _a.a=(_p.a)-_a.a

  #define U128_SUBTRACT_U128(_a, _p, _q) \
    _a.b=(_p.b)-((_p.a)<(_q.a)); \
    _a.a=(_p.a)-(_q.a); \
    _a.b-=(_q.b)

  #define U128_SUBTRACT_U128_SELF(_a, _p) \
    _a.b-=(_p.b)+(_a.a<(_p.a)); \
    _a.a-=(_p.a)

  #define U128_SUBTRACT_U64_HI(_a, _p, _v) \
    _a.b=(_p.b)-(_v)

  #define U128_SUBTRACT_U64_HI_CHECK(_a, _p, _v, _z) \
    _z=(u8)(_z|((_p.b)<(_v))); \
    _a.b=(_p.b)-(_v)

  #define U128_SUBTRACT_U64_HI_SELF(_a, _v) \
    _a.b-=(_v)

  #define U128_SUBTRACT_U64_HI_SELF_CHECK(_a, _v, _z) \
    _z=(u8)(_z|(_a.b<(_v))); \
    _a.b-=(_v)

  #define U128_SUBTRACT_U64_LO_CHECK(_a, _p, _v, _z) \
    _z=(u8)(_z|((!_p.b)&&((_p.a)<(_v)))); \
    _a.b-=((_p.a)<(_v)); \
    _a.a=(_p.a)-(_v)

  #define U128_SUBTRACT_U64_LO_SELF(_a, _v) \
    _a.b-=((_a.a)<(_v)); \
    _a.a-=(_v)

  #define U128_SUBTRACT_U64_LO_SELF_CHECK(_a, _v, _z) \
    _z=(u8)(_z|((!_a.b)&&((_a.a)<(_v)))); \
    _a.b-=((_a.a)<(_v)); \
    _a.a-=(_v)

  #define U128_SUBTRACT_U64_SHIFTED(_a, _b, _p, _v) \
    if(_b){ \
      _a.b=(_p.b)-((_v)>>(64-(_b))); \
    } \
    _a.a=(_p.a)-((_v)<<(_b)); \
    _a.b-=((_p.a)<_a.a)

  #define U128_SUBTRACT_U64_SHIFTED_SELF(_a, _b, _v) \
    do{ \
      u64 __w; \
      \
      if(_b){ \
        _a.b-=(_v)>>(64-(_b)); \
      } \
      __w=_a.a; \
      _a.a-=((_v)<<(_b)); \
      _a.b-=(__w<_a.a); \
    }while(0)

  #define U128_SWAP(_p, _q) \
    (_p.a)^=(_q.a); \
    (_p.b)^=(_q.b); \
    (_q.a)^=(_p.a); \
    (_q.b)^=(_p.b); \
    (_p.a)^=(_q.a); \
    (_p.b)^=(_q.b)

  #define U128_TO_U64_HI(_u, _p) \
    _u=(_p.b)

  #define U128_TO_U64_LO(_t, _p) \
    _t=(_p.a)

  #define U128_TO_U64_PAIR(_t, _u, _p) \
    _t=(_p.a); \
    _u=(_p.b)
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

#define FRU128_FROM_FTD128(_a, _p) \
  _a.a=(_p); \
  _a.b=(_p)

#define FRU128_FROM_FTD64_HI(_a, _w) \
  U128_FROM_U64_HI(_a.a, _w); \
  U128_FROM_U64_HI_SATURATE(_a.b, _w)

#define FRU128_FROM_FTD64_LO(_a, _v) \
  U128_FROM_U64_LO(_a.a, _v); \
  U128_FROM_U64_LO(_a.b, _v)

#define FRU128_FROM_FRU64_HI(_a, _v) \
  U128_FROM_U64_HI(_a.a, _v.a); \
  U128_FROM_U64_HI_SATURATE(_a.b, _v.b)

#define FRU128_FROM_FRU64_LO(_a, _v) \
  U128_FROM_U64_LO(_a.a, _v.a); \
  U128_FROM_U64_LO(_a.b, _v.b)

#define FRU128_FROM_FRU64(_a, _v) \
  U128_FROM_U64_HI(_a.a, _v.a); \
  U128_FROM_U64_HI_SATURATE(_a.b, _v.b)

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

#define FRU128_FROM_U128_PAIR(_a, _p, _q) \
  _a.a=(_p); \
  _a.b=(_q)

#define FRU128_LOG_DELTA_U64(_a, _v, _z) \
  _z=(u8)(_z|fracterval_u128_log_delta_u64(&_a, _v))

#define FRU128_LOG_DELTA_U64_CACHED(_a, _l0, _l1, _k, _v, _z) \
  _z=(u8)(_z|fracterval_u128_log_delta_u64_cached(&_a, &_l0, _l1, _k, _v))

#define FRU128_LOG_DELTA_U64_CACHED_UNSAFE(_a, _l0, _l1, _k, _v) \
  do{ \
    u8 _s; \
    \
    _s=1; \
    if((_v)<=(_l0)){ \
      _a=(_k)[_v]; \
      _s=U128_IS_ONES(_a.a); \
    } \
    if(_s){ \
      fracterval_u128_log_delta_u64_cached(&_a, _v); \
    } \
  }while(0)

#define FRU128_LOG_MANTISSA_DELTA_U64(_a, _p, _q, _z) \
  _z=(u8)(_z|fracterval_u128_log_mantissa_delta_u64(&_a, _p, _q))

#define FRU128_LOG_MANTISSA_U64(_a, _p, _z) \
  _z=(u8)(_z|fracterval_u128_log_mantissa_u64(&_a, _p))

#define FRU128_LOG_U64(_a, _v, _z) \
  _z=(u8)(_z|fracterval_u128_log_u64(&_a, _v))

#define FRU128_LOG_U64_CACHED(_a, _l0, _l1, _k, _v, _z) \
  _z=(u8)(_z|fracterval_u128_log_u64_cached(&_a, &_l0, _l1, _k, _v))

#define FRU128_LOG_U64_CACHED_UNSAFE(_a, _l0, _l1, _k, _v) \
  do{ \
    u8 _s; \
    \
    _s=1; \
    if((_v)<=(_l0)){ \
      _a=(_k)[_v]; \
      _s=U128_IS_ONES(_a.a); \
    } \
    if(_s){ \
      fracterval_u128_log_u64_cached(&_a, _v); \
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
  _z=(u8)(_z|fracteroid_u128_ratio_u128_saturate(&_a.a, _p, _q)); \
  _a.b=_a.a

#define FRU128_RATIO_U64_SATURATE(_a, _v, _w, _z) \
  _z=(u8)(_z|fractoid_u128_ratio_u64_saturate(&_a.a, _v, _w)); \
  _a.b=_a.a

#define FRU128_RECIPROCAL_U128_SATURATE(_a, _v, _z) \
  _z=(u8)(_z|fractoid_u128_reciprocal_u128_saturate(&_a.a, _p)); \
  _a.b=_a.a

#define FRU128_RECIPROCAL_U64_SATURATE(_a, _v, _z) \
  _z=(u8)(_z|fractoid_u128_reciprocal_u64_saturate(&_a.a, _v)); \
  _a.b=_a.a

#define FRU128_SCALE_U128(_a, _p, _q) \
  _a.a=fractoid_u128_scale_u128(_p, _q); \
  _a.b=_a.a

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

#define FTD128_RATIO_U128_SATURATE(_a, _p, _q, _z) \
  _z=(u8)(_z|fractoid_u128_ratio_u128_saturate(&_a, _p, _q))

#define FTD128_RATIO_U128_SATURATE_SELF(_a, _p, _z) \
  _z=(u8)(_z|fractoid_u128_ratio_u128_saturate(&_a, _a, _p))

#define FTD128_RATIO_U64_SATURATE(_a, _v, _w, _z) \
  _z=(u8)(_z|fractoid_u128_ratio_u64_saturate(&_a, _v, _w))

#define FTD128_RECIPROCAL_U128_SATURATE(_a, _p, _z) \
  _z=(u8)(_z|fractoid_u128_reciprocal_u128_saturate(&_a, _p))

#define FTD128_RECIPROCAL_U64_SATURATE(_a, _v, _z) \
  _z=(u8)(_z|fractoid_u128_reciprocal_u64_saturate(&_a, _v))

#define FTD128_SCALE_U128(_a, _p, _q) \
  _a=fractoid_u128_scale_u128(_p, _q)
