/*
Agnentro
Copyright 2017 Russell Leidich
http://agnentropy.blogspot.com

This collection of files constitutes the Agnentro Library. (This is a
library in the abstact sense; it's not intended to compile to a ".lib"
file.)

The Agnentro Library is free software: you can redistribute it and/or
modify it under the terms of the GNU Limited General Public License as
published by the Free Software Foundation, version 3.

The Agnentro Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
Limited General Public License version 3 for more details.

You should have received a copy of the GNU Limited General Public
License version 3 along with the Agnentro Library (filename
"COPYING"). If not, see http://www.gnu.org/licenses/ .
*/
/*
Fixed-Point Entropy Approximation Kernel
*/
#include "flag.h"
#include "flag_fracterval_u128.h"
#include "flag_fracterval_u64.h"
#include "flag_biguint.h"
#include "flag_loggamma.h"
#include "flag_poissocache.h"
#include "flag_agnentroprox.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "constant.h"
#include "debug.h"
#include "debug_xtrn.h"
#include "fracterval_u64.h"
#include "fracterval_u64_xtrn.h"
#include "fracterval_u128.h"
#include "fracterval_u128_xtrn.h"
#include "loggamma.h"
#include "loggamma_xtrn.h"
#include "poissocache.h"
#include "poissocache_xtrn.h"
#include "agnentroprox.h"
#include "agnentroprox_xtrn.h"

fru128
agnentroprox_compressivity_get(fru128 entropy, fru128 entropy_raw){
/*
Convert agnentropy to compressivity, or diventropy to divcompressivity.

In:

  entropy is the agnentropy or diventropy to convert.

  entropy_raw is the raw entropy of the mask list, i.e. ((mask_idx_max+1)*log(mask_max+1)).

Out:

  Returns the corresponding compressivity or divcompressivity.
*/
  fru128 compressivity0;
  fru128 compressivity1;
  u8 overflow_status;
/*
If (entropy_raw<=entropy) then:

  (div)compressivity=entropy_raw/(2*(div)entropy)

else:

  (div)compressivity=1-(entropy/(2*(div)entropy_raw))

Overflows are safe to ignore because by definition (div)compressivity is on [0.0, 1.0], and result saturation does the right thing.
*/
  FRU128_SET_ZERO(compressivity0);
  overflow_status=1;
  if(U128_IS_LESS_EQUAL(entropy.a, entropy_raw.b)){
    overflow_status=0;
    FRU128_DIVIDE_FRU128(compressivity0, entropy, entropy_raw, overflow_status);
    FRU128_SHIFT_RIGHT_SELF(compressivity0, 1);
    FRU128_NOT_SELF(compressivity0);
  }
  if(overflow_status){
    overflow_status=0;
    FRU128_DIVIDE_FRU128(compressivity1, entropy_raw, entropy, overflow_status);
    FRU128_SHIFT_RIGHT_SELF(compressivity1, 1);
    compressivity0.a=compressivity1.a;
    if(!overflow_status){
      compressivity0.b=compressivity1.b;
    }
  }
  return compressivity0;
}

fru128
agnentroprox_diventropy_get(agnentroprox_t *agnentroprox_base, ULONG haystack_mask_idx_max, u8 *haystack_mask_list_base, u8 *overflow_status_base){
/*
Compute the diventropy of a preloaded needle frequency list with respect to a haystack frequency list.

In:

  The needle frequency list must have been preloaded with agnentroprox_needle_mask_list_load() prior to calling this function, with no intervening alterations.

  agnentroprox_base is the return value of agenentroprox_init().

  haystack_mask_idx_max is one less than the number of masks in the haystack, such that if agnentroprox_init():In:overlap_status was one, then this value would need to be increased in order to account for mask overlap. For example, if the sweep contains 5 of 3-byte masks, then this value would be 4 _without_ overlap, or 12 _with_ overlap. Must not exceed agnentroprox_init():In:mask_idx_max_max. See also agnentroprox_mask_idx_max_get().

  *haystack_mask_list_base is the haystack.

  *overflow_status_base is the OR-cummulative fracterval overflow status.

Out:

  Returns the diventropy of the needle with respect to the haystack.

  *overflow_status_base is one if a fracterval overflow occurred, else unchanged.
*/
  fru128 diventropy;
  fru128 diventropy_delta;
  ULONG freq_agnostic;
  ULONG haystack_freq;
  ULONG *haystack_freq_list_base;
  ULONG haystack_mask_count;
  ULONG haystack_mask_count_plus_span;
  fru64 log;
  ULONG log_idx_max;
  fru64 *log_list_base;
  u64 *log_parameter_list_base;
  u32 mask;
  u32 mask_max;
  ULONG mask_span;
  ULONG needle_freq;
  ULONG *needle_freq_list_base;
  ULONG needle_mask_count;
  u8 overflow_status;

  haystack_freq_list_base=agnentroprox_base->freq_list_base1;
  mask_max=agnentroprox_base->mask_max;
  overflow_status=*overflow_status_base;
  agnentroprox_ulong_list_zero((ULONG)(mask_max), haystack_freq_list_base);
  agnentroprox_base->mask_count1=0;
  agnentroprox_mask_list_accrue(agnentroprox_base, 1, haystack_mask_idx_max, haystack_mask_list_base);
  log_idx_max=agnentroprox_base->log_idx_max;
  log_list_base=agnentroprox_base->log_list_base;
  log_parameter_list_base=agnentroprox_base->log_parameter_list_base;
  needle_freq_list_base=agnentroprox_base->freq_list_base0;
  needle_mask_count=agnentroprox_base->mask_count0;
/*
Compute the diventropy, D:

  D=(Q0*log(Q1+Z))-Σ(M=(0, Z-1), F0[M]*log(F1[M]+1))

where:

  Q0=(needle mask count)
  Q1=(haystack mask count)
  F0[M]=(needle frequency of mask M)
  F1[M]=(haystack frequency of mask M)
  Z=(mask span)
*/
  FRU128_SET_ZERO(diventropy);
  haystack_mask_count=haystack_mask_idx_max+1;
  mask_span=(ULONG)(mask_max)+1;
  haystack_mask_count_plus_span=haystack_mask_count+mask_span;
  FRU64_LOG_U64_NONZERO_CACHED(log, log_idx_max, log_list_base, log_parameter_list_base, (u64)(haystack_mask_count_plus_span));
  FRU128_FROM_FRU64_MULTIPLY_U64(diventropy, log, (u64)(needle_mask_count));
  mask=0;
  do{
    haystack_freq=haystack_freq_list_base[mask];
    needle_freq=needle_freq_list_base[mask];
    if(haystack_freq){
      freq_agnostic=haystack_freq+1;
      FRU64_LOG_U64_NONZERO_CACHED(log, log_idx_max, log_list_base, log_parameter_list_base, (u64)(freq_agnostic));
      FRU128_FROM_FRU64_MULTIPLY_U64(diventropy_delta, log, (u64)(needle_freq));
      FRU128_SUBTRACT_FRU128_SELF(diventropy, diventropy_delta, overflow_status);
    }
  }while((mask++)!=mask_max);
/*
Convert D from 6.58 to 6.64 fixed point.
*/
  FRU128_SHIFT_LEFT_SELF(diventropy, 64-58, overflow_status);
  *overflow_status_base=overflow_status;
  return diventropy;
}

ULONG
agnentroprox_diventropy_transform(agnentroprox_t *agnentroprox_base, u8 append_mode, fru128 *diventropy_list_base, ULONG haystack_mask_idx_max, u8 *haystack_mask_list_base, ULONG match_idx_max_max, ULONG *match_u8_idx_list_base, u8 *overflow_status_base, ULONG sweep_mask_idx_max){
/*
Compute the diventropy transform of a haystack with respect to a preloaded needle frequency list, given a particular sweep.

In:

  The needle frequency list must have been preloaded with agnentroprox_needle_mask_list_load() prior to calling this function, with no intervening alterations.

  agnentroprox_base is the return value of agenentroprox_init().

  append_mode is 2 to append diventropies one at at time to *diventropy_list_base, ordered ascending by the base index of the sweep window. Else one to sort results ascending by diventropy, or zero to sort descending.

  *diventropy_list_base contains (match_idx_max_max+1) undefined items.

  haystack_mask_idx_max is one less than the number of masks in the haystack, such that if agnentroprox_init():In:overlap_status was one, then this value would need to be increased in order to account for mask overlap. For example, if the sweep contains 5 of 3-byte masks, then this value would be 4 _without_ overlap, or 12 _with_ overlap. Must not exceed agnentroprox_init():In:mask_idx_max_max. See also agnentroprox_mask_idx_max_get().

  *haystack_mask_list_base is the haystack.

  match_idx_max_max is one less than the maximum number of matches to report.

  *match_u8_idx_list_base is NULL if the sweep base indexes associated with matches can be discarded, else the base of (match_idx_max_max+1) undefined items to hold such indexes.

  *overflow_status_base is the OR-cummulative fracterval overflow status.

  sweep_mask_idx_max is one less than the number of masks in the sweep which, like haystack_mask_idx_max, must account for mask overlap if enabled.  On [0, haystack_mask_idx_max].

Out:

  Returns the number of matches found, which is simply (MIN((haystack_mask_idx_max-sweep_mask_idx_max))+1, match_idx_max_max).

  *diventropy_list_base contains (return value) items which represent the matches identified during the search, sorted according to append_mode.

  *overflow_status_base is one if a fracterval overflow occurred, else unchanged.
*/
  fru128 diventropy;
  fru128 diventropy_delta;
  u128 diventropy_mean;
  u128 diventropy_threshold;
  u8 granularity;
  ULONG haystack_freq;
  ULONG haystack_freq_agnostic;
  ULONG *haystack_freq_list_base;
  ULONG haystack_freq_old;
  fru64 log_delta;
  ULONG log_delta_idx_max;
  fru64 *log_delta_list_base;
  u64 *log_delta_parameter_list_base;
  u32 mask;
  u32 mask_old;
  u8 mask_u8;
  ULONG match_count;
  ULONG match_idx;
  ULONG match_idx_min;
  u8 match_status_not;
  ULONG needle_freq;
  ULONG *needle_freq_list_base;
  ULONG needle_freq_old;
  u8 overflow_status;
  u8 overlap_status;
  ULONG u8_idx;
  ULONG u8_idx_old;
  ULONG u8_idx_delta;
  ULONG u8_idx_max;

  overflow_status=*overflow_status_base;
  diventropy=agnentroprox_diventropy_get(agnentroprox_base, sweep_mask_idx_max, haystack_mask_list_base, &overflow_status);
  FRU128_MEAN_TO_FTD128(diventropy_mean, diventropy);
  match_count=1;
  if(match_u8_idx_list_base){
    match_u8_idx_list_base[0]=0;
  }
  diventropy_list_base[0]=diventropy;
  U128_FROM_BOOL(diventropy_threshold, append_mode);
  if(!match_idx_max_max){
    diventropy_threshold=diventropy_mean;
  }
  granularity=agnentroprox_base->granularity;
  haystack_freq_list_base=agnentroprox_base->freq_list_base1;
  log_delta_idx_max=agnentroprox_base->log_delta_idx_max;
  log_delta_list_base=agnentroprox_base->log_delta_list_base;
  log_delta_parameter_list_base=agnentroprox_base->log_delta_parameter_list_base;
  needle_freq_list_base=agnentroprox_base->freq_list_base0;
  overlap_status=agnentroprox_base->overlap_status;
  u8_idx_delta=(u8)((u8)(granularity*(!overlap_status))+1);
  u8_idx=(sweep_mask_idx_max+1)*u8_idx_delta;
  u8_idx_max=haystack_mask_idx_max*u8_idx_delta;
  u8_idx_old=0;
  while(u8_idx<=u8_idx_max){
    mask=haystack_mask_list_base[u8_idx];
    mask_old=haystack_mask_list_base[u8_idx_old];
    if(granularity){
      mask_u8=haystack_mask_list_base[u8_idx+U16_BYTE_MAX];
      mask|=(u32)(mask_u8)<<U8_BITS;
      mask_u8=haystack_mask_list_base[u8_idx_old+U16_BYTE_MAX];
      mask_old|=(u32)(mask_u8)<<U8_BITS;
      if(U16_BYTE_MAX<granularity){
        mask_u8=haystack_mask_list_base[u8_idx+U24_BYTE_MAX];
        mask|=(u32)(mask_u8)<<U16_BITS;
        mask_u8=haystack_mask_list_base[u8_idx_old+U24_BYTE_MAX];
        mask_old|=(u32)(mask_u8)<<U16_BITS;
        if(U24_BYTE_MAX<granularity){
          mask_u8=haystack_mask_list_base[u8_idx+U32_BYTE_MAX];
          mask|=(u32)(mask_u8)<<U24_BITS;
          mask_u8=haystack_mask_list_base[u8_idx_old+U32_BYTE_MAX];
          mask_old|=(u32)(mask_u8)<<U24_BITS;
        }
      }
    }
/*
Increment the u8 indexes now, so that if we get a match, u8_idx_old will be the first index of the new sweep, as opposed to the old one.
*/
    u8_idx+=u8_idx_delta;
    u8_idx_old+=u8_idx_delta;
    if(mask!=mask_old){
/*
Account for the diventropy difference, dD, due to the frequency difference in both the haystack and the needle between the new and old masks. This is derived from the formula for D itself, as computed in agnentroprox_diventropy_get():

  dD=((F0[mask_old]*(log(F1[mask_old]+1)-log(F1[mask_old])))-(F0[mask]*(log(F1[mask]+2)-log(F1[mask]+1))))
  dD=((F0[mask_old]*log_delta(F1[mask_old]))-(F0[mask]*log_delta(F1[mask]+1)))

where:

  Q0=(needle mask count)
  Q1=(sweep mask count)
  F0[M]=(needle frequency of mask M)
  F1[M]=(sweep frequency of mask M)

By the way, (64-58) in the shifts below reflect conversion from 6.58 to 6.64 fixed point.
*/
      haystack_freq=haystack_freq_list_base[mask];
      haystack_freq_old=haystack_freq_list_base[mask_old];
      needle_freq=needle_freq_list_base[mask];
      needle_freq_old=needle_freq_list_base[mask_old];
      haystack_freq_list_base[mask_old]=haystack_freq_old-1;
      FRU64_LOG_DELTA_U64_NONZERO_CACHED(log_delta, log_delta_idx_max, log_delta_list_base, log_delta_parameter_list_base, (u64)(haystack_freq_old));
      FRU128_FROM_FRU64_MULTIPLY_U64(diventropy_delta, log_delta, (u64)(needle_freq_old));
      FRU128_SHIFT_LEFT_SELF(diventropy_delta, 64-58, overflow_status);
      FRU128_ADD_FRU128_SELF(diventropy, diventropy_delta, overflow_status);
      haystack_freq_agnostic=haystack_freq+1;
      haystack_freq_list_base[mask]=haystack_freq_agnostic;
      FRU64_LOG_DELTA_U64_NONZERO_CACHED(log_delta, log_delta_idx_max, log_delta_list_base, log_delta_parameter_list_base, (u64)(haystack_freq_agnostic));
      FRU128_FROM_FRU64_MULTIPLY_U64(diventropy_delta, log_delta, (u64)(needle_freq));
      FRU128_SHIFT_LEFT_SELF(diventropy_delta, 64-58, overflow_status);
      FRU128_SUBTRACT_FRU128_SELF(diventropy, diventropy_delta, overflow_status);
      FRU128_MEAN_TO_FTD128(diventropy_mean, diventropy);
    }
    if(!append_mode){
      match_status_not=U128_IS_LESS(diventropy_mean, diventropy_threshold);
      if(!match_status_not){
        match_status_not=fracterval_u128_rank_list_insert_descending(diventropy, &match_count, &match_idx, match_idx_max_max, diventropy_list_base, &diventropy_threshold);
      }
    }else if(append_mode==1){
      match_status_not=U128_IS_LESS(diventropy_threshold, diventropy_mean);
      if(!match_status_not){
        match_status_not=fracterval_u128_rank_list_insert_ascending(diventropy, &match_count, &match_idx, match_idx_max_max, diventropy_list_base, &diventropy_threshold);
      }
    }else{
      match_idx=match_count;
      match_status_not=1;
      if(match_count<=match_idx_max_max){
        match_status_not=0;
        diventropy_list_base[match_count]=diventropy;
        match_count++;
      }
    }
    if((!match_status_not)&&match_u8_idx_list_base){
      match_idx_min=match_idx;
      match_idx=match_count-1;
      while(match_idx!=match_idx_min){
        match_u8_idx_list_base[match_idx]=match_u8_idx_list_base[match_idx-1];
        match_idx--;
      }
      match_u8_idx_list_base[match_idx]=u8_idx_old;
    }
  }
  *overflow_status_base=overflow_status;
  return match_count;
}

fru128
agnentroprox_dyspoissonism_get(fru128 entropy, fru128 entropy_raw){
/*
Convert logfreedom to dyspoissonism, or Shannon entropy to shannonism.

In:

  entropy is the logfreedom or Shannon entropy to convert.

  entropy_raw is the raw entropy of the mask list, i.e. ((mask_idx_max+1)*log(mask_max+1)).

Out:

  Returns the corresponding dyspoissonism or shannonism.
*/
  fru128 dyspoissonism;
  u8 overflow_status;
/*
Compute:

  (dyspoissonism or shannonism)=(1-(entropy/entropy_raw)).

Overflows are safe to ignore because by definition dyspoissonism and shannonism are on [0.0, 1.0], and result saturation does the right thing.
*/
  overflow_status=0;
  FRU128_DIVIDE_FRU128(dyspoissonism, entropy, entropy_raw, overflow_status);
  FRU128_NOT_SELF(dyspoissonism);
  return dyspoissonism;
}

