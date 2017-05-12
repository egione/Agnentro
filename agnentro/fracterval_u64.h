/*
Fracterval U64
Copyright 2017 Russell Leidich

This collection of files constitutes the Fracterval U64 Library. (This is a
library in the abstact sense; it's not intended to compile to a ".lib"
file.)

The Fracterval U64 Library is free software: you can redistribute it and/or
modify it under the terms of the GNU Limited General Public License as
published by the Free Software Foundation, version 3.

The Fracterval U64 Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
Limited General Public License version 3 for more details.

You should have received a copy of the GNU Limited General Public
License version 3 along with the Fracterval U64 Library (filename
"COPYING"). If not, see http://www.gnu.org/licenses/ .
*/
/*
64-bit Unsigned Fracterval and Fractoid Macros

ASSUMPTIONS

The following assumptions apply to the entire Fracterval U64 codebase:

* {X, Y} means "a value at least (X/(2^64)) and at most ((Y+1)/(2^64))", where X and Y are taken as nonnegative. In other words, the "fracterval" (FRU64) {X, Y} is equivalent to the interval [X/K, (Y/K)+ULP], where K is 64 in this case, and "ULP" refers to a "unit of the last place", which is (2^(-K)). The fracterval {X, X} is known as the "fractoid" (FTD64) X, denoted "{X}", because it's a fraction on [X/K, (X/K)+ULP]. By contrast, the "mantissa" X is exactly equal to (X/K). Finally, the 64-bit whole number X is just a "u64" (U64).

* X never exceeds Y, which implies that the minimum uncertainty is (1/(2^64)). This constraint also applies to all outputs automatically. Otherwise there are no restrictions whatsoever on input values; all macros behave in a well-defined platform-independent manner without generating machine exceptions.

* All outputs {X, Y} shall be clipped to {0, ((2^64)-1)}, corresponding to the floating-point interval [0.0, 1.0]. Division by zero, including zero divided by zero, shall return ((2^64)-1). However, the low limit of a fracterval after division by zero may still be some valid and lesser value, depending on the divisor.

* When using these macros, terminate them with a semicolon, as with any C statement. When defining them, omit the semicolon.

* Inputs may be one and the same, e.g. _p and _q may be the same. Other than _z outputs, however, outputs may not be directed to an input variable. Inputs and outputs must be disjoint; if they overlap, use a macro ending in "_SELF". This partitioning is required in order to facilitate future optimization.

* In macro parameter lists, outputs shall appear to the left of inputs; within this constraint, numerators, minuends, and dividends appear to the left of denominators, subtrahends, and divisors, respectively.

* These macros should be used instead of calling C functions directly, so that they can be transparently optimized as needed for the best tradeoff between speed and instruction cache footprint.

* Shift counts must not exceed the MSB of the type in question (but _can_ result in loss of the MSB on left shift, which shall result in all ones).

* By default, the "FRU64" macros perform overflow checking if overflow is possible; those that end in "UNSAFE" don't, and assume that the programmer is careful enough to verify that overflow either can't occur or would be innocuous. The advantage to "UNSAFE" macros is that they're optimized for speed, typically involving inline code.

VARIABLE NAMING CONVENTIONS

The code in the entire Fracterval U64 codebase mostly follows the guideline below, but as a practical matter, some exceptions are necessary.

* _a is the primary 64(x2)-bit output, which is a u64 fracterval, u64 fractoid, u64 mantissa, or u64 whole number. In "_SELF" macros, it's also an input.

* _b is a bit count, which should be a u8 for optimal performance but need not be.

* _c is a u8, such as a borrow or a carry.

* _d is analogous but secondary to _a.

* _i is an index private to a macro.

* _j is an index private to a macro.

* _k ("kache") is base of a list.

* _l is the maximum ("limit") index of a list. Specifically, _l0 denotes the current limit, whereas _l1 denotes the upper bound.

* _m is the base of a data structure other than a list, which is always _k.

* _n is a ULONG, which may be an index into a list.

* _p is the primary 64(x2)-bit input, which is a u64 fracterval, u64 fractoid, u64 mantissa, or u64 whole number.

* _q is analogous but secondary to _p.

* _s is a status private to a macro.

* _t is the primary 64-bit output, which is a u64 fractoid, u64 mantissa, or u64 whole number, but never a fracterval. Note that this differs from the Fracterval U128 definition.

* _u is analogous but secondary to _t.

* _v is the primary 64-bit input, which is a u64 fractoid, u64 mantissa, or u64 whole number, but never a fracterval. Note that this differs from the Fracterval U128 definition.

* _w is analogous but secondary to _v.

* _z is an overflow status which is set to one when a result wholly or partially outside of [0.0, 1.0] is clipped to that interval. _z is otherwise unchanged, so that exceptions can occur in a silent and well-defined manner, to be discovered in some future context after the computation. (Merely equalling 0.0 or 1.0 should not set _z, but in theory that could occur due to precision limitations, in particular with transcendental functions.)
*/
TYPEDEF_START
  u64 a;
  u64 b;
