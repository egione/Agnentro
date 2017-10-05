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
License version 3 along with the Agnentro Library (haystack_filename
"COPYING"). If not, see http://www.gnu.org/licenses/ .
*/
/*
Agnentropic Encoding/Decoding Kernel
*/
#include "flag.h"
#include "flag_biguint.h"
#include "flag_fracterval_u128.h"
#include "flag_loggamma.h"
#include "flag_agnentrocodec.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "constant.h"
#include "debug.h"
#include "debug_xtrn.h"
#include "fracterval_u128.h"
#include "fracterval_u128_xtrn.h"
#include "loggamma.h"
#include "loggamma_xtrn.h"
#include "agnentrocodec.h"
#include "agnentrocodec_xtrn.h"
#include "biguint.h"
#include "biguint_xtrn.h"
#include "bitscan.h"
#include "bitscan_xtrn.h"

u64
agnentrocodec_code_bit_idx_max_max_get(loggamma_t *loggamma_base, ULONG mask_idx_max_max, u32 mask_max){
/*
Compute an upper bound for the maximum bit index of the agnentropic encoding of a given mask count and mask span.

In:

  agnentrocodec_base is the return value of agnentrocodec_init().

  loggamma_base is the return value of loggamma_init().

  mask_idx_max_max is as defined in agnentrocodec_init().

  mask_max is as defined in agnentrocodec_init().

Out:

  Returns U64_MAX on failure, else the index described in the summary above.
*/
  fru128 bit_count;
  u64 bit_idx_max_max;
  u8 ignored_status;
  fru128 loggamma;
  u64 loggamma_parameter;
  fru128 nat_count;
  u8 status;
/*
A list of (mask_idx_max_max+1) masks on [0, mask_max], when agnentropically encoded, results in one of S states, given by:

  S = Pochhammer(mask_max+1, mask_idx_max_max+1)

where

  Pochhammer(a, b) = ((a+b-1)!/(a-1)!),

as one can see at WolframAlpha.

such that the maximum possible protoagnentropic code is (S-1) (but is usually less because there are fewer unique such codes than S, although a protoagnentropic code in noncanonical form could actually equal (S-1)). The infinitely precise maximum agnentropic code A is therefore given by:

  A = (S-1) / S

Now A, which is on [0, 1), must be computed to at most (B+2) bits, where B is the MSB of S. (If S happens to be a power of 2, then B bits would actually suffice, but that's rare.) In the worst case, (B+2) bits may be needed to guarantee the correct recovery of P, and thus the original uncompressed mask list, given A and S. (It might seem like only (B+1) bits would be required, but we actually need to reserve (B+2) because sometimes an extra bit of precision is required for a particular agnentropic code to "own" its terminating bit, in the sense that all subsequent bits could be altered without loss of reversibility.)

Putting it all together, we can derive an expression for B itself:

  B = floor(log2((mask_idx_max_max+mask_max+1)!/mask_max!))
  B = floor(log((mask_idx_max_max+mask_max+1)!/mask_max!)/log(2))

Given that

  log(X/Y) = ((log(X))-(log(Y))), Y > 0

and

  log(X!) = loggamma(X+1), X > 0

this simplifies to:

  B = floor((loggamma(mask_idx_max_max+mask_max+2)-loggamma(mask_max+1))/log(2))

which we can compute safely using fractervals and fracterval_u128_nats_to_bits().
*/
  status=biguint_init(0, 0);
  bit_idx_max_max=0;
  ignored_status=0;
  loggamma_parameter=(u64)(mask_max)+2;
  loggamma_parameter+=mask_idx_max_max;
  status=(u8)(status|(loggamma_parameter<mask_idx_max_max));
  if(!status){
    LOGGAMMA_U64(loggamma, loggamma_base, loggamma_parameter, status);
    loggamma_parameter-=mask_idx_max_max;
    loggamma_parameter--;
    nat_count=loggamma;
/*
Ignore the status output from LOGGAMMA_U64() because we already tried a greater input value above.
*/
    LOGGAMMA_U64(loggamma, loggamma_base, loggamma_parameter, ignored_status);
/*
Subtract the lesser loggamma from the greater one. Due to numerical error, the lower bound of the result could theoretically be negative. However, we don't care about the lower bound, only the upper bound, because this function is concerned with maximum possible memory footprint. For its part, the upper bound is guaranteed valid. So ignore the returned status.
*/
    FRU128_SUBTRACT_FRU128_SELF(nat_count, loggamma, ignored_status);
    FRU128_NATS_TO_BITS(bit_count, nat_count, status);
/*
bit_count_max_minus_1 is encoded as a 64.64 fixed point fracterval, so the high u64 is at least (B-1). We might need up to (B+2) bits, so add 2 in order to return (B+1) as the maximum bit index required.
*/
    U128_TO_U64_HI(bit_idx_max_max, bit_count.b);
    bit_idx_max_max+=2;
    status=(u8)(status|(bit_idx_max_max<2));
  }
  if(status){
    bit_idx_max_max=U64_MAX;
  }
  return bit_idx_max_max;
}