fru128
agnentroprox_entropy_delta_get(agnentroprox_t *agnentroprox_base, ULONG mask_idx_max, u8 *mask_list_base, u16 mode, u8 new_status, u8 *overflow_status_base, u8 rollback_status){
/*
Compute the change in a particular type of entropy which would occur as the result of accruing a mask list to an existing cummulative frequency list.

In:

  agnentroprox_base is the return value of agenentroprox_init().

  mask_idx_max is one less than the number of masks in the mask list, such that if agnentroprox_init():In:overlap_status was one, then this value would need to be increased in order to account for mask overlap. For example, if the sweep contains 5 of 3-byte masks, then this value would be 4 _without_ overlap, or 12 _with_ overlap. Must not exceed agnentroprox_init():In:mask_idx_max_max. See also agnentroprox_mask_idx_max_get().

  *mask_list_base is the mask list.

  mode is AGNENTROPROX_MODE_AGNENTROPY, AGNENTROPROX_MODE_KURTOSIS, AGNENTROPROX_MODE_LOGFREEDOM, AGNENTROPROX_MODE_SHANNON, or AGNENTROPROX_MODE_VARIANCE to compute the agnentropy, obtuse kurtosis, logfreedom, Shannon, or obtuse variance entropy delta, respectively. For AGNENTROPROX_MODE_KURTOSIS and AGNENTROPROX_MODE_VARIANCE, the global mean must have been precomputed by agnentroprox_mask_list_mean_get().

  new_status is one to reset the cummulative frequency list, else zero. Thus it must be one on the first call. It must also be one if any other Agnentroprox functions have been called since the previous call to this one, as they may have destroyed the cummulative frequency list.

  *overflow_status_base is the OR-cummulative fracterval overflow status.

  rollback_status is one to reverse the accrual of masks from *mask_list_base after computing the entropy delta, else zero. Rollback could be used, for instance, to allow the caller to select the particular lossy data compression scheme which maximizes reconstruction quality per bit of data preserved. Having chosen one particular scheme, rollback would then be disabled in order to accrue its associated masks to the cummulative frequency list.

Out:

  Returns the entropy, of the requested type, implied by the cummulative frequency list after accruing all of the masks at mask_list_base, less the entropy implied by the cummulative frequency list before doing so -- in other words, the change in entropy associated with the accrual of *mask_list_base.

  If rollback_status is zero, then *mask_list_base has been accrued to the internal cummulative frequency list.

  *overflow_status_base is one if a fracterval overflow occurred, else unchanged.
*/
  u128 delta;
  fru128 delta_power;
  fru128 delta_power_product;
  fru128 entropy;
  fru128 entropy_delta;
  u8 fixed_point_shift;
  ULONG freq;
  ULONG *freq_list_base;
  u128 freq_mantissa;
  u8 freq_pop_cache_idx;
  ULONG freq_pop_idx_max;
  ULONG *freq_pop_list_base0;
  ULONG *freq_pop_list_base1;
  ULONG freq_pop_ulong_idx;
  loggamma_t *loggamma_base;
  ULONG loggamma_idx_max;
  fru128 *loggamma_list_base;
  u64 loggamma_parameter;
  u64 *loggamma_parameter_list_base;
  poissocache_t *poissocache_base;
  u32 mask;
  ULONG mask_count;
  u8 mask_idx_max_msb;
  u32 mask_max;
  u8 mask_max_msb;
  u32 mask_sign_mask;
  ULONG mask_span;
  u32 mask_unsigned;
  u8 mean_shift;
  u128 mean_unsigned;
  u8 overflow_status;
  ULONG pop;
  u8 sign_status;
  u8 status;
  fru128 sum_quartics;
  fru128 sum_squares;
  fru128 sum_squares_squared;
  fru128 term;
  u8 variance_shift;
  u128 zero;

  freq_list_base=agnentroprox_base->freq_list_base0;
  mask_max=agnentroprox_base->mask_max;
  overflow_status=*overflow_status_base;
  if(new_status){
    U128_SET_ZERO(zero);
    agnentroprox_base->entropy.a=zero;
    agnentroprox_base->entropy.b=zero;
    agnentroprox_base->sum_quartics.a=zero;
    agnentroprox_base->sum_quartics.b=zero;
    agnentroprox_base->sum_squares.a=zero;
    agnentroprox_base->sum_squares.b=zero;
    agnentroprox_ulong_list_zero((ULONG)(mask_max), freq_list_base);
    agnentroprox_base->mask_count0=0;
  }
  agnentroprox_mask_list_accrue(agnentroprox_base, 0, mask_idx_max, mask_list_base);
  loggamma_base=agnentroprox_base->loggamma_base;
  loggamma_idx_max=agnentroprox_base->loggamma_idx_max;
  loggamma_list_base=agnentroprox_base->loggamma_list_base;
  loggamma_parameter_list_base=agnentroprox_base->loggamma_parameter_list_base;
  mask_count=agnentroprox_base->mask_count0;
  FRU128_SET_ZERO(entropy);
  mask_span=(ULONG)(mask_max)+1;
  variance_shift=0;
  if(mode==AGNENTROPROX_MODE_AGNENTROPY){
/*
Compute (loggamma(mask_count_plus_span)-loggamma(mask_span)), which is the upper bound of agnentropy in nats.
*/
    loggamma_parameter=(u64)(mask_count)+mask_span;
    LOGGAMMA_U64_CACHED(entropy, loggamma_base, loggamma_idx_max, loggamma_list_base, loggamma_parameter_list_base, loggamma_parameter, overflow_status);
    loggamma_parameter=(u64)(mask_span);
    LOGGAMMA_U64_CACHED(term, loggamma_base, loggamma_idx_max, loggamma_list_base, loggamma_parameter_list_base, loggamma_parameter, overflow_status);
    FRU128_SUBTRACT_FRU128_SELF(entropy, term, overflow_status);
/*
Subtract the sum of loggamma(freq+1), which is log(freq!), over all masks. This accounts for the compressive effect of the span in agnentro_encode().
*/
    mask=0;
    do{
      freq=freq_list_base[mask];
      if(1<freq){
        loggamma_parameter=(u64)(freq)+1;
        LOGGAMMA_U64_CACHED(term, loggamma_base, loggamma_idx_max, loggamma_list_base, loggamma_parameter_list_base, loggamma_parameter, overflow_status);
        FRU128_SUBTRACT_FRU128_SELF(entropy, term, overflow_status);
      }
    }while((mask++)!=mask_max);
  }else if(mode==AGNENTROPROX_MODE_LOGFREEDOM){
    poissocache_base=agnentroprox_base->poissocache_base;
    if(new_status){
      poissocache_reset(poissocache_base);
    }
    freq_pop_idx_max=poissocache_parameters_get(&freq_pop_list_base0, poissocache_base);
/*
Count the populations of all frequencies. Use the same indexing and hashing method expected by agnentroprox_entropy_transform(). See the comments there.
*/
    mask=0;
    do{
      freq=freq_list_base[mask];
      POISSOCACHE_ITEM_ULONG_IDX_GET(freq_pop_ulong_idx, freq_pop_idx_max, freq, freq_pop_list_base0, freq_pop_list_base1, poissocache_base);
      freq_pop_list_base1[freq_pop_ulong_idx+1]++;
    }while((mask++)!=mask_max);
/*
Evaluate the logfreedom in nats (AKA "eubits" in https://dyspoissonism.blogspot.com/2015/05/the-logfreedom-formula.html), where H0 has been subsumed into the sums:

  L=log(Q!)+log(Z!)-Σ(F=(0, K), log(H[F]!))-Σ(F=(0, K),H[F]*log(F!))

where:

  F=(frequency)
  H[F]=(population of frequency F)
  K=(maximum frequency with nonzero population)
  Q=mask_count
  Z=mask_span
*/
    loggamma_parameter=(u64)(mask_count)+1;
    LOGGAMMA_U64_CACHED(entropy, loggamma_base, loggamma_idx_max, loggamma_list_base, loggamma_parameter_list_base, loggamma_parameter, overflow_status);
    loggamma_parameter=(u64)(mask_span)+1;
    LOGGAMMA_U64_CACHED(term, loggamma_base, loggamma_idx_max, loggamma_list_base, loggamma_parameter_list_base, loggamma_parameter, overflow_status);
    FRU128_ADD_FRU128_SELF(entropy, term, overflow_status);
    freq_pop_cache_idx=0;
    freq_pop_ulong_idx=0;
    do{
      status=poissocache_item_get_serialized(&freq_pop_cache_idx, &freq_pop_ulong_idx, &freq, poissocache_base, &pop);
      if((!status)&&pop){
        loggamma_parameter=(u64)(freq)+1;
        LOGGAMMA_U64_CACHED(term, loggamma_base, loggamma_idx_max, loggamma_list_base, loggamma_parameter_list_base, loggamma_parameter, overflow_status);
        if(1<pop){
          FRU128_MULTIPLY_U64_SELF(term, pop, overflow_status);
        }
        FRU128_SUBTRACT_FRU128_SELF(entropy, term, overflow_status);
        if(1<pop){
          loggamma_parameter=(u64)(pop)+1;
          LOGGAMMA_U64_CACHED(term, loggamma_base, loggamma_idx_max, loggamma_list_base, loggamma_parameter_list_base, loggamma_parameter, overflow_status);
          FRU128_SUBTRACT_FRU128_SELF(entropy, term, overflow_status);
        }
      }
    }while(!status);
  }else if(mode==AGNENTROPROX_MODE_SHANNON){
    entropy=agnentroprox_shannon_entropy_get(agnentroprox_base, 0, &overflow_status);
  }else{
/*
Evaluate the sum-of-squares for variance:

  V=Σ(M=(0, Z-1), F[M]*(|M-U|^2))

At the same time, if (mode==AGNENTROPROX_MODE_KURTOSIS), evaluate the sum-of-quartics for kurtosis:

  K=Σ(M=(0, Z-1), F[M]*(|M-U|^4))

where:

  F[M]=(frequency of mask M)
  M=mask
  U=(mean of all masks)
  Z=mask_span

Note that we do not account for Bessel's correction.
*/
    mask_idx_max_msb=0;
    if(mask_idx_max){
      mask_idx_max_msb=ULONG_BIT_MAX;
      while(!(mask_idx_max>>mask_idx_max_msb)){
        mask_idx_max_msb--;
      }
    }
    mask_max_msb=agnentroprox_base->mask_max_msb;
    mask_sign_mask=agnentroprox_base->mask_sign_mask;
    mean_shift=agnentroprox_base->mean_shift;
    mean_unsigned=agnentroprox_base->mean_unsigned;
    sign_status=agnentroprox_base->sign_status;
    sum_quartics=agnentroprox_base->sum_quartics;
    sum_squares=agnentroprox_base->sum_squares;
    U128_SET_ZERO(freq_mantissa);
    mask=0;
    if(!sign_status){
      mask_sign_mask=0;
    }
    variance_shift=(u8)(mean_shift-mask_max_msb-mask_idx_max_msb-2);
    agnentroprox_base->sweep_mask_idx_max_bit_count=(u8)(mask_idx_max_msb+1);
    agnentroprox_base->variance_shift=variance_shift;
    do{
      freq=freq_list_base[mask];
      if(freq){
        mask_unsigned=mask^mask_sign_mask;
        U128_FROM_U64_LO(delta, (u64)(mask_unsigned));
        U128_SHIFT_LEFT_SELF(delta, mean_shift);
/*
Compute |M-U|. We can do this without fractervals because the difference between a fractoid (mean) and an integer (delta) still has at most 1ULP of error, and furthermore that error is still fully owned by the last bit.
*/
        if(U128_IS_LESS_EQUAL(delta, mean_unsigned)){
          U128_SUBTRACT_FROM_U128_SELF(delta, mean_unsigned);
        }else{
          U128_DECREMENT_SELF(delta);
          U128_SUBTRACT_U128_SELF(delta, mean_unsigned);
        }
/*
Now compute powers 2 and maybe 4 of |M-U|. For this, we need fractervals because multiplication by fractoids can cause a shift in the error span, even though it shrinks in magnitude, which could cause it to straddle bit boundaries.
*/
        FRU128_FROM_FTD128(delta_power, delta);
        FRU128_MULTIPLY_FRU128_SELF(delta_power, delta_power);
        if(freq!=mask_count){
          U128_FROM_U64_LO(freq_mantissa, (u64)(freq));
          U128_SHIFT_LEFT_SELF(freq_mantissa, (u8)(U128_BIT_MAX-mask_idx_max_msb));
          FRU128_MULTIPLY_MANTISSA_U128(delta_power_product, delta_power, freq_mantissa);
        }else{
          delta_power_product=delta_power;
        }
        FRU128_ADD_FRU128_SELF(sum_squares, delta_power_product, overflow_status);
        if(mode==AGNENTROPROX_MODE_KURTOSIS){
          FRU128_MULTIPLY_FRU128_SELF(delta_power, delta_power);
          if(freq!=mask_count){
            FRU128_MULTIPLY_MANTISSA_U128(delta_power_product, delta_power, freq_mantissa);
          }else{
            delta_power_product=delta_power;
          }
          FRU128_ADD_FRU128_SELF(sum_quartics, delta_power_product, overflow_status);
        }
      }
    }while((mask++)!=mask_max);
    if(!rollback_status){
      agnentroprox_base->sum_quartics=sum_quartics;
      agnentroprox_base->sum_squares=sum_squares;
    }
    if(mode==AGNENTROPROX_MODE_VARIANCE){
/*
Set the entropy, E, to the variance:

  E=[sum_squares]/mask_count.
*/
      FRU128_DIVIDE_U64(entropy, sum_squares, (u64)(mask_count), overflow_status);
      fixed_point_shift=(u8)(U128_BITS-variance_shift);
    }else{
/*
Set the entropy, E, to the kurtosis:

  E=(mask_count*[sum_quartics])/[sum_squares].
*/
      FRU128_MULTIPLY_FRU128(sum_squares_squared, sum_squares, sum_squares);
      if(mask_count&mask_idx_max){
        U128_FROM_U64_LO(freq_mantissa, (u64)(mask_count));
        U128_SHIFT_LEFT_SELF(freq_mantissa, (u8)(U128_BIT_MAX-mask_idx_max_msb));
        FRU128_MULTIPLY_MANTISSA_U128_SELF(sum_quartics, freq_mantissa);
      }
      fixed_point_shift=0;
      while(U128_IS_LESS_EQUAL(sum_squares_squared.a, sum_quartics.b)){
        if(U128_IS_NOT_SIGNED(sum_squares_squared.b)){
          FRU128_SHIFT_LEFT_SELF(sum_squares_squared, 1, overflow_status);
        }else{
          FRU128_SHIFT_RIGHT_SELF(sum_quartics, 1);
        }
        fixed_point_shift++;
      }
      FRU128_DIVIDE_FRU128(entropy, sum_quartics, sum_squares_squared, overflow_status);
    }
    if(fixed_point_shift){
      if(fixed_point_shift<=U64_BITS){
        fixed_point_shift=(u8)(U64_BITS-fixed_point_shift);
        FRU128_SHIFT_RIGHT_SELF(entropy, fixed_point_shift);
      }else{
        fixed_point_shift=(u8)(fixed_point_shift-U64_BITS);
        FRU128_SHIFT_LEFT_SELF(entropy, fixed_point_shift, overflow_status);
      }
    }
  }
/*
Set entropy_delta to the previous entropy value, which will be converted to a delta below.
*/
  entropy_delta=agnentroprox_base->entropy;
  if(rollback_status){
    agnentroprox_mask_list_unaccrue(agnentroprox_base, 0, mask_idx_max, mask_list_base);
  }else{
/*
Allow the changes to *freq_list_base to accrue, so we can compute an additional entropy delta on the next call.
*/
    agnentroprox_base->entropy=entropy;
  }
/*
The only way that either entropy_delta.b could be zero is if the previously computed entropy (or the initialized value, if none) is zero. In that cases, doing the subtract below would result in underflow, so don't do it.
*/
  if(U128_IS_NOT_ZERO(entropy_delta.b)){
    FRU128_SUBTRACT_FRU128_SELF(entropy_delta, entropy, overflow_status);
  }else{
    entropy_delta=entropy;
  }
  *overflow_status_base=overflow_status;
  return entropy_delta;
}

fru128
agnentroprox_entropy_raw_get(agnentroprox_t *agnentroprox_base, ULONG mask_idx_max, u8 *overflow_status_base){
/*
Compute (Q log Z) where Q is the mask count and Z is the number of possible masks.

In:

  agnentroprox_base is the return value of agenentroprox_init().

  mask_idx_max is one less than the number of masks in the mask list, such that if agnentroprox_init():In:overlap_status was one, then this value would need to be increased in order to account for mask overlap. For example, if the sweep contains 5 of 3-byte masks, then this value would be 4 _without_ overlap, or 12 _with_ overlap. Must not exceed agnentroprox_init():In:mask_idx_max_max. See also agnentroprox_mask_idx_max_get().

  *overflow_status_base is the OR-cummulative fracterval overflow status.

Out:

  Returns ((mask_idx_max+1)*log((agnentroprox_init():In:mask_max)+1)), which is the raw entropy implied by mask_idx_max.

  *overflow_status_base is one if a fracterval overflow occurred, else unchanged.
*/
  fru128 entropy_raw;
  ULONG mask_count;
  fru128 mask_span_log;
  u8 overflow_status;

  overflow_status=*overflow_status_base;
  mask_span_log=agnentroprox_base->mask_span_log;
  mask_count=(ULONG)(mask_idx_max)+1;
  FRU128_MULTIPLY_U64(entropy_raw, mask_span_log, (u64)(mask_count), overflow_status);
  *overflow_status_base=overflow_status;
  return entropy_raw;
}