TYPEDEF_END(fru64);

#define FRU64_LOG2_FLOOR 0xB17217F7D1CF79ABULL

#define FTD64_RATIO_U64_SATURATE(_a, _v, _w, _z) \
  _z=(u8)(_z|fractoid_u64_ratio_u64_saturate(&_a, _v, _w))

#define FTD64_RECIPROCAL_U64_SATURATE(_a, _v, _z) \
  _z=(u8)(_z|fractoid_u64_reciprocal_u64_saturate(&_a, _v))

#define FTD64_SCALE_U64(_a, _v, _w) \
  _a=fractoid_u64_scale_u64(_v, _w)

#define FRU64_ADD_FRU64(_a, _p, _q, _z) \
  _a.a=_p.a+_q.a; \
  _a.b=_p.b+_q.b+1; \
  if(_a.b<=_q.b){ \
    _a.b=U64_MAX; \
    _z=1; \
    if(_a.a<_q.a){ \
      _a.a=_a.b; \
    } \
  }

#define FRU64_ADD_FRU64_SELF(_a, _p, _z) \
  _a.a+=_p.a; \
  _a.b+=_p.b+1; \
  if(_a.b<=_p.b){ \
    _a.b=U64_MAX; \
    _z=1; \
    if(_a.a<_p.a){ \
      _a.a=_a.b; \
    } \
  }

#define FRU64_ADD_FTD64(_a, _p, _q, _z) \
  _a.a=_p.a+(_q); \
  _a.b=_p.b+(_q)+1; \
  if(_a.b<=(_q)){ \
    _a.b=U64_MAX; \
    _z=1; \
    if(_a.a<(_q)){ \
      _a.a=_a.b; \
    } \
  }

#define FRU64_ADD_FTD64_SELF(_a, _p, _z) \
  _a.a+=(_p); \
  _a.b+=(_p)+1; \
  if(_a.b<=(_p)){ \
    _a.b=U64_MAX; \
    _z=1; \
    if(_a.a<(_p)){ \
      _a.a=_a.b; \
    } \
  }

#define FRU64_DIVIDE_FRU64(_a, _p, _q, _z) \
  _z=(u8)(_z|fracterval_u64_divide_fracterval_u64(&_a, _p, _q))

#define FRU64_DIVIDE_FRU64_SELF(_a, _p, _z) \
  _z=(u8)(_z|fracterval_u64_divide_fracterval_u64(&_a, _a, _p))

#define FRU64_DIVIDE_U64(_a, _p, _v, _z) \
  _z=(u8)(_z|fracterval_u64_divide_u64(&_a, _p, _v))

#define FRU64_DIVIDE_U64_SELF(_a, _v, _z) \
  _z=(u8)(_z|fracterval_u64_divide_u64(&_a, _a, _v))

#define FRU64_EXPAND_DOWN(_a, _p, _z) \
  _a.a=_p.a-1; \
  if(!_p.a){ \
    _a.a++; \
    _z=1; \
  }

#define FRU64_EXPAND_DOWN_SELF(_a, _z) \
  if(_a.a){ \
    _a.a--; \
  }else{ \
    _z=1; \
  }

#define FRU64_EXPAND_UP(_a, _p, _z) \
  _a.b=_p.b+1; \
  if(!_a.b){ \
    _a.b--; \
    _z=1; \
  }

#define FRU64_EXPAND_UP_SELF(_a, _z) \
  _a.b++; \
  if(!_a.b){ \
    _a.b--; \
    _z=1; \
  }

#define FRU64_FROM_FTD64(_a, _p) \
  _a.a=(_p); \
  _a.b=(_p)

#define FRU64_FROM_U64_PAIR(_a, _v, _w) \
  _a.a=(_v); \
  _a.b=(_w)

#define FRU64_LOG_DELTA_U64(_a, _v, _z) \
  _z=(u8)(_z|fracterval_u64_log_delta_u64(&_a, _v))