ULONG
agnentrocodec_code_chunk_idx_max_max_get(loggamma_t *loggamma_base, ULONG mask_idx_max_max, u32 mask_max){
/*
Compute an upper bound for the maximum ULONG index of the agnentropic encoding of a given mask count and mask span.

In:

  agnentrocodec_base is the return value of agnentrocodec_init().

  loggamma_base is the return value of loggamma_init().

  mask_idx_max_max is as defined in agnentrocodec_init().

  mask_max is as defined in agnentrocodec_init().

Out:

  Returns ULONG_MAX on failure, else the index described in the summary above.
*/
  u64 bit_idx_max_max;
  ULONG chunk_idx_max_max;

  bit_idx_max_max=agnentrocodec_code_bit_idx_max_max_get(loggamma_base, mask_idx_max_max, mask_max);
  chunk_idx_max_max=(ULONG)(bit_idx_max_max);
  if(bit_idx_max_max!=U64_MAX){
    chunk_idx_max_max=chunk_idx_max_max>>ULONG_BITS_LOG2;
    #ifdef _32_
      if(chunk_idx_max_max!=(bit_idx_max_max>>ULONG_BITS_LOG2)){
        chunk_idx_max_max=ULONG_MAX;
      }
    #endif
  }
  return chunk_idx_max_max;
}

u8
agnentrocodec_code_export(agnentrocodec_t *agnentrocodec_base, u64 bit_idx_min, ULONG *chunk_idx_max_base, ULONG chunk_idx_max_max, ULONG *chunk_list_base){
/*
Export an agnentropic code from Agnentrocodec local storage to a bitmap.

In:

  agnentrocodec_base is the return value of agnentrocodec_init().

  bit_idx_min is the bit index at chunk_list_base at which to begin export.

  *chunk_idx_max_base is the index containing the MSB of *chunk_list_base.

  chunk_idx_max_max is the upper bound of *chunk_idx_max_base.

  chunk_list_base is the base of the bitmap into which to copy the agnentropic code.

Out:

  Returns zero on success. Else one if the code is too large to export, in which case the caller may have ignored the return value from agnentrocodec_encode().

  *chunk_idx_max_base is updated.
*/
  u64 bit_idx_max;
  ULONG chunk_idx_max;
  u64 code_bit_idx_max;
  ULONG code_chunk_idx_max;
  ULONG *code_chunk_list_base;
  u8 status;

  code_bit_idx_max=agnentrocodec_base->code_bit_idx_max;
  code_chunk_idx_max=agnentrocodec_base->code_chunk_idx_max;
  bit_idx_max=bit_idx_min+code_bit_idx_max;
  status=1;
  if(((bit_idx_max>>ULONG_BITS_LOG2)<=chunk_idx_max_max)&&(bit_idx_min<=bit_idx_max)){
    chunk_idx_max=*chunk_idx_max_base;
    code_chunk_list_base=agnentrocodec_base->code_chunk_list_base;
    chunk_idx_max=biguint_bitmap_copy(code_bit_idx_max, bit_idx_min, 0, chunk_idx_max, code_chunk_idx_max, chunk_list_base, code_chunk_list_base);
    status=0;
    *chunk_idx_max_base=chunk_idx_max;
  }
  return status;
}

u8
agnentrocodec_code_import(agnentrocodec_t *agnentrocodec_base, u64 bit_count_minus_1, u64 bit_idx_min, ULONG chunk_idx_max, ULONG *chunk_list_base){
/*
Import an agnentropic code from a bitmap to Agnentrocodec local storage.

In:

  agnentrocodec_base is the return value of agnentrocodec_init().

  bit_count_minus_1 is the number of bits to import, less one, which may exceed the MSB of *chunk_list_base. In order to guarantee successful decoding, this value must be at least the return value of the agnentrocodec_encode() instance which returned the agnentropic code. It will be automatically saturated to the maximum possible such value, as determined during the corresponding agnentrocodec_init().

  bit_idx_min is the bit index at chunk_list_base at which to begin import.

  chunk_idx_max is the index containing the MSB of *chunk_list_base.

  chunk_list_base is the base of the bitmap out of which to copy the agnentropic code.

Out:

  Returns zero on success. Else one if the code is too large to import, in which case the caller may have ignored the return value from agnentrocodec_code_bit_idx_max_max_get().
*/
  u64 bit_idx_max;
  u64 code_bit_idx_max_max;
  ULONG code_chunk_idx_max;
  ULONG *code_chunk_list_base;
  u8 status;

  code_bit_idx_max_max=agnentrocodec_base->code_bit_idx_max_max;
  bit_count_minus_1=MIN(bit_count_minus_1, code_bit_idx_max_max);
  bit_idx_max=bit_idx_min+bit_count_minus_1;
  status=1;
  if(bit_idx_min<=bit_idx_max){
    code_chunk_list_base=agnentrocodec_base->code_chunk_list_base;
    BIGUINT_SET_ZERO(code_chunk_idx_max, code_chunk_list_base);
    agnentrocodec_base->code_bit_idx_max=bit_count_minus_1;
    code_chunk_idx_max=biguint_bitmap_copy(bit_count_minus_1, 0, bit_idx_min, code_chunk_idx_max, chunk_idx_max, code_chunk_list_base, chunk_list_base);
    agnentrocodec_base->code_chunk_idx_max=code_chunk_idx_max;
    status=0;
  }
  return status;
}