ULONG
agnentroprox_entropy_transform(agnentroprox_t *agnentroprox_base, u8 append_mode, fru128 *entropy_list_base, ULONG mask_idx_max, u8 *mask_list_base, ULONG match_idx_max_max, ULONG *match_u8_idx_list_base, u16 mode, u8 *overflow_status_base, ULONG sweep_mask_idx_max){
/*
Compute a particular type of entropy transform of a mask list, given a particular sweep.

In:

  agnentroprox_base is the return value of agenentroprox_init().

  append_mode is 2 to append diventropies one at at time to *diventropy_list_base, ordered ascending by the base index of the sweep window. Else one to sort results ascending by diventropy, or zero to sort descending.

  *entropy_list_base contains (match_idx_max_max+1) undefined items.
    
  mask_idx_max is one less than the number of masks in the mask list, such that if agnentroprox_init():In:overlap_status was one, then this value would need to be increased in order to account for mask overlap. For example, if the sweep contains 5 of 3-byte masks, then this value would be 4 _without_ overlap, or 12 _with_ overlap. Must not exceed agnentroprox_init():In:mask_idx_max_max. See also agnentroprox_mask_idx_max_get().

  *mask_list_base is the mask list.

  match_idx_max_max is one less than the maximum number of matches to report.

  *match_u8_idx_list_base is NULL if the sweep base indexes associated with matches can be discarded, else the base of (match_idx_max_max+1) undefined items to hold such indexes.

  mode is AGNENTROPROX_MODE_AGNENTROPY, AGNENTROPROX_MODE_KURTOSIS, AGNENTROPROX_MODE_JSET, AGNENTROPROX_MODE_LET, AGNENTROPROX_MODE_LOGFREEDOM, AGNENTROPROX_MODE_SHANNON, or AGNENTROPROX_MODE_VARIANCE to compute the agnentropy, obtuse kurtosis, (1-(normalized Jensen-Shannon exodivergence)), (1-(normalized Leidich exodivergence)), logfreedom, Shannon, or obtuse variance entropy transform, respectively. For AGNENTROPROX_MODE_KURTOSIS and AGNENTROPROX_MODE_VARIANCE, the global mean must have been precomputed by agnentroprox_mask_list_mean_get().

  *overflow_status_base is the OR-cummulative fracterval overflow status.

  sweep_mask_idx_max is one less than the number of masks in the sweep, which like mask_idx_max, must account for mask overlap if enabled.  On [0, mask_idx_max].

Out:

  Returns the number of matches found, which is simply (MIN((mask_idx_max-sweep_mask_idx_max))+1, match_idx_max_max).

  *entropy_list_base contains (return value) items which represent the matches identified during the search, sorted according to append_mode.

  *overflow_status_base is one if a fracterval overflow occurred, else unchanged.
*/
  fru128 coeff0;
  fru128 coeff1;
  fru128 coeff2;
  fru128 coeff3;
  fru128 coeff4;
  fru128 coeff5;
  fru128 coeff6;
  fru128 coeff7;
  u128 delta;
  fru128 delta_power;
  fru128 delta_power_shifted;
  fru128 entropy;
  fru128 entropy_delta;
  u128 entropy_mean;
  u128 entropy_threshold;
  ULONG exo_freq;
  ULONG exo_freq_minus_1;
  ULONG exo_freq_old;
  ULONG exo_freq_old_plus_1;
  ULONG exo_mask_count;
  ULONG exo_mask_count_plus_span;
  u128 exo_mask_count_recip_half;
  u8 fixed_point_shift;
  ULONG freq;
  ULONG *freq_list_base0;
  ULONG *freq_list_base1;
  u128 freq_mantissa;
  ULONG freq_plus_1;
  ULONG freq_pop_idx_max;
  ULONG *freq_pop_list_base0;
  ULONG *freq_pop_list_base1;
  ULONG freq_pop_ulong_idx;
  ULONG freq_old;
  ULONG freq_old_minus_1;
  u8 granularity;
  u8 ignored_status;
  fru128 ld_coeff;
  u8 ld_shift;
  u128 log2_recip_half;
  fru64 log;
  ULONG log_delta_idx_max;
  fru64 *log_delta_list_base;
  u64 *log_delta_parameter_list_base;
  ULONG log_idx_max;
  fru64 *log_list_base;
  u64 *log_parameter_list_base;
  fru128 log_u128;
  ULONG log_u128_idx_max;
  fru128 *log_u128_list_base;
  u128 *log_u128_parameter_list_base;
  u32 mask;
  u32 mask_max;
  u32 mask_old;
  u32 mask_sign_mask;
  u8 mask_u8;
  u32 mask_unsigned;
  ULONG match_count;
  ULONG match_idx;
  ULONG match_idx_min;
  u8 match_status_not;
  u8 mean_shift;
  u128 mean_unsigned;
  u8 mode_bit_idx;
  u8 overflow_status;
  u8 overlap_status;
  poissocache_t *poissocache_base;
  ULONG pop;
  u8 sign_status;
  fru128 sum_quartics;
  fru128 sum_squares;
  fru128 sum_squares_squared;
  ULONG sweep_mask_count;
  u128 sweep_mask_count_recip_half;
  u8 sweep_mask_idx_max_bit_count;
  fru64 term_minus;
  fru64 term_plus;
  fru128 term_u128;
  fru128 term_u128_minus;
  fru128 term_u128_plus;
  ULONG u8_idx;
  ULONG u8_idx_old;
  ULONG u8_idx_delta;
  ULONG u8_idx_max;
  u128 uint0;
  u128 uint1;
  u128 uint2;
  u128 uint3;
  u128 uint4;
  u128 uint5;
  u128 uint6;
  u128 uint7;
  u8 variance_shift;
/*
In cases where ignored_status is used, overflows are safe to ignore because the correct result of the operation is guaranteed to be on [0.0, 1.0], and result saturation does the right thing. The right shifts in this function are there in order to facilitate this optimization.
*/
  overflow_status=*overflow_status_base;
  freq_list_base0=agnentroprox_base->freq_list_base0;
  freq_list_base1=agnentroprox_base->freq_list_base1;
  mask_max=agnentroprox_base->mask_max;
  log_idx_max=agnentroprox_base->log_idx_max;
  log_list_base=agnentroprox_base->log_list_base;
  log_parameter_list_base=agnentroprox_base->log_parameter_list_base;
  FRU128_SET_ZERO(entropy);
  ignored_status=0;
  sweep_mask_count=sweep_mask_idx_max+1;
  if((mode!=AGNENTROPROX_MODE_EXOENTROPY)&&(mode!=AGNENTROPROX_MODE_JSET)&&(mode!=AGNENTROPROX_MODE_LET)){
    entropy=agnentroprox_entropy_delta_get(agnentroprox_base, sweep_mask_idx_max, mask_list_base, mode, 1, &overflow_status, 0);
  }else{
    agnentroprox_ulong_list_zero((ULONG)(mask_max), freq_list_base0);
    agnentroprox_base->mask_count0=0;
    agnentroprox_mask_list_accrue(agnentroprox_base, 0, sweep_mask_idx_max, mask_list_base);
    if(mode==AGNENTROPROX_MODE_JSET){
      agnentroprox_jsd_get(agnentroprox_base, 1, mask_idx_max, mask_list_base);
      entropy=agnentroprox_base->entropy;
      FRU128_SHIFT_RIGHT_SELF(entropy, 2);
    }else if(mode==AGNENTROPROX_MODE_LET){
      agnentroprox_ld_get(agnentroprox_base, 1, mask_idx_max, mask_list_base);
      entropy=agnentroprox_base->entropy;
    }else{
      agnentroprox_ulong_list_zero((ULONG)(mask_max), freq_list_base1);
      agnentroprox_base->mask_count1=0;
      agnentroprox_mask_list_accrue(agnentroprox_base, 1, mask_idx_max, mask_list_base);
      agnentroprox_mask_list_subtract(agnentroprox_base);
      exo_mask_count=agnentroprox_base->mask_count1;
      exo_mask_count_plus_span=exo_mask_count+mask_max+1;
/*
Evaluate the Shannon entropy in nats, over all masks M, as implied by the frequencies F0[M] at freq_list_base0, but using the agnostic probabilities implied by the frequencies F1[M] at freq_list_base1:

  S=(Q0*log(Q1+Z))-Σ(M=(0, Z-1), F0[M]*log(F1[M]+1))

where:

  F[M]=MIN((frequency of mask M), 1)
  M=mask
  Q0=sweep_mask_count
  Q1=(exo_mask_count_plus_span (after subtracting Q0))
  Z=mask_span
*/
      FRU64_LOG_U64_NONZERO_CACHED(term_plus, log_idx_max, log_list_base, log_parameter_list_base, (u64)(exo_mask_count_plus_span));
      FRU128_FROM_FRU64_MULTIPLY_U64(entropy, term_plus, (u64)(sweep_mask_count));
      FRU128_SHIFT_LEFT_SELF(entropy, 64-58, overflow_status);
      mask=0;
      do{
        freq=freq_list_base1[mask]+1;
        FRU64_LOG_U64_NONZERO_CACHED(term_minus, log_idx_max, log_list_base, log_parameter_list_base, (u64)(freq));
        freq=freq_list_base0[mask];
        FRU128_FROM_FRU64_MULTIPLY_U64(entropy_delta, term_minus, (u64)(freq));
        FRU128_SHIFT_LEFT_SELF(entropy_delta, 64-58, overflow_status);
        FRU128_SUBTRACT_FRU128_SELF(entropy, entropy_delta, overflow_status);
      }while((mask++)!=mask_max);
    }
  }
  FRU128_MEAN_TO_FTD128(entropy_mean, entropy);
  match_count=1;
  if(match_u8_idx_list_base){
    match_u8_idx_list_base[0]=0;
  }
  entropy_list_base[0]=entropy;
  U128_FROM_BOOL(entropy_threshold, append_mode);
  if(!match_idx_max_max){
    entropy_threshold=entropy_mean;
  }
  exo_mask_count=agnentroprox_base->mask_count1;
  exo_mask_count_recip_half=agnentroprox_base->haystack_mask_count_recip_half;
  granularity=agnentroprox_base->granularity;
  log_delta_idx_max=agnentroprox_base->log_delta_idx_max;
  log_delta_list_base=agnentroprox_base->log_delta_list_base;
  log_delta_parameter_list_base=agnentroprox_base->log_delta_parameter_list_base;
  log_u128_idx_max=agnentroprox_base->log_u128_idx_max;
  log_u128_list_base=agnentroprox_base->log_u128_list_base;
  log_u128_parameter_list_base=agnentroprox_base->log_u128_parameter_list_base;
  mask_sign_mask=agnentroprox_base->mask_sign_mask;
  mean_shift=agnentroprox_base->mean_shift;
  mean_unsigned=agnentroprox_base->mean_unsigned;
  overlap_status=agnentroprox_base->overlap_status;
  sign_status=agnentroprox_base->sign_status;
  sweep_mask_count_recip_half=agnentroprox_base->needle_mask_count_recip_half;
  sweep_mask_idx_max_bit_count=agnentroprox_base->sweep_mask_idx_max_bit_count;
  variance_shift=agnentroprox_base->variance_shift;
  mode_bit_idx=0;
  while(mode>>mode_bit_idx>>1){
    mode_bit_idx++;
  }
  if(!sign_status){
    mask_sign_mask=0;
  }
  u8_idx_delta=(u8)((u8)(granularity*(!overlap_status))+1);
  u8_idx=sweep_mask_count*u8_idx_delta;
  u8_idx_max=mask_idx_max*u8_idx_delta;
  u8_idx_old=0;
  while(u8_idx<=u8_idx_max){
    mask=mask_list_base[u8_idx];
    mask_old=mask_list_base[u8_idx_old];
    if(granularity){
      mask_u8=mask_list_base[u8_idx+U16_BYTE_MAX];
      mask|=(u32)(mask_u8)<<U8_BITS;
      mask_u8=mask_list_base[u8_idx_old+U16_BYTE_MAX];
      mask_old|=(u32)(mask_u8)<<U8_BITS;
      if(U16_BYTE_MAX<granularity){
        mask_u8=mask_list_base[u8_idx+U24_BYTE_MAX];
        mask|=(u32)(mask_u8)<<U16_BITS;
        mask_u8=mask_list_base[u8_idx_old+U24_BYTE_MAX];
        mask_old|=(u32)(mask_u8)<<U16_BITS;
        if(U24_BYTE_MAX<granularity){
          mask_u8=mask_list_base[u8_idx+U32_BYTE_MAX];
          mask|=(u32)(mask_u8)<<U24_BITS;
          mask_u8=mask_list_base[u8_idx_old+U32_BYTE_MAX];
          mask_old|=(u32)(mask_u8)<<U24_BITS;
        }
      }
    }
/*
Increment the u8 indexes now, so that if we get a match, u8_idx_old will be the first index of the new sweep, as opposed to the old one.
*/
    u8_idx+=u8_idx_delta;
    u8_idx_old+=u8_idx_delta;
    if(mask!=mask_old){
      freq=freq_list_base0[mask];
      freq_old=freq_list_base0[mask_old];
      freq_old_minus_1=freq_old-1;
      freq_plus_1=freq+1;
      freq_list_base0[mask]=freq_plus_1;
      freq_list_base0[mask_old]=freq_old_minus_1;
/*
Account for the entropy difference due to the difference in frequency in the haystack between the mask being added and the mask being subtracted due to shifting the sweep window by one (maybe overlapping) mask:
*/
      switch(mode_bit_idx){
      case AGNENTROPROX_MODE_AGNENTROPY_BIT_IDX:
/*
The agnentropy difference, dA, is:

  dA=log(freq_old)-log(freq_plus_1)
*/
        FRU64_LOG_U64_NONZERO_CACHED(term_minus, log_idx_max, log_list_base, log_parameter_list_base, (u64)(freq_plus_1));
        FRU64_LOG_U64_NONZERO_CACHED(term_plus, log_idx_max, log_list_base, log_parameter_list_base, (u64)(freq_old));
        if(freq_plus_1<freq_old){
          FRU64_SUBTRACT_FRU64_SELF(term_plus, term_minus, overflow_status);
          FRU128_ADD_FRU64_SHIFTED_SELF(entropy, 64-58, term_plus, overflow_status);
        }else if(freq_plus_1!=freq_old){
          FRU64_SUBTRACT_FRU64_SELF(term_minus, term_plus, overflow_status);
          FRU128_SUBTRACT_FRU64_SHIFTED_SELF(entropy, 64-58, term_minus, overflow_status);
        }
        break;
      case AGNENTROPROX_MODE_JSET_BIT_IDX:
/*
Compute (1-(normalized Jensen-Shannon exodivergence)) delta according to the method described for JSET(H, J, N, Z) in http://vixra.org/abs/1710.0261 with division by (2QnQs) already built into the coefficients.
*/
        exo_freq=freq_list_base1[mask];
        exo_freq_old=freq_list_base1[mask_old];
        U128_FROM_U64_PRODUCT(uint0, (u64)(exo_freq), (u64)(sweep_mask_count));
        FRU128_LOG_U128_NONZERO_CACHED(log_u128, log_u128_idx_max, log_u128_list_base, log_u128_parameter_list_base, uint0);
        FRU128_FROM_FTD128_U64_PRODUCT(coeff0, exo_mask_count_recip_half, (u64)(exo_freq), ignored_status);
        FRU128_MULTIPLY_FRU128(term_u128_plus, coeff0, log_u128);
        FRU128_SHIFT_RIGHT_SELF(term_u128_plus, 2);
        if(exo_freq_old){
          U128_FROM_U64_PRODUCT(uint1, (u64)(exo_freq_old), (u64)(sweep_mask_count));
          FRU128_LOG_U128_NONZERO_CACHED(log_u128, log_u128_idx_max, log_u128_list_base, log_u128_parameter_list_base, uint1);
          FRU128_FROM_FTD128_U64_PRODUCT(coeff1, exo_mask_count_recip_half, (u64)(exo_freq_old), ignored_status);
          FRU128_MULTIPLY_FRU128(term_u128, coeff1, log_u128);
          FRU128_SHIFT_RIGHT_SELF(term_u128, 2);
          FRU128_ADD_FRU128_SELF(term_u128_plus, term_u128, ignored_status);
        }else{
          FRU128_SET_ZERO(coeff1);
          U128_SET_ZERO(uint1);
        }
        if(freq){
          U128_FROM_U64_PRODUCT(uint2, (u64)(freq), (u64)(exo_mask_count));
          FRU128_LOG_U128_NONZERO_CACHED(log_u128, log_u128_idx_max, log_u128_list_base, log_u128_parameter_list_base, uint2);
          FRU128_FROM_FTD128_U64_PRODUCT(coeff2, sweep_mask_count_recip_half, (u64)(freq), ignored_status);
          FRU128_MULTIPLY_FRU128(term_u128, coeff2, log_u128);
          FRU128_SHIFT_RIGHT_SELF(term_u128, 2);
          FRU128_ADD_FRU128_SELF(term_u128_plus, term_u128, ignored_status);
        }else{
          FRU128_SET_ZERO(coeff2);
          U128_SET_ZERO(uint2);
        }
        U128_FROM_U64_PRODUCT(uint3, (u64)(freq_old), (u64)(exo_mask_count));
        FRU128_LOG_U128_NONZERO_CACHED(log_u128, log_u128_idx_max, log_u128_list_base, log_u128_parameter_list_base, uint3);
        FRU128_FROM_FTD128_U64_PRODUCT(coeff3, sweep_mask_count_recip_half, (u64)(freq_old), ignored_status);
        FRU128_MULTIPLY_FRU128(term_u128, coeff3, log_u128);
        FRU128_SHIFT_RIGHT_SELF(term_u128, 2);
        FRU128_ADD_FRU128_SELF(term_u128_plus, term_u128, ignored_status);
        if(exo_freq!=1){
          U128_SUBTRACT_U64_LO(uint4, uint0, (u64)(sweep_mask_count));
          FRU128_LOG_U128_NONZERO_CACHED(log_u128, log_u128_idx_max, log_u128_list_base, log_u128_parameter_list_base, uint4);
          FRU128_SUBTRACT_FTD128(coeff4, coeff0, exo_mask_count_recip_half, ignored_status);
          FRU128_MULTIPLY_FRU128(term_u128_minus, coeff4, log_u128);
          FRU128_SHIFT_RIGHT_SELF(term_u128_minus, 2);
        }else{
          FRU128_SET_ZERO(coeff4);
          FRU128_SET_ZERO(term_u128_minus);
          U128_SET_ZERO(uint4);
        }
        U128_ADD_U64_LO(uint5, uint1, (u64)(sweep_mask_count));
        FRU128_LOG_U128_NONZERO_CACHED(log_u128, log_u128_idx_max, log_u128_list_base, log_u128_parameter_list_base, uint5);
        FRU128_ADD_FTD128(coeff5, coeff1, exo_mask_count_recip_half, ignored_status);
        FRU128_MULTIPLY_FRU128(term_u128, coeff5, log_u128);
        FRU128_SHIFT_RIGHT_SELF(term_u128, 2);
        FRU128_ADD_FRU128_SELF(term_u128_minus, term_u128, ignored_status);
        U128_ADD_U64_LO(uint6, uint2, (u64)(exo_mask_count));
        FRU128_LOG_U128_NONZERO_CACHED(log_u128, log_u128_idx_max, log_u128_list_base, log_u128_parameter_list_base, uint6);
        FRU128_ADD_FTD128(coeff6, coeff2, sweep_mask_count_recip_half, ignored_status);
        FRU128_MULTIPLY_FRU128(term_u128, coeff6, log_u128);
        FRU128_SHIFT_RIGHT_SELF(term_u128, 2);
        FRU128_ADD_FRU128_SELF(term_u128_minus, term_u128, ignored_status);
        if(freq_old!=1){
          U128_SUBTRACT_U64_LO(uint7, uint3, (u64)(exo_mask_count));
          FRU128_LOG_U128_NONZERO_CACHED(log_u128, log_u128_idx_max, log_u128_list_base, log_u128_parameter_list_base, uint7);
          FRU128_SUBTRACT_FTD128(coeff7, coeff3, sweep_mask_count_recip_half, ignored_status);
          FRU128_MULTIPLY_FRU128(term_u128, coeff7, log_u128);
          FRU128_SHIFT_RIGHT_SELF(term_u128, 2);
          FRU128_ADD_FRU128_SELF(term_u128_minus, term_u128, ignored_status);
        }else{
          FRU128_SET_ZERO(coeff7);
          U128_SET_ZERO(uint7);
        }
        U128_ADD_U128(uint4, uint0, uint2);
        FRU128_LOG_U128_NONZERO_CACHED(log_u128, log_u128_idx_max, log_u128_list_base, log_u128_parameter_list_base, uint4);
        FRU128_ADD_FRU128(coeff4, coeff0, coeff2, ignored_status);
        FRU128_MULTIPLY_FRU128(term_u128, coeff4, log_u128);
        FRU128_SHIFT_RIGHT_SELF(term_u128, 2);
        FRU128_ADD_FRU128_SELF(term_u128_minus, term_u128, ignored_status);
        U128_ADD_U128(uint5, uint1, uint3);
        FRU128_LOG_U128_NONZERO_CACHED(log_u128, log_u128_idx_max, log_u128_list_base, log_u128_parameter_list_base, uint5);
        FRU128_ADD_FRU128(coeff5, coeff1, coeff3, ignored_status);
        FRU128_MULTIPLY_FRU128(term_u128, coeff5, log_u128);
        FRU128_SHIFT_RIGHT_SELF(term_u128, 2);
        FRU128_ADD_FRU128_SELF(term_u128_minus, term_u128, ignored_status);
        U128_ADD_U64_LO_SELF(uint4, (u64)(exo_mask_count));
        U128_SUBTRACT_U64_LO_SELF(uint4, (u64)(sweep_mask_count));
        FRU128_LOG_U128_NONZERO_CACHED(log_u128, log_u128_idx_max, log_u128_list_base, log_u128_parameter_list_base, uint4);
        FRU128_ADD_FTD128_SELF(coeff4, sweep_mask_count_recip_half, overflow_status);
        FRU128_SUBTRACT_FTD128_SELF(coeff4, exo_mask_count_recip_half, ignored_status);
        FRU128_MULTIPLY_FRU128(term_u128, coeff4, log_u128);
        FRU128_SHIFT_RIGHT_SELF(term_u128, 2);
        FRU128_ADD_FRU128_SELF(term_u128_plus, term_u128, ignored_status);
        U128_ADD_U64_LO_SELF(uint5, (u64)(sweep_mask_count));
        U128_SUBTRACT_U64_LO_SELF(uint5, (u64)(exo_mask_count));
        FRU128_LOG_U128_NONZERO_CACHED(log_u128, log_u128_idx_max, log_u128_list_base, log_u128_parameter_list_base, uint5);
        FRU128_ADD_FTD128_SELF(coeff5, exo_mask_count_recip_half, overflow_status);
        FRU128_SUBTRACT_FTD128_SELF(coeff5, sweep_mask_count_recip_half, ignored_status);
        FRU128_MULTIPLY_FRU128(term_u128, coeff5, log_u128);
        FRU128_SHIFT_RIGHT_SELF(term_u128, 2);
        FRU128_ADD_FRU128_SELF(term_u128_plus, term_u128, overflow_status);
        FRU128_ADD_FRU128_SELF(entropy, term_u128_plus, overflow_status);
        FRU128_SUBTRACT_FRU128_SELF(entropy, term_u128_minus, ignored_status);
        freq_list_base1[mask]=exo_freq-1;
        freq_list_base1[mask_old]=exo_freq_old+1;
        break;
      case AGNENTROPROX_MODE_LET_BIT_IDX:
/*
Compute (1-(normalized Leidich exodivergence)) delta according to the method described for LET(H, J, N, Z) in http://vixra.org/abs/1710.0261 .
*/
        exo_freq=freq_list_base1[mask];
        exo_freq_old=freq_list_base1[mask_old];
        FRU64_LOG_U64_NONZERO_CACHED(log, log_idx_max, log_list_base, log_parameter_list_base, (u64)(exo_freq));
        FRU128_ADD_FRU64_LO_SELF(entropy, log, overflow_status);
        FRU64_LOG_U64_NONZERO_CACHED(log, log_idx_max, log_list_base, log_parameter_list_base, (u64)(freq_old));
        FRU128_ADD_FRU64_LO_SELF(entropy, log, overflow_status);
        exo_freq--;
        if(exo_freq){
          FRU64_LOG_DELTA_U64_NONZERO_CACHED(log, log_delta_idx_max, log_delta_list_base, log_delta_parameter_list_base, (u64)(exo_freq));
          FRU128_FROM_FRU64_MULTIPLY_U64(entropy_delta, log, (u64)(exo_freq));
          FRU128_ADD_FRU128_SELF(entropy, entropy_delta, overflow_status);
        }
        freq_list_base1[mask]=exo_freq;
        if(freq_old_minus_1){
          FRU64_LOG_DELTA_U64_NONZERO_CACHED(log, log_delta_idx_max, log_delta_list_base, log_delta_parameter_list_base, (u64)(freq_old_minus_1));
          FRU128_FROM_FRU64_MULTIPLY_U64(entropy_delta, log, (u64)(freq_old_minus_1));
          FRU128_ADD_FRU128_SELF(entropy, entropy_delta, overflow_status);
        }
        if(exo_freq_old){
          FRU64_LOG_DELTA_U64_NONZERO_CACHED(log, log_delta_idx_max, log_delta_list_base, log_delta_parameter_list_base, (u64)(exo_freq_old));
          FRU128_FROM_FRU64_MULTIPLY_U64(entropy_delta, log, (u64)(exo_freq_old));
          FRU128_SUBTRACT_FRU128_SELF(entropy, entropy_delta, overflow_status);
        }
        if(freq){
          FRU64_LOG_DELTA_U64_NONZERO_CACHED(log, log_delta_idx_max, log_delta_list_base, log_delta_parameter_list_base, (u64)(freq));
          FRU128_FROM_FRU64_MULTIPLY_U64(entropy_delta, log, (u64)(freq));
          FRU128_SUBTRACT_FRU128_SELF(entropy, entropy_delta, overflow_status);
        }
        exo_freq_old++;
        FRU64_LOG_U64_NONZERO_CACHED(log, log_idx_max, log_list_base, log_parameter_list_base, (u64)(exo_freq_old));
        freq_list_base1[mask_old]=exo_freq_old;
        FRU128_SUBTRACT_FRU64_LO_SELF(entropy, log, ignored_status);
        FRU64_LOG_U64_NONZERO_CACHED(log, log_idx_max, log_list_base, log_parameter_list_base, (u64)(freq_plus_1));
        FRU128_SUBTRACT_FRU64_LO_SELF(entropy, log, ignored_status);
        break;
      case AGNENTROPROX_MODE_EXOENTROPY_BIT_IDX:
        exo_freq=freq_list_base1[mask];
        exo_freq_old=freq_list_base1[mask_old];
        exo_freq_minus_1=exo_freq-1;
        exo_freq_old_plus_1=exo_freq_old+1;
        freq_list_base1[mask]=exo_freq_minus_1;
        freq_list_base1[mask_old]=exo_freq_old_plus_1;
/*
The exoentropy difference, dE, is:

  dE=(freq*log(exo_freq+1))+(freq_old*log(exo_freq_old+1)))-((freq+1)*log(exo_freq))-((freq_old-1)*log(exo_freq_old+2))
  dE=freq*(log(exo_freq+1)-log(exo_freq))+log(exo_freq_old+1)-((freq_old-1)*(log(exo_freq_old+2)-log(exo_freq_old+1)))-log(exo_freq)
  dE=(freq*log_delta(exo_freq))+log(exo_freq_old+1)-((freq_old-1)*log_delta(exo_freq_old+1))-log(exo_freq)
  dE=(freq*log_delta(exo_freq))+log(exo_freq_old_plus_1)-(freq_old_minus_1*log_delta(exo_freq_old_plus_1))-log(exo_freq)
*/
        FRU64_LOG_DELTA_U64_NONZERO_CACHED(term_plus, log_delta_idx_max, log_delta_list_base, log_delta_parameter_list_base, (u64)(exo_freq));
        FRU128_FROM_FRU64_MULTIPLY_U64(entropy_delta, term_plus, (u64)(freq));
        FRU64_LOG_U64_NONZERO_CACHED(term_plus, log_idx_max, log_list_base, log_parameter_list_base, (u64)(exo_freq_old_plus_1));
        FRU128_ADD_FRU64_LO_SELF(entropy_delta, term_plus, overflow_status);
        FRU128_SHIFT_LEFT_SELF(entropy_delta, 64-58, overflow_status);
        FRU128_ADD_FRU128_SELF(entropy, entropy_delta, overflow_status);
        FRU64_LOG_DELTA_U64_NONZERO_CACHED(term_minus, log_delta_idx_max, log_delta_list_base, log_delta_parameter_list_base, (u64)(exo_freq_old_plus_1));
        FRU128_FROM_FRU64_MULTIPLY_U64(entropy_delta, term_minus, (u64)(freq_old_minus_1));
        FRU64_LOG_U64_NONZERO_CACHED(term_minus, log_idx_max, log_list_base, log_parameter_list_base, (u64)(exo_freq));
        FRU128_ADD_FRU64_LO_SELF(entropy_delta, term_minus, overflow_status);
        FRU128_SHIFT_LEFT_SELF(entropy_delta, 64-58, overflow_status);
        FRU128_SUBTRACT_FRU128_SELF(entropy, entropy_delta, overflow_status);
        break;
      case AGNENTROPROX_MODE_LOGFREEDOM_BIT_IDX:
/*
The logfreedom difference in terms the frequencies in question and their respective populations, dL, is:

  dL=+log(pop(freq_old))+log(freq_old!)
     -log(pop(freq_old_minus_1)+1)-log(freq_old_minus_1!)
     +log(pop(freq))+log(freq!)
     -log(pop(freq_plus_1)+1)-log(freq_plus_1!)

  dL=+log(freq_old)-log(freq_plus_1)+log(pop(freq_old))-log(pop(freq_old_minus_1)+1)
     +log(pop(freq))-log(pop(freq_plus_1)+1)

where we need to serialize updates to the populations of the old and new masks, hence the division into 2 separate lines above.
*/
        if(freq_old!=freq_plus_1){
          poissocache_base=agnentroprox_base->poissocache_base;
          freq_pop_idx_max=poissocache_parameters_get(&freq_pop_list_base0, poissocache_base);
          POISSOCACHE_ITEM_ULONG_IDX_GET(freq_pop_ulong_idx, freq_pop_idx_max, freq_old, freq_pop_list_base0, freq_pop_list_base1, poissocache_base);
          pop=freq_pop_list_base1[freq_pop_ulong_idx+1];
          freq_pop_list_base1[freq_pop_ulong_idx+1]=pop-1;
          FRU64_LOG_U64_NONZERO_CACHED(term_plus, log_idx_max, log_list_base, log_parameter_list_base, (u64)(freq_old));
          FRU64_LOG_U64_NONZERO_CACHED(term_minus, log_idx_max, log_list_base, log_parameter_list_base, (u64)(freq_plus_1));
          if(pop!=1){
            FRU64_LOG_U64_NONZERO_CACHED(log, log_idx_max, log_list_base, log_parameter_list_base, (u64)(pop));
            FRU64_ADD_FRU64_SELF(term_plus, log, overflow_status);
          }
          POISSOCACHE_ITEM_ULONG_IDX_GET(freq_pop_ulong_idx, freq_pop_idx_max, freq_old_minus_1, freq_pop_list_base0, freq_pop_list_base1, poissocache_base);
          pop=freq_pop_list_base1[freq_pop_ulong_idx+1];
          pop++;
          freq_pop_list_base1[freq_pop_ulong_idx+1]=pop;
          if(pop!=1){
            FRU64_LOG_U64_NONZERO_CACHED(log, log_idx_max, log_list_base, log_parameter_list_base, (u64)(pop));
            FRU64_ADD_FRU64_SELF(term_minus, log, overflow_status);
          }
          POISSOCACHE_ITEM_ULONG_IDX_GET(freq_pop_ulong_idx, freq_pop_idx_max, freq, freq_pop_list_base0, freq_pop_list_base1, poissocache_base);
          pop=freq_pop_list_base1[freq_pop_ulong_idx+1];
          freq_pop_list_base1[freq_pop_ulong_idx+1]=pop-1;
          if(pop!=1){
            FRU64_LOG_U64_NONZERO_CACHED(log, log_idx_max, log_list_base, log_parameter_list_base, (u64)(pop));
            FRU64_ADD_FRU64_SELF(term_plus, log, overflow_status);
          }
          POISSOCACHE_ITEM_ULONG_IDX_GET(freq_pop_ulong_idx, freq_pop_idx_max, freq_plus_1, freq_pop_list_base0, freq_pop_list_base1, poissocache_base);
          pop=freq_pop_list_base1[freq_pop_ulong_idx+1];
          pop++;
          freq_pop_list_base1[freq_pop_ulong_idx+1]=pop;
          if(pop!=1){
            FRU64_LOG_U64_NONZERO_CACHED(log, log_idx_max, log_list_base, log_parameter_list_base, (u64)(pop));
            FRU64_ADD_FRU64_SELF(term_minus, log, overflow_status);
          }
          FRU128_ADD_FRU64_SHIFTED_SELF(entropy, 64-58, term_plus, overflow_status);
          FRU128_SUBTRACT_FRU64_SHIFTED_SELF(entropy, 64-58, term_minus, overflow_status);
        }
        break;
      case AGNENTROPROX_MODE_SHANNON_BIT_IDX:
/*
The Shannon entropy difference, dS, is:

  dS=(freq_old*log(freq_old))+(freq*log(freq))-(freq_old_minus_1*log(freq_old_minus_1))-(freq_plus_1*log(freq_plus_1))
  dS=((freq_old_minus_1*(log(freq_old)-log(freq_old_minus_1)))+log(freq_old))-((freq*(log(freq_plus_1)-log(freq)))+log(freq_plus_1))
  dS=((freq_old_minus_1*log_delta(freq_old_minus_1))+log(freq_old))-((freq*log_delta(freq))+log(freq_plus_1))

where any log(0) or log_delta(0) is treated as though it equals zero.
*/
        if(freq_old!=freq_plus_1){
          if(freq_old_minus_1){
            FRU64_LOG_DELTA_U64_NONZERO_CACHED(term_plus, log_delta_idx_max, log_delta_list_base, log_delta_parameter_list_base, (u64)(freq_old_minus_1));
            FRU128_FROM_FRU64_MULTIPLY_U64(entropy_delta, term_plus, (u64)(freq_old_minus_1));
            FRU64_LOG_U64_NONZERO_CACHED(term_plus, log_idx_max, log_list_base, log_parameter_list_base, (u64)(freq_old));
            FRU128_ADD_FRU64_LO_SELF(entropy_delta, term_plus, overflow_status);
            FRU128_SHIFT_LEFT_SELF(entropy_delta, 64-58, overflow_status);
            FRU128_ADD_FRU128_SELF(entropy, entropy_delta, overflow_status);
          }
          if(freq){
            if(freq_plus_1!=sweep_mask_count){
              FRU64_LOG_DELTA_U64_NONZERO_CACHED(term_minus, log_delta_idx_max, log_delta_list_base, log_delta_parameter_list_base, (u64)(freq));
              FRU128_FROM_FRU64_MULTIPLY_U64(entropy_delta, term_minus, (u64)(freq));
              FRU64_LOG_U64_NONZERO_CACHED(term_minus, log_idx_max, log_list_base, log_parameter_list_base, (u64)(freq_plus_1));
              FRU128_ADD_FRU64_LO_SELF(entropy_delta, term_minus, overflow_status);
              FRU128_SHIFT_LEFT_SELF(entropy_delta, 64-58, overflow_status);
              FRU128_SUBTRACT_FRU128_SELF(entropy, entropy_delta, overflow_status);
            }else{
              FRU128_SET_ZERO(entropy);
            }
          }
        }
        break;
      default:
/*
This must be kurtosis or variance. The change, dV, in sum-of-squares for variance is:

  dV=(|mask-U|^2)-(|mask_old-U|^2)

The change, dK, in sum-of-quartics for kurtosis is:

  dK=((|mask-U|^2)^2)-((|mask_old-U|^2)^2)

where U is the mean of the entire haystack because we're computing obtuse variance or kurtosis as opposed to the fully precise versions thereof.

This is done in a manner analagous to the variance and kurtosis bits of agnentroprox_entropy_delta_get(), so see that function for comments.
*/
        sum_quartics=agnentroprox_base->sum_quartics;
        sum_squares=agnentroprox_base->sum_squares;
        mask_unsigned=mask^mask_sign_mask;
        U128_FROM_U64_LO(delta, mask_unsigned);
        U128_SHIFT_LEFT_SELF(delta, mean_shift);
        if(U128_IS_LESS_EQUAL(delta, mean_unsigned)){
          U128_SUBTRACT_FROM_U128_SELF(delta, mean_unsigned);
        }else{
          U128_DECREMENT_SELF(delta);
          U128_SUBTRACT_U128_SELF(delta, mean_unsigned);
        }
        FRU128_FROM_FTD128(delta_power, delta);
        FRU128_MULTIPLY_FRU128_SELF(delta_power, delta_power);
        FRU128_SHIFT_RIGHT(delta_power_shifted, sweep_mask_idx_max_bit_count, delta_power);
        FRU128_ADD_FRU128_SELF(sum_squares, delta_power_shifted, overflow_status);
        if(mode==AGNENTROPROX_MODE_KURTOSIS){
          FRU128_MULTIPLY_FRU128_SELF(delta_power, delta_power);
          FRU128_SHIFT_RIGHT(delta_power_shifted, sweep_mask_idx_max_bit_count, delta_power);
          FRU128_ADD_FRU128_SELF(sum_quartics, delta_power_shifted, overflow_status);
        }
        mask_unsigned=mask_old^mask_sign_mask;
        U128_FROM_U64_LO(delta, (u64)(mask_unsigned));
        U128_SHIFT_LEFT_SELF(delta, mean_shift);
        if(U128_IS_LESS_EQUAL(delta, mean_unsigned)){
          U128_SUBTRACT_FROM_U128_SELF(delta, mean_unsigned);
        }else{
          U128_DECREMENT_SELF(delta);
          U128_SUBTRACT_U128_SELF(delta, mean_unsigned);
        }
        FRU128_FROM_FTD128(delta_power, delta);
        FRU128_MULTIPLY_FRU128_SELF(delta_power, delta_power);
        FRU128_SHIFT_RIGHT(delta_power_shifted, sweep_mask_idx_max_bit_count, delta_power);
        FRU128_SUBTRACT_FRU128_SELF(sum_squares, delta_power_shifted, overflow_status);
        if(mode==AGNENTROPROX_MODE_KURTOSIS){
          FRU128_MULTIPLY_FRU128_SELF(delta_power, delta_power);
          FRU128_SHIFT_RIGHT(delta_power_shifted, sweep_mask_idx_max_bit_count, delta_power);
          FRU128_SUBTRACT_FRU128_SELF(sum_quartics, delta_power_shifted, overflow_status);
        }
        agnentroprox_base->sum_quartics=sum_quartics;
        agnentroprox_base->sum_squares=sum_squares;
        if(mode==AGNENTROPROX_MODE_VARIANCE){
          FRU128_DIVIDE_U64(entropy, sum_squares, (u64)(sweep_mask_count), overflow_status);
          fixed_point_shift=(u8)(U128_BITS-variance_shift);
        }else{
          FRU128_MULTIPLY_FRU128(sum_squares_squared, sum_squares, sum_squares);
          if(sweep_mask_count&(sweep_mask_count-1)){
            U128_FROM_U64_LO(freq_mantissa, (u64)(sweep_mask_count));
            U128_SHIFT_LEFT_SELF(freq_mantissa, (u8)(U128_BITS-sweep_mask_idx_max_bit_count));
            FRU128_MULTIPLY_MANTISSA_U128_SELF(sum_quartics, freq_mantissa);
          }
          fixed_point_shift=0;
          while(U128_IS_LESS_EQUAL(sum_squares_squared.a, sum_quartics.b)){
            if(U128_IS_NOT_SIGNED(sum_squares_squared.b)){
              FRU128_SHIFT_LEFT_SELF(sum_squares_squared, 1, overflow_status);
            }else{
              FRU128_SHIFT_RIGHT_SELF(sum_quartics, 1);
            }
            fixed_point_shift++;
          }
          FRU128_DIVIDE_FRU128(entropy, sum_quartics, sum_squares_squared, overflow_status);
        }
        if(fixed_point_shift){
          if(fixed_point_shift<=U64_BITS){
            fixed_point_shift=(u8)(U64_BITS-fixed_point_shift);
            FRU128_SHIFT_RIGHT_SELF(entropy, fixed_point_shift);
          }else{
            fixed_point_shift=(u8)(fixed_point_shift-U64_BITS);
            FRU128_SHIFT_LEFT_SELF(entropy, fixed_point_shift, overflow_status);
          }
        }
      }
      FRU128_MEAN_TO_FTD128(entropy_mean, entropy);
    }
    if(!append_mode){
      match_status_not=U128_IS_LESS(entropy_mean, entropy_threshold);
      if(!match_status_not){
        match_status_not=fracterval_u128_rank_list_insert_descending(entropy, &match_count, &match_idx, match_idx_max_max, entropy_list_base, &entropy_threshold);
      }
    }else if(append_mode==1){
      match_status_not=U128_IS_LESS(entropy_threshold, entropy_mean);
      if(!match_status_not){
        match_status_not=fracterval_u128_rank_list_insert_ascending(entropy, &match_count, &match_idx, match_idx_max_max, entropy_list_base, &entropy_threshold);
      }
    }else{
      match_idx=match_count;
      match_status_not=1;
      if(match_count<=match_idx_max_max){
        match_status_not=0;
        entropy_list_base[match_count]=entropy;
        match_count++;
      }
    }
    if((!match_status_not)&&match_u8_idx_list_base){
      match_idx_min=match_idx;
      match_idx=match_count-1;
      while(match_idx!=match_idx_min){
        match_u8_idx_list_base[match_idx]=match_u8_idx_list_base[match_idx-1];
        match_idx--;
      }
      match_u8_idx_list_base[match_idx]=u8_idx_old;
    }
  }
  match_idx=0;
  if(mode_bit_idx==AGNENTROPROX_MODE_JSET_BIT_IDX){
    log2_recip_half=agnentroprox_base->log2_recip_half;
    do{      
      entropy=entropy_list_base[match_idx];
/*
These finalization operations are the same as in agnentroprox_jsd_get(), but for an additional 2 shifts to compensate for all the right shifts above.
*/
      FRU128_MULTIPLY_FTD128_SELF(entropy, log2_recip_half);
      FRU128_SHIFT_LEFT_SELF(entropy, U128_BITS_LOG2+3, ignored_status);
      entropy_list_base[match_idx]=entropy;
      match_idx++;
    }while(match_idx!=match_count);
  }else if(mode_bit_idx==AGNENTROPROX_MODE_LET_BIT_IDX){
    ld_coeff=agnentroprox_base->ld_coeff;
    ld_shift=agnentroprox_base->ld_shift;
    do{      
      entropy=entropy_list_base[match_idx];
/*
These finalization operations are the same as in agnentroprox_ld_get().
*/
      FRU128_SHIFT_LEFT_SELF(entropy, ld_shift, ignored_status);
      FRU128_MULTIPLY_FRU128_SELF(entropy, ld_coeff);
      FRU128_SHIFT_LEFT_SELF(entropy, 2, ignored_status);
      entropy_list_base[match_idx]=entropy;
      match_idx++;
    }while(match_idx!=match_count);
  }
/*
Write ignored_status to prevent the compiler from complaining about it not being used.
*/
  agnentroprox_base->ignored_status=ignored_status;
  *overflow_status_base=overflow_status;
  return match_count;
}

