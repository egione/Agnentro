/*
Loggamma
Copyright 2017 Russell Leidich

This collection of files constitutes the Loggamma Library. (This is a
library in the abstact sense; it's not intended to compile to a ".lib"
file.)

The Loggamma Library is free software: you can redistribute it and/or
modify it under the terms of the GNU Limited General Public License as
published by the Free Software Foundation, version 3.

The Loggamma Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
Limited General Public License version 3 for more details.

You should have received a copy of the GNU Limited General Public
License version 3 along with the Loggamma Library (filename
"COPYING"). If not, see http://www.gnu.org/licenses/ .
*/
/*
Loggamma Fracterval Kernel
*/
#include "flag.h"
#include "flag_biguint.h"
#include "flag_fracterval_u128.h"
#include "flag_loggamma.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "constant.h"
#include "debug.h"
#include "debug_xtrn.h"
#include "biguint.h"
#include "biguint_xtrn.h"
#include "fracterval_u128.h"
#include "fracterval_u128_xtrn.h"
#include "loggamma.h"
#include "loggamma_xtrn.h"
/*
loggamma_hex_list_base_base contains (LOGGAMMA_COEFF_IDX_MAX+1) coefficients, each consisting of a numerator followed by a denominator. They were generated via WolframAlpha using via queries of the following form, from the coefficients of Stirling's convergent series (as provided in http://vixra.org/abs/1609.0210):

  Sum[Sum[b*Abs[StirlingS1[a,b]]/((b+1)(b+2)),{b,1,a}]/(2a),{a,5,5}]

where the pair of "5"s in this case refer to coefficient number 5; replace them with any pair of equal natural numbers. The result is (533/280), which is (0x215/0x118), matching the 5th line below.
*/
const char *loggamma_hex_base_list_base[(LOGGAMMA_COEFF_IDX_MAX+1)<<1]={ \
  "1", "C", \
  "1", "C", \
  "3B", "168", \
  "1D", "3C", \
  "215", "118", \
  "629", "A8", \
  "44729", "13B0", \
  "10EBF", "B4", \
  "228C5D7", "2E68", \
  "6D885F", "108", \
  "F14684CF3", "3AA70", \
  "77AFDD76B", "2AA8", \
  "6AED9FC9A9", "3330", \
  "4BDA889841", "2D0", \
  "835FE327730B1", "59A60", \
  "105282780B23F", "BF4", \
  "10DCDFA48528926B", "C6B88", \
  "59726B8210E0431", "3E58", \
  "19CF34F57B0124E603", "1012B0", \
  "CD763D946BC4F1D95", "6C48", \
  "89CC2DE758115E55C05D", "3A5ED0", \
  "C7890473AB7968A5929", "40B0", \
  "143B1CA4B594F4937BDBF03", "4CA5E0", \
  "F1C581C572415B873C8F9D", "27FD8", \
  "87755DA66357B124D0F8D2FF", "EFF10", \
  "A615554D9A592A1027EEED7", "BD0", \
  "391D2C0E07E50785B166E90D5F7", "2824E0", \
};

void *
loggamma_free(void *base){
/*
To maximize portability and debuggability, this is the only function in which Loggamma calls free().

In:

  base is the base of a memory region to free. May be NULL.

Out:

  Returns NULL so that the caller can easily maintain the good practice of NULLing out invalid pointers.

  *base is freed.
*/
  DEBUG_FREE_PARANOID(base);
  return NULL;
}