void
agnentrocodec_decode(agnentrocodec_t *agnentrocodec_base, ULONG mask_idx_max, u8 *mask_list_base){
/*
Decode an agenetropic code from local storage to a mask list.

In:

  To write an agnentropic code to local storage, see agnentrocodec_code_import(). Otherwise this function will use the one left there by agnentrocodec_encode().

  agnentrocodec_base is the return value of agnentrocodec_init().

  mask_idx_max is the number of masks to decode to mask_list_base, less one, which must not exceed the same value passed to agnentrocodec_encode() in order to generate the agnentropic code.

  *mask_list_base is undefined and writable for mask_idx_max masks.
*/
  u64 code_bit_count;
  u64 code_bit_idx_max;
  ULONG code_chunk_idx_max;
  ULONG *code_chunk_list_base;
  u64 code_msb;
  u64 code_shift;
  u8 depth_idx;
  ULONG floor;
  ULONG floor_min;
  ULONG freq;
  ULONG freq_idx;
  ULONG *freq_idx_min_list_base;
  ULONG freq_sum;
  ULONG *freq_tree_base;
  u8 granularity;
  u32 mask;
  ULONG mask_count_plus_span;
  u32 mask_max;
  u32 mask_min;
  u64 remainder;
  ULONG u8_idx;
  ULONG u8_idx_max;

  agnentrocodec_reset(agnentrocodec_base);
  code_bit_idx_max=agnentrocodec_base->code_bit_idx_max;
  code_chunk_idx_max=agnentrocodec_base->code_chunk_idx_max;
  code_chunk_list_base=agnentrocodec_base->code_chunk_list_base;
  code_msb=biguint_msb_get(code_chunk_idx_max, code_chunk_list_base);
  code_chunk_idx_max=biguint_reverse(code_chunk_idx_max, code_chunk_list_base);
  if(code_msb<=code_bit_idx_max){
    code_shift=code_bit_idx_max-code_msb;
    code_chunk_idx_max=biguint_shift_left(code_shift, code_chunk_idx_max, code_chunk_list_base);
  }else{
    code_shift=code_msb-code_bit_idx_max;
    code_chunk_idx_max=biguint_shift_right(code_shift, code_chunk_idx_max, code_chunk_list_base);
  }
  code_bit_count=code_bit_idx_max+1;
  freq_idx_min_list_base=&agnentrocodec_base->freq_idx_min_list_base[0];
  freq_tree_base=agnentrocodec_base->freq_tree_base;
  granularity=agnentrocodec_base->granularity;
  freq=1;
  mask_count_plus_span=agnentrocodec_base->mask_max;
  u8_idx=0;
  u8_idx_max=(mask_idx_max*(u8)(granularity+1))+granularity;
  do{
    mask_count_plus_span++;
    code_chunk_idx_max=biguint_multiply_u64(code_chunk_idx_max, code_chunk_list_base, mask_count_plus_span);
    floor=0;
    biguint_bitmap_copy(ULONG_BIT_MAX, 0, code_bit_count, 0, code_chunk_idx_max, &floor, code_chunk_list_base);
    mask_max=agnentrocodec_base->mask_max;
    mask_min=0;
    do{
      depth_idx=0;
      floor_min=0;
      mask=mask_max-((mask_max-mask_min)>>1);
      do{
        freq_idx=freq_idx_min_list_base[depth_idx]+mask;
        if(mask&1){
          floor_min=floor_min+freq_tree_base[freq_idx-1];
        }
        depth_idx++;
        mask>>=1;
      }while(mask);
      mask=mask_max-((mask_max-mask_min)>>1);
      if(floor_min<=floor){
        mask_min=mask;
      }else{
        mask_max=mask-1;
      }
    }while(mask_min!=mask_max);
    mask=mask_max;
    mask_list_base[u8_idx]=(u8)(mask);
    u8_idx++;
    if(granularity){
      mask_list_base[u8_idx]=(u8)(mask>>U8_BITS);
      u8_idx++;
      if(U16_BYTE_MAX<granularity){
        mask_list_base[u8_idx]=(u8)(mask>>U16_BITS);
        u8_idx++;
        if(U24_BYTE_MAX<granularity){
          mask_list_base[u8_idx]=(u8)(mask>>U24_BITS);
          u8_idx++;
        }
      }
    }
    freq=freq_tree_base[mask];
    mask_max=agnentrocodec_base->mask_max;
    depth_idx=0;
    floor=0;
    do{
      freq_idx=freq_idx_min_list_base[depth_idx]+mask;
      freq_sum=freq_tree_base[freq_idx];
      if(mask&1){
        floor=freq_tree_base[freq_idx-1]+floor;
      }
      depth_idx++;
      freq_sum++;
      mask>>=1;
      mask_max>>=1;
      freq_tree_base[freq_idx]=freq_sum;
    }while(mask_max);
    code_chunk_idx_max=biguint_subtract_u64_shifted(code_bit_count, code_chunk_idx_max, code_chunk_list_base, floor);
    code_chunk_idx_max=biguint_divide_u64(code_chunk_idx_max, code_chunk_list_base, &remainder, freq);
    if(remainder){
      code_chunk_idx_max=biguint_increment(code_chunk_idx_max, code_chunk_list_base);
    }
  }while(u8_idx<=u8_idx_max);
  return;
}