ULONG
agnentroprox_exoelasticity_transform(agnentroprox_t *agnentroprox_base, u8 append_mode, fru128 *exoelasticity_list_base, ULONG mask_idx_max, u8 *mask_list_base, ULONG match_idx_max_max, ULONG *match_u8_idx_list_base, u8 *overflow_status_base, ULONG sweep_mask_idx_max){
/*
Compute an exoelasticity transform of a mask list, given a particular sweep.

In:

  agnentroprox_base is the return value of agenentroprox_init().

  append_mode is 2 to append diventropies one at at time to *diventropy_list_base, ordered ascending by the base index of the sweep window. Else one to sort results ascending by diventropy, or zero to sort descending.

  *exoelasticity_list_base contains (match_idx_max_max+1) undefined items.
    
  mask_idx_max is one less than the number of masks in the mask list, such that if agnentroprox_init():In:overlap_status was one, then this value would need to be increased in order to account for mask overlap. For example, if the sweep contains 5 of 3-byte masks, then this value would be 4 _without_ overlap, or 12 _with_ overlap. Must not exceed agnentroprox_init():In:mask_idx_max_max. See also agnentroprox_mask_idx_max_get().

  *mask_list_base is the mask list.

  match_idx_max_max is one less than the maximum number of matches to report.

  *match_u8_idx_list_base is NULL if the sweep base indexes associated with matches can be discarded, else the base of (match_idx_max_max+1) undefined items to hold such indexes.

  *overflow_status_base is the OR-cummulative fracterval overflow status.

  sweep_mask_idx_max is one less than the number of masks in the sweep, which like mask_idx_max, must account for mask overlap if enabled.  On [0, mask_idx_max].

Out:

  Returns the number of matches found, which is simply (MIN((mask_idx_max-sweep_mask_idx_max))+1, match_idx_max_max).

  *entropy_list_base contains (return value) items which represent the matches identified during the search, sorted according to append_mode.

  *overflow_status_base is one if a fracterval overflow occurred, else unchanged.
*/
  fru128 entropy_delta;
  fru128 exoelasticity;
  u128 exoelasticity_mean;
  u128 exoelasticity_threshold;
  fru128 exoentropy;
  ULONG exo_freq;
  ULONG exo_freq_minus_1;
  ULONG exo_freq_old;
  ULONG exo_freq_old_plus_1;
  ULONG freq;
  ULONG *freq_list_base0;
  ULONG *freq_list_base1;
  ULONG freq_plus_1;
  ULONG freq_old;
  ULONG freq_old_minus_1;
  u8 granularity;
  ULONG log_delta_idx_max;
  fru64 *log_delta_list_base;
  u64 *log_delta_parameter_list_base;
  ULONG log_idx_max;
  fru64 *log_list_base;
  u64 *log_parameter_list_base;
  u32 mask;
  ULONG mask_count;
  u32 mask_max;
  u32 mask_old;
  u8 mask_u8;
  ULONG match_count;
  ULONG mask_count_plus_span;
  ULONG match_idx;
  ULONG match_idx_min;
  u8 match_status_not;
  u8 overflow_status;
  u8 overlap_status;
  fru128 shannon_entropy;
  ULONG sweep_mask_count;
  fru64 term_minus;
  fru64 term_plus;
  ULONG u8_idx;
  ULONG u8_idx_old;
  ULONG u8_idx_delta;
  ULONG u8_idx_max;

  overflow_status=*overflow_status_base;
  freq_list_base0=agnentroprox_base->freq_list_base0;
  freq_list_base1=agnentroprox_base->freq_list_base1;
  mask_max=agnentroprox_base->mask_max;
  log_idx_max=agnentroprox_base->log_idx_max;
  log_list_base=agnentroprox_base->log_list_base;
  log_parameter_list_base=agnentroprox_base->log_parameter_list_base;
  FRU128_SET_ZERO(exoentropy);
  sweep_mask_count=sweep_mask_idx_max+1;
  agnentroprox_ulong_list_zero((ULONG)(mask_max), freq_list_base0);
  agnentroprox_base->mask_count0=0;
  agnentroprox_mask_list_accrue(agnentroprox_base, 0, sweep_mask_idx_max, mask_list_base);
  shannon_entropy=agnentroprox_shannon_entropy_get(agnentroprox_base, 0, &overflow_status);
  agnentroprox_ulong_list_zero((ULONG)(mask_max), freq_list_base1);
  agnentroprox_base->mask_count1=0;
  agnentroprox_mask_list_accrue(agnentroprox_base, 1, mask_idx_max, mask_list_base);
  agnentroprox_mask_list_subtract(agnentroprox_base);
  mask_count=agnentroprox_base->mask_count1;
  mask_count_plus_span=mask_count+mask_max+1;
/*
Evaluate the Shannon entropy in nats, over all masks M, as implied by the frequencies F0[M] at freq_list_base0, but using the agnostic probabilities implied by the frequencies F1[M] at freq_list_base1:

  S=(Q0*log(Q1+Z))-Σ(M=(0, Z-1), F0[M]*log(F1[M]+1))

where:

  F[M]=MIN((frequency of mask M), 1)
  M=mask
  Q0=sweep_mask_count
  Q1=(mask_count_plus_span (after subtracting Q0))
  Z=mask_span
*/
  FRU64_LOG_U64_NONZERO_CACHED(term_plus, log_idx_max, log_list_base, log_parameter_list_base, (u64)(mask_count_plus_span));
  FRU128_FROM_FRU64_MULTIPLY_U64(exoentropy, term_plus, (u64)(sweep_mask_count));
  FRU128_SHIFT_LEFT_SELF(exoentropy, 64-58, overflow_status);
  mask=0;
  do{
    freq=freq_list_base1[mask]+1;
    FRU64_LOG_U64_NONZERO_CACHED(term_minus, log_idx_max, log_list_base, log_parameter_list_base, (u64)(freq));
    freq=freq_list_base0[mask];
    FRU128_FROM_FRU64_MULTIPLY_U64(entropy_delta, term_minus, (u64)(freq));
    FRU128_SHIFT_LEFT_SELF(entropy_delta, 64-58, overflow_status);
    FRU128_SUBTRACT_FRU128_SELF(exoentropy, entropy_delta, overflow_status);
  }while((mask++)!=mask_max);
  FRU128_DIVIDE_FRU128(exoelasticity, shannon_entropy, exoentropy, overflow_status);
  FRU128_MEAN_TO_FTD128(exoelasticity_mean, exoelasticity);
  match_count=1;
  if(match_u8_idx_list_base){
    match_u8_idx_list_base[0]=0;
  }
  exoelasticity_list_base[0]=exoelasticity;
  U128_FROM_BOOL(exoelasticity_threshold, append_mode);
  if(!match_idx_max_max){
    exoelasticity_threshold=exoelasticity_mean;
  }
  granularity=agnentroprox_base->granularity;
  log_delta_idx_max=agnentroprox_base->log_delta_idx_max;
  log_delta_list_base=agnentroprox_base->log_delta_list_base;
  log_delta_parameter_list_base=agnentroprox_base->log_delta_parameter_list_base;
  overlap_status=agnentroprox_base->overlap_status;
  u8_idx_delta=(u8)((u8)(granularity*(!overlap_status))+1);
  u8_idx=sweep_mask_count*u8_idx_delta;
  u8_idx_max=mask_idx_max*u8_idx_delta;
  u8_idx_old=0;
  while(u8_idx<=u8_idx_max){
    mask=mask_list_base[u8_idx];
    mask_old=mask_list_base[u8_idx_old];
    if(granularity){
      mask_u8=mask_list_base[u8_idx+U16_BYTE_MAX];
      mask|=(u32)(mask_u8)<<U8_BITS;
      mask_u8=mask_list_base[u8_idx_old+U16_BYTE_MAX];
      mask_old|=(u32)(mask_u8)<<U8_BITS;
      if(U16_BYTE_MAX<granularity){
        mask_u8=mask_list_base[u8_idx+U24_BYTE_MAX];
        mask|=(u32)(mask_u8)<<U16_BITS;
        mask_u8=mask_list_base[u8_idx_old+U24_BYTE_MAX];
        mask_old|=(u32)(mask_u8)<<U16_BITS;
        if(U24_BYTE_MAX<granularity){
          mask_u8=mask_list_base[u8_idx+U32_BYTE_MAX];
          mask|=(u32)(mask_u8)<<U24_BITS;
          mask_u8=mask_list_base[u8_idx_old+U32_BYTE_MAX];
          mask_old|=(u32)(mask_u8)<<U24_BITS;
        }
      }
    }
/*
Increment the u8 indexes now, so that if we get a match, u8_idx_old will be the first index of the new sweep, as opposed to the old one.
*/
    u8_idx+=u8_idx_delta;
    u8_idx_old+=u8_idx_delta;
    if(mask!=mask_old){
      freq=freq_list_base0[mask];
      freq_old=freq_list_base0[mask_old];
      freq_old_minus_1=freq_old-1;
      freq_plus_1=freq+1;
      freq_list_base0[mask]=freq_plus_1;
      freq_list_base0[mask_old]=freq_old_minus_1;
      exo_freq=freq_list_base1[mask];
      exo_freq_old=freq_list_base1[mask_old];
      exo_freq_minus_1=exo_freq-1;
      exo_freq_old_plus_1=exo_freq_old+1;
      freq_list_base1[mask]=exo_freq_minus_1;
      freq_list_base1[mask_old]=exo_freq_old_plus_1;
/*
The exoentropy difference, dE, is:

  dE=(freq*log(exo_freq+1))+(freq_old*log(exo_freq_old+1)))-((freq+1)*log(exo_freq))-((freq_old-1)*log(exo_freq_old+2))
  dE=freq*(log(exo_freq+1)-log(exo_freq))+log(exo_freq_old+1)-((freq_old-1)*(log(exo_freq_old+2)-log(exo_freq_old+1)))-log(exo_freq)
  dE=(freq*log_delta(exo_freq))+log(exo_freq_old+1)-((freq_old-1)*log_delta(exo_freq_old+1))-log(exo_freq)
  dE=(freq*log_delta(exo_freq))+log(exo_freq_old_plus_1)-(freq_old_minus_1*log_delta(exo_freq_old_plus_1))-log(exo_freq)
*/
      FRU64_LOG_DELTA_U64_NONZERO_CACHED(term_plus, log_delta_idx_max, log_delta_list_base, log_delta_parameter_list_base, (u64)(exo_freq));
      FRU128_FROM_FRU64_MULTIPLY_U64(entropy_delta, term_plus, (u64)(freq));
      FRU64_LOG_U64_NONZERO_CACHED(term_plus, log_idx_max, log_list_base, log_parameter_list_base, (u64)(exo_freq_old_plus_1));
      FRU128_ADD_FRU64_LO_SELF(entropy_delta, term_plus, overflow_status);
      FRU128_SHIFT_LEFT_SELF(entropy_delta, 64-58, overflow_status);
      FRU128_ADD_FRU128_SELF(exoentropy, entropy_delta, overflow_status);
      FRU64_LOG_DELTA_U64_NONZERO_CACHED(term_minus, log_delta_idx_max, log_delta_list_base, log_delta_parameter_list_base, (u64)(exo_freq_old_plus_1));
      FRU128_FROM_FRU64_MULTIPLY_U64(entropy_delta, term_minus, (u64)(freq_old_minus_1));
      FRU64_LOG_U64_NONZERO_CACHED(term_minus, log_idx_max, log_list_base, log_parameter_list_base, (u64)(exo_freq));
      FRU128_ADD_FRU64_LO_SELF(entropy_delta, term_minus, overflow_status);
      FRU128_SHIFT_LEFT_SELF(entropy_delta, 64-58, overflow_status);
      FRU128_SUBTRACT_FRU128_SELF(exoentropy, entropy_delta, overflow_status);
/*
The Shannon entropy difference, dS, is:

  dS=(freq_old*log(freq_old))+(freq*log(freq))-(freq_old_minus_1*log(freq_old_minus_1))-(freq_plus_1*log(freq_plus_1))
  dS=((freq_old_minus_1*(log(freq_old)-log(freq_old_minus_1)))+log(freq_old))-((freq*(log(freq_plus_1)-log(freq)))+log(freq_plus_1))
  dS=((freq_old_minus_1*log_delta(freq_old_minus_1))+log(freq_old))-((freq*log_delta(freq))+log(freq_plus_1))

where any log(0) or log_delta(0) is treated as though it equals zero.
*/
      if(freq_old!=freq_plus_1){
        if(freq_old_minus_1){
          FRU64_LOG_DELTA_U64_NONZERO_CACHED(term_plus, log_delta_idx_max, log_delta_list_base, log_delta_parameter_list_base, (u64)(freq_old_minus_1));
          FRU128_FROM_FRU64_MULTIPLY_U64(entropy_delta, term_plus, (u64)(freq_old_minus_1));
          FRU64_LOG_U64_NONZERO_CACHED(term_plus, log_idx_max, log_list_base, log_parameter_list_base, (u64)(freq_old));
          FRU128_ADD_FRU64_LO_SELF(entropy_delta, term_plus, overflow_status);
          FRU128_SHIFT_LEFT_SELF(entropy_delta, 64-58, overflow_status);
          FRU128_ADD_FRU128_SELF(shannon_entropy, entropy_delta, overflow_status);
        }
        if(freq){
          if(freq_plus_1!=sweep_mask_count){
            FRU64_LOG_DELTA_U64_NONZERO_CACHED(term_minus, log_delta_idx_max, log_delta_list_base, log_delta_parameter_list_base, (u64)(freq));
            FRU128_FROM_FRU64_MULTIPLY_U64(entropy_delta, term_minus, (u64)(freq));
            FRU64_LOG_U64_NONZERO_CACHED(term_minus, log_idx_max, log_list_base, log_parameter_list_base, (u64)(freq_plus_1));
            FRU128_ADD_FRU64_LO_SELF(entropy_delta, term_minus, overflow_status);
            FRU128_SHIFT_LEFT_SELF(entropy_delta, 64-58, overflow_status);
            FRU128_SUBTRACT_FRU128_SELF(shannon_entropy, entropy_delta, overflow_status);
          }else{
            FRU128_SET_ZERO(shannon_entropy);
          }
        }
      }
      FRU128_DIVIDE_FRU128(exoelasticity, shannon_entropy, exoentropy, overflow_status);
      FRU128_MEAN_TO_FTD128(exoelasticity_mean, exoelasticity);
    }
    if(!append_mode){
      match_status_not=U128_IS_LESS(exoelasticity_mean, exoelasticity_threshold);
      if(!match_status_not){
        match_status_not=fracterval_u128_rank_list_insert_descending(exoelasticity, &match_count, &match_idx, match_idx_max_max, exoelasticity_list_base, &exoelasticity_threshold);
      }
    }else if(append_mode==1){
      match_status_not=U128_IS_LESS(exoelasticity_threshold, exoelasticity_mean);
      if(!match_status_not){
        match_status_not=fracterval_u128_rank_list_insert_ascending(exoelasticity, &match_count, &match_idx, match_idx_max_max, exoelasticity_list_base, &exoelasticity_threshold);
      }
    }else{
      match_idx=match_count;
      match_status_not=1;
      if(match_count<=match_idx_max_max){
        match_status_not=0;
        exoelasticity_list_base[match_count]=exoelasticity;
        match_count++;
      }
    }
    if((!match_status_not)&&match_u8_idx_list_base){
      match_idx_min=match_idx;
      match_idx=match_count-1;
      while(match_idx!=match_idx_min){
        match_u8_idx_list_base[match_idx]=match_u8_idx_list_base[match_idx-1];
        match_idx--;
      }
      match_u8_idx_list_base[match_idx]=u8_idx_old;
    }
  }
  *overflow_status_base=overflow_status;
  return match_count;
}

