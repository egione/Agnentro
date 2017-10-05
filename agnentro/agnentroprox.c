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
  agnentroprox_base->entropy=diventropy;
  agnentroprox_base->log_idx_max=log_idx_max;
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
  diventropy=agnentroprox_base->entropy;
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
  agnentroprox_base->log_delta_idx_max=log_delta_idx_max;
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
  agnentroprox_base->loggamma_idx_max=loggamma_idx_max;
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

  mode is AGNENTROPROX_MODE_AGNENTROPY, AGNENTROPROX_MODE_KURTOSIS, AGNENTROPROX_MODE_LOGFREEDOM, AGNENTROPROX_MODE_SHANNON, or AGNENTROPROX_MODE_VARIANCE to compute the agnentropy, obtuse kurtosis, logfreedom, Shannon, or obtuse variance entropy delta, respectively. For AGNENTROPROX_MODE_KURTOSIS and AGNENTROPROX_MODE_VARIANCE, the global mean must have been precomputed by agnentroprox_mask_list_mean_get().

  *overflow_status_base is the OR-cummulative fracterval overflow status.

  sweep_mask_idx_max is one less than the number of masks in the sweep, which like mask_idx_max, must account for mask overlap if enabled.  On [0, mask_idx_max].

Out:

  Returns the number of matches found, which is simply (MIN((mask_idx_max-sweep_mask_idx_max))+1, match_idx_max_max).

  *entropy_list_base contains (return value) items which represent the matches identified during the search, sorted according to append_mode.

  *overflow_status_base is one if a fracterval overflow occurred, else unchanged.
*/
  u128 delta;
  fru128 delta_power;
  fru128 delta_power_shifted;
  fru128 entropy;
  fru128 entropy_delta;
  u128 entropy_mean;
  u128 entropy_threshold;
  ULONG exofreq;
  ULONG exofreq_minus_1;
  ULONG exofreq_old;
  ULONG exofreq_old_plus_1;
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
  fru64 log;
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
  u32 mask_sign_mask;
  u8 mask_u8;
  u32 mask_unsigned;
  ULONG match_count;
  ULONG mask_count_plus_span;
  ULONG match_idx;
  ULONG match_idx_min;
  u8 match_status_not;
  u8 mean_shift;
  u128 mean_unsigned;
  u8 overflow_status;
  u8 overlap_status;
  poissocache_t *poissocache_base;
  ULONG pop;
  u8 sign_status;
  fru128 sum_quartics;
  fru128 sum_squares;
  fru128 sum_squares_squared;
  ULONG sweep_mask_count;
  u8 sweep_mask_idx_max_bit_count;
  fru64 term_minus;
  fru64 term_plus;
  ULONG u8_idx;
  ULONG u8_idx_old;
  ULONG u8_idx_delta;
  ULONG u8_idx_max;
  u8 variance_shift;

  overflow_status=*overflow_status_base;
  freq_list_base0=agnentroprox_base->freq_list_base0;
  freq_list_base1=agnentroprox_base->freq_list_base1;
  mask_max=agnentroprox_base->mask_max;
  log_idx_max=agnentroprox_base->log_idx_max;
  log_list_base=agnentroprox_base->log_list_base;
  log_parameter_list_base=agnentroprox_base->log_parameter_list_base;
  FRU128_SET_ZERO(entropy);
  sweep_mask_count=sweep_mask_idx_max+1;
  if(mode!=AGNENTROPROX_MODE_EXOENTROPY){
    entropy=agnentroprox_entropy_delta_get(agnentroprox_base, sweep_mask_idx_max, mask_list_base, mode, 1, &overflow_status, 0);
  }else{
    agnentroprox_ulong_list_zero((ULONG)(mask_max), freq_list_base0);
    agnentroprox_base->mask_count0=0;
    agnentroprox_mask_list_accrue(agnentroprox_base, 0, sweep_mask_idx_max, mask_list_base);
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
  granularity=agnentroprox_base->granularity;
  log_delta_idx_max=agnentroprox_base->log_delta_idx_max;
  log_delta_list_base=agnentroprox_base->log_delta_list_base;
  log_delta_parameter_list_base=agnentroprox_base->log_delta_parameter_list_base;
  mask_sign_mask=agnentroprox_base->mask_sign_mask;
  mean_shift=agnentroprox_base->mean_shift;
  mean_unsigned=agnentroprox_base->mean_unsigned;
  overlap_status=agnentroprox_base->overlap_status;
  sign_status=agnentroprox_base->sign_status;
  sweep_mask_idx_max_bit_count=agnentroprox_base->sweep_mask_idx_max_bit_count;
  variance_shift=agnentroprox_base->variance_shift;
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
      if(mode==AGNENTROPROX_MODE_AGNENTROPY){
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
      }else if(mode==AGNENTROPROX_MODE_EXOENTROPY){
        exofreq=freq_list_base1[mask];
        exofreq_old=freq_list_base1[mask_old];
        exofreq_minus_1=exofreq-1;
        exofreq_old_plus_1=exofreq_old+1;
        freq_list_base1[mask]=exofreq_minus_1;
        freq_list_base1[mask_old]=exofreq_old_plus_1;
/*
The exoentropy difference, dE, is:

  dE=(freq*log(exofreq+1))+(freq_old*log(exofreq_old+1)))-((freq+1)*log(exofreq))-((freq_old-1)*log(exofreq_old+2))
  dE=freq*(log(exofreq+1)-log(exofreq))+log(exofreq_old+1)-((freq_old-1)*(log(exofreq_old+2)-log(exofreq_old+1)))-log(exofreq)
  dE=(freq*log_delta(exofreq))+log(exofreq_old+1)-((freq_old-1)*log_delta(exofreq_old+1))-log(exofreq)
  dE=(freq*log_delta(exofreq))+log(exofreq_old_plus_1)-(freq_old_minus_1*log_delta(exofreq_old_plus_1))-log(exofreq)
*/
        FRU64_LOG_DELTA_U64_NONZERO_CACHED(term_plus, log_delta_idx_max, log_delta_list_base, log_delta_parameter_list_base, (u64)(exofreq));
        FRU128_FROM_FRU64_MULTIPLY_U64(entropy_delta, term_plus, (u64)(freq));
        FRU64_LOG_U64_NONZERO_CACHED(term_plus, log_idx_max, log_list_base, log_parameter_list_base, (u64)(exofreq_old_plus_1));
        FRU128_ADD_FRU64_LO_SELF(entropy_delta, term_plus, overflow_status);
        FRU128_SHIFT_LEFT_SELF(entropy_delta, 64-58, overflow_status);
        FRU128_ADD_FRU128_SELF(entropy, entropy_delta, overflow_status);
        FRU64_LOG_DELTA_U64_NONZERO_CACHED(term_minus, log_delta_idx_max, log_delta_list_base, log_delta_parameter_list_base, (u64)(exofreq_old_plus_1));
        FRU128_FROM_FRU64_MULTIPLY_U64(entropy_delta, term_minus, (u64)(freq_old_minus_1));
        FRU64_LOG_U64_NONZERO_CACHED(term_minus, log_idx_max, log_list_base, log_parameter_list_base, (u64)(exofreq));
        FRU128_ADD_FRU64_LO_SELF(entropy_delta, term_minus, overflow_status);
        FRU128_SHIFT_LEFT_SELF(entropy_delta, 64-58, overflow_status);
        FRU128_SUBTRACT_FRU128_SELF(entropy, entropy_delta, overflow_status);
      }else if(mode==AGNENTROPROX_MODE_LOGFREEDOM){
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
      }else if(mode==AGNENTROPROX_MODE_SHANNON){
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
      }else{
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
  agnentroprox_base->log_delta_idx_max=log_delta_idx_max;
  agnentroprox_base->log_idx_max=log_idx_max;
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
  ULONG exofreq;
  ULONG exofreq_minus_1;
  ULONG exofreq_old;
  ULONG exofreq_old_plus_1;
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
      exofreq=freq_list_base1[mask];
      exofreq_old=freq_list_base1[mask_old];
      exofreq_minus_1=exofreq-1;
      exofreq_old_plus_1=exofreq_old+1;
      freq_list_base1[mask]=exofreq_minus_1;
      freq_list_base1[mask_old]=exofreq_old_plus_1;
/*
The exoentropy difference, dE, is:

  dE=(freq*log(exofreq+1))+(freq_old*log(exofreq_old+1)))-((freq+1)*log(exofreq))-((freq_old-1)*log(exofreq_old+2))
  dE=freq*(log(exofreq+1)-log(exofreq))+log(exofreq_old+1)-((freq_old-1)*(log(exofreq_old+2)-log(exofreq_old+1)))-log(exofreq)
  dE=(freq*log_delta(exofreq))+log(exofreq_old+1)-((freq_old-1)*log_delta(exofreq_old+1))-log(exofreq)
  dE=(freq*log_delta(exofreq))+log(exofreq_old_plus_1)-(freq_old_minus_1*log_delta(exofreq_old_plus_1))-log(exofreq)
*/
      FRU64_LOG_DELTA_U64_NONZERO_CACHED(term_plus, log_delta_idx_max, log_delta_list_base, log_delta_parameter_list_base, (u64)(exofreq));
      FRU128_FROM_FRU64_MULTIPLY_U64(entropy_delta, term_plus, (u64)(freq));
      FRU64_LOG_U64_NONZERO_CACHED(term_plus, log_idx_max, log_list_base, log_parameter_list_base, (u64)(exofreq_old_plus_1));
      FRU128_ADD_FRU64_LO_SELF(entropy_delta, term_plus, overflow_status);
      FRU128_SHIFT_LEFT_SELF(entropy_delta, 64-58, overflow_status);
      FRU128_ADD_FRU128_SELF(exoentropy, entropy_delta, overflow_status);
      FRU64_LOG_DELTA_U64_NONZERO_CACHED(term_minus, log_delta_idx_max, log_delta_list_base, log_delta_parameter_list_base, (u64)(exofreq_old_plus_1));
      FRU128_FROM_FRU64_MULTIPLY_U64(entropy_delta, term_minus, (u64)(freq_old_minus_1));
      FRU64_LOG_U64_NONZERO_CACHED(term_minus, log_idx_max, log_list_base, log_parameter_list_base, (u64)(exofreq));
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
  agnentroprox_base->log_delta_idx_max=log_delta_idx_max;
  agnentroprox_base->log_idx_max=log_idx_max;
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
    fracterval_u64_free(agnentroprox_base->log_parameter_list_base);
    fracterval_u64_free(agnentroprox_base->log_list_base);
    fracterval_u64_free(agnentroprox_base->log_delta_parameter_list_base);
    fracterval_u64_free(agnentroprox_base->log_delta_list_base);
    poissocache_free_all(agnentroprox_base->poissocache_base);
    agnentroprox_free(agnentroprox_base->freq_list_base2);
    agnentroprox_free(agnentroprox_base->freq_list_base1);
    agnentroprox_free(agnentroprox_base->freq_list_base0);
    agnentroprox_base=agnentroprox_free(agnentroprox_base);
  }
  return agnentroprox_base;
}