loggamma_t *
loggamma_free_all(loggamma_t *loggamma_base){
/*
Free all Loggamma private data structures.

In:

  loggamma_base is the return value of loggamma_init().

Out:

  Returns NULL so that the caller can easily maintain the good practice of NULLing out invalid pointers.

  *loggamma_base is freed.
*/
  ULONG **coeff_base_list_base;
  ULONG coeff_idx;

  if(loggamma_base){
    biguint_free(loggamma_base->temp_chunk_list_base3);
    biguint_free(loggamma_base->temp_chunk_list_base2);
    biguint_free(loggamma_base->temp_chunk_list_base1);
    biguint_free(loggamma_base->temp_chunk_list_base0);
    coeff_base_list_base=loggamma_base->coeff_base_list_base;
    if(coeff_base_list_base){
/*
Free all coefficient numerators and denominators.
*/
      coeff_idx=(LOGGAMMA_COEFF_IDX_MAX<<1)+1;
      do{
        biguint_free(coeff_base_list_base[coeff_idx]);
      }while(coeff_idx--);
      DEBUG_FREE_PARANOID(coeff_base_list_base);
    }
    biguint_free(loggamma_base->chunk_idx_max_list_base);
    DEBUG_FREE_PARANOID(loggamma_base);
  }
  return NULL;
}

loggamma_t *
loggamma_init(u32 build_break_count, u32 build_feature_count){
/*
Verify that the source code is sufficiently updated.

In:

  build_break_count is the caller's most recent knowledge of LOGGAMMA_BUILD_BREAK_COUNT, which will fail if the caller is unaware of all critical updates.

  build_feature_count is the caller's most recent knowledge of LOGGAMMA_BUILD_FEATURE_COUNT, which will fail if this library is not up to date with the caller's expectations.

Out:

  Returns NULL if (build_break_count!=LOGGAMMA_BUILD_BREAK_COUNT) or (build_feature_count>LOGGAMMA_BUILD_FEATURE_COUNT) or if we're out of memory. Otherwise, returns the base of a loggamma_t for use with other functions in this library and which must ultimately be freed via loggamma_free_all().
*/
  ULONG *chunk_idx_max_list_base;
  ULONG chunk_idx_max;
  ULONG chunk_idx_max_max;
  ULONG *chunk_list_base;
  ULONG **coeff_base_list_base;
  ULONG coeff_idx;
  ULONG digit_count;
  loggamma_t *loggamma_base;
  u8 status;
  ULONG *temp_chunk_list_base0;
  ULONG *temp_chunk_list_base1;
  ULONG *temp_chunk_list_base2;
  ULONG *temp_chunk_list_base3;

  loggamma_base=NULL;
  status=(u8)(build_break_count!=LOGGAMMA_BUILD_BREAK_COUNT);
  status=(u8)(status|(LOGGAMMA_BUILD_FEATURE_COUNT<build_feature_count));
  status=(u8)(status|biguint_init(0, 0));
  status=(u8)(status|fracterval_u128_init(2, 0));
  if(!status){
    loggamma_base=(loggamma_t *)(DEBUG_CALLOC_PARANOID(sizeof(loggamma_t)));
    if(loggamma_base){
      chunk_idx_max_list_base=biguint_malloc((LOGGAMMA_COEFF_IDX_MAX<<1)+1);
      loggamma_base->chunk_idx_max_list_base=chunk_idx_max_list_base;
      coeff_base_list_base=(ULONG **)(DEBUG_CALLOC_PARANOID((size_t)(((LOGGAMMA_COEFF_IDX_MAX+1)<<1)*sizeof(ULONG *))));
      loggamma_base->coeff_base_list_base=coeff_base_list_base;
      if(chunk_idx_max_list_base&&coeff_base_list_base){
        chunk_idx_max_max=0;
        coeff_idx=0;
        do{
          digit_count=(ULONG)(strlen(loggamma_hex_base_list_base[coeff_idx]));
          chunk_idx_max=(digit_count-1)>>(ULONG_SIZE_LOG2+1);
          chunk_idx_max_list_base[coeff_idx]=chunk_idx_max;
          chunk_list_base=biguint_malloc(chunk_idx_max);
          status=(u8)(status|!chunk_list_base);
          coeff_base_list_base[coeff_idx]=chunk_list_base;
          if(!status){
            status=biguint_from_ascii_hex(&chunk_idx_max, chunk_idx_max, chunk_list_base, loggamma_hex_base_list_base[coeff_idx]);
            chunk_idx_max_max=MAX(chunk_idx_max, chunk_idx_max_max);
          }
        }while((coeff_idx++)!=((LOGGAMMA_COEFF_IDX_MAX<<1)+1));
        if(!status){
/*
Allocate enough temporary space to store the largest coefficient plus 64 bits of fractoid plus (LOGGAMMA_COEFF_IDX_MAX+1) extra (u64)s to compute the Pochhammer product in the denominator. We need 4 of these, one each for the numerator, denominator, Pochhammer product, and lower fracterval bound.
*/
          chunk_idx_max_max+=(U64_BITS>>ULONG_BITS_LOG2)+(((LOGGAMMA_COEFF_IDX_MAX+1)<<U64_SIZE_LOG2)>>ULONG_SIZE_LOG2);
          temp_chunk_list_base0=biguint_malloc(chunk_idx_max_max);
          loggamma_base->temp_chunk_list_base0=temp_chunk_list_base0;
          temp_chunk_list_base1=biguint_malloc(chunk_idx_max_max);
          loggamma_base->temp_chunk_list_base1=temp_chunk_list_base1;
          temp_chunk_list_base2=biguint_malloc(chunk_idx_max_max);
          loggamma_base->temp_chunk_list_base2=temp_chunk_list_base2;
          temp_chunk_list_base3=biguint_malloc(chunk_idx_max_max);
          loggamma_base->temp_chunk_list_base3=temp_chunk_list_base3;
          status=!(temp_chunk_list_base0&&temp_chunk_list_base1&&temp_chunk_list_base2&&temp_chunk_list_base3);
        }
      }else{
        status=1;
      }
      if(status){
        loggamma_base=loggamma_free_all(loggamma_base);
      }
    }
  }
  return loggamma_base;
}