void *
agnentroprox_free(void *base){
/*
To maximize portability and debuggability, this is the only function in which Agnentroprox calls free().

In:

  base is the base of a memory region to free. May be NULL.

Out:

  Returns NULL so that the caller can easily maintain the good practice of NULLing out invalid pointers.

  *base is freed.
*/
  DEBUG_FREE_PARANOID(base);
  return NULL;
}

agnentroprox_t *
agnentroprox_free_all(agnentroprox_t *agnentroprox_base){
/*
Free all private storage.

In:

  agnentroprox_base is the return value of agnentroprox_init().

Out:

  Returns NULL so that the caller can easily maintain the good practice of NULLing out invalid pointers.

  *agnentroprox_base and all its child allocations are freed.
*/
  if(agnentroprox_base){
    loggamma_free(agnentroprox_base->loggamma_parameter_list_base);
    loggamma_free(agnentroprox_base->loggamma_list_base);
    fracterval_u128_free(agnentroprox_base->log_u128_parameter_list_base);
    fracterval_u128_free(agnentroprox_base->log_u128_list_base);
    fracterval_u64_free(agnentroprox_base->log_parameter_list_base);
    fracterval_u64_free(agnentroprox_base->log_list_base);
    fracterval_u64_free(agnentroprox_base->log_delta_parameter_list_base);
    fracterval_u64_free(agnentroprox_base->log_delta_list_base);
    poissocache_free_all(agnentroprox_base->poissocache_base);
    agnentroprox_free(agnentroprox_base->freq_list_base1);
    agnentroprox_free(agnentroprox_base->freq_list_base0);
    agnentroprox_base=agnentroprox_free(agnentroprox_base);
  }
  return agnentroprox_base;
}

agnentroprox_t *
agnentroprox_init(u32 build_break_count, u32 build_feature_count, u8 granularity, loggamma_t *loggamma_base, ULONG mask_idx_max_max, u32 mask_max_max, u16 mode_bitmap, u8 overlap_status, ULONG sweep_mask_idx_max_max){
/*
Verify that the source code is sufficiently updated and initialize private storage.

In:

  build_break_count is the caller's most recent knowledge of AGNENTROPROX_BUILD_BREAK_COUNT, which will fail if the caller is unaware of all critical updates.

  build_feature_count is the caller's most recent knowledge of AGNENTROPROX_BUILD_FEATURE_COUNT, which will fail if this library is not up to date with the caller's expectations.

  granularity is the size of each mask, less one. Limits may change from one version of this code to then next, so there is no explicit restriction.

  loggamma_base is the return value of loggamma_init().

  mask_idx_max_max is the maximum possible number of masks that could possibly occur within a haystack, less one. If overlap_status is set, then this value should be the maximum possible size of a haystack (in bytes, not masks), less (granularity+1). See also agnentroprox_mask_idx_max_get().

  mask_max_max is the maximum possible mask, which must not exeed (((granularity+1)<<U8_BITS_LOG2)-1). For example, when dealing with bytes, this would be U8_MAX. Due to implied allocation requirements, only compiling with (-D_64_) will allow the full u32 range. The limits on it are tricky to compute so this function tests them explicitly. Zero is tested for and disallowed because it creates more trouble than it's worth. For AGNENTROPROX_MODE_KURTOSIS and AGNENTROPROX_MODE_VARIANCE, this value must be one ones less than a power of 2, due to assumptions about where signed and unsigned masks are centered. The reason that this is mask_max_max and not mask_max is that agnentroprox_mask_list_densify() may result in mask_max to being less than mask_max_max for a given mask list. If densification is not desired, then this value is equivalent to the usual mask_max.

  mode_bitmap is the OR of (AGNENTROPROX_MODE)s which express the caller's intended target services. If the caller breaks the promise, results will still be correct, but computed more slowly due to suboptimal math cache configuration.

  overlap_status is zero to load each mask separately, or one to load them overlapping at successive byte addresses. It should be zero if masks should be considered as purely probabilistic in origin, or one if they are contextually dependent in some Bayesian manner. Note that this value has no effect if granularity is zero. Must be zero if mode_bitmap has AGNENTROPROX_MODE_KURTOSIS or AGNENTROPROX_MODE_VARIANCE set, as overlap in those modes would be nonsensical due to masks having meaningful magnitudes.

  sweep_mask_idx_max_max is the maximum possible mask index of any transform sweep which the caller intends to invoke. If in doubt, set it equal to mask_idx_max_max. This value has no correctness consequences; it's merely a performance hint for the size optimization of the math caches. On [0, mask_idx_max_max].

Out:

  Returns NULL if (build_break_count!=AGNENTROPROX_BUILD_BREAK_COUNT); (build_feature_count>AGNENTROPROX_BUILD_FEATURE_COUNT); there is insufficient memory; or one of the input parameters falls outside its valid range. Else, returns the base of an agnentroprox_t to be used with other Agnentroprox functions. It must be freed with agnentroprox_free_all(); *loggamma_base must not be freed prior to doing so.
*/
  agnentroprox_t *agnentroprox_base;
  ULONG cache_idx_max;
  ULONG *freq_list_base0;
  ULONG *freq_list_base1;
  ULONG log_delta_idx_max;
  fru64 *log_delta_list_base;
  u64 *log_delta_parameter_list_base;
  ULONG log_idx_max;
  fru64 *log_list_base;
  u64 *log_parameter_list_base;
  ULONG log_u128_idx_max;
  fru128 *log_u128_list_base;
  u128 *log_u128_parameter_list_base;
  u128 log2_recip_half;
  ULONG loggamma_idx_max;
  fru128 *loggamma_list_base;
  u64 *loggamma_parameter_list_base;
  ULONG mask_count_max_max;
  ULONG mask_count_plus_span_max_max;
  u32 mask_sign_mask;
  u64 mask_span;
  fru128 mask_span_log;
  u8 mask_span_power_of_2_status;
  u8 mean_shift;
  u8 msb;
  u32 n;
  u32 n_minus_2;
  poissocache_t *poissocache_base;
  ULONG poissocache_item_idx_max;
  u8 overflow_status;
  u8 status;

  agnentroprox_base=NULL;
  overflow_status=0;
  status=fracterval_u128_init(FRU128_BUILD_BREAK_COUNT_EXPECTED, 1);
  status=(u8)(status|fracterval_u64_init(FRU64_BUILD_BREAK_COUNT_EXPECTED, 0));
  status=(u8)(status|(AGNENTROPROX_BUILD_FEATURE_COUNT<build_feature_count));
  status=(u8)(status|(build_break_count!=AGNENTROPROX_BUILD_BREAK_COUNT));
  status=(u8)(status|(!mask_max_max));
  mask_count_max_max=mask_idx_max_max+1;
  status=(u8)(status|(!mask_count_max_max));
  mask_span=(u64)(mask_max_max)+1;
  mask_count_plus_span_max_max=(ULONG)(mask_count_max_max+mask_span);
  status=(u8)(status|(mask_count_plus_span_max_max<=mask_span));
  status=(u8)(status|(U32_BYTE_MAX<granularity));
  granularity=(u8)(granularity&U32_BYTE_MAX);
  status=(u8)(status|((u32)((1U<<(granularity<<U8_BITS_LOG2)<<U8_BITS)-1)<mask_max_max));
  status=(u8)(status|(mask_idx_max_max<sweep_mask_idx_max_max));
  mask_span_power_of_2_status=!(mask_max_max&(mask_max_max+1));
  if(mode_bitmap&(AGNENTROPROX_MODE_KURTOSIS|AGNENTROPROX_MODE_VARIANCE)){
    status=(u8)(status|(!mask_span_power_of_2_status)|overlap_status);
  }
/*
For logfreedom computation, we need a list of pairs (frequency, population) which give the populations of various frequencies. The maximum possible number of _unique_ nonzero frequencies with nonzero population occurs when there is one mask with frequency one, one with frequency 2, one with frequency 3, etc. If the greatest such frequency is N, then the minimum mask count needed to construct this scenario is (1+2+3+...+N), which is just ((N*(N+1))>>1). Furthermore the maximum mask count guaranteed to have at most N unique nonzero frequencies of nonzero population is (((N*(N+1))>>1)+N). However, up to (N+1) unique frequencies could have nonzero population because frequency zero might have nonzero population. Therefore N is in fact the maximum possible index of a list of such pairs. Unfortunately, due to the way in which logfreedom is evaluated, it's possible that the number of nonzero populations would be as much as 2 greater than its theoretical maximum, at any given time. Given all this, find the minimum such N such that:

  (mask_idx_max_max+1)<=((((N-2)*((N-2)+1))>>1)+(N-2))

Technically, we should use sweep_mask_idx_max_max instead of mask_idx_max_max, but the former is defined on In to be speculative, so it wouldn't be safe to do so. This is acceptable because N merely scales are the root of the latter. Furthermore, (N-2) need not exceed mask_max_max because at most (mask_max_max+1) unique masks cannot possibly give rise to more than equally many unique frequencies of nonzero population (but we could still temporarily overrun by 2). So N can be clipped to any value small enough such that:

  (mask_max_max+1)<=(N-2)
  mask_max_max<(N-2)

And finally, it's good enough to find N which is one less than a power of 2, which is simple and fast. In fact, this is required for the correct operation of Poissocache, as well as all the result caches which inherit poissocache_item_idx_max as their maximum index.
*/
  n=U32_MAX;
  do{
    n_minus_2=n-2;
    if((n_minus_2<=mask_max_max)&&((((((u64)(n_minus_2)+1)*n_minus_2)>>1)+n_minus_2)<(mask_idx_max_max+1))){
      break;
    }
    n>>=1;
  }while(1<n);
  status=(u8)(status|(n==U32_MAX));
/*
N was shifted to the right by one bit too many (unless perhaps (N==3), in which case we might as well double it anyway). Roll back one shift. We will then use it to allocate a Poisson cache below.
*/
  n=(n<<1)+1;
  poissocache_item_idx_max=n;
  if(!status){
/*
Allocate private storage. We need to use calloc() instead of malloc() because if there's an allocation failure, we'll free all the list bases below. If one of those bases happens to be uninitialized, then it had better be zero (NULL) so as not to cause an instruction fault. (Technically, NULL can be nonzero, but try finding one platform where that was ever the case.)
*/
    agnentroprox_base=DEBUG_CALLOC_PARANOID((ULONG)(sizeof(agnentroprox_t)));
    if(agnentroprox_base){
      msb=U32_BIT_MAX;
      while(!(mask_max_max>>msb)){
        msb--;
      }
      mask_sign_mask=1U<<msb;
      mean_shift=(u8)(U128_BIT_MAX-msb);
      agnentroprox_base->loggamma_base=loggamma_base;
      agnentroprox_base->mask_max=mask_max_max;
      agnentroprox_base->mask_max_max=mask_max_max;
      agnentroprox_base->mask_max_msb=msb;
      agnentroprox_base->mask_sign_mask=mask_sign_mask;
      agnentroprox_base->granularity=granularity;
      agnentroprox_base->mean_shift=mean_shift;
      agnentroprox_base->overlap_status=overlap_status;
      freq_list_base0=agnentroprox_ulong_list_malloc((ULONG)(mask_max_max));
      status=(u8)(status|!freq_list_base0);
      agnentroprox_base->freq_list_base0=freq_list_base0;
      freq_list_base1=agnentroprox_ulong_list_malloc((ULONG)(mask_max_max));
      status=(u8)(status|!freq_list_base1);
      agnentroprox_base->freq_list_base1=freq_list_base1;
      poissocache_base=poissocache_init(POISSOCACHE_BUILD_BREAK_COUNT_EXPECTED, 0, poissocache_item_idx_max);
      status=(u8)(status|!poissocache_base);
      agnentroprox_base->poissocache_base=poissocache_base;
      if(!status){
/*
Allocate math caches for previously computed log, log delta, and loggamma fractervals. Start with the maximum reasonable expectation of the number of unique results we would need to recall at any given time, which is essentially the greater of poissocache_item_idx_max and sweep_mask_idx_max_max, then back off exponentially until we succeed. cache_idx_max will be one less than this value. We must set it to one less than a power of 2 because the cache functions will use it as an index AND mask.

Caches which are not required shall have single-item allocations just to satisfy accessability expectations.
*/
        cache_idx_max=MAX(poissocache_item_idx_max, sweep_mask_idx_max_max);
        cache_idx_max|=1;
        msb=U32_BIT_MAX;
        while(!(cache_idx_max>>msb)){
          msb--;
        }
        cache_idx_max=(ULONG)((2ULL<<msb)-1);
        log_idx_max=0;
        log_delta_idx_max=0;
        log_u128_idx_max=0;
        loggamma_idx_max=0;
        if(mode_bitmap&(AGNENTROPROX_MODE_DIVENTROPY|AGNENTROPROX_MODE_EXOELASTICITY|AGNENTROPROX_MODE_EXOENTROPY|AGNENTROPROX_MODE_LDT|AGNENTROPROX_MODE_LET|AGNENTROPROX_MODE_SHANNON)){
          log_delta_idx_max=cache_idx_max;
        }
        if(mode_bitmap&(AGNENTROPROX_MODE_AGNENTROPY|AGNENTROPROX_MODE_DIVENTROPY|AGNENTROPROX_MODE_EXOELASTICITY|AGNENTROPROX_MODE_EXOENTROPY|AGNENTROPROX_MODE_LDT|AGNENTROPROX_MODE_LET|AGNENTROPROX_MODE_LOGFREEDOM|AGNENTROPROX_MODE_SHANNON)){
          log_idx_max=cache_idx_max;
        }
        if(mode_bitmap&AGNENTROPROX_MODE_JSDT){
/*
Due to the way that the Jensen-Shannon divergence works, we end up taking the logs of pairs of frequencies, which means that in theory the result cache could require the square as many items as with other transforms. Grab whatever we can get, up to that ceiling.
*/
          log_u128_idx_max=ULONG_MAX;
          if(cache_idx_max<UHALF_MAX){
            log_u128_idx_max=cache_idx_max+1;
            log_u128_idx_max*=log_u128_idx_max;
            log_u128_idx_max--;
          }
        }
        if(mode_bitmap&(AGNENTROPROX_MODE_AGNENTROPY|AGNENTROPROX_MODE_LOGFREEDOM)){
          loggamma_idx_max=cache_idx_max;
        }
        do{
          log_delta_parameter_list_base=fracterval_u64_log_u64_cache_init(log_delta_idx_max, &log_delta_list_base);
          status=!log_delta_parameter_list_base;
          log_parameter_list_base=fracterval_u64_log_u64_cache_init(log_idx_max, &log_list_base);
          status=(u8)(status|!log_parameter_list_base);
          log_u128_parameter_list_base=fracterval_u128_log_u128_cache_init(log_u128_idx_max, &log_u128_list_base);
          status=(u8)(status|!log_u128_parameter_list_base);
          loggamma_parameter_list_base=loggamma_u64_cache_init(loggamma_idx_max, &loggamma_list_base);
          status=(u8)(status|!loggamma_parameter_list_base);
          if(!status){
            U128_FROM_U64_PAIR(log2_recip_half, FTD128_2LOG2_RECIP_FLOOR_LO, FTD128_2LOG2_RECIP_FLOOR_HI);
            agnentroprox_base->log_delta_idx_max=log_delta_idx_max;
            agnentroprox_base->log_delta_list_base=log_delta_list_base;
            agnentroprox_base->log_delta_parameter_list_base=log_delta_parameter_list_base;
            agnentroprox_base->log_idx_max=log_idx_max;
            agnentroprox_base->log_list_base=log_list_base;
            agnentroprox_base->log_parameter_list_base=log_parameter_list_base;
            agnentroprox_base->log_u128_idx_max=log_u128_idx_max;
            agnentroprox_base->log_u128_list_base=log_u128_list_base;
            agnentroprox_base->log_u128_parameter_list_base=log_u128_parameter_list_base;
            agnentroprox_base->log2_recip_half=log2_recip_half;
            agnentroprox_base->loggamma_idx_max=loggamma_idx_max;
            agnentroprox_base->loggamma_list_base=loggamma_list_base;
            agnentroprox_base->loggamma_parameter_list_base=loggamma_parameter_list_base;
/*
Precompute a fracterval giving the log(mask_max_max+1), which is used to compute raw entropy.
*/
            FRU128_LOG_U64(mask_span_log, mask_span, overflow_status);
/*
Convert from 6.122 to 6.64 fixed-point.
*/
            FRU128_SHIFT_RIGHT_SELF(mask_span_log, 122-64);
            agnentroprox_base->mask_span_log=mask_span_log;
            break;
          }else{
            loggamma_free(loggamma_parameter_list_base);
            loggamma_free(loggamma_list_base);
            fracterval_u128_free(log_u128_parameter_list_base);
            fracterval_u128_free(log_u128_list_base);
            fracterval_u64_free(log_parameter_list_base);
            fracterval_u64_free(log_list_base);
            fracterval_u64_free(log_delta_parameter_list_base);
            fracterval_u64_free(log_delta_list_base);
/*
Shrink one of the caches by a factor of 2, ordered so as to cause minimal performance degradation.
*/
            if(log_delta_idx_max==loggamma_idx_max){
              loggamma_idx_max>>=1;
            }else if(log_delta_idx_max==log_idx_max){
              log_idx_max>>=1;
            }else{
              log_delta_idx_max>>=1;
            }
          }
        }while(log_delta_idx_max|log_idx_max|loggamma_idx_max);
      }
      if(!status){
        agnentroprox_ulong_list_zero((ULONG)(mask_max_max), freq_list_base0);
        agnentroprox_ulong_list_zero((ULONG)(mask_max_max), freq_list_base1);
      }else{
        agnentroprox_base=agnentroprox_free_all(agnentroprox_base);
      }
    }
  }
  return agnentroprox_base;
}

fru128
agnentroprox_jsd_get(agnentroprox_t *agnentroprox_base, u8 exo_status, ULONG haystack_mask_idx_max, u8 *haystack_mask_list_base){
/*
Compute (1-(normalized Jensen-Shannon divergence)) between needle and haystack frequency lists. (It's commutative so the terms "needle" and "haystack" are only used for the sake of consistency with other functions in which direction matters.)

In:

  The needle frequency list must have been preloaded with agnentroprox_needle_mask_list_load() prior to calling this function, with no intervening alterations.

  agnentroprox_base is the return value of agenentroprox_init().

  exo_status is one to assume that the needle is actually the subset of the haystack based at index zero of the latter, and should therefore be excluded from the haystack itself; in this case the number of masks in the preloaded needle must not exceed haystack_mask_idx_max. Else zero.

  haystack_mask_idx_max is one less than the number of masks in the haystack, such that if agnentroprox_init():In:overlap_status was one, then this value would need to be increased in order to account for mask overlap. For example, if the sweep contains 5 of 3-byte masks, then this value would be 4 _without_ overlap, or 12 _with_ overlap. Must not exceed agnentroprox_init():In:mask_idx_max_max. See also agnentroprox_mask_idx_max_get().

  *haystack_mask_list_base is the haystack.

Out:

  Returns (1-(normalized Jensen-Shannon divergence)) between a needle and a haystack.
*/
  fru128 fh_over_qh_half;
  fru128 fn_over_qn_half;
  fru128 fh_over_qh_plus_fn_over_qn_half;
  ULONG haystack_freq;
  ULONG *haystack_freq_list_base;
  ULONG haystack_mask_count;
  fru128 haystack_mask_count_log;
  u128 haystack_mask_count_recip_half;
  u8 ignored_status;
  fru128 jsd;
  fru128 jsd_minus_half;
  fru128 jsd_plus_half;
  fru128 log;
  ULONG log_u128_idx_max;
  fru128 *log_u128_list_base;
  u128 *log_u128_parameter_list_base;
  u128 log2_recip_half;
  u32 mask;
  u64 mask_count_x2;
  u32 mask_max;
  ULONG needle_freq;
  ULONG *needle_freq_list_base;
  ULONG needle_mask_count;
  fru128 needle_mask_count_log;
  u128 needle_mask_count_recip_half;
  fru128 term;
  u128 uint0;
  u128 uint1;
/*
In this function, we ignore returned overflow status (via ignored_status) because the JSD is naturally on [0, 1], so saturation already does the right thing.
*/
  haystack_freq_list_base=agnentroprox_base->freq_list_base1;
  mask_max=agnentroprox_base->mask_max;
  ignored_status=0;
  agnentroprox_base->mask_count1=0;
  agnentroprox_ulong_list_zero((ULONG)(mask_max), haystack_freq_list_base);
  agnentroprox_mask_list_accrue(agnentroprox_base, 1, haystack_mask_idx_max, haystack_mask_list_base);
  if(exo_status){
    agnentroprox_mask_list_subtract(agnentroprox_base);
  }
  haystack_mask_count=agnentroprox_base->mask_count1;
  log_u128_idx_max=agnentroprox_base->log_u128_idx_max;
  log_u128_list_base=agnentroprox_base->log_u128_list_base;
  log_u128_parameter_list_base=agnentroprox_base->log_u128_parameter_list_base;
  needle_freq_list_base=agnentroprox_base->freq_list_base0;
  needle_mask_count=agnentroprox_base->mask_count0;
/*
Compute (1-(normalized Jensen-Shannon divergence)) according to the method described for JSD(N, S, Z) in http://vixra.org/abs/1710.0261 .
*/
  U128_FROM_U64_LO(uint0, (u64)(haystack_mask_count));
  FRU128_LOG_U128_NONZERO_CACHED(haystack_mask_count_log, log_u128_idx_max, log_u128_list_base, log_u128_parameter_list_base, uint0);
  mask_count_x2=(u64)(haystack_mask_count)<<1;
  FTD128_RECIPROCAL_U64_SATURATE(haystack_mask_count_recip_half, mask_count_x2, ignored_status);
  U128_FROM_U64_LO(uint0, (u64)(needle_mask_count));
  FRU128_LOG_U128_NONZERO_CACHED(needle_mask_count_log, log_u128_idx_max, log_u128_list_base, log_u128_parameter_list_base, uint0);
  mask_count_x2=(u64)(needle_mask_count)<<1;
  FTD128_RECIPROCAL_U64_SATURATE(needle_mask_count_recip_half, mask_count_x2, ignored_status);
  FRU128_ADD_FRU128(jsd, haystack_mask_count_log, needle_mask_count_log, ignored_status);
  FRU128_SHIFT_RIGHT_SELF(jsd, 1);
  FRU128_SET_ZERO(jsd_minus_half);
  FRU128_SET_ZERO(jsd_plus_half);
  mask=0;
  agnentroprox_base->haystack_mask_count_recip_half=haystack_mask_count_recip_half;
  agnentroprox_base->needle_mask_count_recip_half=needle_mask_count_recip_half;
  do{
    haystack_freq=haystack_freq_list_base[mask];
    needle_freq=needle_freq_list_base[mask];
    if(haystack_freq|needle_freq){
      FRU128_SET_ZERO(fh_over_qh_half);
      if(haystack_freq){
        U128_FROM_U64_LO(uint0, (u64)(haystack_freq));
        FRU128_LOG_U128_NONZERO_CACHED(log, log_u128_idx_max, log_u128_list_base, log_u128_parameter_list_base, uint0);
        FRU128_FROM_FTD128_U64_NONZERO_PRODUCT(fh_over_qh_half, haystack_mask_count_recip_half, (u64)(haystack_freq), ignored_status);
        FRU128_MULTIPLY_FRU128(term, fh_over_qh_half, log);
        FRU128_ADD_FRU128_SELF(jsd_minus_half, term, ignored_status);
      }
      FRU128_SET_ZERO(fn_over_qn_half);
      if(needle_freq){
        U128_FROM_U64_LO(uint0, (u64)(needle_freq));
        FRU128_LOG_U128_NONZERO_CACHED(log, log_u128_idx_max, log_u128_list_base, log_u128_parameter_list_base, uint0);
        FRU128_FROM_FTD128_U64_NONZERO_PRODUCT(fn_over_qn_half, needle_mask_count_recip_half, (u64)(needle_freq), ignored_status);
        FRU128_MULTIPLY_FRU128(term, fn_over_qn_half, log);
        FRU128_ADD_FRU128_SELF(jsd_minus_half, term, ignored_status);
      }
      FRU128_ADD_FRU128(fh_over_qh_plus_fn_over_qn_half, fh_over_qh_half, fn_over_qn_half, ignored_status);
      U128_FROM_U64_PRODUCT(uint0, (u64)(haystack_freq), (u64)(needle_mask_count));
      U128_FROM_U64_PRODUCT(uint1, (u64)(haystack_mask_count), (u64)(needle_freq));
      U128_ADD_U128_SELF(uint0, uint1);
      FRU128_LOG_U128_NONZERO_CACHED(log, log_u128_idx_max, log_u128_list_base, log_u128_parameter_list_base, uint0);
      FRU128_MULTIPLY_FRU128(term, fh_over_qh_plus_fn_over_qn_half, log);
      FRU128_ADD_FRU128_SELF(jsd_plus_half, term, ignored_status);
    }
  }while((mask++)!=mask_max);
  FRU128_SUBTRACT_FROM_FRU128_SELF(jsd, jsd_plus_half, ignored_status);
  FRU128_SUBTRACT_FRU128_SELF(jsd, jsd_minus_half, ignored_status);
  log2_recip_half=agnentroprox_base->log2_recip_half;
/*
We need to multiply jsd by ((2^U128_BITS_LOG2)/(ln 2)), for normalization and because the log computations produced results in 7.121 fixed-point. We already divided by 2 simply by virtue of halving all the relevant coefficients. So multiply it by the reciprocal of (2 ln 2), then shift it left by (U128_BITS_LOG2+1). But store the result to (agnentroprox_base->entropy) first, because agnentroprox_jsd_transform() may have called us, and it assumes that that value is in this uncooked but proportionally correct form; doing this slightly accelerates searches.
*/
  agnentroprox_base->entropy=jsd;
  FRU128_MULTIPLY_FTD128_SELF(jsd, log2_recip_half);
  FRU128_SHIFT_LEFT_SELF(jsd, U128_BITS_LOG2+1, ignored_status);
/*
Write ignored_status to prevent the compiler from complaining about it not being used.
*/
  agnentroprox_base->ignored_status=ignored_status;
  return jsd;
}