#define FRU64_LOG_DELTA_U64_CACHED(_a, _l0, _l1, _k, _v, _z) \
  _z=(u8)(_z|fracterval_u64_log_delta_u64_cached(&_a, &_l0, _l1, _k, _v))

#define FRU64_LOG_DELTA_U64_CACHED_UNSAFE(_a, _l0, _l1, _k, _v) \
  do{ \
    u8 _s; \
    \
    _s=1; \
    if((_v)<=(_l0)){ \
      _a=(_k)[_v]; \
      _s=(_a.a==U64_MAX); \
    } \
    if(_s){ \
      fracterval_u64_log_delta_u64_cached(&_a, &_l0, _l1, _k, _v); \
    } \
  }while(0)

#define FRU64_LOG_MANTISSA_DELTA_U64(_a, _p, _q, _z) \
  _z=(u8)(_z|fracterval_u64_log_mantissa_delta_u64(&_a, _p, _q))

#define FRU64_LOG_MANTISSA_U64(_a, _p, _z) \
  _z=(u8)(_z|fracterval_u64_log_mantissa_u64(&_a, _p))

#define FRU64_LOG_U64(_a, _v, _z) \
  _z=(u8)(_z|fracterval_u64_log_u64(&_a, _v))

#define FRU64_LOG_U64_CACHED(_a, _l0, _l1, _k, _v, _z) \
  _z=(u8)(_z|fracterval_u64_log_u64_cached(&_a, &_l0, _l1, _k, _v))

#define FRU64_LOG_U64_CACHED_UNSAFE(_a, _l0, _l1, _k, _v) \
  do{ \
    u8 _s; \
    \
    _s=1; \
    if((_v)<=(_l0)){ \
      _a=(_k)[_v]; \
      _s=(_a.a==U64_MAX); \
    } \
    if(_s){ \
      fracterval_u64_log_u64_cached(&_a, &_l0, _l1, _k, _v); \
    } \
  }while(0)
/*
FRU64_MEAN_TO_FTD64() computes the mean of fracterval {A, B} in a manner consistent with the division of a fracterval by an integer, namely 2. The result is then a fracterval that we wish to approximate as a fractoid.

  mean({A, B})={(A+B)>>1, ((A+B+1)>>1)-(!((A+B+1)&1))}

This is equivalent to:

  {(A>>1)+(B>>1), (A>>1)+(B>>1)}, if neither A nor B is odd
  {(A>>1)+(B>>1)+1, (A>>1)+(B>>1)+1}, if both A and B are odd

such that, in fact, the result is always contained by a fractoid.
*/
#define FRU64_MEAN_TO_FTD64(_a, _p) \
  _a=(_p.a>>1)+(_p.b>>1)+(1&_p.a&_p.b)

#define FRU64_MULTIPLY_FRU64(_a, _p, _q) \
  fracterval_u64_multiply_fracterval_u64(&_a, _p, _q)

#define FRU64_MULTIPLY_FRU64_SELF(_a, _p) \
  fracterval_u64_multiply_fracterval_u64(&_a, _a, _p)

#define FRU64_MULTIPLY_FTD64(_a, _p, _q) \
  fracterval_u64_multiply_fractoid_u64(&_a, _p, _q)

#define FRU64_MULTIPLY_FTD64_SELF(_a, _p) \
  fracterval_u64_multiply_fractoid_u64(&_a, _a, _p)

#define FRU64_MULTIPLY_MANTISSA_U64(_a, _p, _q) \
  fracterval_u64_multiply_mantissa_u64(&_a, _p, _q)

#define FRU64_MULTIPLY_MANTISSA_U64_SELF(_a, _p) \
  fracterval_u64_multiply_mantissa_u64(&_a, _a, _p)

#define FRU64_MULTIPLY_U64(_a, _p, _v, _z) \
  _z=(u8)(_z|fracterval_u64_multiply_u64(&_a, _p, _v))

#define FRU64_MULTIPLY_U64_SELF(_a, _v, _z) \
  _z=(u8)(_z|fracterval_u64_multiply_u64(&_a, _a, _v))

#define FRU64_NATS_FROM_BITS(_a, _p) \
  fracterval_u64_nats_from_bits(&_a, _p)

#define FRU64_NATS_TO_BITS(_a, _p, _z) \
  _z=(u8)(_z|fracterval_u64_nats_to_bits(&_a, _p))

#define FRU64_NOT_FRU64(_a, _p) \
  _a.b=~_p.a; \
  _a.a=~_p.b