agnentroprox_t *
agnentroprox_init(u32 build_break_count, u32 build_feature_count, u8 granularity, loggamma_t *loggamma_base, ULONG mask_idx_max_max, u32 mask_max, u16 mode_bitmap, u8 overlap_status, ULONG sweep_mask_idx_max_max){
/*
Verify that the source code is sufficiently updated and initialize private storage.

In:

  build_break_count is the caller's most recent knowledge of AGNENTROPROX_BUILD_BREAK_COUNT, which will fail if the caller is unaware of all critical updates.

  build_feature_count is the caller's most recent knowledge of AGNENTROPROX_BUILD_FEATURE_COUNT, which will fail if this library is not up to date with the caller's expectations.

  granularity is the size of each mask, less one. Limits may change from one version of this code to then next, so there is no explicit restriction.

  loggamma_base is the return value of loggamma_init().

  mask_idx_max_max is the maximum possible number of masks that could possibly occur within a haystack, less one. If overlap_status is set, then this value should be the maximum possible size of a haystack (in bytes, not masks), less (granularity+1). See also agnentroprox_mask_idx_max_get().

  mask_max is the maximum possible mask, which must not exeed (((granularity+1)<<U8_BITS_LOG2)-1). For example, when dealing with bytes, this would be U8_MAX. Due to implied allocation requirements, only compiling with (-D_64_) will allow the full u32 range. The limits on it are tricky to compute so this function tests them explicitly. Zero is tested for and disallowed because it creates more trouble than it's worth. For AGNENTROPROX_MODE_KURTOSIS and AGNENTROPROX_MODE_VARIANCE, this value must be one ones less than a power of 2, due to assumptions about where signed and unsigned masks are centered.

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
  ULONG *freq_list_base2;
  ULONG log_delta_idx_max;
  fru64 *log_delta_list_base;
  u64 *log_delta_parameter_list_base;
  ULONG log_idx_max;
  fru64 *log_list_base;
  u64 *log_parameter_list_base;
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
  status=fracterval_u128_init(1, 1);
  status=(u8)(status|fracterval_u64_init(0, 0));
  status=(u8)(status|(AGNENTROPROX_BUILD_FEATURE_COUNT<build_feature_count));
  status=(u8)(status|(build_break_count!=AGNENTROPROX_BUILD_BREAK_COUNT));
  status=(u8)(status|(!mask_max));
  mask_count_max_max=mask_idx_max_max+1;
  status=(u8)(status|(!mask_count_max_max));
  mask_span=(u64)(mask_max)+1;
  mask_count_plus_span_max_max=(ULONG)(mask_count_max_max+mask_span);
  status=(u8)(status|(mask_count_plus_span_max_max<=mask_span));
  if(mode_bitmap&AGNENTROPROX_MODE_JSD){
/*
In order to compute JSD, we need to ensure that double the haystack mask count won't wrap.
*/
    status=(u8)(status|((mask_count_max_max<<1)<mask_count_max_max));
  }
  status=(u8)(status|(U32_BYTE_MAX<granularity));
  granularity=(u8)(granularity&U32_BYTE_MAX);
  status=(u8)(status|((u32)((1U<<(granularity<<U8_BITS_LOG2)<<U8_BITS)-1)<mask_max));
  status=(u8)(status|(mask_idx_max_max<sweep_mask_idx_max_max));
  mask_span_power_of_2_status=!(mask_max&(mask_max+1));
  if(mode_bitmap&(AGNENTROPROX_MODE_KURTOSIS|AGNENTROPROX_MODE_VARIANCE)){
    status=(u8)(status|(!mask_span_power_of_2_status)|overlap_status);
  }