ULONG
agnentroprox_jsd_transform(agnentroprox_t *agnentroprox_base, u8 append_mode, ULONG haystack_mask_idx_max, u8 *haystack_mask_list_base, fru128 *jsd_list_base, ULONG match_idx_max_max, ULONG *match_u8_idx_list_base, ULONG sweep_mask_idx_max){
/*
Compute the (1-(normalized Jensen-Shannon divergence)) transform of a haystack with respect to a preloaded needle frequency list, given a particular sweep.

In:

  The needle frequency list must have been preloaded with agnentroprox_needle_mask_list_load() prior to calling this function, with no intervening alterations.

  agnentroprox_base is the return value of agenentroprox_init().

  append_mode is 2 to append diventropies one at at time to *diventropy_list_base, ordered ascending by the base index of the sweep window. Else one to sort results ascending by diventropy, or zero to sort descending.

  haystack_mask_idx_max is one less than the number of masks in the haystack, such that if agnentroprox_init():In:overlap_status was one, then this value would need to be increased in order to account for mask overlap. For example, if the sweep contains 5 of 3-byte masks, then this value would be 4 _without_ overlap, or 12 _with_ overlap. Must not exceed agnentroprox_init():In:mask_idx_max_max. See also agnentroprox_mask_idx_max_get().

  *haystack_mask_list_base is the haystack.

  *jsd_list_base contains (match_idx_max_max+1) undefined items.

  match_idx_max_max is one less than the maximum number of matches to report.

  *match_u8_idx_list_base is NULL if the sweep base indexes associated with matches can be discarded, else the base of (match_idx_max_max+1) undefined items to hold such indexes.

  sweep_mask_idx_max is one less than the number of masks in the sweep which, like haystack_mask_idx_max, must account for mask overlap if enabled.  On [0, haystack_mask_idx_max].

Out:

  Returns the number of matches found, which is simply (MIN((haystack_mask_idx_max-sweep_mask_idx_max))+1, match_idx_max_max), where sweep_mask_idx_max is just agnentroprox_needle_mask_list_load():In:mask_idx_max.

  *jsd_list_base contains (return value) items which represent the matches identified during the search, sorted according to append_mode.
*/
  fru128 coeff0;
  fru128 coeff1;
  fru128 coeff2;
  fru128 coeff3;
  u8 granularity;
  u8 ignored_status;
  fru128 jsd;
  fru128 jsd_delta_plus;
  fru128 jsd_delta_minus;
  u128 jsd_mean;
  u128 jsd_threshold;
  fru128 log;
  ULONG log_u128_idx_max;
  fru128 *log_u128_list_base;
  u128 *log_u128_parameter_list_base;
  u128 log2_recip_half;
  u32 mask;
  u32 mask_old;
  u8 mask_u8;
  ULONG match_count;
  ULONG match_idx;
  ULONG match_idx_min;
  u8 match_status_not;
  ULONG needle_freq;
  ULONG *needle_freq_list_base;
  ULONG needle_freq_old;
  ULONG needle_mask_count;
  u128 needle_mask_count_recip_half;
  u8 overlap_status;
  ULONG sweep_freq;
  ULONG *sweep_freq_list_base;
  ULONG sweep_freq_old;
  ULONG sweep_mask_count;
  u128 sweep_mask_count_recip_half;
  fru128 term;
  ULONG u8_idx;
  ULONG u8_idx_old;
  ULONG u8_idx_delta;
  ULONG u8_idx_max;
  u128 uint0;
  u128 uint1;
  u128 uint2;
  u128 uint3;
/*
In this function, we ignore returned overflow status (via ignored_status) because the JSD is naturally on [0, 1], so saturation already does the right thing.

The right shifts in this function are there in order to facilitate this optimization.
*/
  ignored_status=0;
  agnentroprox_jsd_get(agnentroprox_base, 0, sweep_mask_idx_max, haystack_mask_list_base);
  jsd=agnentroprox_base->entropy;
  FRU128_SHIFT_RIGHT_SELF(jsd, 1);
  FRU128_MEAN_TO_FTD128(jsd_mean, jsd);
  match_count=1;
  if(match_u8_idx_list_base){
    match_u8_idx_list_base[0]=0;
  }
  jsd_list_base[0]=jsd;
  U128_FROM_BOOL(jsd_threshold, append_mode);
  if(!match_idx_max_max){
    jsd_threshold=jsd_mean;
  }
  granularity=agnentroprox_base->granularity;
  log_u128_idx_max=agnentroprox_base->log_u128_idx_max;
  log_u128_list_base=agnentroprox_base->log_u128_list_base;
  log_u128_parameter_list_base=agnentroprox_base->log_u128_parameter_list_base;
  log2_recip_half=agnentroprox_base->log2_recip_half;
  needle_freq_list_base=agnentroprox_base->freq_list_base0;
  needle_mask_count=agnentroprox_base->mask_count0;
  needle_mask_count_recip_half=agnentroprox_base->needle_mask_count_recip_half;
  overlap_status=agnentroprox_base->overlap_status;
  sweep_freq_list_base=agnentroprox_base->freq_list_base1;
  sweep_mask_count_recip_half=agnentroprox_base->haystack_mask_count_recip_half;
  sweep_mask_count=agnentroprox_base->mask_count1;
  u8_idx_delta=(u8)((u8)(granularity*(!overlap_status))+1);
  u8_idx=(sweep_mask_idx_max+1)*u8_idx_delta;
  u8_idx_max=haystack_mask_idx_max*u8_idx_delta;
  u8_idx_old=0;
  while(u8_idx<=u8_idx_max){
    mask=haystack_mask_list_base[u8_idx];
    mask_old=haystack_mask_list_base[u8_idx_old];
    if(granularity){
      mask_u8=haystack_mask_list_base[u8_idx+U16_BYTE_MAX];
      mask|=(u32)(mask_u8)<<U8_BITS;
      mask_u8=haystack_mask_list_base[u8_idx_old+U16_BYTE_MAX];
      mask_old|=(u32)(mask_u8)<<U8_BITS;
      if(U16_BYTE_MAX<granularity){
        mask_u8=haystack_mask_list_base[u8_idx+U24_BYTE_MAX];
        mask|=(u32)(mask_u8)<<U16_BITS;
        mask_u8=haystack_mask_list_base[u8_idx_old+U24_BYTE_MAX];
        mask_old|=(u32)(mask_u8)<<U16_BITS;
        if(U24_BYTE_MAX<granularity){
          mask_u8=haystack_mask_list_base[u8_idx+U32_BYTE_MAX];
          mask|=(u32)(mask_u8)<<U24_BITS;
          mask_u8=haystack_mask_list_base[u8_idx_old+U32_BYTE_MAX];
          mask_old|=(u32)(mask_u8)<<U24_BITS;
        }
      }
    }
/*
Increment the u8 indexes now, so that if we get a match, u8_idx_old will be the first index of the new sweep, as opposed to the old one.
*/
    u8_idx+=u8_idx_delta;
    u8_idx_old+=u8_idx_delta;
    if(mask!=mask_old){
/*
Compute (1-(normalized Jensen-Shannon divergence)) delta according to the method described for JSDT(H, J, N, Z) in http://vixra.org/abs/1710.0261 with division by (2QnQs) already built into the coefficients.
*/
      needle_freq=needle_freq_list_base[mask];
      needle_freq_old=needle_freq_list_base[mask_old];
      sweep_freq=sweep_freq_list_base[mask];
      sweep_freq_old=sweep_freq_list_base[mask_old];
      if(sweep_freq){
        U128_FROM_U64_PRODUCT(uint0, (u64)(sweep_freq), (u64)(needle_mask_count));
        FRU128_LOG_U128_NONZERO_CACHED(log, log_u128_idx_max, log_u128_list_base, log_u128_parameter_list_base, uint0);
        FRU128_FROM_FTD128_U64_PRODUCT(coeff0, sweep_mask_count_recip_half, (u64)(sweep_freq), ignored_status);
        FRU128_MULTIPLY_FRU128(term, coeff0, log);
        FRU128_SHIFT_RIGHT(jsd_delta_plus, 1, term);
      }else{
        FRU128_SET_ZERO(coeff0);
        FRU128_SET_ZERO(jsd_delta_plus);
        U128_SET_ZERO(uint0);
      }
      U128_FROM_U64_PRODUCT(uint1, (u64)(sweep_freq_old), (u64)(needle_mask_count));
      FRU128_LOG_U128_NONZERO_CACHED(log, log_u128_idx_max, log_u128_list_base, log_u128_parameter_list_base, uint1);
      FRU128_FROM_FTD128_U64_PRODUCT(coeff1, sweep_mask_count_recip_half, (u64)(sweep_freq_old), ignored_status);
      FRU128_MULTIPLY_FRU128(term, coeff1, log);
      FRU128_SHIFT_RIGHT_SELF(term, 1);
      FRU128_ADD_FRU128_SELF(jsd_delta_plus, term, ignored_status);
      U128_ADD_U64_LO(uint2, uint0, (u64)(needle_mask_count));
      FRU128_LOG_U128_NONZERO_CACHED(log, log_u128_idx_max, log_u128_list_base, log_u128_parameter_list_base, uint2);
      FRU128_ADD_FTD128(coeff2, coeff0, sweep_mask_count_recip_half, ignored_status);
      FRU128_MULTIPLY_FRU128(term, coeff2, log);
      FRU128_SHIFT_RIGHT(jsd_delta_minus, 1, term);
      U128_SUBTRACT_U64_LO(uint3, uint1, (u64)(needle_mask_count));
      if(sweep_freq_old!=1){
        FRU128_LOG_U128_NONZERO_CACHED(log, log_u128_idx_max, log_u128_list_base, log_u128_parameter_list_base, uint3);
        FRU128_SUBTRACT_FTD128(coeff3, coeff1, sweep_mask_count_recip_half, ignored_status);
        FRU128_MULTIPLY_FRU128(term, coeff3, log);
        FRU128_SHIFT_RIGHT_SELF(term, 1);
        FRU128_ADD_FRU128_SELF(jsd_delta_minus, term, ignored_status);
      }
      sweep_freq_old--;
      sweep_freq_list_base[mask_old]=sweep_freq_old;
      if(needle_freq|sweep_freq){
        U128_FROM_U64_PRODUCT(uint2, (u64)(needle_freq), (u64)(sweep_mask_count));
        U128_ADD_U128_SELF(uint2, uint0);
        FRU128_LOG_U128_NONZERO_CACHED(log, log_u128_idx_max, log_u128_list_base, log_u128_parameter_list_base, uint2);
        FRU128_FROM_FTD128_U64_PRODUCT(coeff2, needle_mask_count_recip_half, (u64)(needle_freq), ignored_status);
        FRU128_ADD_FRU128_SELF(coeff2, coeff0, ignored_status);
        FRU128_MULTIPLY_FRU128(term, coeff2, log);
        FRU128_SHIFT_RIGHT_SELF(term, 1);
        FRU128_ADD_FRU128_SELF(jsd_delta_minus, term, ignored_status);
      }else{
        FRU128_SET_ZERO(coeff2);
        U128_SET_ZERO(uint2);
      }
      sweep_freq++;
      sweep_freq_list_base[mask]=sweep_freq;
      U128_FROM_U64_PRODUCT(uint3, (u64)(needle_freq_old), (u64)(sweep_mask_count));
      U128_ADD_U128_SELF(uint3, uint1);
      FRU128_LOG_U128_NONZERO_CACHED(log, log_u128_idx_max, log_u128_list_base, log_u128_parameter_list_base, uint3);
      FRU128_FROM_FTD128_U64_PRODUCT(coeff3, needle_mask_count_recip_half, (u64)(needle_freq_old), ignored_status);
      FRU128_ADD_FRU128_SELF(coeff3, coeff1, ignored_status);
      FRU128_MULTIPLY_FRU128(term, coeff3, log);
      FRU128_SHIFT_RIGHT_SELF(term, 1);
      FRU128_ADD_FRU128_SELF(jsd_delta_minus, term, ignored_status);
      U128_ADD_U64_LO_SELF(uint2, (u64)(needle_mask_count));
      FRU128_LOG_U128_NONZERO_CACHED(log, log_u128_idx_max, log_u128_list_base, log_u128_parameter_list_base, uint2);
      FRU128_ADD_FTD128_SELF(coeff2, sweep_mask_count_recip_half, ignored_status);
      FRU128_MULTIPLY_FRU128(term, coeff2, log);
      FRU128_SHIFT_RIGHT_SELF(term, 1);
      FRU128_ADD_FRU128_SELF(jsd_delta_plus, term, ignored_status);
      U128_SUBTRACT_U64_LO_SELF(uint3, (u64)(needle_mask_count));
      if(U128_IS_NOT_ZERO(uint3)){
        FRU128_LOG_U128_NONZERO_CACHED(log, log_u128_idx_max, log_u128_list_base, log_u128_parameter_list_base, uint3);
        FRU128_SUBTRACT_FTD128_SELF(coeff3, sweep_mask_count_recip_half, ignored_status);
        FRU128_MULTIPLY_FRU128(term, coeff3, log);
        FRU128_SHIFT_RIGHT_SELF(term, 1);
        FRU128_ADD_FRU128_SELF(jsd_delta_plus, term, ignored_status);
      }
      FRU128_ADD_FRU128_SELF(jsd, jsd_delta_plus, ignored_status);
      FRU128_SUBTRACT_FRU128_SELF(jsd, jsd_delta_minus, ignored_status);
      FRU128_MEAN_TO_FTD128(jsd_mean, jsd);
    }
    if(!append_mode){
      match_status_not=U128_IS_LESS(jsd_mean, jsd_threshold);
      if(!match_status_not){
        match_status_not=fracterval_u128_rank_list_insert_descending(jsd, &match_count, &match_idx, match_idx_max_max, jsd_list_base, &jsd_threshold);
      }
    }else if(append_mode==1){
      match_status_not=U128_IS_LESS(jsd_threshold, jsd_mean);
      if(!match_status_not){
        match_status_not=fracterval_u128_rank_list_insert_ascending(jsd, &match_count, &match_idx, match_idx_max_max, jsd_list_base, &jsd_threshold);
      }
    }else{
      match_idx=match_count;
      match_status_not=1;
      if(match_count<=match_idx_max_max){
        match_status_not=0;
        jsd_list_base[match_count]=jsd;
        match_count++;
      }
    }
    if((!match_status_not)&&match_u8_idx_list_base){
      match_idx_min=match_idx;
      match_idx=match_count-1;
      while(match_idx!=match_idx_min){
        match_u8_idx_list_base[match_idx]=match_u8_idx_list_base[match_idx-1];
        match_idx--;
      }
      match_u8_idx_list_base[match_idx]=u8_idx_old;
    }
  }
  match_idx=0;
  do{      
    jsd=jsd_list_base[match_idx];
/*
These finalization operations are the same as in agnentroprox_jsd_get(), but for an additional shift to compensate for all the right shifts above.
*/
    FRU128_MULTIPLY_FTD128_SELF(jsd, log2_recip_half);
    FRU128_SHIFT_LEFT_SELF(jsd, U128_BITS_LOG2+2, ignored_status);
    jsd_list_base[match_idx]=jsd;
    match_idx++;
  }while(match_idx!=match_count);
/*
Write ignored_status to prevent the compiler from complaining about it not being used.
*/
  agnentroprox_base->ignored_status=ignored_status;
  return match_count;
}

fru128
agnentroprox_ld_get(agnentroprox_t *agnentroprox_base, u8 exo_status, ULONG haystack_mask_idx_max, u8 *haystack_mask_list_base){
/*
Compute (1-(normalized Leidich divergence)) between needle and haystack frequency lists. (It's commutative so the terms "needle" and "haystack" are only used for the sake of consistency with other functions in which direction matters.)

In:

  The needle frequency list must have been preloaded with agnentroprox_needle_mask_list_load() prior to calling this function, with no intervening alterations.

  agnentroprox_base is the return value of agenentroprox_init().

  exo_status is one to assume that the needle is actually the subset of the haystack based at index zero of the latter, and should therefore be excluded from the haystack itself; in this case the number of masks in the preloaded needle must not exceed haystack_mask_idx_max. Else zero.

  haystack_mask_idx_max is one less than the number of masks in the haystack, such that if agnentroprox_init():In:overlap_status was one, then this value would need to be increased in order to account for mask overlap. For example, if the sweep contains 5 of 3-byte masks, then this value would be 4 _without_ overlap, or 12 _with_ overlap. Must not exceed agnentroprox_init():In:mask_idx_max_max. See also agnentroprox_mask_idx_max_get().

  *haystack_mask_list_base is the haystack.

Out:

  Returns (1-(normalized Leidich divergence)) between a needle and a haystack.
*/
  fru128 coeff;
  ULONG freq;
  ULONG haystack_freq;
  ULONG *haystack_freq_list_base;
  ULONG haystack_mask_count;
  u8 ignored_status;
  fru128 ld;
  fru128 ld_minus;
  fru128 ld_plus;
  fru64 log;
  ULONG log_idx_max;
  fru64 *log_list_base;
  u64 *log_parameter_list_base;
  u128 log2_recip_half;
  u32 mask;
  u64 mask_count;
  u64 mask_count_numerator;
  u32 mask_max;
  ULONG needle_freq;
  ULONG *needle_freq_list_base;
  ULONG needle_mask_count;
  u8 shift;
  fru128 term;
/*
In this function, we ignore returned overflow status (via ignored_status) because the LD is naturally on [0, 1], so saturation already does the right thing.
*/
  haystack_freq_list_base=agnentroprox_base->freq_list_base1;
  mask_max=agnentroprox_base->mask_max;
  ignored_status=0;
  agnentroprox_base->mask_count1=0;
  agnentroprox_ulong_list_zero((ULONG)(mask_max), haystack_freq_list_base);
  agnentroprox_mask_list_accrue(agnentroprox_base, 1, haystack_mask_idx_max, haystack_mask_list_base);
  if(exo_status){
    agnentroprox_mask_list_subtract(agnentroprox_base);
  }
  haystack_mask_count=agnentroprox_base->mask_count1;
  log_idx_max=agnentroprox_base->log_idx_max;
  log_list_base=agnentroprox_base->log_list_base;
  log_parameter_list_base=agnentroprox_base->log_parameter_list_base;
  needle_freq_list_base=agnentroprox_base->freq_list_base0;
  needle_mask_count=agnentroprox_base->mask_count0;
/*
Compute (1-(normalized Leidich divergence)) according to the method described for LD(N, S, Z) in http://vixra.org/abs/1710.0261 .
*/
  mask_count=(u64)(haystack_mask_count)+needle_mask_count;
  shift=2;
  while((1ULL<<shift)<=mask_count){
    shift++;
  }
  shift--;
  mask_count_numerator=1ULL<<shift;
/*
The most we can shift to the left without overflow is (U64_BIT_MAX+U64_BITS_LOG2-shift). The U64_BITS_LOG2 comes from the fact that all the logs we compute are 6.58 fixed-point, and we're ultimately looking to produce a fracterval result of the form 0.128. The conversion from fru64 logs to a fru128 result actually requires a left shift by (U64_BITS-shift), not (U64_BIT_MAX-shift), but the former could cause overflow due to interval uncertainty, and we're better off not dealing with the complexities of saturation, even though they would produce the correct result. So this means that we need to shift left by one at the last step. But then coeff will be multiplied by log2_recip_half, when it should really be multiplied by twice that value. So in total, we'll need to shift left by 2 at the last step.
*/
  shift=(u8)(U64_BIT_MAX+U64_BITS_LOG2-shift);
  agnentroprox_base->ld_shift=shift;
  FRU128_RATIO_U64_SATURATE(coeff, mask_count_numerator, mask_count, ignored_status);
  log2_recip_half=agnentroprox_base->log2_recip_half;
  FRU128_MULTIPLY_FTD128_SELF(coeff, log2_recip_half);
  agnentroprox_base->ld_coeff=coeff;
  FRU128_SET_ZERO(ld);
  FRU128_SET_ZERO(ld_minus);
  FRU128_SET_ZERO(ld_plus);
  mask=0;
  do{
    haystack_freq=haystack_freq_list_base[mask];
    needle_freq=needle_freq_list_base[mask];
    if(haystack_freq|needle_freq){
      if(haystack_freq){
        FRU64_LOG_U64_NONZERO_CACHED(log, log_idx_max, log_list_base, log_parameter_list_base, (u64)(haystack_freq));
        FRU128_FROM_FRU64_MULTIPLY_U64(term, log, (u64)(haystack_freq));
        FRU128_ADD_FRU128_SELF(ld_minus, term, ignored_status);
      }
      if(needle_freq){
        FRU64_LOG_U64_NONZERO_CACHED(log, log_idx_max, log_list_base, log_parameter_list_base, (u64)(needle_freq));
        FRU128_FROM_FRU64_MULTIPLY_U64(term, log, (u64)(needle_freq));
        FRU128_ADD_FRU128_SELF(ld_minus, term, ignored_status);
      }
      freq=haystack_freq+needle_freq;
      FRU64_LOG_U64_NONZERO_CACHED(log, log_idx_max, log_list_base, log_parameter_list_base, (u64)(freq));
      FRU128_FROM_FRU64_MULTIPLY_U64(term, log, (u64)(freq));
      FRU128_ADD_FRU128_SELF(ld_plus, term, ignored_status);
    }
  }while((mask++)!=mask_max);
  FRU128_SUBTRACT_FRU128(ld, ld_plus, ld_minus, ignored_status);
  agnentroprox_base->entropy=ld;
/*
See the comments above about why shift is what it is, and why we need the extra shift by 2.
*/
  FRU128_SHIFT_LEFT_SELF(ld, shift, ignored_status);
  FRU128_MULTIPLY_FRU128_SELF(ld, coeff);
  FRU128_SHIFT_LEFT_SELF(ld, 2, ignored_status);
/*
Write ignored_status to prevent the compiler from complaining about it not being used.
*/
  agnentroprox_base->ignored_status=ignored_status;
  return ld;
}

