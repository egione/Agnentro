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
64-bit Unsigned Fracterval and Fractoid Kernel

Documentation is in the header of fracterval_u64.h.
*/
#include "flag.h"
#include "flag_fracterval_u64.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "constant.h"
#include "debug.h"
#include "debug_xtrn.h"
#include "bitscan.h"
#include "bitscan_xtrn.h"
#include "fracterval_u64.h"
#include "fracterval_u64_xtrn.h"

u8
fracterval_u64_divide_fracterval_u64(fru64 *a_base, fru64 p, fru64 q){
/*
Use FRU64_DIVIDE_FRU64() instead of calling here directly.

Divide one fracterval by another.

In:

  *a_base is undefined.

  p is the dividend fracterval.

  q is the divisor fracterval.

Out:

  Returns one if ((q.b+1)/p.a) exceeds one, or q.a is zero; else zero.

  *a_base is (p/q) expressed as a fracterval.
*/
  fru64 a;
  #ifdef _64_
    u128 dividend;
    u64 p_b_plus_1;
    u64 q_b_plus_1;
  #else
    u64 dividend;
    u64 dividend0;
    u64 dividend1;
    u8 divisor_shift;
    u32 divisor_u32;
    u64 divisor_u64;
    u32 divisor_u64_hi;
    u32 divisor_u64_lo;
    u8 phase;
    u64 product;
    u64 product_shifted;
    u32 quotient;
  #endif
  u8 status;

  status=0;
  #ifdef _64_
    a.a=p.a;
    dividend=(u128)(p.a)<<64;
    q_b_plus_1=q.b+1;
    if(q_b_plus_1){
      a.a=U64_MAX;
      if(p.a<q_b_plus_1){
        a.a=(u64)(dividend/q_b_plus_1);
      }
    }
    a.b=U64_MAX;
    p_b_plus_1=p.b+1;
    if((p_b_plus_1<=q.a)&&p_b_plus_1){
      status=1;
      if(p_b_plus_1<q.a){
        dividend=(u128)(p_b_plus_1)<<64;
        a.b=(u64)(dividend/q.a);
        a.b-=!(dividend%q.a);
        status=0;
      }
    }
  #else
    a.b=0;
    phase=0;
    do{
      a.a=a.b;
      if(!phase){
        dividend=p.a;
        divisor_u64=q.b+1;
      }else{
        dividend=p.b+1;
        divisor_u64=q.a;
      }
      if(divisor_u64&&(dividend<=divisor_u64)){
        a.b=0;
        divisor_shift=0;
        while(!(divisor_u64>>(U64_BITS-U8_BITS-divisor_shift))){
          divisor_shift=(u8)(divisor_shift+8);
        }
        dividend0=0;
        dividend1=dividend<<divisor_shift;
        dividend=dividend1;
        divisor_u64<<=divisor_shift;
        divisor_u64_hi=(u32)(divisor_u64>>U32_BITS);
        divisor_u64_lo=(u32)(divisor_u64);
        divisor_u32=divisor_u64_hi;
        divisor_u32++;
        quotient=(u32)(dividend>>U32_BITS);
        if(divisor_u32){
          quotient=(u32)(dividend/divisor_u32);
        }
        a.b=(u64)(quotient)<<U32_BITS;
        product=(u64)(divisor_u64_lo)*quotient;
        product_shifted=product<<U32_BITS;
        dividend1-=(dividend0<product_shifted);
        dividend0-=product_shifted;
        product_shifted=product>>(U64_BITS-U32_BITS);
        dividend1-=product_shifted;
        product=(u64)(divisor_u64_hi)*quotient;
        dividend1-=product;
        dividend=(dividend0>>(U64_BITS-22))|(dividend1<<22);
        quotient=(u32)(dividend>>U32_BITS);
        if(divisor_u32){
          quotient=(u32)(dividend/divisor_u32);
        }
        a.b+=(u64)(quotient)<<(U32_BITS-22);
        product=(u64)(divisor_u64_lo)*quotient;
        product_shifted=product<<(U32_BITS-22);
        dividend1-=(dividend0<product_shifted);
        dividend0-=product_shifted;
        product_shifted=product>>(U64_BITS-(U32_BITS-22));
        dividend1-=product_shifted;
        product=(u64)(divisor_u64_hi)*quotient;
        product_shifted=product<<(U64_BITS-22);
        dividend1-=(dividend0<product_shifted);
        dividend0-=product_shifted;
        product_shifted=product>>22;
        dividend1-=product_shifted;
        dividend=(dividend0>>(U64_BITS-(22+22)))|(dividend1<<(22+22));
        quotient=(u32)(dividend>>U32_BITS);
        if(divisor_u32){
          quotient=(u32)(dividend/divisor_u32);
        }
        quotient>>=22+22-32;
        a.b+=(u64)(quotient);
        product=(u64)(divisor_u64_lo)*quotient;
        dividend1-=(dividend0<product);
        dividend0-=product;
        product=(u64)(divisor_u64_hi)*quotient;
        product_shifted=product<<U32_BITS;
        dividend1-=(dividend0<product_shifted);
        dividend0-=product_shifted;
        product_shifted=product>>(U64_BITS-U32_BITS);
        dividend1-=product_shifted;
        if((divisor_u64<=dividend0)||dividend1){
          a.b++;
          dividend0-=divisor_u64;
        }
        if(!phase){
          if((!a.b)&&p.a){
            a.b=U64_MAX;
          }
        }else{
          if(!dividend0){
            if(p.b==U64_MAX){
              status=1;
            }
            a.b--;
          }
        }
      }else{
        if((!divisor_u64)&&(!phase)){
          a.b=p.a;
        }else{
          a.b=U64_MAX;
          status=1;
        }
      }
      phase=!phase;
    }while(phase);
  #endif
  *a_base=a;
  return status;
}