u64
agnentrocodec_encode(agnentrocodec_t *agnentrocodec_base, ULONG mask_idx_max, u8 *mask_list_base){
/*
Encode a mask list using agnentropic encoding, store the result in local storage, then return its maximum bit index (which might exceed its MSB). Typically, the point is to verify that computed agnentropy is consistent with actual compressed size. This is not a practical data compression engine, on account of O(N^2) complexity, although it could be used that way.

In:

  agnentrocodec_base is the return value of agnentrocodec_init().

  mask_idx_max is the number of masks at mask_list_base, less one, which must not exceed the mask_idx_max_max that was passed to agnentrocodec_init().

  *mask_list_base contains the mask list to encode.

Out:

  Returns one less than the number of bits in the agnentropic encoding of *mask_list_base, such that even a single bit less would not be adequate for unambiguous decoding. (Codes of zero length are only possible with a mask_max of zero, which is disallowed by agnentrocodec_init().) This value should not be effected by mask order; if so, there's bug.

  To read an agnentropic code from local storage, see agnentrocodec_code_export().
*/
  u8 bit00;
  u8 bit01;
  u8 bit10;
  u8 bit11;
  u64 code_bit_idx;
  u64 code_bit_idx_max;
  ULONG code_chunk_idx_max;
  ULONG *code_chunk_list_base;
  u8 delta;
  ULONG mask_count_plus_span;
  ULONG pochhammer_chunk_idx_max;
  ULONG *pochhammer_chunk_list_base;
  ULONG remainder_chunk_idx_max;
  ULONG *remainder_chunk_list_base;
  ULONG span_chunk_idx_max;
  ULONG *span_chunk_list_base;
  ULONG term_chunk_idx_max;
  ULONG *term_chunk_list_base;

  agnentrocodec_protoagnentropic_get(agnentrocodec_base, mask_idx_max, mask_list_base);
  code_chunk_idx_max=agnentrocodec_base->code_chunk_idx_max;
  code_chunk_list_base=agnentrocodec_base->code_chunk_list_base;
  mask_count_plus_span=agnentrocodec_base->mask_max;
  pochhammer_chunk_list_base=agnentrocodec_base->pochhammer_chunk_list_base;
  remainder_chunk_list_base=agnentrocodec_base->remainder_chunk_list_base;
  span_chunk_idx_max=agnentrocodec_base->span_chunk_idx_max;
  span_chunk_list_base=agnentrocodec_base->span_chunk_list_base;
  term_chunk_list_base=agnentrocodec_base->term_chunk_list_base;
  mask_count_plus_span++;
  BIGUINT_SET_ULONG(pochhammer_chunk_idx_max, pochhammer_chunk_list_base, mask_count_plus_span);
  mask_count_plus_span++;
  pochhammer_chunk_idx_max=biguint_pochhammer_multiply(pochhammer_chunk_idx_max, pochhammer_chunk_list_base, mask_idx_max, mask_count_plus_span);
  term_chunk_idx_max=biguint_copy(pochhammer_chunk_idx_max, term_chunk_list_base, pochhammer_chunk_list_base);
  biguint_divide_biguint(&term_chunk_idx_max, span_chunk_idx_max, term_chunk_list_base, span_chunk_list_base, term_chunk_list_base);
  code_bit_idx_max=biguint_msb_get(term_chunk_idx_max, term_chunk_list_base);
  code_bit_idx_max++;
  code_bit_idx=code_bit_idx_max+1;
  term_chunk_idx_max=biguint_copy(code_chunk_idx_max, term_chunk_list_base, code_chunk_list_base);
  term_chunk_idx_max=biguint_add_biguint(term_chunk_idx_max, span_chunk_idx_max, term_chunk_list_base, span_chunk_list_base);
  term_chunk_idx_max=biguint_shift_left(code_bit_idx, term_chunk_idx_max, term_chunk_list_base);
  biguint_divide_biguint(&term_chunk_idx_max, pochhammer_chunk_idx_max, term_chunk_list_base, pochhammer_chunk_list_base, term_chunk_list_base);
  term_chunk_idx_max=biguint_decrement(term_chunk_idx_max, term_chunk_list_base);
  bit10=biguint_bit_get(0, term_chunk_idx_max, term_chunk_list_base);
  bit11=biguint_bit_get(1, term_chunk_idx_max, term_chunk_list_base);
  code_chunk_idx_max=biguint_shift_left(code_bit_idx, code_chunk_idx_max, code_chunk_list_base);
  remainder_chunk_idx_max=biguint_divide_biguint(&code_chunk_idx_max, pochhammer_chunk_idx_max, code_chunk_list_base, pochhammer_chunk_list_base, remainder_chunk_list_base);
  if(BIGUINT_IS_NOT_ZERO(remainder_chunk_idx_max, remainder_chunk_list_base)){
    code_chunk_idx_max=biguint_increment(code_chunk_idx_max, code_chunk_list_base);
  }
  bit00=biguint_bit_get(0, code_chunk_idx_max, code_chunk_list_base);
  bit01=biguint_bit_get(1, code_chunk_idx_max, code_chunk_list_base);
  delta=(u8)(((bit10|(bit11<<1))-(bit00|(bit01<<1)))&3);
  if(delta==3){
    if((!(bit00|bit01))&(bit10&bit11)){
      code_chunk_idx_max=biguint_shift_right(2, code_chunk_idx_max, code_chunk_list_base);
      code_bit_idx_max-=2;
    }else{
      delta=2;
    }
  }
  if(delta==2){
    if(bit00){
      code_chunk_idx_max=biguint_increment(code_chunk_idx_max, code_chunk_list_base);
    }    
    code_chunk_idx_max=biguint_shift_right(1, code_chunk_idx_max, code_chunk_list_base);
    code_bit_idx_max--;
  }else if(delta==1){
    if(bit01==bit11){
      code_chunk_idx_max=biguint_shift_right(1, code_chunk_idx_max, code_chunk_list_base);
      code_bit_idx_max--;
    }
  }
  code_bit_idx=biguint_msb_get(code_chunk_idx_max, code_chunk_list_base);
  code_chunk_idx_max=biguint_reverse(code_chunk_idx_max, code_chunk_list_base);
  code_bit_idx=code_bit_idx_max-code_bit_idx;
  code_chunk_idx_max=biguint_shift_left(code_bit_idx, code_chunk_idx_max, code_chunk_list_base);
  agnentrocodec_base->code_bit_idx_max=code_bit_idx_max;
  agnentrocodec_base->code_chunk_idx_max=code_chunk_idx_max;
  return code_bit_idx_max;
}