ULONG
agnentroprox_ld_transform(agnentroprox_t *agnentroprox_base, u8 append_mode, ULONG haystack_mask_idx_max, u8 *haystack_mask_list_base, fru128 *ld_list_base, ULONG match_idx_max_max, ULONG *match_u8_idx_list_base, ULONG sweep_mask_idx_max){
/*
Compute the (1-(normalized Leidich divergence)) transform of a haystack with respect to a preloaded needle frequency list, given a particular sweep.

In:

  The needle frequency list must have been preloaded with agnentroprox_needle_mask_list_load() prior to calling this function, with no intervening alterations.

  agnentroprox_base is the return value of agenentroprox_init().

  append_mode is 2 to append diventropies one at at time to *diventropy_list_base, ordered ascending by the base index of the sweep window. Else one to sort results ascending by diventropy, or zero to sort descending.

  haystack_mask_idx_max is one less than the number of masks in the haystack, such that if agnentroprox_init():In:overlap_status was one, then this value would need to be increased in order to account for mask overlap. For example, if the sweep contains 5 of 3-byte masks, then this value would be 4 _without_ overlap, or 12 _with_ overlap. Must not exceed agnentroprox_init():In:mask_idx_max_max. See also agnentroprox_mask_idx_max_get().

  *haystack_mask_list_base is the haystack.

  *ld_list_base contains (match_idx_max_max+1) undefined items.

  match_idx_max_max is one less than the maximum number of matches to report.

  *match_u8_idx_list_base is NULL if the sweep base indexes associated with matches can be discarded, else the base of (match_idx_max_max+1) undefined items to hold such indexes.

  sweep_mask_idx_max is one less than the number of masks in the sweep which, like haystack_mask_idx_max, must account for mask overlap if enabled.  On [0, haystack_mask_idx_max].

Out:

  Returns the number of matches found, which is simply (MIN((haystack_mask_idx_max-sweep_mask_idx_max))+1, match_idx_max_max), where sweep_mask_idx_max is just agnentroprox_needle_mask_list_load():In:mask_idx_max.

  *ld_list_base contains (return value) items which represent the matches identified during the search, sorted according to append_mode.
*/
  fru128 coeff;
  ULONG freq;
  u8 granularity;
  u8 ignored_status;
  fru128 ld;
  fru128 ld_delta_plus;
  fru128 ld_delta_minus;
  u128 ld_mean;
  u128 ld_threshold;
  fru64 log;
  ULONG log_idx_max;
  fru64 *log_list_base;
  u64 *log_parameter_list_base;
  ULONG log_delta_idx_max;
  fru64 *log_delta_list_base;
  u64 *log_delta_parameter_list_base;
  u32 mask;
  u32 mask_old;
  u8 mask_u8;
  ULONG match_count;
  ULONG match_idx;
  ULONG match_idx_min;
  u8 match_status_not;
  ULONG needle_freq;
  ULONG *needle_freq_list_base;
  ULONG needle_freq_old;
  u8 overlap_status;
  u8 shift;
  ULONG sweep_freq;
  ULONG *sweep_freq_list_base;
  ULONG sweep_freq_old;
  fru128 term;
  ULONG u8_idx;
  ULONG u8_idx_old;
  ULONG u8_idx_delta;
  ULONG u8_idx_max;
/*
In this function, we ignore returned overflow status (via ignored_status) because the LD is naturally on [0, 1], so saturation already does the right thing.
*/
  ignored_status=0;
  agnentroprox_ld_get(agnentroprox_base, 0, sweep_mask_idx_max, haystack_mask_list_base);
  ld=agnentroprox_base->entropy;
  FRU128_MEAN_TO_FTD128(ld_mean, ld);
  match_count=1;
  if(match_u8_idx_list_base){
    match_u8_idx_list_base[0]=0;
  }
  ld_list_base[0]=ld;
  U128_FROM_BOOL(ld_threshold, append_mode);
  if(!match_idx_max_max){
    ld_threshold=ld_mean;
  }
  granularity=agnentroprox_base->granularity;
  log_delta_idx_max=agnentroprox_base->log_delta_idx_max;
  log_delta_list_base=agnentroprox_base->log_delta_list_base;
  log_delta_parameter_list_base=agnentroprox_base->log_delta_parameter_list_base;
  log_idx_max=agnentroprox_base->log_idx_max;
  log_list_base=agnentroprox_base->log_list_base;
  log_parameter_list_base=agnentroprox_base->log_parameter_list_base;
  needle_freq_list_base=agnentroprox_base->freq_list_base0;
  overlap_status=agnentroprox_base->overlap_status;
  sweep_freq_list_base=agnentroprox_base->freq_list_base1;
  u8_idx_delta=(u8)((u8)(granularity*(!overlap_status))+1);
  u8_idx=(sweep_mask_idx_max+1)*u8_idx_delta;
  u8_idx_max=haystack_mask_idx_max*u8_idx_delta;
  u8_idx_old=0;
  while(u8_idx<=u8_idx_max){
    mask=haystack_mask_list_base[u8_idx];
    mask_old=haystack_mask_list_base[u8_idx_old];
    if(granularity){
      mask_u8=haystack_mask_list_base[u8_idx+U16_BYTE_MAX];
      mask|=(u32)(mask_u8)<<U8_BITS;
      mask_u8=haystack_mask_list_base[u8_idx_old+U16_BYTE_MAX];
      mask_old|=(u32)(mask_u8)<<U8_BITS;
      if(U16_BYTE_MAX<granularity){
        mask_u8=haystack_mask_list_base[u8_idx+U24_BYTE_MAX];
        mask|=(u32)(mask_u8)<<U16_BITS;
        mask_u8=haystack_mask_list_base[u8_idx_old+U24_BYTE_MAX];
        mask_old|=(u32)(mask_u8)<<U16_BITS;
        if(U24_BYTE_MAX<granularity){
          mask_u8=haystack_mask_list_base[u8_idx+U32_BYTE_MAX];
          mask|=(u32)(mask_u8)<<U24_BITS;
          mask_u8=haystack_mask_list_base[u8_idx_old+U32_BYTE_MAX];
          mask_old|=(u32)(mask_u8)<<U24_BITS;
        }
      }
    }
/*
Increment the u8 indexes now, so that if we get a match, u8_idx_old will be the first index of the new sweep, as opposed to the old one.
*/
    u8_idx+=u8_idx_delta;
    u8_idx_old+=u8_idx_delta;
    if(mask!=mask_old){
/*
Compute (1-(normalized Leidich divergence)) delta according to the method described for LDT(H, J, N, Z) in http://vixra.org/abs/1710.0261 .
*/
      needle_freq=needle_freq_list_base[mask];
      needle_freq_old=needle_freq_list_base[mask_old];
      sweep_freq=sweep_freq_list_base[mask];
      sweep_freq_old=sweep_freq_list_base[mask_old];
      FRU64_LOG_U64_NONZERO_CACHED(log, log_idx_max, log_list_base, log_parameter_list_base, (u64)(sweep_freq_old));
      FRU128_FROM_FRU64_LO(ld_delta_plus, log);
      freq=needle_freq_old+sweep_freq_old;
      FRU64_LOG_U64_NONZERO_CACHED(log, log_idx_max, log_list_base, log_parameter_list_base, (u64)(freq));
      FRU128_FROM_FRU64_LO(ld_delta_minus, log);
      freq=sweep_freq+1;
      FRU64_LOG_U64_NONZERO_CACHED(log, log_idx_max, log_list_base, log_parameter_list_base, (u64)(freq));
      FRU128_ADD_FRU64_LO_SELF(ld_delta_minus, log, ignored_status);
      freq+=needle_freq;
      sweep_freq_list_base[mask]=sweep_freq;
      FRU64_LOG_U64_NONZERO_CACHED(log, log_idx_max, log_list_base, log_parameter_list_base, (u64)(freq));
      FRU128_ADD_FRU64_LO_SELF(ld_delta_plus, log, ignored_status);
      freq=sweep_freq_old-1;
      sweep_freq_list_base[mask_old]=freq;
      if(freq){
        FRU64_LOG_DELTA_U64_NONZERO_CACHED(log, log_delta_idx_max, log_delta_list_base, log_delta_parameter_list_base, (u64)(freq));
        FRU128_FROM_FRU64_MULTIPLY_U64(term, log, (u64)(freq));
        FRU128_ADD_FRU128_SELF(ld_delta_plus, term, ignored_status);
      }
      if(sweep_freq){
        FRU64_LOG_DELTA_U64_NONZERO_CACHED(log, log_delta_idx_max, log_delta_list_base, log_delta_parameter_list_base, (u64)(sweep_freq));
        FRU128_FROM_FRU64_MULTIPLY_U64(term, log, (u64)(sweep_freq));
        FRU128_ADD_FRU128_SELF(ld_delta_minus, term, ignored_status);
      }
      freq=needle_freq_old+sweep_freq_old-1;
      if(freq){
        FRU64_LOG_DELTA_U64_NONZERO_CACHED(log, log_delta_idx_max, log_delta_list_base, log_delta_parameter_list_base, (u64)(freq));
        FRU128_FROM_FRU64_MULTIPLY_U64(term, log, (u64)(freq));
        FRU128_ADD_FRU128_SELF(ld_delta_minus, term, ignored_status);
      }
      freq=needle_freq+sweep_freq;
      FRU64_LOG_DELTA_U64_NONZERO_CACHED(log, log_delta_idx_max, log_delta_list_base, log_delta_parameter_list_base, (u64)(freq));
      FRU128_FROM_FRU64_MULTIPLY_U64(term, log, (u64)(freq));
      FRU128_ADD_FRU128_SELF(ld_delta_plus, term, ignored_status);
      FRU128_ADD_FRU128_SELF(ld, ld_delta_plus, ignored_status);
      FRU128_SUBTRACT_FRU128_SELF(ld, ld_delta_minus, ignored_status);
      FRU128_MEAN_TO_FTD128(ld_mean, ld);
    }
    if(!append_mode){
      match_status_not=U128_IS_LESS(ld_mean, ld_threshold);
      if(!match_status_not){
        match_status_not=fracterval_u128_rank_list_insert_descending(ld, &match_count, &match_idx, match_idx_max_max, ld_list_base, &ld_threshold);
      }
    }else if(append_mode==1){
      match_status_not=U128_IS_LESS(ld_threshold, ld_mean);
      if(!match_status_not){
        match_status_not=fracterval_u128_rank_list_insert_ascending(ld, &match_count, &match_idx, match_idx_max_max, ld_list_base, &ld_threshold);
      }
    }else{
      match_idx=match_count;
      match_status_not=1;
      if(match_count<=match_idx_max_max){
        match_status_not=0;
        ld_list_base[match_count]=ld;
        match_count++;
      }
    }
    if((!match_status_not)&&match_u8_idx_list_base){
      match_idx_min=match_idx;
      match_idx=match_count-1;
      while(match_idx!=match_idx_min){
        match_u8_idx_list_base[match_idx]=match_u8_idx_list_base[match_idx-1];
        match_idx--;
      }
      match_u8_idx_list_base[match_idx]=u8_idx_old;
    }
  }
  coeff=agnentroprox_base->ld_coeff;
  shift=agnentroprox_base->ld_shift;
  match_idx=0;
  do{      
    ld=ld_list_base[match_idx];
/*
These finalization operations are the same as in agnentroprox_ld_get().
*/
    FRU128_SHIFT_LEFT_SELF(ld, shift, ignored_status);
    FRU128_MULTIPLY_FRU128_SELF(ld, coeff);
    FRU128_SHIFT_LEFT_SELF(ld, 2, ignored_status);
    ld_list_base[match_idx]=ld;
    match_idx++;
  }while(match_idx!=match_count);
/*
Write ignored_status to prevent the compiler from complaining about it not being used.
*/
  agnentroprox_base->ignored_status=ignored_status;
  return match_count;
}

ULONG
agnentroprox_mask_idx_max_get(u8 granularity, u8 *granularity_status_base, ULONG mask_list_size, u8 overlap_status){
/*
Get the maximum mask index safely addressable, given the mask granularity, mask list size, and mask overlap status.

In:

  granularity is the agnentroprox_init():In:granularity, or will be if agnentroprox_init() has not yet been called.

  granularity_status_base is undefined.

  mask_list_size is the size of the mask list, which may or may not contain a whole number of masks.

  overlap_status is agnentroprox_init():In:overlap_status, or will be if agnentroprox_init() has not yet been called.

Out:

  Returns ULONG_MAX if (granularity<=mask_list_size), in order to defeat downstream mask list allocation attempts; otherwise the index of the maximum safely addressable mask. If overlap_status is one, then this is a byte-granular index; otherwise it's a (granularity+1)-byte-granular index.

  *granularity_status_base is one if mask_list_size is a nonzero multiple of (granularity+1), else one.
*/
  ULONG mask_count;
  ULONG mask_idx_max;
  u8 mask_size;
  u8 status;

  mask_count=0;
  status=1;
  if(granularity<mask_list_size){
    if(!overlap_status){
      mask_size=(u8)(granularity+1);
      mask_count=mask_list_size/mask_size;
      status=!!(mask_list_size%mask_size);
    }else{
      mask_count=mask_list_size-granularity;
      status=0;
    }
  }
  *granularity_status_base=status;
  mask_idx_max=mask_count-1;
  return mask_idx_max;
}

void
agnentroprox_mask_list_accrue(agnentroprox_t *agnentroprox_base, u8 freq_list_idx, ULONG mask_idx_max, u8 *mask_list_base){
/*
Add the frequencies of masks in a mask list to a cummulative frequency list.

In:

  agnentroprox_base is the return value of agenentroprox_init().

  freq_list_idx is the index of the frequency list, which is just zero or one.

  mask_idx_max is one less than the number of masks in the mask list, such that if agnentroprox_init():In:overlap_status was one, then this value would need to be increased in order to account for mask overlap. For example, if the sweep contains 5 of 3-byte masks, then this value would be 4 _without_ overlap, or 12 _with_ overlap. Must not exceed agnentroprox_init():In:mask_idx_max_max. See also agnentroprox_mask_idx_max_get().

  *mask_list_base is the mask list.

Out:

  The frequencies in the indicated frequency list are increased by the frequencies of the corresponding masks at mask_list_base.
*/
  ULONG freq;
  ULONG *freq_list_base;
  u8 granularity;
  u32 mask;
  ULONG mask_count;
  u8 mask_u8;
  u8 overlap_status;
  ULONG u8_idx;
  ULONG u8_idx_delta;
  ULONG u8_idx_max;

  freq_list_base=agnentroprox_base->freq_list_base0;
  if(freq_list_idx){
    freq_list_base=agnentroprox_base->freq_list_base1;
  }
  granularity=agnentroprox_base->granularity;
  overlap_status=agnentroprox_base->overlap_status;
  u8_idx=0;
  u8_idx_delta=(u8)((u8)(granularity*(!overlap_status))+1);
  u8_idx_max=mask_idx_max*u8_idx_delta;
  do{
    mask=mask_list_base[u8_idx];
    if(granularity){
      mask_u8=mask_list_base[u8_idx+U16_BYTE_MAX];
      mask|=(u32)(mask_u8)<<U8_BITS;
      if(U16_BYTE_MAX<granularity){
        mask_u8=mask_list_base[u8_idx+U24_BYTE_MAX];
        mask|=(u32)(mask_u8)<<U16_BITS;
        if(U24_BYTE_MAX<granularity){
          mask_u8=mask_list_base[u8_idx+U32_BYTE_MAX];
          mask|=(u32)(mask_u8)<<U24_BITS;
        }
      }
    }
    freq=freq_list_base[mask];
    freq++;
    u8_idx+=u8_idx_delta;
    freq_list_base[mask]=freq;
  }while(u8_idx<=u8_idx_max);
  mask_count=mask_idx_max+1;
  if(!freq_list_idx){
    agnentroprox_base->mask_count0+=mask_count;
  }else{
    agnentroprox_base->mask_count1+=mask_count;
  }
  return;
}

u8 *
agnentroprox_mask_list_malloc(u8 granularity, ULONG mask_idx_max, u8 overlap_status){
/*
Allocate a list of masks which are addressable as (u8)s but span up to U32_SIZE bytes each.

To maximize portability and debuggability, this is one of the few functions in which Agnentroprox calls malloc().

In:

  granularity is the agnentroprox_init():In:granularity, or will be if agnentroprox_init() has not yet been called.

  mask_idx_max is one less than the number of masks in the mask list, such that if overlap_status is one, then this value would need to be increased in order to account for mask overlap. For example, if the sweep contains 5 of 3-byte masks, then this value would be 4 _without_ overlap, or 12 _with_ overlap. Must not exceed agnentroprox_init():In:mask_idx_max_max. See also agnentroprox_mask_idx_max_get().

  overlap_status is agnentroprox_init():In:overlap_status, or will be if agnentroprox_init() has not yet been called.

Out:

  Returns NULL on failure, else the base of (((mask_idx_max+1)*(granularity*(!overlap_status)+1))+granularity) undefined (u8)s, which should eventually be freed via agnentroprox_free(). Note that this is enough to allow for the maximum possible number of remainder bytes in case overlap_status is zero. This is important because we might want to read a file which is not a multiple of (granularity+1), without causing a buffer overflow.
*/
  u8 *list_base;
  u64 list_bit_count;
  ULONG list_size;
  ULONG mask_count;
  u8 mask_size;

  list_base=NULL;
  mask_count=mask_idx_max+1;
  list_size=mask_count;
  if(!overlap_status){
    mask_size=(u8)(granularity+1);
    list_size*=mask_size;
    if((list_size/mask_size)!=mask_count){
      list_size=0;
    }
  }
  list_size+=granularity;
  if(granularity<list_size){
/*
Ensure that the allocated size in bits can be described in 64 bits.
*/
    list_bit_count=(u64)(list_size)<<U8_BITS_LOG2;
    if((list_bit_count>>U8_BITS_LOG2)==list_size){
      list_base=DEBUG_MALLOC_PARANOID(list_size);
    }
  }
  return list_base;
}

u128
agnentroprox_mask_list_mean_get(agnentroprox_t *agnentroprox_base, ULONG mask_idx_max, u8 *mask_list_base, u8 *sign_status_base){
/*
Get the mean of a list of masks, which may be signed or unsigned integers.

In:

  agnentroprox_base is the return value of agenentroprox_init().

  mask_idx_max is one less than the number of masks in the mask list, such that if agnentroprox_init():In:overlap_status was or will be one, then this value would need to be increased in order to account for mask overlap. For example, if the sweep contains 5 of 3-byte masks, then this value would be 4 _without_ overlap, or 12 _with_ overlap. Must not exceed agnentroprox_init():In:mask_idx_max_max. See also agnentroprox_mask_idx_max_get().

  *mask_list_base is the mask list.

  *sign_status_base is undefined.

Out:

  The effect of any previous call to agnentroprox_needle_mask_list_load() has been destroyed.

  The internal representation of the mean is stored at maximum accuracy, as required for interval math validity. A 64.64 fixed-point approximation is returned for display.

  Returns one of the following:

    If *sign_status_base is zero, then the return value is a 64.64 fixed-point value which is a lower bound for the mean.

    If *sign_status_base is one, and the actual mean is nonnegative, then the return value is a 64.64 fixed-point value which is a lower bound for the mean.

    Otherwise, the return value is a 64.64 fixed-point value which is a lower bound for (the negative of the mean).

  *sign_status_base is zero if the masks are unsigned; one if they're signed and the mean is nonnegative; or otherwise 2, indicating that they're signed and the mean is negative. There is a trivial probability that this assessment of sign status is wrong. This isn't a concern for statistical analysis, but could have security ramifications if *mask_list_base is constructed so as to appear signed to some functions but unsigned to others.
*/
  u32 delta_signed;
  u128 delta_signed_sum;
  u32 delta_unsigned;
  u128 delta_unsigned_sum;
  u64 freq;
  ULONG *freq_list_base;
  u8 ignored_status;
  u32 mask;
  u64 mask_count;
  u32 mask_max;
  u8 mask_max_bit_count;
  u32 mask_sign_mask;
  u128 mean_denominator;
  u128 mean_numerator;
  u8 mean_shift;
  u128 mean_signed;
  u128 mean_unsigned;
  u128 product;
  u8 sign_status;

  freq_list_base=agnentroprox_base->freq_list_base1;
  mask_max=agnentroprox_base->mask_max;
  agnentroprox_ulong_list_zero((ULONG)(mask_max), freq_list_base);
  agnentroprox_base->mask_count1=0;
  agnentroprox_mask_list_accrue(agnentroprox_base, 1, mask_idx_max, mask_list_base);
  mask_sign_mask=agnentroprox_base->mask_sign_mask;
  U128_SET_ZERO(delta_signed_sum);
  U128_SET_ZERO(delta_unsigned_sum);
  ignored_status=0;
  mask=0;
  U128_SET_ZERO(mean_signed);
  U128_SET_ZERO(mean_unsigned);
  do{
    freq=freq_list_base[mask];
    if(freq){
      if(mask<=mask_sign_mask){
        delta_unsigned=mask_sign_mask-mask;
      }else{
        delta_unsigned=mask-mask_sign_mask;
      }
      U128_FROM_U64_PRODUCT(product, (u64)(delta_unsigned), freq);
      U128_ADD_U128_SELF(delta_unsigned_sum, product);
      U128_FROM_U64_PRODUCT(product, (u64)(mask), freq);
      U128_ADD_U128_SELF(mean_unsigned, product);
      delta_signed=mask_sign_mask-delta_unsigned;
      U128_FROM_U64_PRODUCT(product, (u64)(delta_signed), freq);
      U128_ADD_U128_SELF(delta_signed_sum, product);
      mask^=mask_sign_mask;
      U128_FROM_U64_PRODUCT(product, (u64)(mask), freq);
      mask^=mask_sign_mask;
      U128_ADD_U128_SELF(mean_signed, product);
    }
  }while((mask++)!=mask_max);
  mean_numerator=mean_signed;
  sign_status=1;
  if(U128_IS_LESS_EQUAL(delta_unsigned_sum, delta_signed_sum)){
    mean_numerator=mean_unsigned;
    sign_status=0;
  }
  mask_count=(u64)(mask_idx_max)+1;
  mask_max_bit_count=(u8)(agnentroprox_base->mask_max_msb+1);
  U128_FROM_U64_SHIFTED(mean_denominator, mask_max_bit_count, mask_count);
/*
The following divide can't saturate because (mean_numerator<mean_denominator), so we can ignore the return status.
*/
  FTD128_RATIO_U128_SATURATE(mean_unsigned, mean_numerator, mean_denominator, ignored_status);
  agnentroprox_base->mean_unsigned=mean_unsigned;
  if(sign_status){
    U128_BIT_FLIP_SELF(mean_unsigned, U128_BIT_MAX);
    if(U128_IS_SIGNED(mean_unsigned)){
      U128_NOT_SELF(mean_unsigned);
      sign_status=2;
    }
  }
  mean_shift=agnentroprox_base->mean_shift;
  if(mean_shift<=U64_BIT_MAX){
    if(mean_shift){
      mean_shift=(u8)(U64_BITS-mean_shift);
      U128_SHIFT_LEFT_SELF(mean_unsigned, mean_shift);
    }
  }else{
    mean_shift=(u8)(mean_shift-U64_BITS);
    U128_SHIFT_RIGHT_SELF(mean_unsigned, mean_shift);
  }
/*
Write ignored_status to prevent the compiler from complaining about it not being used.
*/
  agnentroprox_base->ignored_status=ignored_status;
  agnentroprox_base->sign_status=sign_status;
  *sign_status_base=sign_status;
  return mean_unsigned;
}