u8
fracterval_u64_divide_u64(fru64 *a_base, fru64 p, u64 v){
/*
Use FRU64_DIVIDE_U64() instead of calling here directly.

Divide a fracterval by a u64.

In:

  *a_base is undefined.

  p is the dividend fracterval.

  v is the divisor.

Out:

  Returns one if v is zero; else zero.

  *a_base is (p/v) expressed as a fracterval.
*/
  fru64 a;
  u8 status;

  if(v){
    a.a=p.a/v;
    a.b=p.b/v;
    status=0;
  }else{
    FRU64_SET_ONES(a);
    status=1;
  }
  *a_base=a;
  return status;
}

void *
fracterval_u64_free(void *base){
/*
To maximize portability and debuggability, this is the only function in which Fracterval U64 calls free().

In:

  base is the base of a memory region to free. May be NULL.

Out:

  Returns NULL so that the caller can easily maintain the good practice of NULLing out invalid pointers.

  *base is freed.
*/
  DEBUG_FREE_PARANOID(base);
  return NULL;
}

u8
fracterval_u64_init(u32 build_break_count, u32 build_feature_count){
/*
Verify that the source code is sufficiently updated.

In:

  build_break_count is the caller's most recent knowledge of FRU64_BUILD_BREAK_COUNT, which will fail if the caller is unaware of all critical updates.

  build_feature_count is the caller's most recent knowledge of FRU64_BUILD_FEATURE_COUNT, which will fail if this library is not up to date with the caller's expectations.

Out:

  Returns one if (build_break_count!=FRU64_BUILD_BREAK_COUNT) or (build_feature_count>FRU64_BUILD_FEATURE_COUNT). Otherwise, returns zero.
*/
  u8 status;

  status=(u8)(build_break_count!=FRU64_BUILD_BREAK_COUNT);
  status=(u8)(status|(FRU64_BUILD_FEATURE_COUNT<build_feature_count));
  return status;
}

fru64 *
fracterval_u64_list_malloc(ULONG fru64_idx_max){
/*
Allocate a list of undefined (fru64)s.

In:

  fru64_idx_max is the number of (fru64)s to allocate, less one.

Out:

  Returns NULL on failure, else the base of (fru64_idx_max+1) undefined items. It must be freed via fracterval_u64_free().
*/
  ULONG fru64_count;
  fru64 *list_base;
  u64 list_bit_count;
  ULONG list_size;

  list_base=NULL;
  fru64_count=fru64_idx_max+1;
  if(fru64_count){
    list_size=fru64_count<<(U64_SIZE_LOG2+1);
    if((list_size>>(U64_SIZE_LOG2+1))==fru64_count){
/*
Ensure that the allocated size in bits can be described in 64 bits.
*/
      list_bit_count=(u64)(list_size)<<U8_BITS_LOG2;
      if((list_bit_count>>U8_BITS_LOG2)==list_size){
        list_base=DEBUG_MALLOC_PARANOID(list_size);
      }
    }
  }
  return list_base;
}

u8
fracterval_u64_log_delta_u64(fru64 *a_base, u64 v){
/*
Use FRU64_LOG_DELTA_U64() instead of calling here directly.

Compute the difference between successive natural logs in 6.58 fixed-point.

In:

  *a_base is undefined.

  v is the u64 from which to compute (log(v+1)-log(v)), on [1, U64_MAX].

Out:

  Returns one if v is zero, else zero.

  *a_base is (log(v+1)-log(v)) expressed as a 6.58 fixed-point fracterval if v is nonzero, else the ambiguous fracterval.
*/
  fru64 log_delta;
  u64 mantissa0;
  u64 mantissa1;
  u8 msb;
  u8 shift;
  u8 status;
  u64 v_plus_1;

  FRU64_SET_ZERO(log_delta);
  status=0;
  v_plus_1=v+1;
  if(2<v_plus_1){
    BITSCAN_MSB64_SMALL_GET(msb, v);
    shift=(u8)(U64_BIT_MAX-msb);
    mantissa0=v<<shift;
    mantissa1=v_plus_1<<shift;
/*
mantissa0 and mantissa1 have their high bits set, with (mantissa0<mantissa1), so fracterval_u64_log_mantissa_delta_u64() is guaranteed to return zero status.
*/
    fracterval_u64_log_mantissa_delta_u64(&log_delta, mantissa0, mantissa1);
    FRU64_SHIFT_RIGHT_SELF(log_delta, U64_BITS_LOG2);
  }else{
    if(v){
      if(v_plus_1){
        log_delta.a=FRU64_LOG2_FLOOR>>U64_BITS_LOG2;
        log_delta.b=log_delta.a;
      }
    }else{
      log_delta.b=~log_delta.b;
      status=1;
    }
  }
  *a_base=log_delta;
  return status;
}