void *
agnentrocodec_free(void *base){
/*
To maximize portability and debuggability, this is the only function in which Agnentrocodec calls free().

In:

  base is the base of a memory region to free. May be NULL.

Out:

  Returns NULL so that the caller can easily maintain the good practice of NULLing out invalid pointers.

  *base is freed.
*/
  DEBUG_FREE_PARANOID(base);
  return NULL;
}

agnentrocodec_t *
agnentrocodec_free_all(agnentrocodec_t *agnentrocodec_base){
/*
Free all private storage.

In:

  agnentrocodec_base is the return value of agnentrocodec_init().

Out:

  Returns NULL so that the caller can easily maintain the good practice of NULLing out invalid pointers.

  *agnentrocodec_base and all its child allocations are freed.
*/
  if(agnentrocodec_base){
    biguint_free(agnentrocodec_base->code_chunk_list_base);
    biguint_free(agnentrocodec_base->term_chunk_list_base);
    biguint_free(agnentrocodec_base->span_chunk_list_base);
    biguint_free(agnentrocodec_base->remainder_chunk_list_base);
    biguint_free(agnentrocodec_base->pochhammer_chunk_list_base);
    biguint_free(agnentrocodec_base->freq_tree_base);
    agnentrocodec_base=agnentrocodec_free(agnentrocodec_base);
  }
  return agnentrocodec_base;
}

void
agnentrocodec_freq_tree_sync(agnentrocodec_t *agnentrocodec_base){
/*
Sync all levels of the frequency counter tree to its leaves.

In:

  agnentrocodec_base is the return value of agnentrocodec_init().
*/
  u8 depth_idx;
  ULONG freq_idx_child;
  ULONG freq_idx_child_max;
  ULONG *freq_idx_min_list_base;
  ULONG freq_idx_parent;
  ULONG *freq_tree_base;
  u32 mask;

  freq_idx_min_list_base=&agnentrocodec_base->freq_idx_min_list_base[0];
  freq_tree_base=agnentrocodec_base->freq_tree_base;
  mask=agnentrocodec_base->mask_max;
  depth_idx=0;
/*
The root level of the tree contains 2 parent nodes, so stop when (mask<2). (mask_max is guaranteed nonzero by agnentrocodec_init(). If it's one, then the entire tree consists of just 2 leaves, so it's already in sync.)
*/
  while(1<mask){
    freq_idx_child=freq_idx_min_list_base[depth_idx];
    freq_idx_parent=freq_idx_min_list_base[depth_idx+1];
    freq_idx_child_max=freq_idx_child+mask;
    do{
      freq_tree_base[freq_idx_parent]=freq_tree_base[freq_idx_child]+freq_tree_base[freq_idx_child+1];
      freq_idx_child+=2;
      freq_idx_parent++;
    }while(freq_idx_child<freq_idx_child_max);
    if(freq_idx_child==freq_idx_child_max){
      freq_tree_base[freq_idx_parent]=freq_tree_base[freq_idx_child_max];
    }
    depth_idx++;
    mask>>=1;
  }
  return;
}