u8 *
agnentroprox_mask_list_realloc(u8 granularity, ULONG mask_idx_max, u8 *mask_list_base, u8 overlap_status){
/*
Change the size of previously allocated mask list.

To maximize portability and debuggability, this is one of the few functions in which Agnentroprox calls realloc().

In:

  granularity is the agnentroprox_init():In:granularity, or will be if agnentroprox_init() has not yet been called.

  mask_idx_max is one less than the number of masks in the mask list, such that if overlap_status is one, then this value would need to be increased in order to account for mask overlap. For example, if the sweep contains 5 of 3-byte masks, then this value would be 4 _without_ overlap, or 12 _with_ overlap. Must not exceed agnentroprox_init():In:mask_idx_max_max. See also agnentroprox_mask_idx_max_get().

  overlap_status is agnentroprox_init():In:overlap_status, or will be if agnentroprox_init() has not yet been called.

Out:

  Returns NULL on failure, else the base of (((mask_idx_max+1)*(granularity*(!overlap_status)+1))+granularity) undefined (u8)s, which should eventually be freed via agnentroprox_free(). Note that this is enough to allow for the maximum possible number of remainder bytes in case overlap_status is zero. This is important because we might want to read a file which is not a multiple of (granularity+1), without causing a buffer overflow. On failure, as with realloc(), the existing allocation remains unchanged; on success, it has been freed.
*/
  u8 *list_base;
  u64 list_bit_count;
  ULONG list_size;
  ULONG mask_count;
  u8 mask_size;

  list_base=NULL;
  mask_count=mask_idx_max+1;
  list_size=mask_count;
  if(!overlap_status){
    mask_size=(u8)(granularity+1);
    list_size*=mask_size;
    if((list_size/mask_size)!=mask_count){
      list_size=0;
    }
  }
  list_size+=granularity;
  if(granularity<list_size){
/*
Ensure that the allocated size in bits can be described in 64 bits.
*/
    list_bit_count=(u64)(list_size)<<U8_BITS_LOG2;
    if((list_bit_count>>U8_BITS_LOG2)==list_size){
      list_base=DEBUG_REALLOC_PARANOID(mask_list_base, list_size);
    }
  }
  return list_base;
}

void
agnentroprox_mask_list_subtract(agnentroprox_t *agnentroprox_base){
/*
Subtract the cummulative frequency list at index zero from the cummulative frequency list at index one.

In:

  agnentroprox_base is the return value of agenentroprox_init().

Out:

  The difference of the cummulative frequency lists has been computed and saved internally, as stated in the summary.
*/
  ULONG freq0;
  ULONG freq1;
  ULONG *freq_list_base0;
  ULONG *freq_list_base1;
  u32 mask;
  u32 mask_max;

  freq_list_base0=agnentroprox_base->freq_list_base0;
  freq_list_base1=agnentroprox_base->freq_list_base1;
  mask=0;
  mask_max=agnentroprox_base->mask_max;
  do{
    freq0=freq_list_base0[mask];
    freq1=freq_list_base1[mask];
    freq1-=freq0;
    freq_list_base1[mask]=freq1;
  }while((mask++)!=mask_max);
  agnentroprox_base->mask_count1-=agnentroprox_base->mask_count0;
  return;
}

void
agnentroprox_mask_list_unaccrue(agnentroprox_t *agnentroprox_base, u8 freq_list_idx, ULONG mask_idx_max, u8 *mask_list_base){
/*
Subtract the frequencies of masks in a mask list from a cummulative frequency list.

In:

  agnentroprox_base is the return value of agenentroprox_init().

  freq_list_idx is the index of the frequency list, which is just zero or one.

  mask_idx_max is one less than the number of masks in the mask list, such that if agnentroprox_init():In:overlap_status was one, then this value would need to be increased in order to account for mask overlap. For example, if the sweep contains 5 of 3-byte masks, then this value would be 4 _without_ overlap, or 12 _with_ overlap. Must not exceed agnentroprox_init():In:mask_idx_max_max. See also agnentroprox_mask_idx_max_get().

  *mask_list_base is the mask list.

Out:

  The frequencies in the indicated frequency list are decreased by the frequencies of the corresponding masks at mask_list_base.
*/
  ULONG freq;
  ULONG *freq_list_base;
  u8 granularity;
  u32 mask;
  ULONG mask_count;
  u8 mask_u8;
  u8 overlap_status;
  ULONG u8_idx;
  ULONG u8_idx_delta;
  ULONG u8_idx_max;

  freq_list_base=agnentroprox_base->freq_list_base0;
  if(freq_list_idx){
    freq_list_base=agnentroprox_base->freq_list_base1;
  }
  granularity=agnentroprox_base->granularity;
  overlap_status=agnentroprox_base->overlap_status;
  u8_idx_delta=(u8)((u8)(granularity*(!overlap_status))+1);
  u8_idx_max=mask_idx_max*u8_idx_delta;
  u8_idx=u8_idx_max;
  do{
    mask=mask_list_base[u8_idx];
    if(granularity){
      mask_u8=mask_list_base[u8_idx+U16_BYTE_MAX];
      mask|=(u32)(mask_u8)<<U8_BITS;
      if(U16_BYTE_MAX<granularity){
        mask_u8=mask_list_base[u8_idx+U24_BYTE_MAX];
        mask|=(u32)(mask_u8)<<U16_BITS;
        if(U24_BYTE_MAX<granularity){
          mask_u8=mask_list_base[u8_idx+U32_BYTE_MAX];
          mask|=(u32)(mask_u8)<<U24_BITS;
        }
      }
    }
    freq=freq_list_base[mask];
    freq--;
    u8_idx-=u8_idx_delta;
    freq_list_base[mask]=freq;
  }while(u8_idx<=u8_idx_max);
  mask_count=mask_idx_max+1;
  if(!freq_list_idx){
    agnentroprox_base->mask_count0-=mask_count;
  }else{
    agnentroprox_base->mask_count1-=mask_count;
  }
  return;
}

void
agnentroprox_mask_max_reset(agnentroprox_t *agnentroprox_base){
/*
Restore mask_max to its default value of mask_max_max.

In:

  agnentroprox_base is the return value of agenentroprox_init().

Out:

  The next call to agnentroprox_entropy_delta_get() must have (new_status=1).
*/
  agnentroprox_mask_max_set(agnentroprox_base, agnentroprox_base->mask_max_max);
  return;
}

void
agnentroprox_mask_max_set(agnentroprox_t *agnentroprox_base, u32 mask_max){
/*
Don't call this dangerous function unless you're able to abide by the constraints given in Out. It sets a custom mask_max pursuant to mask span reduction.

In:

  Usually this function will be called pursuant to maskops_densify() in order to exploit the mask span reduction which it affords.

  In:

  agnentroprox_base is the return value of agenentroprox_init().

  mask_max is at least the maximum mask value which could be encountered until the next time this function is called. If zero, it will be internally converted to one in order to meet the requirements of agnentroprox_init(). The whole point is to increase the speed and sensistivity of various entropy transforms. However, these goals may or may not be realized because, for example, the speed and accuracy of the log function improves as the parameter increases.

Out:

  agnentroprox_entropy_delta_get() must be called with (new_status=1) each time this function changes mask_max. Neither that function nor agnentroprox_entropy_transform(), may be called for kurtosis (AGNENTROPROX_MODE_KURTOSIS) or variance (AGNENTROPROX_MODE_VARIANCE) modes. agnentroprox_mask_list_mean_get() may not be called at all. These constraints will be removed by to calling agnentroprox_mask_max_reset().
*/
  u8 ignored_status;
  u64 mask_span;
  fru128 mask_span_log;

  mask_max+=!mask_max;
  mask_span=(u64)(mask_max)+1;
  agnentroprox_base->mask_max=mask_max;
/*
Update mask_span_log in a manner consistent with agnentroprox_init().
*/
  ignored_status=0;
  FRU128_LOG_U64(mask_span_log, mask_span, ignored_status);
  FRU128_SHIFT_RIGHT_SELF(mask_span_log, 122-64);
  agnentroprox_base->ignored_status=ignored_status;
  agnentroprox_base->mask_span_log=mask_span_log;
/*
Unless mask_max equals agnentroprox_init():In:mask_max_max, agnentroprox_base->mask_sign_mask, agnentroprox_base->mask_max_msb, agnentroprox_base->mean_shift, agnentroprox_base->mean_unsigned, and agnentroprox_base->sign_status are now inconsistent with mask_max, hence the postexit constraints on the caller.
*/
  return;
}

ULONG
agnentroprox_match_find(u8 ascending_status, u8 case_insensitive_status, u8 granularity, ULONG haystack_mask_idx_max, u8 *haystack_mask_list_base, ULONG match_idx_max_max, ULONG *match_u8_idx_list_base, ULONG needle_mask_idx_max, u8 *needle_mask_list_base, u8 overlap_status){
/*
Find exact matches of a needle in a haystack.

In:

  ascending_status is one to search from the beginning of the haystack forward, else zero to search from the end backward.

  case_insensitive_status is one to treat upper and lower case letters the same. Note that this effects only 26 characters, so it's not fully internationalized.

  granularity is the agnentroprox_init():In:granularity, or will be if agnentroprox_init() has not yet been called.

  haystack_mask_idx_max is one less than the number of masks in the mask list, such that if overlap_status is one, then this value would need to be increased in order to account for mask overlap. For example, if the sweep contains 5 of 3-byte masks, then this value would be 4 _without_ overlap, or 12 _with_ overlap. Must not exceed agnentroprox_init():In:mask_idx_max_max. See also agnentroprox_mask_idx_max_get().

  *haystack_mask_list_base is the haystack.
  
  match_idx_max_max is one less than the maximum number of matches to report starting from either end of the haystack, depending on ascending_status.

  *match_u8_idx_list_base is NULL if the sweep base indexes associated with matches can be discarded, else the base of (match_idx_max_max+1) undefined items to hold such indexes.
  
  needle_mask_idx_max is the needle equivalent of haystack_mask_idx_max.

  *needle_mask_list_base is the needle.

  overlap_status is one if a match can be found on any byte boundary, else one if it can only be found on a (granularity+1)-byte boundary. In either case, matches can overlap one another. For example, in {7, 2, 2, 2, 2, 5}, there are 3 matches {2, 2}, with the middle one overlapping the others. Setting this value to one also affects the meaning of the meaning of haystack_mask_idx_max and needle_mask_idx_max, as explained above.

Out:

  Returns the number of matches found, on [0, match_idx_max_max+1].

  *match_u8_idx_list_base contains (return value) u8 indexes, each pointing to the base of a copy of *needle_list_base. As explained above, match regions may overlap, regardless of overlap_status.
*/
  u8 haystack_u8;
  ULONG haystack_u8_idx;
  ULONG haystack_u8_idx_max;
  ULONG mask_list_size_delta;
  ULONG match_idx;
  ULONG needle_mask_list_size;
  u8 needle_u8;
  u8 needle_u8_first;
  u8 needle_u8_first_alternate;
  ULONG needle_u8_idx;
  ULONG needle_u8_idx_max;
  u8 status;
  ULONG u8_idx_delta;

  match_idx=0;
  if(needle_mask_idx_max<=haystack_mask_idx_max){
    needle_u8_first=needle_mask_list_base[0];
    u8_idx_delta=(u8)((u8)(granularity*(!overlap_status))+1);
    haystack_u8_idx_max=(haystack_mask_idx_max*u8_idx_delta)+granularity;
    needle_u8_idx_max=(needle_mask_idx_max*u8_idx_delta)+granularity;
    mask_list_size_delta=haystack_u8_idx_max-needle_u8_idx_max;
    needle_mask_list_size=needle_u8_idx_max+1;
    haystack_u8_idx=0;
    if(!ascending_status){
      haystack_u8_idx=mask_list_size_delta;
      u8_idx_delta=0U-u8_idx_delta;
    }
    if(!case_insensitive_status){
      do{
        if(haystack_mask_list_base[haystack_u8_idx]==needle_u8_first){
          status=!!memcmp(&haystack_mask_list_base[haystack_u8_idx], needle_mask_list_base, (size_t)(needle_mask_list_size));
          if(!status){
            if(match_u8_idx_list_base){
              match_u8_idx_list_base[match_idx]=haystack_u8_idx;
            }
            match_idx++;
            if(match_idx_max_max<match_idx){
              break;
            }
          }
        }
        haystack_u8_idx+=u8_idx_delta;
      }while(haystack_u8_idx<=mask_list_size_delta);
    }else{
      needle_u8_first_alternate=needle_u8_first;
      if((u8)((needle_u8_first|('a'-'A'))-'a')<=('z'-'a')){
        needle_u8_first_alternate=(u8)(needle_u8_first^('a'-'A'));
      }
      do{
        haystack_u8=haystack_mask_list_base[haystack_u8_idx];
        if((haystack_u8==needle_u8_first)||(haystack_u8==needle_u8_first_alternate)){
          status=0;
          for(needle_u8_idx=1; needle_u8_idx<needle_mask_list_size; needle_u8_idx++){
            haystack_u8=haystack_mask_list_base[haystack_u8_idx+needle_u8_idx];
            needle_u8=needle_mask_list_base[needle_u8_idx];
            if((haystack_u8^needle_u8)&(~('a'-'A'))){
              status=1;
              break;
            }
            if((haystack_u8^needle_u8)==('a'-'A')){
              if(('z'-'a')<(u8)((haystack_u8|('a'-'A'))-'a')){
                status=1;
                break;
              }
            }
          }
          if(!status){
            if(match_u8_idx_list_base){
              match_u8_idx_list_base[match_idx]=haystack_u8_idx;
            }
            match_idx++;
            if(match_idx_max_max<match_idx){
              break;
            }
          }
        }
        haystack_u8_idx+=u8_idx_delta;
      }while(haystack_u8_idx<=mask_list_size_delta);
    }
  }
  return match_idx;
}

void
agnentroprox_needle_mask_list_load(agnentroprox_t *agnentroprox_base, ULONG mask_idx_max, u8 *mask_list_base){
/*
Copy the frequencies of masks in a needle to an internal frequency list, in preparation for a series of bivalent entropy transforms.

In:

  agnentroprox_base is the return value of agenentroprox_init().

  mask_idx_max is one less than the number of masks in the mask list, such that if agnentroprox_init():In:overlap_status was one, then this value would need to be increased in order to account for mask overlap. For example, if the sweep contains 5 of 3-byte masks, then this value would be 4 _without_ overlap, or 12 _with_ overlap. Must not exceed agnentroprox_init():In:mask_idx_max_max. See also agnentroprox_mask_idx_max_get().

  *mask_list_base is the needle.

Out:

  The mask frequencies in the needle have been copied to an internal frequency list.
*/
  ULONG *freq_list_base;
  u32 mask_max;

  freq_list_base=agnentroprox_base->freq_list_base0;
  mask_max=agnentroprox_base->mask_max;
  agnentroprox_ulong_list_zero((ULONG)(mask_max), freq_list_base);
  agnentroprox_base->mask_count0=0;
  agnentroprox_mask_list_accrue(agnentroprox_base, 0, mask_idx_max, mask_list_base);
  return;
}

fru128
agnentroprox_shannon_entropy_get(agnentroprox_t *agnentroprox_base, u8 freq_list_idx, u8 *overflow_status_base){
/*
Compute the Shannon entropy of a preloaded frequency list.

In:

  The needle frequency list must have been preloaded with agnentroprox_mask_list_accrue() or agnentroprox_needle_mask_list_load() prior to calling this function, with no intervening alterations.

  agnentroprox_base is the return value of agenentroprox_init().

  freq_list_idx is the index of the frequency list, which is just zero or one.

  *overflow_status_base is the OR-cummulative fracterval overflow status.

Out:

  Returns the Shannon entropy of the preloaded frequency list, in 64.64 fixed point.

  *overflow_status_base is one if a fracterval overflow occurred, else unchanged.
*/
  fru128 entropy;
  ULONG freq;
  ULONG *freq_list_base;
  fru64 log;
  ULONG log_idx_max;
  fru64 *log_list_base;
  u64 *log_parameter_list_base;
  u32 mask;
  ULONG mask_count;
  u32 mask_max;
  u8 overflow_status;
  fru128 term;

  freq_list_base=agnentroprox_base->freq_list_base0;
  mask_count=agnentroprox_base->mask_count0;
  overflow_status=*overflow_status_base;
  if(freq_list_idx){
    freq_list_base=agnentroprox_base->freq_list_base1;
    mask_count=agnentroprox_base->mask_count1;
  }
  log_idx_max=agnentroprox_base->log_idx_max;
  log_list_base=agnentroprox_base->log_list_base;
  log_parameter_list_base=agnentroprox_base->log_parameter_list_base;
  mask_max=agnentroprox_base->mask_max;
  FRU128_SET_ZERO(entropy);
/*
Compute the Shannon entropy S in nats:

  S=(Q*log(Q))-Σ(M=(0, Z-1), F[M]*log(F[M]))

where:

  F[M]=MIN((frequency of mask M), 1)
  M=mask
  Q=mask_count
  Z=(mask_max+1)
*/
  if(1<mask_count){
    FRU64_LOG_U64_NONZERO_CACHED(log, log_idx_max, log_list_base, log_parameter_list_base, (u64)(mask_count));
    FRU128_FROM_FRU64_MULTIPLY_U64(entropy, log, (u64)(mask_count));
    mask=0;
    do{
      freq=freq_list_base[mask];
      if(1<freq){
        if(freq==mask_count){
/*
Shannon entropy is zero because all symbols are identical. Tread carefully because otherwise we'll get fracterval underflow.
*/
          FRU128_SET_ZERO(entropy);
          break;
        }
        FRU64_LOG_U64_NONZERO_CACHED(log, log_idx_max, log_list_base, log_parameter_list_base, (u64)(freq));
        FRU128_FROM_FRU64_MULTIPLY_U64(term, log, (u64)(freq));
        FRU128_SUBTRACT_FRU128_SELF(entropy, term, overflow_status);
      }
    }while((mask++)!=mask_max);
    if(U128_IS_NOT_ZERO(entropy.b)){
      FRU128_SHIFT_LEFT_SELF(entropy, 64-58, overflow_status);
    }
  }
  *overflow_status_base=overflow_status;
  return entropy;
}

u128 *
agnentroprox_u128_list_malloc(ULONG u128_idx_max){
/*
Allocate a list of undefined (u128)s.

To maximize portability and debuggability, this is one of the few functions in which Agnentroprox calls malloc().

In:

  u128_idx_max is the number of (u128)s to allocate, less one.

Out:

  Returns NULL on failure, else the base of (u128_idx_max+1) undefined items.
*/
  u128 *list_base;
  u64 list_bit_count;
  ULONG list_size;
  ULONG u128_count;

  list_base=NULL;
  u128_count=u128_idx_max+1;
  if(u128_count){
    list_size=u128_count<<U128_SIZE_LOG2;
    if((list_size>>U128_SIZE_LOG2)==u128_count){
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

u32 *
agnentroprox_u32_list_malloc(ULONG u32_idx_max){
/*
Allocate a list of undefined (u32)s.

To maximize portability and debuggability, this is one of the few functions in which Agnentroprox calls malloc().

In:

  u32_idx_max is the number of (u32)s to allocate, less one.

Out:

  Returns NULL on failure, else the base of (u32_idx_max+1) undefined items.
*/
  u32 *list_base;
  u64 list_bit_count;
  ULONG list_size;
  ULONG u32_count;

  list_base=NULL;
  u32_count=u32_idx_max+1;
  if(u32_count){
    list_size=u32_count<<U32_SIZE_LOG2;
    if((list_size>>U32_SIZE_LOG2)==u32_count){
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

void
agnentroprox_u32_list_zero(ULONG u32_idx_max, u32 *u32_list_base){
/*
Zero a list of (u32)s.

In:

  u32_idx_max is the number of (u32)s to zero, less one.

  u32_list_base is the base of the list to zero.

Out:

  The list is zeroed as specified above.
*/
  ULONG list_size;

  list_size=(u32_idx_max+1)<<U32_SIZE_LOG2;
  memset(u32_list_base, 0, (size_t)(list_size));
  return;
}

u64 *
agnentroprox_u64_list_malloc(ULONG u64_idx_max){
/*
Allocate a list of undefined (u64)s.

To maximize portability and debuggability, this is one of the few functions in which Agnentroprox calls malloc().

In:

  u64_idx_max is the number of (u64)s to allocate, less one.

Out:

  Returns NULL on failure, else the base of (u64_idx_max+1) undefined items.
*/
  u64 *list_base;
  u64 list_bit_count;
  ULONG list_size;
  ULONG u64_count;

  list_base=NULL;
  u64_count=u64_idx_max+1;
  if(u64_count){
    list_size=u64_count<<U64_SIZE_LOG2;
    if((list_size>>U64_SIZE_LOG2)==u64_count){
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

void
agnentroprox_u64_list_zero(ULONG u64_idx_max, u64 *u64_list_base){
/*
Zero a list of (u64)s.

In:

  u64_idx_max is the number of (u64)s to zero, less one.

  u64_list_base is the base of the list to zero.

Out:

  The list is zeroed as specified above.
*/
  ULONG list_size;

  list_size=(u64_idx_max+1)<<U64_SIZE_LOG2;
  memset(u64_list_base, 0, (size_t)(list_size));
  return;
}

void
agnentroprox_ulong_list_copy(ULONG ulong_idx_max, ULONG *ulong_list_base0, ULONG *ulong_list_base1){
/*
Copy a list of (ULONG)s to another one of at least the same size.

In:

  ulong_idx_max is the number of (ULONG)s to zero, less one.

  ulong_list_base0 is the base of the list to copy.

  ulong_list_base1 is the base of the undefined region to hold a copy of *ulong_list_base0.

Out:

  *ulong_list_base1 is identical to *ulong_list_base0 for the first (ulong_idx_max+1) (ULONG)s.
*/
  ULONG list_size;

  list_size=(ulong_idx_max+1)<<ULONG_SIZE_LOG2;
  memcpy(ulong_list_base1, ulong_list_base0, (size_t)(list_size));
  return;
}