fru64 *
fracterval_u64_log_delta_u64_cache_init(ULONG log_delta_idx_max_max){
/*
Allocate a cache for saving previous fracterval_u64_log_delta_u64() results.

In:

  log_delta_idx_max_max is one less than the number of items to allocate in the cache.

Out:

  Returns NULL on failure, else the base of a list containing log_delta_idx_max_max undefined 64-bit fractervals, except for the first one, which is set to ones in order to force a cache miss because it's an error case, whereas cache hits must be associated with good status. It must be freed via fracterval_u64_free().
*/
  fru64 *log_delta_list_base;
  u64 ones;

  log_delta_list_base=fracterval_u64_list_malloc(log_delta_idx_max_max);
  if(log_delta_list_base){
    ones=U64_MAX;
    log_delta_list_base[0].a=ones;
    log_delta_list_base[0].b=ones;
  }
  return log_delta_list_base;
}

u8
fracterval_u64_log_delta_u64_cached(fru64 *a_base, ULONG *log_delta_idx_max_base, ULONG log_delta_idx_max_max, fru64 *log_delta_list_base, u64 v){
/*
Use FRU64_LOG_DELTA_U64_CACHED() instead of calling here directly.

Deliver cached results from fracterval_u64_log_delta_u64().

In:

  *a_base is undefined.

  *log_delta_idx_max_base is initially zero, and fed back thereafter. It defines the maximum index at which the cache is defined.

  log_delta_idx_max_max is fracterval_u64_log_delta_u64_cache_init():In:log_delta_idx_max_max.

  log_delta_list_base is the return value of fracterval_u64_log_delta_u64_cache_init().

  v is as defined in fracterval_u64_log_delta_u64().

Out:

  Returns as defined for fracterval_u64_log_delta_u64(a, v).

  *a_base is as defined in fracterval_u64_log_delta_u64().

  *log_delta_idx_max_base is updated.

  *log_delta_list_base is consistent through index *log_delta_idx_max_base.
*/
  fru64 a;
  ULONG log_delta_idx;
  ULONG log_delta_idx_max;
  u64 ones;
  u8 status;

  log_delta_idx_max=*log_delta_idx_max_base;
  log_delta_idx=(ULONG)(v);
  ones=U64_MAX;
  status=1;
  if(log_delta_idx<=log_delta_idx_max){
    a=log_delta_list_base[log_delta_idx];
    status=(a.a==ones);
  }
  if(status){
    status=fracterval_u64_log_delta_u64(&a, v);
/*
Only cache results which can fit in the cache and returned good status because good status is always returned on cache hits.
*/
    if((log_delta_idx<=log_delta_idx_max_max)&&(!status)){
      if(log_delta_idx_max<log_delta_idx){
        while(log_delta_idx!=(++log_delta_idx_max)){
          log_delta_list_base[log_delta_idx_max].a=ones;
          log_delta_list_base[log_delta_idx_max].b=ones;
        }
        *log_delta_idx_max_base=log_delta_idx;
      }
      log_delta_list_base[log_delta_idx]=a;
    }
  }
  *a_base=a;
  return status;
}

u8
fracterval_u64_log_mantissa_delta_u64(fru64 *a_base, u64 p, u64 q){
/*
Use FRU64_LOG_MANTISSA_DELTA_U64() instead of calling here directly.

Compute the absolute value of the difference between the natural logs of sequential mantissas on [0.5, 1.0).

In:

  *a_base is undefined.

  p is a mantissa on [U64_SPAN_HALF, U64_MAX-1], corresponding to [0.5, 1.0). Unlike a fractoid which has an uncertainty of one ULP, a mantissa is assumed to have no uncertainty.

  q is analogous to p, but on [p+1, U64_MAX].

Out:

  Returns one if (p<U64_SPAN_HALF), else zero.

  *a_base is (log(q)-log(p)) expressed as a 6.58 fracterval if (U64_SPAN_HALF<=p), else the ambiguous fracterval.
*/
  u64 denominator;
  fru64 log_delta;
  u64 p_minus_q;
  fru64 p_power;
  fru64 q_power;
  u8 status;
  fru64 term;
/*
This is just 2 instances of fracterval_u64_log_mantissa_u64() fused together for speed and accuracy. See that function for comments.
*/
  FRU64_SET_ZERO(log_delta);
  status=0;
  if(p>>U64_BIT_MAX){
    denominator=1;
    p=(0U-p);
    q=(0U-q);
    FRU64_FROM_FTD64(p_power, p);
    FRU64_FROM_FTD64(q_power, q);
    p_minus_q=p-q;
    FRU64_FROM_FTD64(log_delta, p_minus_q);
    do{
      FRU64_MULTIPLY_MANTISSA_U64_SELF(p_power, p);
      FRU64_MULTIPLY_MANTISSA_U64_SELF(q_power, q);
      denominator++;
/*
Only underflow can occur in the following subtract, which is safe to ignore.
*/
      FRU64_SUBTRACT_FRU64(term, p_power, q_power, status);
      FRU64_DIVIDE_U64_SELF(term, denominator, status);
      FRU64_ADD_FRU64_SELF(log_delta, term, status);
    }while(term.b);
    FRU64_EXPAND_UP_SELF(log_delta, status);
/*
If status is one, it's because underflow occured, which doesn't affect correctness of the result. Forget it.
*/
    status=0;
  }else{
    log_delta.b=~log_delta.b;
    status=1;
  }
  *a_base=log_delta;
  return status;
}