agnentrocodec_t *
agnentrocodec_init(u32 build_break_count, u32 build_feature_count, u8 granularity, loggamma_t *loggamma_base, ULONG mask_idx_max_max, u32 mask_max){
/*
Verify that the source code is sufficiently updated and initialize private storage.

In:

  build_break_count is the caller's most recent knowledge of AGNENTROCODEC_BUILD_BREAK_COUNT, which will fail if the caller is unaware of all critical updates.

  build_feature_count is the caller's most recent knowledge of AGNENTROCODEC_BUILD_FEATURE_COUNT, which will fail if this library is not up to date with the caller's expectations.

  granularity is the size of each mask, less one. Limits may change from one version of this code to then next, so there is no explicit restriction.

  loggamma_base is the return value of loggamma_init().

  mask_idx_max_max is the maximum possible number of masks that we could possibly accrue to a frequency list, less one. The limits on it are tricky to compute so this function tests them explicitly.

  mask_max is the maximum possible mask, which must not exeed (((granularity+1)<<U8_BITS_LOG2)-1). For example, when dealing with bytes, this would be U8_MAX. Due to implied allocation requirements, only compiling with (-D_64_) will allow the full u32 range. The limits on it are tricky to compute so this function tests them explicitly. Zero is tested for and disallowed because it creates more trouble than it's worth.

Out:

  Returns NULL if (build_break_count!=AGNENTROCODEC_BUILD_BREAK_COUNT); (build_feature_count>AGNENTROCODEC_BUILD_FEATURE_COUNT); there is insufficient memory; or one of the input parameters falls outside its valid range. Else, returns the base of an agnentrocodec_t to be used with other Agnentrocodec functions. It must be freed with agnentrocodec_free_all().
*/
  agnentrocodec_t *agnentrocodec_base;
  u64 code_bit_idx_max_max;
  ULONG code_chunk_idx_max_max;
  ULONG *code_chunk_list_base;
  u8 depth_idx;
  ULONG freq_idx_min;
  ULONG *freq_idx_min_list_base;
  ULONG *freq_tree_base;
  u32 mask;
  ULONG mask_count_max_max;
  ULONG mask_count_plus_span_max_max;
  u64 mask_span;
  ULONG pochhammer_chunk_idx_max_max;
  ULONG *pochhammer_chunk_list_base;
  ULONG *remainder_chunk_list_base;
  ULONG *span_chunk_list_base;
  ULONG *term_chunk_list_base;
  u8 status;

  agnentrocodec_base=NULL;
  status=biguint_init(0, 0);
  status=(u8)(status|fracterval_u128_init(2, 0));
  status=(u8)(status|(AGNENTROCODEC_BUILD_FEATURE_COUNT<build_feature_count));
  status=(u8)(status|(build_break_count!=AGNENTROCODEC_BUILD_BREAK_COUNT));
  status=(u8)(status|(!mask_max));
  mask_count_max_max=mask_idx_max_max+1;
  status=(u8)(status|(!mask_count_max_max));
  mask_span=(u64)(mask_max)+1;
  mask_count_plus_span_max_max=(ULONG)(mask_count_max_max+mask_span);
  status=(u8)(status|(mask_count_plus_span_max_max<=mask_span));
  status=(u8)(status|(U32_BYTE_MAX<granularity));
  granularity=(u8)(granularity&U32_BYTE_MAX);
  status=(u8)(status|((u32)((1U<<(granularity<<U8_BITS_LOG2)<<U8_BITS)-1)<mask_max));
  if(!status){
/*
We need to use calloc() instead of malloc() because if there's an allocation failure, we'll free all the list bases below. If one of those bases happens to be uninitialized, then it had better be zero (NULL) so as not to cause an instruction fault. (Technically, NULL can be nonzero, but try finding one platform where that was ever the case.)
*/
    agnentrocodec_base=DEBUG_CALLOC_PARANOID((ULONG)(sizeof(agnentrocodec_t)));
    if(agnentrocodec_base){
      agnentrocodec_base->granularity=granularity;
      agnentrocodec_base->mask_max=mask_max;
/*
Find out how many levels we need in a binary tree to store the sums of power-of-2 clusters of frequencies. This will be used to calculate floor values during agnentropic encoding. Most applications will just use level zero, which is simply the frequency list, but we need to allocate enough space for the tree in case we need it. This loop calculates its worst case size.

The frequency binary tree contains, at each level, the sum of its children, such that level zero is set to all ones by agnentrocodec_reset(). It facilitates fast mask floor lookup.
*/
      depth_idx=0;
      freq_idx_min=0;
      freq_idx_min_list_base=&agnentrocodec_base->freq_idx_min_list_base[0];
      mask=mask_max;
      status=0;
      do{
        freq_idx_min_list_base[depth_idx]=freq_idx_min;
        freq_idx_min=freq_idx_min+mask+1;
        depth_idx++;
        status=(u8)(status|(freq_idx_min<=mask));
        mask>>=1;
/*
This won't overflow memory because depth_idx is on [1, U32_BITS] on account of the number of bits in mask, which has been accounted for in the declaration of agnentrocodec_base->freq_idx_min_list_base.
*/
      }while(mask);
      if(!status){
        freq_tree_base=biguint_malloc(freq_idx_min);
        agnentrocodec_base->freq_tree_base=freq_tree_base;
        status=!freq_tree_base;
        if(!status){
          code_bit_idx_max_max=agnentrocodec_code_bit_idx_max_max_get(loggamma_base, mask_idx_max_max, mask_max);
          status=(!(~code_bit_idx_max_max));
          if(!status){
/*
Each biguint needs to be able to fit the entire worst case Pochhammer product. Allocate accordingly.
*/
            pochhammer_chunk_idx_max_max=(ULONG)(code_bit_idx_max_max>>ULONG_BITS_LOG2);
            agnentrocodec_base->code_bit_idx_max_max=code_bit_idx_max_max;
            pochhammer_chunk_list_base=biguint_malloc(pochhammer_chunk_idx_max_max);
            status=(u8)(status|!pochhammer_chunk_list_base);
            agnentrocodec_base->pochhammer_chunk_list_base=pochhammer_chunk_list_base;
            remainder_chunk_list_base=biguint_malloc(pochhammer_chunk_idx_max_max);
            status=(u8)(status|!remainder_chunk_list_base);
            agnentrocodec_base->remainder_chunk_list_base=remainder_chunk_list_base;
            span_chunk_list_base=biguint_malloc(pochhammer_chunk_idx_max_max);
            status=(u8)(status|!span_chunk_list_base);
            agnentrocodec_base->span_chunk_list_base=span_chunk_list_base;
/*
The code biguint itself needs twice as much space as other allocations because it needs to store twice the number of bits in the Pochhammer, plus an extra 2 bits, for the biguint_shift_left() by code_shift in agnentrocodec_encode(). Therefore we allocate an extra ULONG, to account for the 2 bits, as well as the multiplication of the code by mask_count_plus_span in agnentro_decode(), itself a ULONG. Ditto for "term", which is just scratch space for itermediate terms. By the way, (pochhammer_chunk_idx_max_max+1) is guaranteed not to wrap because it was already used with biguint_malloc() above, subject to accrued status.
*/
            code_chunk_idx_max_max=(pochhammer_chunk_idx_max_max+1)<<1;
            status=(u8)(status|(code_chunk_idx_max_max<pochhammer_chunk_idx_max_max));
            if(!status){
              code_chunk_list_base=biguint_malloc(code_chunk_idx_max_max);
              status=!code_chunk_list_base;
              agnentrocodec_base->code_chunk_list_base=code_chunk_list_base;
              term_chunk_list_base=biguint_malloc(code_chunk_idx_max_max);
              status=(u8)(status|!term_chunk_list_base);
              agnentrocodec_base->term_chunk_list_base=term_chunk_list_base;
            }
          }
        }
      }
      if(!status){
        agnentrocodec_reset(agnentrocodec_base);
      }else{
        agnentrocodec_base=agnentrocodec_free_all(agnentrocodec_base);
      }
    }
  }
  return agnentrocodec_base;
}