u8
loggamma_u64(fru128 *a_base, loggamma_t *loggamma_base, u64 v){
/*
Use LOGGAMMA_U64() instead of calling here directly.

Compute the loggamma of a u64 as a 64.64 fracterval, according to the method described in (http://vixra.org/abs/1609.0210) with precomputed results for values up to 0x1F.

In:

  *a_base is undefined.

  loggamma_base is the return value of loggamma_init().

  v is the u64 whose loggamma to compute, on [1, LOGGAMMA_PARAMETER_MAX]. The idea is that the loggamma is only computed on allocated ULONG counts, which are at most (1ULL<<(U64_BITS-U64_BITS_LOG2)) because they are required by biguint_malloc() to be bitwise addressable with a u64 index. LOGGAMMA_PARAMETER_MAX is greater than this value, so this function is meaningful and safe for all such counts.

Out:

  Returns zero if v was is in the required domain, else one.

  *a_base is a fracterval for loggamma(v) if v is in the required domain, the one fracterval if v was zero, or the ambiguous fracterval otherwise. Values of v on [0, 0x1F] are precomputed as fractoids (with an implicit error of one ULP). This provides some acceleration because the loggamma actually takes a lot longer for these small values.
*/
  ULONG *chunk_idx_max_list_base;
  ULONG **coeff_base_list_base;
  ULONG coeff_idx;
  ULONG denominator_chunk_idx_max;
  ULONG *denominator_chunk_list_base;
  ULONG diameter_minus_1_in_ulps;
  u64 factor;
  ULONG fracterval_chunk_idx_max;
  ULONG *fracterval_chunk_list_base;
  u128 log_2pi_half;
  fru128 log_half;
  fru128 log_product;
  fru128 log;
  fru128 loggamma;
  ULONG numerator_chunk_idx_max;
  ULONG *numerator_chunk_list_base;
  ULONG pochhammer_chunk_idx_max;
  ULONG *pochhammer_chunk_list_base;
  u8 status;
  ULONG term_count;
  u128 v_u128;

  status=0;
  if((0x1F<v)&&(v<=LOGGAMMA_PARAMETER_MAX)){
    chunk_idx_max_list_base=loggamma_base->chunk_idx_max_list_base;
    coeff_base_list_base=loggamma_base->coeff_base_list_base;
    denominator_chunk_list_base=loggamma_base->temp_chunk_list_base0;
    fracterval_chunk_list_base=loggamma_base->temp_chunk_list_base1;
    numerator_chunk_list_base=loggamma_base->temp_chunk_list_base2;
    pochhammer_chunk_list_base=loggamma_base->temp_chunk_list_base3;
    coeff_idx=0;
    factor=v;
    BIGUINT_SET_ZERO(fracterval_chunk_idx_max, fracterval_chunk_list_base);
    BIGUINT_SET_ULONG(pochhammer_chunk_idx_max, pochhammer_chunk_list_base, 1);
    do{
      numerator_chunk_idx_max=chunk_idx_max_list_base[coeff_idx];
      biguint_copy(numerator_chunk_idx_max, numerator_chunk_list_base, coeff_base_list_base[coeff_idx]);
      coeff_idx++;
      denominator_chunk_idx_max=chunk_idx_max_list_base[coeff_idx];
      biguint_copy(denominator_chunk_idx_max, denominator_chunk_list_base, coeff_base_list_base[coeff_idx]);
      coeff_idx++;
      factor++;
      pochhammer_chunk_idx_max=biguint_multiply_u64(pochhammer_chunk_idx_max, pochhammer_chunk_list_base, factor);
      denominator_chunk_idx_max=biguint_multiply_biguint(denominator_chunk_idx_max, pochhammer_chunk_idx_max, denominator_chunk_list_base, pochhammer_chunk_list_base);
      numerator_chunk_idx_max=biguint_shift_left(U64_BITS, numerator_chunk_idx_max, numerator_chunk_list_base);
      biguint_divide_biguint(&numerator_chunk_idx_max, denominator_chunk_idx_max, numerator_chunk_list_base, denominator_chunk_list_base, numerator_chunk_list_base);
      fracterval_chunk_idx_max=biguint_add_biguint(fracterval_chunk_idx_max, numerator_chunk_idx_max, fracterval_chunk_list_base, numerator_chunk_list_base);
    }while(numerator_chunk_list_base[0]||numerator_chunk_idx_max);
    loggamma.a=biguint_to_u128_wrap(fracterval_chunk_idx_max, fracterval_chunk_list_base);
    term_count=coeff_idx>>1;
/*
The last term registed as zero, meaning that it's something less than one ULP. Per the whitepaper cited above, the maximum possible error is thus less than (term_count/v) ULPs. We reflect this fact by setting (loggamma.b=loggamma.a+ceil(term_count/v)-1). However, we never accounted for the error due to the terms already computed. So actually we need to set loggamma.b=loggamma.a+ceil(term_count/v)+term_count-1). (-1) reflects the fact that fracterval upper bounds are understated by a value on (0, 1] ULP.
*/
    diameter_minus_1_in_ulps=(ULONG)((term_count/v)-!(term_count%v)+term_count-1);
    U128_ADD_U64_LO(loggamma.b, loggamma.a, diameter_minus_1_in_ulps);
    FRU128_LOG_U64(log, v, status);
    FRU128_SHIFT_RIGHT_SELF(log, U128_BITS-U64_BITS_LOG2-U64_BITS);
    FRU128_MULTIPLY_U64(log_product, log, v, status);
    FRU128_ADD_FRU128_SELF(loggamma, log_product, status);
    FRU128_SHIFT_RIGHT(log_half, 1, log);
    FRU128_SUBTRACT_FRU128_SELF(loggamma, log_half, status);
    U128_FROM_U64_HI(v_u128, v);
    FRU128_SUBTRACT_FTD128_SELF(loggamma, v_u128, status);
    U128_FROM_U64_LO(log_2pi_half, LOGGAMMA_LOG_2PI_HALF);
    FRU128_ADD_FTD128_SELF(loggamma, log_2pi_half, status);
  }else{
/*
Load a precomputed result. You can expand this switch statement to include more values, but do not reduce it below (v==0x1F). Otherwise the loop above will walk off the end of its memory allocation!
*/
    switch(v){
    case 0:
      U128_SET_ONES(loggamma.a);
      status=1;
      break;
    case 1:
    case 2:
      U128_SET_ZERO(loggamma.a);
      break;
    case 3:
      U128_FROM_U64_PAIR(loggamma.a, LOGGAMMA_03_LO, LOGGAMMA_03_HI);
      break;
    case 4:
      U128_FROM_U64_PAIR(loggamma.a, LOGGAMMA_04_LO, LOGGAMMA_04_HI);
      break;
    case 5:
      U128_FROM_U64_PAIR(loggamma.a, LOGGAMMA_05_LO, LOGGAMMA_05_HI);
      break;
    case 6:
      U128_FROM_U64_PAIR(loggamma.a, LOGGAMMA_06_LO, LOGGAMMA_06_HI);
      break;
    case 7:
      U128_FROM_U64_PAIR(loggamma.a, LOGGAMMA_07_LO, LOGGAMMA_07_HI);
      break;
    case 8:
      U128_FROM_U64_PAIR(loggamma.a, LOGGAMMA_08_LO, LOGGAMMA_08_HI);
      break;
    case 9:
      U128_FROM_U64_PAIR(loggamma.a, LOGGAMMA_09_LO, LOGGAMMA_09_HI);
      break;
    case 0xA:
      U128_FROM_U64_PAIR(loggamma.a, LOGGAMMA_0A_LO, LOGGAMMA_0A_HI);
      break;
    case 0xB:
      U128_FROM_U64_PAIR(loggamma.a, LOGGAMMA_0B_LO, LOGGAMMA_0B_HI);
      break;
    case 0xC:
      U128_FROM_U64_PAIR(loggamma.a, LOGGAMMA_0C_LO, LOGGAMMA_0C_HI);
      break;
    case 0xD:
      U128_FROM_U64_PAIR(loggamma.a, LOGGAMMA_0D_LO, LOGGAMMA_0D_HI);
      break;
    case 0xE:
      U128_FROM_U64_PAIR(loggamma.a, LOGGAMMA_0E_LO, LOGGAMMA_0E_HI);
      break;
    case 0xF:
      U128_FROM_U64_PAIR(loggamma.a, LOGGAMMA_0F_LO, LOGGAMMA_0F_HI);
      break;
    case 0x10:
      U128_FROM_U64_PAIR(loggamma.a, LOGGAMMA_10_LO, LOGGAMMA_10_HI);
      break;
    case 0x11:
      U128_FROM_U64_PAIR(loggamma.a, LOGGAMMA_11_LO, LOGGAMMA_11_HI);
      break;
    case 0x12:
      U128_FROM_U64_PAIR(loggamma.a, LOGGAMMA_12_LO, LOGGAMMA_12_HI);
      break;
    case 0x13:
      U128_FROM_U64_PAIR(loggamma.a, LOGGAMMA_13_LO, LOGGAMMA_13_HI);
      break;
    case 0x14:
      U128_FROM_U64_PAIR(loggamma.a, LOGGAMMA_14_LO, LOGGAMMA_14_HI);
      break;
    case 0x15:
      U128_FROM_U64_PAIR(loggamma.a, LOGGAMMA_15_LO, LOGGAMMA_15_HI);
      break;
    case 0x16:
      U128_FROM_U64_PAIR(loggamma.a, LOGGAMMA_16_LO, LOGGAMMA_16_HI);
      break;
    case 0x17:
      U128_FROM_U64_PAIR(loggamma.a, LOGGAMMA_17_LO, LOGGAMMA_17_HI);
      break;
    case 0x18:
      U128_FROM_U64_PAIR(loggamma.a, LOGGAMMA_18_LO, LOGGAMMA_18_HI);
      break;
    case 0x19:
      U128_FROM_U64_PAIR(loggamma.a, LOGGAMMA_19_LO, LOGGAMMA_19_HI);
      break;
    case 0x1A:
      U128_FROM_U64_PAIR(loggamma.a, LOGGAMMA_1A_LO, LOGGAMMA_1A_HI);
      break;
    case 0x1B:
      U128_FROM_U64_PAIR(loggamma.a, LOGGAMMA_1B_LO, LOGGAMMA_1B_HI);
      break;
    case 0x1C:
      U128_FROM_U64_PAIR(loggamma.a, LOGGAMMA_1C_LO, LOGGAMMA_1C_HI);
      break;
    case 0x1D:
      U128_FROM_U64_PAIR(loggamma.a, LOGGAMMA_1D_LO, LOGGAMMA_1D_HI);
      break;
    case 0x1E:
      U128_FROM_U64_PAIR(loggamma.a, LOGGAMMA_1E_LO, LOGGAMMA_1E_HI);
      break;
    case 0x1F:
      U128_FROM_U64_PAIR(loggamma.a, LOGGAMMA_1F_LO, LOGGAMMA_1F_HI);
      break;
    default:
      FRU128_SET_AMBIGUOUS(loggamma);
      status=1;
    }
    if(!status){
      loggamma.b=loggamma.a;
    }
  }
  *a_base=loggamma;
  return status;
}