u8
fracterval_u64_log_mantissa_u64(fru64 *a_base, u64 p){
/*
Use FRU64_LOG_MANTISSA_U64() instead of calling here directly.

Compute the negative natural log of a mantissa on [0.5, 1.0).

In:

  *a_base is undefined.

  p is a mantissa which is the operand of the log, on [U64_SPAN_HALF, U64_MAX], corresponding to [0.5, 1.0). Unlike a fractoid which has an uncertainty of one ULP, a mantissa is assumed to have no uncertainty.

Out:

  Returns one if (p<U64_SPAN_HALF), else zero.

  *a_base is (-log(p/(2^64))) expressed as a fracterval if (U64_SPAN_HALF<=p), else the ambiguous fracterval.
*/
  u64 denominator;
  fru64 log;
  fru64 power;
  u8 status;
  fru64 term;

  FRU64_SET_ZERO(log);
  status=0;
  if(p>>U64_BIT_MAX){
    denominator=1;
/*
Negate the mantissa to produce the (x-1) term in the log series. As stated above, mantissas are assumed to have zero uncertainty, so we can do this with u64 negation instead of fracterval negation.
*/
    p=(0U-p);
/*
Compute the terms of the log series. We lose some accuracy by converting a mantissa to a fracterval, but so what. On the plus side, mantissa multiplication is more accurate than fractoid multiplication.
*/
    FRU64_FROM_FTD64(power, p);
    FRU64_FROM_FTD64(log, p);
    do{
      FRU64_MULTIPLY_MANTISSA_U64_SELF(power, p);
      denominator++;
      FRU64_DIVIDE_U64(term, power, denominator, status);
      FRU64_ADD_FRU64_SELF(log, term, status);
    }while(term.b);
/*
The error is bounded by the magnitude of the last term computed because terms are scaling by a factor of less than (1/2). The last term spans one ULP because term.b is zero.
*/
    FRU64_EXPAND_UP_SELF(log, status);
/*
If status is one, it's because underflow occured, which doesn't affect correctness of the result. Forget it.
*/
    status=0;
  }else{
    log.b=~log.b;
    status=1;
  }
  *a_base=log;
  return status;
}

u8
fracterval_u64_log_u64(fru64 *a_base, u64 v){
/*
Use FRU64_LOG_U64() instead of calling here directly.

Compute the 6.58 fixed-point natural log of a u64.

In:

  *a_base is undefined.

  v is the u64 whose log to compute.

Out:

  Returns one if v is zero, else zero.

  *a_base is log(v) expressed as a 6.58 fixed-point fracterval if v is nonzero, else the ambiguous fracterval.
*/
  fru64 log_fractoid;
  fru64 log;
  fru64 log2;
  u64 log2_count;
  u64 mantissa;
  u8 msb;
  u8 shift;
  u8 status;

  status=0;
  if(2<v){
    BITSCAN_MSB64_SMALL_GET(msb, v);
    shift=(u8)(U64_BIT_MAX-msb);
    mantissa=v<<shift;
/*
mantissa has its high bit set, so fracterval_u64_log_mantissa_u64() is guaranteed to return zero status.
*/
    fracterval_u64_log_mantissa_u64(&log_fractoid, mantissa);
    FRU64_SHIFT_RIGHT_SELF(log_fractoid, U64_BITS_LOG2);
    FRU64_FROM_FTD64(log2, FRU64_LOG2_FLOOR);
    FRU64_SHIFT_RIGHT_SELF(log2, U64_BITS_LOG2);
    log2_count=(u8)(msb+1);
    FRU64_MULTIPLY_U64_SELF(log2, log2_count, status);
    FRU64_SUBTRACT_FRU64(log, log2, log_fractoid, status);
  }else{
    if(v==2){
      log.a=FRU64_LOG2_FLOOR>>U64_BITS_LOG2;
      log.b=log.a;
    }else{
      log.a=0;
      if(v){
        log.b=log.a;
      }else{
        log.b=~log.a;
        status=1;
      }
    }
  }
  *a_base=log;
  return status;
}