#define FRU64_NOT_SELF(_a) \
  do{ \
    u64 _p; \
    \
    _p=~_a.a; \
    _a.a=~_a.b; \
    _a.b=_p; \
  }while(0)

#define FRU64_RATIO_U64_SATURATE(_a, _v, _w, _z) \
  _z=(u8)(_z|fractoid_u64_ratio_u64_saturate(&_a.a, _v, _w)); \
  _a.b=_a.a

#define FRU64_RECIPROCAL_U64_SATURATE(_a, _v, _z) \
  _z=(u8)(_z|fractoid_u64_reciprocal_u64_saturate(&_a.a, _v)); \
  _a.b=_a.a

#define FRU64_SCALE_U64(_a, _v, _w) \
  _a.a=fractoid_u64_scale_u64(_v, _w); \
  _a.b=_a.a

#define FRU64_SET_AMBIGUOUS(_a) \
  _a.a=0; \
  _a.b=~_a.a

#define FRU64_SET_HALF_BIASED_HI(_a) \
  _a.a=U64_SPAN_HALF; \
  _a.b=_a.a

#define FRU64_SET_HALF_BIASED_LO(_a) \
  _a.a=U64_SPAN_HALF-1; \
  _a.b=_a.a

#define FRU64_SET_HALF_UNBIASED(_a) \
  _a.b=U64_SPAN_HALF; \
  _a.a=~_a.b

#define FRU64_SET_ONES(_a) \
  _a.a=U64_MAX; \
  _a.b=_a.a

#define FRU64_SET_ZERO(_a) \
  _a.a=0; \
  _a.b=_a.a

#define FRU64_SHIFT_LEFT(_a, _b, _p, _z) \
  _z=(u8)(_z|fracterval_u64_shift_left(&_a, _b, _p))

#define FRU64_SHIFT_LEFT_SELF(_a, _b, _z) \
  _z=(u8)(_z|fracterval_u64_shift_left(&_a, _b, _a))

#define FRU64_SHIFT_RIGHT(_a, _b, _p) \
  _a.a=_p.a>>(_b); \
  _a.b=_p.b>>(_b)

#define FRU64_SHIFT_RIGHT_SELF(_a, _b) \
  _a.a>>=_b; \
  _a.b>>=_b

#define FRU64_SUBTRACT_FRU64(_a, _p, _q, _z) \
  _a.a=_p.a-_q.b; \
  _a.a--; \
  _a.b=_p.b-_q.a; \
  if(_p.a<=_a.a){ \
    _a.a=0; \
    _z=1; \
    if(_p.b<_a.b){ \
      _a.b=_a.a; \
    } \
  }

#define FRU64_SUBTRACT_FRU64_SELF(_a, _p, _z) \
  do{ \
    fru64 _q; \
    \
    _q=_a; \
    FRU64_SUBTRACT_FRU64(_a, _q, _p, _z); \
  }while(0)

#define FRU64_SUBTRACT_FTD64(_a, _p, _q, _z) \
  _a.a=_p.a-(_q); \
  _a.a--; \
  _a.b=_p.b-(_q); \
  if(_p.a<=_a.a){ \
    _a.a=0; \
    _z=1; \
    if(_p.b<_a.b){ \
      _a.b=_a.a; \
    } \
  }

#define FRU64_SUBTRACT_FTD64_SELF(_a, _p, _z) \
  do{ \
    fru64 _q; \
    \
    _q=_a; \
    FRU64_SUBTRACT_FTD64(_a, _q, _p, _z); \
  }while(0)

#define FRU64_SUBTRACT_FROM_FRU64_SELF(_a, _p, _z) \
  do{ \
    fru64 _q; \
    \
    _q=_a; \
    FRU64_SUBTRACT_FRU64(_a, _p, _q, _z); \
  }while(0)

#define FRU64_TO_DIAMETER_MINUS_1_FTD64(_t, _p) \
  _t=_p.b-_p.a

#define FRU64_TO_U64_PAIR(_t, _u, _p) \
  _t=_p.a; \
  _u=p.b

#define FRU64_UNION(_a, _p, _q) \
  _a.a=((_q.a<=_p.a)?_q.a:_p.a); \
  _a.b=((_p.b<=_q.b)?_q.b:_p.b)

#define FRU64_UNION_SELF(_a, _p) \
  if(_p.a<=_a.a){ \
    _a.a=_p.a; \
  } \
  if(_a.b<=_p.b){ \
    _a.b=_p.b; \
  }