u64 *
loggamma_u64_cache_init(ULONG loggamma_idx_max, fru128 **loggamma_list_base_base){
/*
Allocate a cache for saving previous loggamma_u64() results.

In:

  loggamma_idx_max is one less than the number of items to allocate in the cache. It must be one less than a power of 2.

Out:

  Returns NULL on failure, else the base of (loggamma_idx_max+1) u64 zeroes, all but the first of which indicating that the corresponding cache entry is undefined.

  *loggamma_list_base_base is the base of a list containing (loggamma_idx_max+1) undefined (fru128)s, except for the first one, which corresponds to the loggamma of zero, and is therefore saturated to one. The list must be freed via loggamma_free().
*/
  fru128 *loggamma_list_base;
  u64 *loggamma_parameter_list_base;
  fru128 one;

  loggamma_list_base=fracterval_u128_list_malloc(loggamma_idx_max);
  loggamma_parameter_list_base=fracterval_u128_u64_list_malloc(loggamma_idx_max);
  if(UINT_IS_POWER_OF_2_MINUS_1(loggamma_idx_max)&&loggamma_list_base&&loggamma_parameter_list_base){
    fracterval_u128_u64_list_zero(loggamma_idx_max, loggamma_parameter_list_base);
    FRU128_SET_ONES(one);
    loggamma_list_base[0]=one;
    *loggamma_list_base_base=loggamma_list_base;
  }else{
    loggamma_parameter_list_base=fracterval_u128_free(loggamma_parameter_list_base);
    fracterval_u128_free(loggamma_list_base);
  }
  return loggamma_parameter_list_base;
}