fru64 *
fracterval_u64_log_u64_cache_init(ULONG log_idx_max_max){
/*
Allocate a cache for saving previous fracterval_u64_log_u64() results.

In:

  log_idx_max_max is one less than the number of items to allocate in the cache.

Out:

  Returns NULL on failure, else the base of a list containing log_idx_max_max undefined 64-bit fractervals, except for the first one, which is set to ones in order to force a cache miss because it's an error case, whereas cache hits must be associated with good status. It must be freed via fracterval_u64_free().
*/
  fru64 *log_list_base;
  u64 ones;

  log_list_base=fracterval_u64_list_malloc(log_idx_max_max);
  if(log_list_base){
    ones=U64_MAX;
    log_list_base[0].a=ones;
    log_list_base[0].b=ones;
  }
  return log_list_base;
}

u8
fracterval_u64_log_u64_cached(fru64 *a_base, ULONG *log_idx_max_base, ULONG log_idx_max_max, fru64 *log_list_base, u64 v){
/*
Use FRU64_LOG_U64_CACHED() instead of calling here directly.

Deliver cached results from fracterval_u64_log_u64().

In:

  *a_base is undefined.

  *log_idx_max_base is initially zero, and fed back thereafter. It defines the maximum index at which the cache is defined.

  log_idx_max_max is fracterval_u64_log_u64_cache_init():In:log_idx_max_max.

  log_list_base is the return value of fracterval_u64_log_u64_cache_init().

  v is as defined in fracterval_u64_log_u64().

Out:

  Returns as defined for fracterval_u64_log_u64(a, v).

  *a_base is as defined in fracterval_u64_log_u64().

  *log_idx_max_base is updated.

  *log_list_base is consistent through index *log_idx_max_base.
*/
  fru64 a;
  ULONG log_idx;
  ULONG log_idx_max;
  u64 ones;
  u8 status;

  log_idx_max=*log_idx_max_base;
  log_idx=(ULONG)(v);
  ones=U64_MAX;
  status=1;
  if(log_idx<=log_idx_max){
    a=log_list_base[log_idx];
    status=(a.a==ones);
  }
  if(status){
    status=fracterval_u64_log_u64(&a, v);
/*
Only cache results which can fit in the cache and returned good status because good status is always returned on cache hits.
*/
    if((log_idx<=log_idx_max_max)&&(!status)){
      if(log_idx_max<log_idx){
        while(log_idx!=(++log_idx_max)){
          log_list_base[log_idx_max].a=ones;
          log_list_base[log_idx_max].b=ones;
        }
        *log_idx_max_base=log_idx;
      }
      log_list_base[log_idx]=a;
    }
  }
  *a_base=a;
  return status;
}

void
fracterval_u64_multiply_fracterval_u64(fru64 *a_base, fru64 p, fru64 q){
/*
Use FRU64_MULTIPLY_FRU64() instead of calling here directly.

Multiply one fracterval by another.

In:

  *a_base is undefined.

  p is the first factor fracterval.

  q is the second factor fracterval.

Out:

  *a_base is (p*q) expressed as a fracterval.
*/
  fru64 a;
  #ifdef _64_
    u128 product0;
    u128 product1;
  #else
    u8 carry;
    u32 factor0_0;
    u32 factor0_1;
    u32 factor1_0;
    u32 factor1_1;
    u64 product0;
    u64 product1;
    u64 product2;
    u64 product3;
  #endif

  #ifdef _64_
    product0=(u128)(p.a)*q.a;
    product1=(u128)(p.b)*q.b;
    product1+=p.b;
    product1+=q.b;
    a.a=(u64)(product0>>64);
    a.b=(u64)(product1>>64);
  #else
    factor0_0=(u32)(p.a);
    factor0_1=(u32)(p.a>>U32_BITS);
    factor1_0=(u32)(q.a);
    factor1_1=(u32)(q.a>>U32_BITS);
    product0=(u64)(factor0_0)*factor1_0;
    product1=(u64)(factor0_1)*factor1_0;
    product2=(u64)(factor0_0)*factor1_1;
    product3=(u64)(factor0_1)*factor1_1;
    a.a=product0>>U32_BITS;
    a.a+=product1;
    a.a+=product2;
    carry=(a.a<product2);
    a.a>>=U32_BITS;
    a.a+=product3+((u64)(carry)<<U32_BITS);
    factor0_0=(u32)(p.b);
    factor0_1=(u32)(p.b>>U32_BITS);
    factor1_0=(u32)(q.b);
    factor1_1=(u32)(q.b>>U32_BITS);
    product0=(u64)(factor0_0)*factor1_0;
    product1=(u64)(factor0_1)*factor1_0;
    product2=(u64)(factor0_0)*factor1_1;
    product3=(u64)(factor0_1)*factor1_1;
    a.b=p.b+product0;
    carry=(a.b<product0);
    a.b+=q.b;
    carry=(u8)(carry+(a.b<q.b));
    a.b>>=U32_BITS;
    a.b+=product1+((u64)(carry)<<U32_BITS);
    carry=(a.b<product1);
    a.b+=product2;
    carry=(u8)(carry+(a.b<product2));
    a.b>>=U32_BITS;
    a.b+=product3+((u64)(carry)<<U32_BITS);
  #endif
  *a_base=a;
  return;
}