u8 *
agnentrocodec_mask_list_malloc(u8 granularity, ULONG mask_idx_max){
/*
Allocate a list of masks which are addressable as (u8)s but span up to U32_SIZE bytes each.

To maximize portability and debuggability, this is one of the few functions in which Agnentrocodec calls malloc().

In:

  granularity is the size of each mask, less one, on [U8_BYTE_MAX, U32_BYTE_MAX].

  mask_idx_max is the number of masks to allocate, less one.

Out:

  Returns NULL on failure, else the base of ((mask_idx_max+1)*(granularity+1)) undefined (u8)s, which should eventually be freed via agnentrocodec_free().
*/
  u8 *list_base;
  ULONG list_size;
  ULONG mask_count;
  u8 mask_size;

  mask_count=mask_idx_max+1;
  list_base=NULL;
  if(mask_count){
    mask_size=(u8)(granularity+1);
    list_size=mask_count*mask_size;
    if(mask_count==(list_size/mask_size)){
      list_base=DEBUG_MALLOC_PARANOID(list_size);
    }
  }
  return list_base;
}

u32
agnentrocodec_mask_max_get(u8 granularity, ULONG mask_idx_max, u8 *mask_list_base){
/*
Return the greatest mask in a mask list, for the purpose of automating discovery of mask_max when it's not a given.

In:

  granularity is agnentrocodec_init():In:granularity.

  mask_idx_max is one less than the number of masks at mask_list_base.

  *mask_list_base is a list of masks, each of which having size (granularity+1).

Out:

  Returns the greatest mask found at mask_list_base.
*/
  ULONG mask_idx;
  u32 mask;
  u32 mask_max;
  u32 mask_max_max;
  u8 mask_u8;
  ULONG u8_idx;

  mask_idx=mask_idx_max;
  mask_max=0;
  mask_max_max=(1U<<(granularity<<U8_BITS_LOG2)<<U8_BITS)-1;
  u8_idx=0;
  do{
    mask=mask_list_base[u8_idx];
    u8_idx++;
    if(granularity){
      mask_u8=mask_list_base[u8_idx];
      mask|=(u32)(mask_u8)<<U8_BITS;
      u8_idx++;
      if(U16_BYTE_MAX<granularity){
        mask_u8=mask_list_base[u8_idx];
        mask|=(u32)(mask_u8)<<U16_BITS;
        u8_idx++;
        if(U24_BYTE_MAX<granularity){
          mask_u8=mask_list_base[u8_idx];
          mask|=(u32)(mask_u8)<<U24_BITS;
          u8_idx++;
        }
      }
    }
    mask_max=MAX(mask, mask_max);
    if(mask_max==mask_max_max){
      break;
    }
  }while(mask_idx--);
  return mask_max;
}