/*
For logfreedom computation, we need a list of pairs (frequency, population) which give the populations of various frequencies. The maximum possible number of _unique_ nonzero frequencies with nonzero population occurs when there is, one mask with frequency one, one with frequency 2, one with frequency 3, etc. If the greatest such frequency is N, then the minimum mask count needed to construct this scenario is (1+2+3+...+N), which is just ((N*(N+1))>>1). Furthermore the maximum mask count guaranteed to have at most N unique nonzero frequencies of nonzero population is (((N*(N+1))>>1)+N). However, up to (N+1) unique frequencies could have nonzero population because frequency zero might have nonzero population. Therefore N is in fact the maximum possible index of a list of such pairs. Unfortunately, due to the way in which logfreedom is evaluated, it's possible that the number of nonzero populations would be as much as 2 greater than its theoretical maximum, at any given time. Given all this, find the minimum such N such that:

  (mask_idx_max_max+1)<=((((N-2)*((N-2)+1))>>1)+(N-2))

Technically, we should use sweep_mask_idx_max_max instead of mask_idx_max_max, but the former is defined on In to be speculative, so it wouldn't be safe to do so. This is acceptable because N merely scales are the root of the latter. Furthermore, (N-2) need not exceed mask_max because at most (mask_max+1) unique masks cannot possibly give rise to more than equally many unique frequencies of nonzero population (but we could still temporarily overrun by 2). So N can be clipped to any value small enough such that:

  (mask_max+1)<=(N-2)
  mask_max<(N-2)

And finally, it's good enough to find N which is one less than a power of 2, which is simple and fast. In fact, this is required for the correct operation of Poissocache.
*/
  n=U32_MAX;
  do{
    n_minus_2=n-2;
    if((n_minus_2<=mask_max)&&((((((u64)(n_minus_2)+1)*n_minus_2)>>1)+n_minus_2)<(mask_idx_max_max+1))){
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

With the exception of JSD, exoentropy and exoelasticity computation, it will be rare that any operand to log, log delta, or loggamma exceeds (sweep_mask_idx_max_max+1), so this should be the maximum index of the subset of those math caches which is required in light of the particular mode_bitmap. For exoentropy, this increases to (mask_idx_max_max+1).
*/
    agnentroprox_base=DEBUG_CALLOC_PARANOID((ULONG)(sizeof(agnentroprox_t)));
    if(agnentroprox_base){
      msb=U32_BIT_MAX;
      while(!(mask_max>>msb)){
        msb--;
      }
      mask_sign_mask=(u32)(1U<<msb);
      mean_shift=(u8)(U128_BIT_MAX-msb);
      agnentroprox_base->loggamma_base=loggamma_base;
      agnentroprox_base->mask_max=mask_max;
      agnentroprox_base->mask_max_msb=msb;
      agnentroprox_base->mask_sign_mask=mask_sign_mask;
      agnentroprox_base->granularity=granularity;
      agnentroprox_base->mean_shift=mean_shift;
      agnentroprox_base->overlap_status=overlap_status;
      freq_list_base0=agnentroprox_ulong_list_malloc((ULONG)(mask_max));
      status=(u8)(status|!freq_list_base0);
      agnentroprox_base->freq_list_base0=freq_list_base0;
      freq_list_base1=agnentroprox_ulong_list_malloc((ULONG)(mask_max));
      status=(u8)(status|!freq_list_base1);
      agnentroprox_base->freq_list_base1=freq_list_base1;
      if(mode_bitmap&AGNENTROPROX_MODE_JSD){
        freq_list_base2=agnentroprox_ulong_list_malloc((ULONG)(mask_max));
        status=(u8)(status|!freq_list_base2);
        agnentroprox_base->freq_list_base2=freq_list_base2;
      }
      poissocache_base=poissocache_init(0, 0, poissocache_item_idx_max);
      status=(u8)(status|!poissocache_base);
      agnentroprox_base->poissocache_base=poissocache_base;

      if(!status){
/*
Allocate math caches for previously computed log, log delta, and loggamma fractervals. Start with the maximum reasonable expectation of the number of unique results we would need to recall at any given time, which is essentially the number of masks in the sweep window, then back off exponentially until we succeed. cache_idx_max will be one less than this value. We must set it to one less than a power of 2 because the cache functions will use it as an index AND mask.
*/
        cache_idx_max=sweep_mask_idx_max_max;
        msb=0;
        if(cache_idx_max){
          msb=ULONG_BIT_MAX;
          while(!(cache_idx_max>>msb)){
            msb--;
          }
        }
        cache_idx_max=1;
        cache_idx_max<<=msb;
        cache_idx_max<<=1;
        cache_idx_max--;
/*
Caches which are not required shall have single-item allocations just to satisfy accessability expectations.
*/
        log_idx_max=0;
        log_delta_idx_max=0;
        loggamma_idx_max=0;
        if(mode_bitmap&(AGNENTROPROX_MODE_DIVENTROPY|AGNENTROPROX_MODE_EXOENTROPY|AGNENTROPROX_MODE_EXOELASTICITY|AGNENTROPROX_MODE_JSD|AGNENTROPROX_MODE_SHANNON)){
          log_delta_idx_max=cache_idx_max;
        }
        if(mode_bitmap&(AGNENTROPROX_MODE_AGNENTROPY|AGNENTROPROX_MODE_EXOENTROPY|AGNENTROPROX_MODE_EXOELASTICITY|AGNENTROPROX_MODE_DIVENTROPY|AGNENTROPROX_MODE_JSD|AGNENTROPROX_MODE_LOGFREEDOM|AGNENTROPROX_MODE_SHANNON)){
          log_idx_max=cache_idx_max;
        }
        if(mode_bitmap&(AGNENTROPROX_MODE_AGNENTROPY|AGNENTROPROX_MODE_LOGFREEDOM)){
          loggamma_idx_max=cache_idx_max;
        }
        do{
        log_delta_parameter_list_base=fracterval_u64_log_u64_cache_init(log_delta_idx_max, &log_delta_list_base);
        status=!log_delta_parameter_list_base;
        log_parameter_list_base=fracterval_u64_log_u64_cache_init(log_idx_max, &log_list_base);
        status=(u8)(status|!log_parameter_list_base);
        loggamma_parameter_list_base=loggamma_u64_cache_init(loggamma_idx_max, &loggamma_list_base);
        status=(u8)(status|!loggamma_parameter_list_base);
          if(!status){
            agnentroprox_base->log_delta_idx_max=log_delta_idx_max;
            agnentroprox_base->log_delta_list_base=log_delta_list_base;
            agnentroprox_base->log_delta_parameter_list_base=log_delta_parameter_list_base;
            agnentroprox_base->log_idx_max=log_idx_max;
            agnentroprox_base->log_list_base=log_list_base;
            agnentroprox_base->log_parameter_list_base=log_parameter_list_base;
            agnentroprox_base->loggamma_idx_max=loggamma_idx_max;
            agnentroprox_base->loggamma_list_base=loggamma_list_base;
            agnentroprox_base->loggamma_parameter_list_base=loggamma_parameter_list_base;
/*
Precompute a fracterval giving the log(mask_max+1), which is used to compute raw entropy.
*/
            FRU128_LOG_U64(mask_span_log, (u64)(mask_span), overflow_status);
/*
Convert from 6.122 to 6.64 fixed-point.
*/
            FRU128_SHIFT_RIGHT_SELF(mask_span_log, 122-64);
            agnentroprox_base->mask_span_log=mask_span_log;
            break;
          }else{
            loggamma_free(loggamma_parameter_list_base);
            loggamma_free(loggamma_list_base);
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
        agnentroprox_ulong_list_zero((ULONG)(mask_max), freq_list_base0);
        agnentroprox_ulong_list_zero((ULONG)(mask_max), freq_list_base1);
      }else{
        agnentroprox_base=agnentroprox_free_all(agnentroprox_base);
      }
    }
  }
  return agnentroprox_base;
}
//rml get rid of freqlistbase2 and references to it, then get rid of the need for mode_bitmap altogether

fru128
agnentroprox_jsd_get(agnentroprox_t *agnentroprox_base, u8 *haystack_mask_list_base, u8 *overflow_status_base){
/*
Compute (1-(normalized Jensen-Shannon divergence)) between needle and haystack frequency lists. (It's commutative so the terms "needle" and "haystack" are only used for the sake of consistency with other functions in which direction matters.)

In:

  The needle frequency list must have been preloaded with agnentroprox_needle_mask_list_load() prior to calling this function, with no intervening alterations.

  agnentroprox_base is the return value of agenentroprox_init().

  *haystack_mask_list_base is the haystack. It must contain the same number of masks as the needle. This can be assured by scaling the needle size via agnentroprox_needle_freq_list_equalize().

  *overflow_status_base is the OR-cummulative fracterval overflow status.

Out:

  Returns (1-(normalized Jensen-Shannon divergence)) between a needle and a haystack.

  *overflow_status_base is one if a fracterval overflow occurred, else unchanged.
*/
  ULONG freq;
  fru128 entropy_delta;
  fru128 haystack_jsd;
  ULONG haystack_freq;
  ULONG *haystack_freq_list_base;
  fru128 haystack_implied_entropy;
  fru128 haystack_shannon_entropy;
  u8 ignored_status;
  fru128 jsd;
  fru128 jsd_coeff;
  ULONG log_idx_max;
  fru64 *log_list_base;
  u64 *log_parameter_list_base;
  fru64 log0;
  fru64 log1;
  u64 log2;
  u128 log2_recip_half;
  u32 mask;
  ULONG mask_count;
  ULONG mask_idx_max;
  u32 mask_max;
  fru128 needle_jsd;
  ULONG needle_freq;
  ULONG *needle_freq_list_base;
  fru128 needle_implied_entropy;
  fru128 needle_shannon_entropy;
  u8 overflow_status;

  haystack_freq_list_base=agnentroprox_base->freq_list_base1;
  mask_max=agnentroprox_base->mask_max;
  overflow_status=*overflow_status_base;
  ignored_status=0;
  agnentroprox_ulong_list_zero((ULONG)(mask_max), haystack_freq_list_base);
  mask_count=agnentroprox_base->mask_count0;
  mask_idx_max=mask_count-1;
  agnentroprox_base->mask_count1=0;
  agnentroprox_mask_list_accrue(agnentroprox_base, 1, mask_idx_max, haystack_mask_list_base);
  haystack_freq_list_base=agnentroprox_base->freq_list_base1;
  log_idx_max=agnentroprox_base->log_idx_max;
  log_list_base=agnentroprox_base->log_list_base;
  log_parameter_list_base=agnentroprox_base->log_parameter_list_base;
  needle_freq_list_base=agnentroprox_base->freq_list_base0;
/*
Compute D=(1-(normalized Jensen-Shannon divergence)):

  D=1-(((I0-S0)+(I1-S1))/(2 ln 2))

where

  I0=(implied entropy of needle with respect to (haystack plus needle))
  I1=(implied entropy of haystack with respect to (haystack plus needle))
  S0=(Shannon entropy of needle)
  S1=(Shannon entropy of haystack)

where

  I0=(Q*log(2Q))-Σ(M=(0, Z-1), F0[M]*log(F0[M]+F1[M]))
  I1=(Q*log(2Q))-Σ(M=(0, Z-1), F1[M]*log(F0[M]+F1[M]))
  S0=(Q*log(Q))-Σ(M=(0, Z-1), F0[M]*log(F0[M]))
  S1=(Q*log(Q))-Σ(M=(0, Z-1), F1[M]*log(F1[M]))

where:

  Q=(needle mask count)=(haystack mask count)
  F0[M]=(needle frequency of mask M)
  F1[M]=(haystack frequency of mask M)
  Z=(mask span)
*/
  FRU128_SET_ZERO(jsd);
  FRU64_LOG_U64_NONZERO_CACHED(log0, log_idx_max, log_list_base, log_parameter_list_base, (u64)(mask_count));
  FRU128_FROM_FRU64_MULTIPLY_U64(haystack_shannon_entropy, log0, (u64)(mask_count));
  needle_shannon_entropy=haystack_shannon_entropy;
  log2=FRU128_LOG2_FLOOR_HI>>(U64_BITS-58);
  FRU64_ADD_FTD64_SELF(log0, log2, overflow_status);
  FRU128_FROM_FRU64_MULTIPLY_U64(haystack_implied_entropy, log0, (u64)(mask_count));
  needle_implied_entropy=haystack_implied_entropy;
  mask=0;
  do{
    haystack_freq=haystack_freq_list_base[mask];
    needle_freq=needle_freq_list_base[mask];
    freq=haystack_freq+needle_freq;
    if(1<freq){
      FRU64_LOG_U64_NONZERO_CACHED(log0, log_idx_max, log_list_base, log_parameter_list_base, (u64)(freq));
      if(haystack_freq){
        if(haystack_freq!=1){
          FRU128_FROM_FRU64_MULTIPLY_U64(entropy_delta, log0, (u64)(haystack_freq));
          FRU128_SUBTRACT_FRU128_SELF(haystack_implied_entropy, entropy_delta, ignored_status);
          FRU64_LOG_U64_NONZERO_CACHED(log1, log_idx_max, log_list_base, log_parameter_list_base, (u64)(haystack_freq));
          FRU128_FROM_FRU64_MULTIPLY_U64(entropy_delta, log1, (u64)(haystack_freq));
          FRU128_SUBTRACT_FRU128_SELF(haystack_shannon_entropy, entropy_delta, ignored_status);
        }else{
          FRU128_SUBTRACT_FRU64_LO_SELF(haystack_implied_entropy, log0, ignored_status);
        }
      }
      if(needle_freq){
        if(needle_freq!=1){
          FRU128_FROM_FRU64_MULTIPLY_U64(entropy_delta, log0, (u64)(needle_freq));
          FRU128_SUBTRACT_FRU128_SELF(needle_implied_entropy, entropy_delta, ignored_status);
          FRU64_LOG_U64_NONZERO_CACHED(log1, log_idx_max, log_list_base, log_parameter_list_base, (u64)(needle_freq));
          FRU128_FROM_FRU64_MULTIPLY_U64(entropy_delta, log1, (u64)(needle_freq));
          FRU128_SUBTRACT_FRU128_SELF(needle_shannon_entropy, entropy_delta, ignored_status);
        }else{
          FRU128_SUBTRACT_FRU64_LO_SELF(needle_implied_entropy, log0, ignored_status);
        }
      }
    }
  }while((mask++)!=mask_max);
/*
Compute C as given above.

Overflows are safe to ignore because by definition the return value is on [0.0, 1.0], and result saturation does the right thing. By the way, (64-58) in the shifts below reflect conversion from 6.58 to 6.64 fixed point.
*/
  FRU128_SUBTRACT_FRU128(haystack_jsd, haystack_implied_entropy, haystack_shannon_entropy, ignored_status);
  U128_FROM_U64_PAIR(log2_recip_half, AGNENTROPROX_LOG2_RECIP_HALF_LO, AGNENTROPROX_LOG2_RECIP_HALF_HI);
  FRU128_FROM_FTD128(jsd_coeff, log2_recip_half);
  FRU128_DIVIDE_U64_SELF(jsd_coeff, (u64)(mask_count), ignored_status);
  FRU128_SUBTRACT_FRU128(needle_jsd, needle_implied_entropy, needle_shannon_entropy, ignored_status);
  FRU128_ADD_FRU128(jsd, haystack_jsd, needle_jsd, ignored_status);
  FRU128_MULTIPLY_FRU128_SELF(jsd, jsd_coeff);
  FRU128_SHIFT_LEFT_SELF(jsd, U64_BITS+(U64_BITS-58), ignored_status);
  FRU128_NOT_SELF(jsd);
  agnentroprox_base->entropy=jsd;
  agnentroprox_base->haystack_implied_entropy=haystack_implied_entropy;
  agnentroprox_base->haystack_shannon_entropy=haystack_shannon_entropy;
/*
Write ignored_status to prevent the compiler from complaining about it not being used.
*/
  agnentroprox_base->ignored_status=ignored_status;
  agnentroprox_base->jsd_coeff=jsd_coeff;
  agnentroprox_base->log_idx_max=log_idx_max;
  agnentroprox_base->needle_implied_entropy=needle_implied_entropy;
  agnentroprox_base->needle_shannon_entropy=needle_shannon_entropy;
  *overflow_status_base=overflow_status;
  return jsd;
}

ULONG
agnentroprox_jsd_transform(agnentroprox_t *agnentroprox_base, u8 append_mode, ULONG haystack_mask_idx_max, u8 *haystack_mask_list_base, fru128 *jsd_list_base, ULONG match_idx_max_max, ULONG *match_u8_idx_list_base, u8 *overflow_status_base){
/*
Compute the (1-(normalized Jensen-Shannon divergence)) transform of a haystack with respect to a preloaded needle frequency list, given a particular sweep.

In:

  The needle frequency list must have been preloaded with agnentroprox_needle_mask_list_load() prior to calling this function, with no intervening alterations. The sweep is implied to equal the number of masks in the needle, which can be assured via agnentroprox_needle_freq_list_equalize().

  agnentroprox_base is the return value of agenentroprox_init().

  append_mode is 2 to append diventropies one at at time to *diventropy_list_base, ordered ascending by the base index of the sweep window. Else one to sort results ascending by diventropy, or zero to sort descending.

  haystack_mask_idx_max is one less than the number of masks in the haystack, such that if agnentroprox_init():In:overlap_status was one, then this value would need to be increased in order to account for mask overlap. For example, if the sweep contains 5 of 3-byte masks, then this value would be 4 _without_ overlap, or 12 _with_ overlap. Must not exceed agnentroprox_init():In:mask_idx_max_max. See also agnentroprox_mask_idx_max_get().

  *haystack_mask_list_base is the haystack.

  *jsd_list_base contains (match_idx_max_max+1) undefined items.

  match_idx_max_max is one less than the maximum number of matches to report.

  *match_u8_idx_list_base is NULL if the sweep base indexes associated with matches can be discarded, else the base of (match_idx_max_max+1) undefined items to hold such indexes.

  *overflow_status_base is the OR-cummulative fracterval overflow status.

Out:

  Returns the number of matches found, which is simply (MIN((haystack_mask_idx_max-sweep_mask_idx_max))+1, match_idx_max_max), where sweep_mask_idx_max is just agnentroprox_needle_mask_list_load():In:mask_idx_max.

  *jsd_list_base contains (return value) items which represent the matches identified during the search, sorted according to append_mode.

  *overflow_status_base is one if a fracterval overflow occurred, else unchanged.
*/
  fru128 entropy_delta;
  ULONG freq;
  u8 granularity;
  ULONG haystack_freq;
  ULONG *haystack_freq_list_base;
  ULONG haystack_freq_old;
  fru128 haystack_implied_entropy;
  fru128 haystack_jsd;
  fru128 haystack_shannon_entropy;
  u8 ignored_status;
  fru128 jsd;
  fru128 jsd_coeff;
  u128 jsd_mean;
  u128 jsd_threshold;
  fru64 log;
  fru64 log_delta;
  ULONG log_delta_idx_max;
  fru64 *log_delta_list_base;
  u64 *log_delta_parameter_list_base;
  ULONG log_idx_max;
  fru64 *log_list_base;
  u64 *log_parameter_list_base;
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
  fru128 needle_implied_entropy;
  fru128 needle_jsd;
  fru128 needle_shannon_entropy;
  u8 overflow_status;
  u8 overlap_status;
  ULONG sweep_mask_idx_max;
  ULONG u8_idx;
  ULONG u8_idx_old;
  ULONG u8_idx_delta;
  ULONG u8_idx_max;

  overflow_status=*overflow_status_base;
  sweep_mask_idx_max=agnentroprox_base->mask_count0-1;
  ignored_status=0;
  agnentroprox_jsd_get(agnentroprox_base, haystack_mask_list_base, &overflow_status);
  haystack_implied_entropy=agnentroprox_base->haystack_implied_entropy;
  haystack_shannon_entropy=agnentroprox_base->haystack_shannon_entropy;
  FRU128_SUBTRACT_FRU128(haystack_jsd, haystack_implied_entropy, haystack_shannon_entropy, ignored_status);
  needle_implied_entropy=agnentroprox_base->needle_implied_entropy;
  needle_shannon_entropy=agnentroprox_base->needle_shannon_entropy;
  FRU128_SUBTRACT_FRU128(needle_jsd, needle_implied_entropy, needle_shannon_entropy, ignored_status);
  FRU128_ADD_FRU128(jsd, haystack_jsd, needle_jsd, ignored_status);
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
  jsd=agnentroprox_base->entropy;
  granularity=agnentroprox_base->granularity;
  haystack_freq_list_base=agnentroprox_base->freq_list_base1;
  log_delta_idx_max=agnentroprox_base->log_delta_idx_max;
  log_delta_list_base=agnentroprox_base->log_delta_list_base;
  log_delta_parameter_list_base=agnentroprox_base->log_delta_parameter_list_base;
  log_idx_max=agnentroprox_base->log_idx_max;
  log_list_base=agnentroprox_base->log_list_base;
  log_parameter_list_base=agnentroprox_base->log_parameter_list_base;
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
Reevaluate the D in terms of the changes in I0, I1, S0, and S1 as defined in agnentroprox_jsd_get(). Given:

  I0=(Q0*log(Q0+Q1))-Σ(M=(0, Z-1), F0[M]*log(F0[M]+F1[M]))
  I1=(Q1*log(Q0+Q1))-Σ(M=(0, Z-1), F1[M]*log(F0[M]+F1[M]))
  S0=(Q0*log(Q0))-Σ(M=(0, Z-1), F0[M]*log(F0[M]))
  S1=(Q1*log(Q1))-Σ(M=(0, Z-1), F1[M]*log(F1[M]))

where

  F0[M]=(needle frequency of mask M)
  F1[M]=(sweep frequency of mask M)
  I0=(implied entropy of needle with respect to (needle plus sweep))
  I1=(implied entropy of sweep with respect to (needle plus sweep))
  Q0=(needle mask count)
  Q1=(sweep mask count)
  S0=(Shannon entropy of needle)
  S1=(Shannon entropy of sweep)
  Z=(mask span)

then the deltas required to update jsd are:

  dI0=+(F0[mask_old]*log(F0[mask_old]+F1[mask_old]))
      -(F0[mask_old]*log(F0[mask_old]+F1[mask_old]-1))
      +(F0[mask]*log(F0[mask]+F1[mask]))
      -(F0[mask]*log(F0[mask]+F1[mask]+1))
  dI0=+(F0[mask_old]*log_delta(F0[mask_old]+F1[mask_old]-1))
      -(F0[mask]*log_delta(F0[mask]+F1[mask]))

  dI1=+(F1[mask_old]*log(F0[mask_old]+F1[mask_old]))
      -((F1[mask_old]-1)*log(F0[mask_old]+F1[mask_old]-1))
      +(F1[mask]*log(F0[mask]+F1[mask]))
      -((F1[mask]+1)*log(F0[mask]+F1[mask]+1))
  dI1=+((F1[mask_old]-1)*log_delta(F0[mask_old]+F1[mask_old]-1))
      +log(F0[mask_old]+F1[mask_old]))
      -(F1[mask]*log_delta(F0[mask]+F1[mask]))
      -log(F0[mask]+F1[mask]+1)

  dS1=+(F1[mask_old]*log(F1[mask_old]))
      -((F1[mask_old]-1)*log(F1[mask_old]-1))
      +(F1[mask]*log(F1[mask]))
      -((F1[mask]+1)*log(F1[mask]+1))
    =+((F1[mask_old]-1)*log_delta(F1[mask_old]-1))
      +log(F1[mask_old])
      -(F1[mask]*log_delta(F1[mask]))
      -log(F1[mask]+1))

where the log of zero is taken to be zero. Note that S0 is constant because the needle is constant. So reevaluate as follows:

  D=1-(((I0+dI0-S0)+(I1+dI1-S1-dS1))/(2 ln 2))
*/
      haystack_freq_old=haystack_freq_list_base[mask_old];
      needle_freq_old=needle_freq_list_base[mask_old];
      freq=haystack_freq_old+needle_freq_old;
      if(freq!=1){
        FRU64_LOG_U64_NONZERO_CACHED(log, log_idx_max, log_list_base, log_parameter_list_base, (u64)(freq));
        FRU128_ADD_FRU64_LO_SELF(haystack_implied_entropy, log, overflow_status);
        freq--;
        FRU64_LOG_DELTA_U64_NONZERO_CACHED(log_delta, log_delta_idx_max, log_delta_list_base, log_delta_parameter_list_base, (u64)(freq));
        FRU128_FROM_FRU64_MULTIPLY_U64(entropy_delta, log_delta, (u64)(needle_freq_old));
        FRU128_ADD_FRU128_SELF(needle_implied_entropy, entropy_delta, overflow_status);
        freq=haystack_freq_old-1;
        if(freq){
          FRU128_FROM_FRU64_MULTIPLY_U64(entropy_delta, log_delta, (u64)(freq));
          FRU128_ADD_FRU128_SELF(haystack_implied_entropy, entropy_delta, overflow_status);
          FRU64_LOG_DELTA_U64_NONZERO_CACHED(log_delta, log_delta_idx_max, log_delta_list_base, log_delta_parameter_list_base, (u64)(freq));
          FRU128_FROM_FRU64_MULTIPLY_U64(entropy_delta, log_delta, (u64)(freq));
          FRU128_ADD_FRU128_SELF(haystack_shannon_entropy, entropy_delta, overflow_status);
          FRU64_LOG_U64_NONZERO_CACHED(log, log_idx_max, log_list_base, log_parameter_list_base, (u64)(haystack_freq_old));
          FRU128_ADD_FRU64_LO_SELF(haystack_shannon_entropy, log, overflow_status);
        }
      }
      haystack_freq=haystack_freq_list_base[mask];
      needle_freq=needle_freq_list_base[mask];
      freq=haystack_freq+needle_freq;
      haystack_freq_old--;
      haystack_freq_list_base[mask_old]=haystack_freq_old;
      if(freq){
        FRU64_LOG_DELTA_U64_NONZERO_CACHED(log_delta, log_delta_idx_max, log_delta_list_base, log_delta_parameter_list_base, (u64)(freq));
        FRU128_FROM_FRU64_MULTIPLY_U64(entropy_delta, log_delta, (u64)(needle_freq));
        FRU128_SUBTRACT_FRU128_SELF(needle_implied_entropy, entropy_delta, ignored_status);
        FRU128_FROM_FRU64_MULTIPLY_U64(entropy_delta, log_delta, (u64)(haystack_freq));
        FRU128_SUBTRACT_FRU128_SELF(haystack_implied_entropy, entropy_delta, ignored_status);
        freq++;
        FRU64_LOG_U64_NONZERO_CACHED(log, log_idx_max, log_list_base, log_parameter_list_base, (u64)(freq));
        FRU128_SUBTRACT_FRU64_LO_SELF(haystack_implied_entropy, log, ignored_status);
        if(haystack_freq){
          FRU64_LOG_DELTA_U64_NONZERO_CACHED(log_delta, log_delta_idx_max, log_delta_list_base, log_delta_parameter_list_base, (u64)(haystack_freq));
          FRU128_FROM_FRU64_MULTIPLY_U64(entropy_delta, log_delta, (u64)(haystack_freq));
          FRU128_SUBTRACT_FRU128_SELF(haystack_shannon_entropy, entropy_delta, ignored_status);
          freq=haystack_freq+1;
          FRU64_LOG_U64_NONZERO_CACHED(log, log_idx_max, log_list_base, log_parameter_list_base, (u64)(freq));
          FRU128_SUBTRACT_FRU64_LO_SELF(haystack_shannon_entropy, log, ignored_status);
        }
      }
/*
Compute C as given above.

Overflows are safe to ignore because by definition the values at jsd_list_base are all on [0.0, 1.0], and result saturation does the right thing. Do a silly dance to get the compiler to think that we actually care.
*/
      haystack_freq++;
      haystack_freq_list_base[mask]=haystack_freq;
      FRU128_SUBTRACT_FRU128(haystack_jsd, haystack_implied_entropy, haystack_shannon_entropy, ignored_status);
      FRU128_SUBTRACT_FRU128(needle_jsd, needle_implied_entropy, needle_shannon_entropy, ignored_status);
      FRU128_ADD_FRU128(jsd, haystack_jsd, needle_jsd, ignored_status);
      FRU128_MEAN_TO_FTD128(jsd_mean, jsd);
    }
    if(!append_mode){
      match_status_not=U128_IS_LESS(jsd_threshold, jsd_mean);
      if(!match_status_not){
        match_status_not=fracterval_u128_rank_list_insert_ascending(jsd, &match_count, &match_idx, match_idx_max_max, jsd_list_base, &jsd_threshold);
      }
    }else if(append_mode==1){
      match_status_not=U128_IS_LESS(jsd_mean, jsd_threshold);
      if(!match_status_not){
        match_status_not=fracterval_u128_rank_list_insert_descending(jsd, &match_count, &match_idx, match_idx_max_max, jsd_list_base, &jsd_threshold);
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
  jsd_coeff=agnentroprox_base->jsd_coeff;
  match_idx=0;
  do{      
    jsd=jsd_list_base[match_idx];
    FRU128_MULTIPLY_FRU128_SELF(jsd, jsd_coeff);
    FRU128_SHIFT_LEFT_SELF(jsd, U64_BITS+(U64_BITS-58), ignored_status);
    FRU128_NOT_SELF(jsd);
    jsd_list_base[match_idx]=jsd;
    match_idx++;
  }while(match_idx!=match_count);
/*
Write ignored_status to prevent the compiler from complaining about it not being used.
*/
  agnentroprox_base->ignored_status=ignored_status;
  agnentroprox_base->log_idx_max=log_idx_max;
  *overflow_status_base=overflow_status;
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

void
agnentroprox_mask_list_deltafy(u8 channel_status, u8 delta_status, u8 granularity, ULONG mask_idx_max, u8 *mask_list_base){
/*
Perform reversible sample (un)differencing on a mask list in order to enhance signal detection or comparison.

In:

  channel_status is zero if each mask is a unified integer, else one if each mask consists of (granularity+1) parallel byte channels. For example, this value should be one for pixels consisting of red, green, and blue bytes.

  delta_status is one if and only if *mask_list_base should be the first delta of its original state. If the latter is {A, B, C...}, then its first delta is {A, (B-A), (C-B)...}, ANDed with ((1U<<(granularity<<U8_BITS_LOG2)<<U8_BITS)-1). Otherwise zero to reverse the process.

  granularity is the agnentroprox_init():In:granularity, or will be if agnentroprox_init() has not yet been called. On [0, U32_BYTE_MAX].

  mask_idx_max is one less than the number of masks of size (granularity+1) at *mask_list_base, regardless of agnentroprox_init():In:overlap_status.

  *mask_list_base is the mask list.

Out:

  *mask_list_base is as described in In:delta_status.
*/
  u32 mask;
  u32 mask_max;
  u32 mask_old;
  u8 mask_u8;
  ULONG u8_idx;
  u32 u8_idx_delta;
  ULONG u8_idx_max;

  mask_max=(1U<<(granularity<<U8_BITS_LOG2)<<U8_BITS)-1;
  mask_old=0;
  u8_idx=0;
  u8_idx_delta=(u8)(granularity+1);
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
    if(delta_status){
      if(!channel_status){
        mask-=mask_old;
        mask_old+=mask;
        mask&=mask_max;
      }else{
        mask=(((mask>>U24_BITS)-(mask_old>>U24_BITS))<<U24_BITS)|((u32)((u8)((mask>>U16_BITS)-(mask_old>>U16_BITS)))<<U16_BITS)|((u32)((u8)((mask>>U8_BITS)-(mask_old>>U8_BITS)))<<U8_BITS)|(u8)(mask-mask_old);
        mask_old=(((mask>>U24_BITS)+(mask_old>>U24_BITS))<<U24_BITS)|((u32)((u8)((mask>>U16_BITS)+(mask_old>>U16_BITS)))<<U16_BITS)|((u32)((u8)((mask>>U8_BITS)+(mask_old>>U8_BITS)))<<U8_BITS)|(u8)(mask+mask_old);
      }
    }else{
      if(!channel_status){
        mask+=mask_old;
        mask_old=mask;
        mask&=mask_max;
      }else{
        mask=(((mask>>U24_BITS)+(mask_old>>U24_BITS))<<U24_BITS)|((u32)((u8)((mask>>U16_BITS)+(mask_old>>U16_BITS)))<<U16_BITS)|((u32)((u8)((mask>>U8_BITS)+(mask_old>>U8_BITS)))<<U8_BITS)|(u8)(mask+mask_old);
        mask_old=mask;
      }
    }
    mask_u8=(u8)(mask);
    mask_list_base[u8_idx]=mask_u8;
    if(granularity){
      mask_u8=(u8)(mask>>U8_BITS);
      mask_list_base[u8_idx+U16_BYTE_MAX]=mask_u8;
      if(U16_BYTE_MAX<granularity){
        mask_u8=(u8)(mask>>U16_BITS);
        mask_list_base[u8_idx+U24_BYTE_MAX]=mask_u8;
        if(U24_BYTE_MAX<granularity){
          mask_u8=(u8)(mask>>U24_BITS);
          mask_list_base[u8_idx+U32_BYTE_MAX]=mask_u8;
        }
      }
    }
    u8_idx+=u8_idx_delta;
  }while(u8_idx<=u8_idx_max);
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
agnentroprox_needle_freq_list_copy(agnentroprox_t *agnentroprox_base, u8 restore_status){
/*
Back up or restore the needle frequency list.

In

  agnentroprox_init():In:mode_bitmap must have had AGNENTROPROX_MODE_JSD set. Otherwise the backup frequency list will not have been allocated.

  agnentroprox_base is the return value of agenentroprox_init().

  restore_status is zero to back up the needle frequency list, else one to restore it.

Out

  The needle frequency list has been backed up or restored, as applicable.  
*/
  ULONG *freq_list_base0;
  ULONG *freq_list_base1;
  u32 mask_max;

  if(!restore_status){
    agnentroprox_base->mask_count2=agnentroprox_base->mask_count0;
    freq_list_base0=agnentroprox_base->freq_list_base0;
    freq_list_base1=agnentroprox_base->freq_list_base2;
  }else{
    agnentroprox_base->mask_count0=agnentroprox_base->mask_count2;
    freq_list_base0=agnentroprox_base->freq_list_base2;
    freq_list_base1=agnentroprox_base->freq_list_base0;
  }
  mask_max=agnentroprox_base->mask_max;
  agnentroprox_ulong_list_copy((ULONG)(mask_max), freq_list_base0, freq_list_base1);
  return;
}

void
agnentroprox_needle_freq_list_equalize(agnentroprox_t *agnentroprox_base, ULONG haystack_mask_idx_max){
/*
Alter the frequencies in the needle frequency list so that they sum to the same mask count as the haystack frequency list. We do this so that the haystack and/or sweep can remain fixed in size throughout a transform; we need only account for the change in needle size. If we didn't do this, the Jensen-Shannon divergence would explode in computational cost due to cache inefficiency, for example.

In

  agnentroprox_needle_mask_list_load() must have been called, so that the needle frequency list has already been loaded.

  agnentroprox_base is the return value of agenentroprox_init().

  haystack_mask_idx_max is one less than the number of masks in the haystack mask list, such that if agnentroprox_init():In:overlap_status was one, then this value would need to be increased in order to account for mask overlap. For example, if the sweep contains 5 of 3-byte masks, then this value would be 4 _without_ overlap, or 12 _with_ overlap. Must not exceed agnentroprox_init():In:mask_idx_max_max. See also agnentroprox_mask_idx_max_get().

Out:

  The needle frequency list has been altered such that the sum of its frequencies is (haystack_mask_idx_max+1), and such that all frequencies are at most one more or less than their fair value (which is rational, and not in general whole, hence the requirement for error dispersion). It's possible that some masks which are present in the needle will now have frequency zero.

  Because the change is irreversible, you may need to back up and restore the needle frequency list using agnentroprox_needle_freq_list_copy().
*/
  u128 freq_product;
  u128 freq_product_min;
  u64 freq_remainder;
  u64 freq_remainder_sum;
  u128 freq_remainder_u128;
  u64 freq_u64;
  u8 ignored_status;
  ULONG haystack_mask_count;
  u32 mask;
  u32 mask_max;
  ULONG *needle_freq_list_base;
  ULONG needle_mask_count;

  mask_max=agnentroprox_base->mask_max;
  needle_freq_list_base=agnentroprox_base->freq_list_base0;
  needle_mask_count=agnentroprox_base->mask_count0;
  haystack_mask_count=haystack_mask_idx_max+1;
  freq_remainder_sum=0;
  ignored_status=0;
  mask=0;
  do{
    freq_u64=needle_freq_list_base[mask];
    if(freq_u64){
      U128_FROM_U64_PRODUCT(freq_product, freq_u64, (u64)(haystack_mask_count));
/*
Dividing by needle_mask_count won't result in a quotient that won't fit into freq_u64, so ignore the return status.
*/
      U128_DIVIDE_U64_TO_U64_SATURATE(freq_u64, freq_product, (u64)(needle_mask_count), ignored_status);
      U128_FROM_U64_PRODUCT(freq_product_min, freq_u64, (u64)(needle_mask_count));
      U128_SUBTRACT_U128(freq_remainder_u128, freq_product, freq_product_min);
      U128_TO_U64_LO(freq_remainder, freq_remainder_u128);
      freq_remainder_sum+=freq_remainder;
      if(needle_mask_count<=freq_remainder_sum){
        freq_remainder_sum-=needle_mask_count;
        freq_u64++;
      }
      needle_freq_list_base[mask]=(ULONG)(freq_u64);
    }
  }while((mask++)!=mask_max);
  agnentroprox_base->mask_count0=haystack_mask_count;
  return;
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
  agnentroprox_base->log_idx_max=log_idx_max;
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