void
fracterval_u64_multiply_fractoid_u64(fru64 *a_base, fru64 p, u64 q){
/*
Use FRU64_MULTIPLY_FTD64() instead of calling here directly.

Multiply a fracterval by a fractoid.

In:

  *a_base is undefined.

  p is the fracterval.

  q is the fractoid.

Out:

  *a_base is (p*q).
*/
  fru64 a;
  #ifdef _64_
    u128 product0;
    u128 product1;
  #else
    u8 carry;
    u32 factor0_0;
    u32 factor0_1;
    u32 factor1_0;
    u32 factor1_1;
    u64 product0;
    u64 product1;
    u64 product2;
    u64 product3;
  #endif

  #ifdef _64_
    product0=(u128)(p.a)*q;
    product1=(u128)(p.b)*q;
    product1+=p.b;
    product1+=q;
    a.a=(u64)(product0>>64);
    a.b=(u64)(product1>>64);
  #else
    factor0_0=(u32)(p.a);
    factor0_1=(u32)(p.a>>U32_BITS);
    factor1_0=(u32)(q);
    factor1_1=(u32)(q>>U32_BITS);
    product0=(u64)(factor0_0)*factor1_0;
    product1=(u64)(factor0_1)*factor1_0;
    product2=(u64)(factor0_0)*factor1_1;
    product3=(u64)(factor0_1)*factor1_1;
    a.a=product0>>U32_BITS;
    a.a+=product1;
    a.a+=product2;
    carry=(a.a<product2);
    a.a>>=U32_BITS;
    a.a+=product3+((u64)(carry)<<U32_BITS);
    factor0_0=(u32)(p.b);
    factor0_1=(u32)(p.b>>U32_BITS);
    product0=(u64)(factor0_0)*factor1_0;
    product1=(u64)(factor0_1)*factor1_0;
    product2=(u64)(factor0_0)*factor1_1;
    product3=(u64)(factor0_1)*factor1_1;
    a.b=p.b+product0;
    carry=(a.b<product0);
    a.b+=q;
    carry=(u8)(carry+(a.b<q));
    a.b>>=U32_BITS;
    a.b+=product1+((u64)(carry)<<U32_BITS);
    carry=(a.b<product1);
    a.b+=product2;
    carry=(u8)(carry+(a.b<product2));
    a.b>>=U32_BITS;
    a.b+=product3+((u64)(carry)<<U32_BITS);
  #endif
  *a_base=a;
  return;
}

void
fracterval_u64_multiply_mantissa_u64(fru64 *a_base, fru64 p, u64 q){
/*
Use FRU64_MULTIPLY_MANTISSA_U64() instead of calling here directly.

Multiply a fracterval by a mantissa.

In:

  *a_base is undefined.

  p is the fracterval.

  q is the mantissa. Unlike a fractoid which has an uncertainty of one ULP, a mantissa is assumed to have no uncertainty.

Out:

  *a_base is (p*q) expressed as a fracterval.
*/
  fru64 a;
  #ifdef _64_
    u128 product0;
    u128 product1;
  #else
    u8 carry;
    u32 factor0_0;
    u32 factor0_1;
    u32 factor1_0;
    u32 factor1_1;
    u32 product_lo_or;
    u64 product0;
    u64 product1;
    u64 product2;
    u64 product3;
  #endif

  #ifdef _64_
    product0=(u128)(p.a)*q;
    product1=(u128)(p.b)*q;
    product1+=q;
    a.a=(u64)(product0>>64);
    a.b=(u64)(product1>>64)-((!(u64)(product1))&&q);
  #else
    factor0_0=(u32)(p.a);
    factor0_1=(u32)(p.a>>U32_BITS);
    factor1_0=(u32)(q);
    factor1_1=(u32)(q>>U32_BITS);
    product0=(u64)(factor0_0)*factor1_0;
    product1=(u64)(factor0_1)*factor1_0;
    product2=(u64)(factor0_0)*factor1_1;
    product3=(u64)(factor0_1)*factor1_1;
    a.a=product0>>U32_BITS;
    a.a+=product1;
    a.a+=product2;
    carry=(a.a<product2);
    a.a>>=U32_BITS;
    a.a+=product3+((u64)(carry)<<U32_BITS);
    factor0_0=(u32)(p.b);
    factor0_1=(u32)(p.b>>U32_BITS);
    product0=(u64)(factor0_0)*factor1_0;
    product1=(u64)(factor0_1)*factor1_0;
    product2=(u64)(factor0_0)*factor1_1;
    product3=(u64)(factor0_1)*factor1_1;
    a.b=product0+q;
    carry=(a.b<product0);
    product_lo_or=(u32)(a.b);
    a.b>>=U32_BITS;
    a.b+=product1+((u64)(carry)<<U32_BITS);
    carry=(a.b<product1);
    a.b+=product2;
    carry=(u8)(carry+(a.b<product2));
    product_lo_or|=(u32)(a.b);
    a.b>>=U32_BITS;
    a.b+=product3+((u64)(carry)<<U32_BITS)-((!product_lo_or)&&q);
  #endif
  *a_base=a;
  return;
}