u8
loggamma_u64_cached(fru128 *a_base, loggamma_t *loggamma_base, ULONG loggamma_idx_max, fru128 *loggamma_list_base, u64 *loggamma_parameter_list_base, u64 v){
/*
Use FRU128_LOGGAMMA_U64_CACHED(), or FRU128_LOGGAMMA_U64_IN_DOMAIN_CACHED() if v is guaranteed to be on [1, LOGGAMMA_PARAMETER_MAX], instead of calling here directly.

Deliver cached results from loggamma_u64().

In:

  *a_base is undefined.

  loggamma_base is the return value of loggamma_init().

  loggamma_idx_max is loggamma_u64_cache_init():In:loggamma_idx_max.

  loggamma_list_base is loggamma_u64_cache_init():Out:*loggamma_list_base_base.

  loggamma_parameter_list_base is the return value of loggamma_u64_cache_init().

  v is as defined in loggamma_u64().

Out:

  Returns as defined for loggamma_u64(a_base, v).

  *a_base is as defined in loggamma_u64().

  *loggamma_list_base contains *a_base at index (loggamma_idx_max&v).

  *loggamma_parameter_list_base contains v at index (loggamma_idx_max&v).
*/
  fru128 a;
  ULONG idx;
  u8 status;

  idx=loggamma_idx_max&(ULONG)(v);
  if(loggamma_parameter_list_base[idx]==v){
    a=loggamma_list_base[idx];
    status=(LOGGAMMA_PARAMETER_MAX<=(v-1U));
  }else{
    loggamma_parameter_list_base[idx]=v;
    status=loggamma_u64(&a, loggamma_base, v);
    loggamma_list_base[idx]=a;
  }
  *a_base=a;
  return status;
}