u64
agnentrocodec_protoagnentropic_get(agnentrocodec_t *agnentrocodec_base, ULONG mask_idx_max, u8 *mask_list_base){
/*
Encode a mask list using protoagnentropic encoding, store the result in local storage, then return its MSB. For agnentropic encoding, call agnentrocodec_encode() instead.

In:

  agnentrocodec_base is the return value of agnentrocodec_init().

  mask_idx_max is the number of masks at mask_list_base, less one, which must not exceed the mask_idx_max_max that was passed to agnentrocodec_init().

  *mask_list_base contains the mask list to encode.

Out:

  To read a protoagnentropic code from local storage, see agnentrocodec_code_export().
*/
  u64 code_bit_idx_max;
  ULONG code_chunk_idx_max;
  ULONG *code_chunk_list_base;
  u8 depth_idx;
  ULONG floor;
  ULONG freq;
  ULONG freq_idx;
  ULONG *freq_idx_min_list_base;
  ULONG freq_sum;
  ULONG *freq_tree_base;
  u8 granularity;
  u32 mask;
  u32 mask_max;
  ULONG mask_count_plus_span;
  u8 mask_u8;
  u64 remainder;
  ULONG span_chunk_idx_max;
  ULONG *span_chunk_list_base;
  ULONG term_chunk_idx_max;
  ULONG *term_chunk_list_base;
  ULONG u8_idx;
  ULONG u8_idx_delta;
  ULONG u8_idx_max;

  agnentrocodec_reset(agnentrocodec_base);
  span_chunk_list_base=agnentrocodec_base->span_chunk_list_base;
  mask_max=agnentrocodec_base->mask_max;
  mask_count_plus_span=mask_max;
  mask_count_plus_span+=2;
  BIGUINT_SET_ULONG(span_chunk_idx_max, span_chunk_list_base, 1U);
  span_chunk_idx_max=biguint_pochhammer_multiply(span_chunk_idx_max, span_chunk_list_base, mask_idx_max, mask_count_plus_span);
  code_chunk_list_base=agnentrocodec_base->code_chunk_list_base;
  freq_idx_min_list_base=&agnentrocodec_base->freq_idx_min_list_base[0];
  freq_tree_base=agnentrocodec_base->freq_tree_base;
  granularity=agnentrocodec_base->granularity;
  span_chunk_list_base=agnentrocodec_base->span_chunk_list_base;
  term_chunk_list_base=agnentrocodec_base->term_chunk_list_base;
  BIGUINT_SET_ZERO(code_chunk_idx_max, code_chunk_list_base);
  u8_idx=0;
  u8_idx_delta=(u8)(granularity+1);
  u8_idx_max=(mask_idx_max*u8_idx_delta)+granularity;
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
    freq=freq_tree_base[mask];
    mask_max=agnentrocodec_base->mask_max;
    depth_idx=0;
    floor=0;
    u8_idx+=u8_idx_delta;
    do{
      freq_idx=freq_idx_min_list_base[depth_idx]+mask;
      freq_sum=freq_tree_base[freq_idx];
      if(mask&1){
        floor=freq_tree_base[freq_idx-1]+floor;
      }
      depth_idx++;
      freq_sum++;
      mask>>=1;
      mask_max>>=1;
      freq_tree_base[freq_idx]=freq_sum;
    }while(mask_max);
    BIGUINT_SET_ULONG(term_chunk_idx_max, term_chunk_list_base, floor);
    term_chunk_idx_max=biguint_multiply_biguint(term_chunk_idx_max, span_chunk_idx_max, term_chunk_list_base, span_chunk_list_base);
    if(u8_idx<=u8_idx_max){
      span_chunk_idx_max=biguint_divide_u64(span_chunk_idx_max, span_chunk_list_base, &remainder, mask_count_plus_span);
      mask_count_plus_span++;
    }
    span_chunk_idx_max=biguint_multiply_u64(span_chunk_idx_max, span_chunk_list_base, freq);
    code_chunk_idx_max=biguint_add_biguint(code_chunk_idx_max, term_chunk_idx_max, code_chunk_list_base, term_chunk_list_base);
  }while(u8_idx<=u8_idx_max);
  code_bit_idx_max=biguint_msb_get(code_chunk_idx_max, code_chunk_list_base);
  agnentrocodec_base->code_chunk_idx_max=code_chunk_idx_max;
  agnentrocodec_base->span_chunk_idx_max=span_chunk_idx_max;
  return code_bit_idx_max;
}

void
agnentrocodec_reset(agnentrocodec_t *agnentrocodec_base){
/*
Reset the Agnentrocodec local data structure in preparation for a new agnentropic encoding or decoding session.

In:

  agnentrocodec_base is the return value of agnentrocodec_init().
*/
  ULONG *freq_tree_base;
  u32 mask;
/*
Set all frequencies to one, which constitutes a minimal expectation about possible future masks.
*/
  freq_tree_base=agnentrocodec_base->freq_tree_base;
  mask=agnentrocodec_base->mask_max;
  do{
    freq_tree_base[mask]=1;
  }while(mask--);
  agnentrocodec_freq_tree_sync(agnentrocodec_base);
  return;
}