u8
fracterval_u64_multiply_u64(fru64 *a_base, fru64 p, u64 v){
/*
Use FRU64_MULTIPLY_U64() instead of calling here directly.

Multiply a fracterval by a u64.

In:

  *a_base is undefined.

  p is the fracterval.

  v is the u64 by which to multiply the fracterval.

Out:

  Returns one if ((p.b+1)*v) exceeds one, else zero.

  *a_base is (p*v) expressed as a fracterval.
*/
  fru64 a;
  u64 a_old;
  u8 carry;
  u32 factor0_0;
  u32 factor0_1;
  u32 factor1_0;
  u32 factor1_1;
  u64 product0;
  u64 product1;
  u64 product2;
  u64 product3;
  u8 status;

  factor0_0=(u32)(p.b);
  factor0_1=(u32)(p.b>>U32_BITS);
  factor1_0=(u32)(v);
  factor1_1=(u32)(v>>U32_BITS);
  product0=(u64)(factor0_0)*factor1_0;
  product1=(u64)(factor0_1)*factor1_0;
  product2=(u64)(factor0_0)*factor1_1;
  product3=(u64)(factor0_1)*factor1_1;
  status=0;
  a.b=product0+v;
  carry=(a.b<v);
  a_old=a.b;
  a.b+=product1<<U32_BITS;
  carry=(u8)(carry+(a.b<a_old));
  a_old=a.b;
  a.b+=product2<<U32_BITS;
  carry=(u8)(carry+(a.b<a_old));
  if((product1>>U32_BITS)|(product2>>U32_BITS)|product3){
    a.b=U64_MAX;
    status=1;    
  }else{
    if(a.b&&carry){
      a.b=0;
      status=1;
    }
    a.b-=!!v;
  }
  a.a=p.a*v;
  if(status){
    if((a.a/v)!=p.a){
      a.a=a.b;
    }
  }
  *a_base=a;
  return status;
}

void
fracterval_u64_nats_from_bits(fru64 *a_base, fru64 p){
/*
Use FRU64_NATS_FROM_BITS() instead of calling here directly.

Convert a fracterval representing a number of bits (log2 units) to nats (natural log units).

In:

  *a_base is undefined.

  p has units of nats.

Out:

  *a_base is (p*log(2)), which has units of bits.
*/
  fru64 a;

  FRU64_MULTIPLY_FTD64(a, p, FRU64_LOG2_FLOOR);
  *a_base=a;
  return;
}

u8
fracterval_u64_nats_to_bits(fru64 *a_base, fru64 p){
/*
Use FRU64_NATS_TO_BITS() instead of calling here directly.

Convert a fracterval representing a number of nats (natural log units) to bits (log2 units).

In:

  *a_base is undefined.

  p has units of bits.

Out:

  Returns one on fracterval saturation, else zero.

  *a_base is (p/log(2)), which has units of nats.
*/
  fru64 a;
  fru64 log2;
  u8 status;

  log2.a=FRU64_LOG2_FLOOR;
  log2.b=log2.a;
  status=0;
  FRU64_DIVIDE_FRU64(a, p, log2, status);
  *a_base=a;
  return status;
}

u8
fracterval_u64_shift_left(fru64 *a_base, u8 b, fru64 p){
/*
Use FRU64_SHIFT_LEFT() instead of calling here directly.

Multiply a fracterval by a power of 2.

In:

  *a_base is undefined.

  b is the power of 2 by which to multiply the fracterval, on [0, U64_BIT_MAX].

  p is the fracterval.

Out:

  Returns one if ((p.b+1)*(2^b)) exceeds one, else zero.

  *a_base is (p*(2^b)) expressed as a fracterval.
*/
  fru64 a;
  u8 status;

  a.a=p.a<<b;
  status=((a.a>>b)!=p.a);
  if(!status){
    a.b=p.b<<b;
    status=((a.b>>b)!=p.b);
    if(!status){
      a.b=p.b+1;
      a.b<<=b;
      a.b--;
    }else{
      a.b=U64_MAX;
    }
  }else{
    a.a=U64_MAX;
    a.b=a.a;
  }
  *a_base=a;
  return status;
}

u8
fractoid_u64_ratio_u64_saturate(u64 *a_base, u64 v, u64 w){
/*
Use FTD64_RATIO_U64_SATURATE() instead of calling here directly.

Set a u64 fractoid to the ratio of 2 (u64)s.

In:

  *a_base is undefined.

  v is the numerator.

  w is the denominator.

Out:

  Returns one if (v/w) exceeds one, or w is zero; else zero.

  *a_base is the minimum value of {v/w} which includes the exact value (v/w), saturating to ones.
*/
  u64 a;
  #ifdef _64_
    u128 dividend_u128;
  #else
    u64 dividend;
    u64 dividend0;
    u64 dividend1;
    u8 divisor_shift;
    u32 divisor_u32;
    u64 divisor_u64;
    u64 product;
    u64 product_shifted;
    u32 quotient;
  #endif
  u8 status;

  status=0;
  #ifdef _64_
    if(v<w){
      dividend_u128=(u128)(v)<<U64_BITS;
      a=(u64)(dividend_u128/w);
      if(!(dividend_u128%w)){
        a-=!!a;
      }
    }else{
      a=U64_MAX;
      status=(w!=v)||!w;
    }
  #else   
    dividend=v;
    divisor_u64=w;
    if(v<w){
      divisor_shift=0;
      while(!(divisor_u64>>(U64_BITS-U8_BITS-divisor_shift))){
        divisor_shift=(u8)(divisor_shift+8);
      }
      dividend0=0;
      dividend1=dividend<<divisor_shift;
      dividend=dividend1;
      divisor_u64<<=divisor_shift;
      divisor_u32=(u32)(divisor_u64>>U32_BITS);
      divisor_u32++;
      quotient=(u32)(dividend>>U32_BITS);
      if(divisor_u32){
        quotient=(u32)(dividend/divisor_u32);
      }
      a=(u64)(quotient)<<U32_BITS;
      product=(u64)((u32)(divisor_u64))*quotient;
      product_shifted=product<<U32_BITS;
      dividend1-=(dividend0<product_shifted);
      dividend0-=product_shifted;
      product_shifted=product>>(U64_BITS-U32_BITS);
      dividend1-=product_shifted;
      product=(divisor_u64>>U32_BITS)*quotient;
      dividend1-=product;
      dividend=(dividend0>>(U64_BITS-22))|(dividend1<<22);
      quotient=(u32)(dividend>>U32_BITS);
      if(divisor_u32){
        quotient=(u32)(dividend/divisor_u32);
      }
      a+=(u64)(quotient)<<(U32_BITS-22);
      product=(u64)((u32)(divisor_u64))*quotient;
      product_shifted=product<<(U32_BITS-22);
      dividend1-=(dividend0<product_shifted);
      dividend0-=product_shifted;
      product_shifted=product>>(U64_BITS-(U32_BITS-22));
      dividend1-=product_shifted;
      product=(divisor_u64>>U32_BITS)*quotient;
      product_shifted=product<<(U64_BITS-22);
      dividend1-=(dividend0<product_shifted);
      dividend0-=product_shifted;
      product_shifted=product>>22;
      dividend1-=product_shifted;
      dividend=(dividend0>>(U64_BITS-(22+22)))|(dividend1<<(22+22));
      quotient=(u32)(dividend>>U32_BITS);
      if(divisor_u32){
        quotient=(u32)(dividend/divisor_u32);
      }
      quotient>>=22+22-32;
      a+=(u64)(quotient);
      product=(u64)((u32)(divisor_u64))*quotient;
      dividend1-=(dividend0<product);
      dividend0-=product;
      product=(divisor_u64>>U32_BITS)*quotient;
      product_shifted=product<<U32_BITS;
      dividend1-=(dividend0<product_shifted);
      dividend0-=product_shifted;
      product_shifted=product>>(U64_BITS-U32_BITS);
      dividend1-=product_shifted;
      if((divisor_u64<=dividend0)||dividend1){
        a++;
        dividend0-=divisor_u64;
      }
      if(!dividend0){
        a-=!!a;
      }
    }else{
      a=U64_MAX;
      status=(w!=v)||!w;
    }
  #endif
  *a_base=a;
  return status;
}

u8
fractoid_u64_reciprocal_u64_saturate(u64 *a_base, u64 v){
/*
Use FTD64_RECIPROCAL_U64_SATURATE() instead of calling here directly.

Set a u64 fractoid to the reciprocal of a u64.

In:

  *a_base is undefined.

  v is the u64 whose reciprocal to compute.

Out:

  Returns one if v is zero, else zero.

  *a_base is the minimum value of {1/v} which includes the exact value (1/v), saturating to ones.
*/
  u64 a;
  u8 status;

  a=U64_MAX;
  status=!v;
  if(1<v){
    a/=v;
  }
  *a_base=a;
  return status;
}

u64
fractoid_u64_scale_u64(u64 v, u64 w){
/*
Use FTD64_SCALE_U64() instead of calling here directly.

Set a u64 fractoid to the product of 2 u64 mantissas.

In:

  *a_base is undefined.

  v is a mantissa factor.

  w is the other mantissa factor.

Out:

  Returns the minimum value of {v*w} which includes the exact value (v*w).
*/
  u128 a;
  #ifdef _32_
    u64 a_old;
    u64 product0;
  #endif
  u64 product1;

  #ifdef _64_
    a=(u128)(v)*w;
    product1=(u64)(a>>64);
    if((!(u64)(a))&&product1){
      product1--;
    }
  #else
    a.a=(u64)((u32)(v))*(u32)(w);
    a.b=(u64)((u32)(v>>U32_BITS))*(u32)(w>>U32_BITS);
    product0=(u64)((u32)(v>>U32_BITS))*(u32)(w);
    product1=(u64)((u32)(v))*(u32)(w>>U32_BITS);
    a_old=a.a;
    a.a+=product0<<U32_BITS;
    a.b+=product0>>U32_BITS;
    a.b+=(a.a<a_old);
    a_old=a.a;
    a.a+=product1<<U32_BITS;
    a.b+=product1>>U32_BITS;
    a.b+=(a.a<a_old);
    if((!a.a)&&a.b){
      a.b--;
    }
    product1=a.b;
  #endif
  return product1;
}
