/*
Biguint
Copyright 2017 Russell Leidich

This collection of files constitutes the Biguint Library. (This is a
library in the abstact sense; it's not intended to compile to a ".lib"
file.)

The Biguint Library is free software: you can redistribute it and/or
modify it under the terms of the GNU Limited General Public License as
published by the Free Software Foundation, version 3.

The Biguint Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
Limited General Public License version 3 for more details.

You should have received a copy of the GNU Limited General Public
License version 3 along with the Biguint Library (filename
"COPYING"). If not, see http://www.gnu.org/licenses/ .
*/
/*
Big Unsigned Integer (Biguint) Arithmetic Kernel
*/
#include "flag.h"
#include "flag_biguint.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "constant.h"
#include "debug.h"
#include "debug_xtrn.h"
#include "biguint.h"
#include "biguint_xtrn.h"
#include "bitscan.h"
#include "bitscan_xtrn.h"

ULONG
biguint_add_biguint(ULONG chunk_idx_max0, ULONG chunk_idx_max1, ULONG *chunk_list_base0, ULONG *chunk_list_base1){
/*
Add one biguint to another.

In:

  chunk_idx_max0 is the index containing the MSB of *chunk_list_base0.

  chunk_idx_max1 is the index containing the MSB of *chunk_list_base1.

  *chunk_list_base0 is a list of chunks of a biguint, with sufficient space to store ((*chunk_list_base0)+(*chunk_list_base1)).

  *chunk_list_base1 is a list of chunks of a biguint.

Out:

  Returns the updated value of chunk_idx_max0.

  *chunk_list_base0 is ((In:*chunk_list_base0)+(*chunk_list_base1)).
*/
  u8 carry;
  ULONG chunk;
  ULONG chunk_idx;
  ULONG chunk_idx_max;
  ULONG chunk_old;

  carry=0;
  chunk_idx_max=MIN(chunk_idx_max0, chunk_idx_max1);
  for(chunk_idx=0; chunk_idx<=chunk_idx_max; chunk_idx++){
    chunk=chunk_list_base1[chunk_idx];
    if(carry){
      chunk++;
      carry=!chunk;
    }
    chunk_old=chunk;
    chunk=chunk_list_base0[chunk_idx]+chunk;
    carry=(u8)(carry|(chunk<chunk_old));
    chunk_list_base0[chunk_idx]=chunk;
  }
  for(chunk_idx=chunk_idx_max+1; chunk_idx<=chunk_idx_max1; chunk_idx++){
    chunk=chunk_list_base1[chunk_idx];
    if(carry){
      chunk++;
      carry=!chunk;
    }
    chunk_list_base0[chunk_idx]=chunk;
  }
  chunk_idx=chunk_idx_max1+1;
  while(carry){
    chunk=0;
    if(chunk_idx<=chunk_idx_max0){
      chunk=chunk_list_base0[chunk_idx];
    }
    chunk++;
    carry=!chunk;
    chunk_list_base0[chunk_idx]=chunk;
    chunk_idx++;
  }
  chunk_idx--;
  chunk_idx_max0=MAX(chunk_idx, chunk_idx_max0);
  return chunk_idx_max0;
}

ULONG
biguint_add_u64(ULONG chunk_idx_max, ULONG *chunk_list_base, u64 uint){
/*
Add a u64 to a biguint.

In:

  chunk_idx_max is the index containing the MSB of *chunk_list_base.

  *chunk_list_base is list of chunks of a biguint, with sufficient space to store ((In:*chunk_list_base)+uint).

  uint is the value to add to *chunk_list_base.

Out:

  Returns the updated value of chunk_idx_max.

  *chunk_list_base has been increased by uint.
*/
  ULONG chunk;
  ULONG chunk_idx;
  ULONG sum;

  chunk_idx=0;
  do{
    chunk=0;
    if(chunk_idx<=chunk_idx_max){
      chunk=chunk_list_base[chunk_idx];
    }
    sum=(ULONG)(chunk+uint);
    #ifdef _64_
      uint=(sum<chunk);
    #else
      uint>>=ULONG_BITS;
      uint+=(sum<chunk);
    #endif
    chunk_list_base[chunk_idx]=sum;
    chunk_idx++;
  }while(uint);
  chunk_idx--;
  chunk_idx_max=MAX(chunk_idx, chunk_idx_max);
  return chunk_idx_max;
}

ULONG
biguint_bit_clear(u64 bit_idx, ULONG chunk_idx_max, ULONG *chunk_list_base){
/*
Clear a bit in a biguint.

In:

  bit_idx is the index of the bit to clear, which may safely be any value.

  chunk_idx_max is the index containing the MSB of *chunk_list_base.

  *chunk_list_base is a list of chunks of a biguint.

Out:

  Returns the updated value of chunk_idx_max.

  *chunk_list_base has cleared the bit at bit index bit_idx (notionally, if not actually).
*/
  ULONG chunk;
  ULONG chunk_idx;
  ULONG mask;

  chunk_idx=(ULONG)(bit_idx>>ULONG_BITS_LOG2);
  if(chunk_idx<=chunk_idx_max){
    chunk=chunk_list_base[chunk_idx];
    mask=1;
    mask=~(mask<<(bit_idx&ULONG_BIT_MAX));
    chunk&=mask;
    chunk_list_base[chunk_idx]=chunk;
    if(!chunk){
      BIGUINT_CANONIZE(chunk_idx_max, chunk_list_base);
    }
  }
  return chunk_idx_max;
}

ULONG
biguint_bit_flip(u64 bit_idx, ULONG chunk_idx_max, ULONG *chunk_list_base){
/*
Flip a bit in a biguint.

In:

  bit_idx is the index of the bit to flip, which must be small enough to ensure sufficient space at chunk_list_base, even if such space is currently undefined and presumed zero.

  chunk_idx_max is the index containing the MSB of *chunk_list_base.

  *chunk_list_base is a list of chunks of a biguint.

Out:

  Returns the updated value of chunk_idx_max.

  *chunk_list_base has flipped the bit at bit index bit_idx.
*/
  ULONG chunk;
  ULONG chunk_idx;
  ULONG mask;

  chunk_idx=(ULONG)(bit_idx>>ULONG_BITS_LOG2);
  mask=1;
  mask<<=(bit_idx&ULONG_BIT_MAX);
  if(chunk_idx<=chunk_idx_max){
    chunk=chunk_list_base[chunk_idx];
    chunk^=mask;
    chunk_list_base[chunk_idx]=chunk;
  }else{
    chunk_list_base[chunk_idx]=mask;
    chunk_idx^=chunk_idx_max;
    chunk_idx_max^=chunk_idx;
    chunk_idx^=chunk_idx_max;
    chunk_idx++;
    while(chunk_idx!=chunk_idx_max){
      chunk_list_base[chunk_idx]=0;
      chunk_idx++;
    }
  }
  return chunk_idx_max;
}

u8
biguint_bit_get(u64 bit_idx, ULONG chunk_idx_max, ULONG *chunk_list_base){
/*
Get a bit from a biguint.

In:

  bit_idx is the index of the bit to return, which may safely be any value.

  chunk_idx_max is the index containing the MSB of *chunk_list_base.

  *chunk_list_base is a list of chunks of a biguint.

Out:

  Returns the requested bit.
*/
  u8 bit;
  ULONG chunk;
  u64 chunk_idx_u64;
  ULONG mask;

  bit=0;
  chunk_idx_u64=bit_idx>>ULONG_BITS_LOG2;
  if(chunk_idx_u64<=chunk_idx_max){
    chunk=chunk_list_base[chunk_idx_u64];
    mask=1;
    mask<<=(bit_idx&ULONG_BIT_MAX);
    bit=!!(mask&chunk);
  }
  return bit;
}

ULONG
biguint_bit_set(u64 bit_idx, ULONG chunk_idx_max, ULONG *chunk_list_base){
/*
Set a bit in a biguint.

In:

  bit_idx is the index of the bit to set, which must be small enough to ensure sufficient space at chunk_list_base, even if such space is currently undefined and presumed zero.

  chunk_idx_max is the index containing the MSB of *chunk_list_base.

  *chunk_list_base is a list of chunks of a biguint.

Out:

  Returns the updated value of chunk_idx_max.

  *chunk_list_base has set the bit at bit index bit_idx.
*/
  ULONG chunk;
  ULONG chunk_idx;
  ULONG mask;

  chunk_idx=(ULONG)(bit_idx>>ULONG_BITS_LOG2);
  mask=1;
  mask<<=(bit_idx&ULONG_BIT_MAX);
  if(chunk_idx<=chunk_idx_max){
    chunk=chunk_list_base[chunk_idx];
    chunk|=mask;
    chunk_list_base[chunk_idx]=chunk;
  }else{
    chunk_list_base[chunk_idx]=mask;
    chunk_idx^=chunk_idx_max;
    chunk_idx_max^=chunk_idx;
    chunk_idx^=chunk_idx_max;
    chunk_idx++;
    while(chunk_idx!=chunk_idx_max){
      chunk_list_base[chunk_idx]=0;
      chunk_idx++;
    }
  }
  return chunk_idx_max;
}

ULONG
biguint_bitmap_clear(u64 bit_count_minus_1, u64 bit_idx_min, ULONG chunk_idx_max, ULONG *chunk_list_base){
/*
Set a region of a bitmap to zeroes.

In:

  bit_count_minus_1 is the number of bits to clear, less one.

  bit_idx_min is the index of the first bit to clear.

  chunk_idx_max is the index containing the MSB of *chunk_list_base.

  *chunk_list_base is the bitmap containing the region to clear.

Out:

  Returns the updated value of chunk_idx_max.

  *chunk_list_base contains zero bits at bit indexes on [bit_idx_min, bit_count_minus_1+bit_idx_min], except that such region is undefined beyond the returned ULONG index.
*/
  u64 bit_count_minus_1_max;
  u64 bit_idx_max;
  ULONG chunk_idx_first;
  ULONG chunk_idx_last;
  ULONG chunk_mask_first;
  ULONG chunk_mask_last;
  ULONG list_size;

  bit_idx_max=((u64)(chunk_idx_max)<<ULONG_BITS_LOG2)+ULONG_BIT_MAX;
  if(bit_idx_min<=bit_idx_max){
    bit_count_minus_1_max=bit_idx_max-bit_idx_min;
    if(bit_count_minus_1_max<=bit_count_minus_1){
      chunk_idx_max=(ULONG)(bit_idx_min>>ULONG_BITS_LOG2);
      bit_count_minus_1_max=ULONG_BIT_MAX-(bit_idx_min&ULONG_BIT_MAX);
    }
    bit_count_minus_1=MIN(bit_count_minus_1, bit_count_minus_1_max);
    bit_idx_max=bit_count_minus_1+bit_idx_min;
    chunk_mask_first=((ULONG)(1)<<(bit_idx_min&ULONG_BIT_MAX))-1;
    chunk_mask_last=~(((ULONG)(1)<<1<<(bit_idx_max&ULONG_BIT_MAX))-1);
    chunk_idx_first=(ULONG)(bit_idx_min>>ULONG_BITS_LOG2);
    chunk_idx_last=(ULONG)(bit_idx_max>>ULONG_BITS_LOG2);
    if(chunk_idx_first!=chunk_idx_last){
      chunk_list_base[chunk_idx_first]=chunk_list_base[chunk_idx_first]&chunk_mask_first;
      list_size=(chunk_idx_last-chunk_idx_first-1)<<ULONG_SIZE_LOG2;
      memset(&chunk_list_base[chunk_idx_first+1], 0, (size_t)(list_size));
      chunk_list_base[chunk_idx_last]=chunk_list_base[chunk_idx_last]&chunk_mask_last;
    }else{
      chunk_list_base[chunk_idx_first]=chunk_list_base[chunk_idx_first]&(chunk_mask_first|chunk_mask_last);
    }
    BIGUINT_CANONIZE(chunk_idx_max, chunk_list_base);
  }
  return chunk_idx_max;
}

ULONG
biguint_bitmap_copy(u64 bit_count_minus_1, u64 bit_idx_min0, u64 bit_idx_min1, ULONG chunk_idx_max0, ULONG chunk_idx_max1, ULONG *chunk_list_base0, ULONG *chunk_list_base1){
/*
Copy a region of a bitmap to a region of another bitmap, even if either region extends past end into implicit zeroes.

Unlike some other Biguint functions, this one doesn't assume 64-bit granularity; ULONG granularity is assumed, with no overshoot allowed.

In:

  bit_count_minus_1 is the number of bits to copy, less one.

  bit_idx_min0 is the index of the first bit to write at chunk_list_base0, which may exceed the maximum defined bit as implied by chunk_idx_max0, but may not exceed the last readable bit.

  bit_idx_min1 is the index of the first bit to read at chunk_list_base1, which may exceed the maximum defined bit as implied by chunk_idx_max1, but may not exceed the last writable bit.

  chunk_idx_max0 is the index containing the MSB of *chunk_list_base0.

  chunk_idx_max1 is the index containing the MSB of *chunk_list_base1.

  *chunk_list_base0 is a bitmap consisting of at least (bit_count_minus_1+bit_idx_min0+1) undefined bits.

  *chunk_list_base1 is a bitmap consisting of at least (bit_count_minus_1+bit_idx_min1+1) bits to copy, even if some of them have been zero-optimized away at the high end, as distinct from biguint_bitmap_copy.

Out:

  Returns the index containing the MSB of *chunk_list_base0. Unlike biguint_bitmap_copy_unsafe(), this function performs trailing zero truncation, so this value may be less than, equal to, or greater than chunk_idx_max0.

  *chunk_list_base0 at bit indexes on [bit_idx_min0, bit_count_minus_1+bit_idx_min0] equals *chunk_list_base1 at bit indexes on [bit_idx_min1, bit_count_minus_1+bit_idx_min1], except that the former region is undefined beyond the returned ULONG index.
*/
  u64 bit_count_minus_1_partial;
  u64 bit_idx_copy_max0;
  u64 bit_idx_gap0;
  u64 bit_idx_max0;
  u64 bit_idx_max1;
  ULONG chunk_idx_copy_max0;

  bit_idx_max0=((u64)(chunk_idx_max0)<<ULONG_BITS_LOG2)+ULONG_BIT_MAX;
  bit_idx_max1=((u64)(chunk_idx_max1)<<ULONG_BITS_LOG2)+ULONG_BIT_MAX;
  if(bit_idx_min1<=bit_idx_max1){
    bit_count_minus_1_partial=bit_idx_max1-bit_idx_min1;
    bit_count_minus_1_partial=MIN(bit_count_minus_1, bit_count_minus_1_partial);
    bit_idx_copy_max0=bit_count_minus_1_partial+bit_idx_min0;
    chunk_idx_copy_max0=(ULONG)(bit_idx_copy_max0>>ULONG_BITS_LOG2);
    if(chunk_idx_max0<chunk_idx_copy_max0){
      chunk_idx_max0=chunk_idx_copy_max0;
      chunk_list_base0[chunk_idx_copy_max0]=0;
    }
    biguint_bitmap_copy_unsafe(bit_count_minus_1_partial, bit_idx_min0, bit_idx_min1, chunk_list_base0, chunk_list_base1);
    BIGUINT_CANONIZE(chunk_idx_max0, chunk_list_base0);
    if(bit_count_minus_1!=bit_count_minus_1_partial){
      bit_idx_gap0=bit_idx_copy_max0+1;
      bit_count_minus_1_partial=bit_count_minus_1-bit_count_minus_1_partial-1;
      chunk_idx_max0=biguint_bitmap_clear(bit_count_minus_1_partial, bit_idx_gap0, chunk_idx_max0, chunk_list_base0);
    }
    if(bit_idx_max0<bit_idx_min0){
      bit_idx_gap0=bit_idx_max0+1;
      if(bit_idx_gap0!=bit_idx_min0){
        bit_count_minus_1_partial=bit_idx_min0-bit_idx_gap0-1;
        chunk_idx_max0=biguint_bitmap_clear(bit_count_minus_1_partial, bit_idx_gap0, chunk_idx_max0, chunk_list_base0);
      }
    }
  }else if(bit_idx_min0<=bit_idx_max0){
    bit_count_minus_1_partial=bit_idx_max0-bit_idx_min0;
    bit_count_minus_1_partial=MIN(bit_count_minus_1, bit_count_minus_1_partial);
    chunk_idx_max0=biguint_bitmap_clear(bit_count_minus_1_partial, bit_idx_min0, chunk_idx_max0, chunk_list_base0);    
  }
  return chunk_idx_max0;
}

ULONG
biguint_bitmap_copy_reverse(u64 bit_count_minus_1, u64 bit_idx_min0, u64 bit_idx_min1, ULONG chunk_idx_max0, ULONG chunk_idx_max1, ULONG *chunk_list_base0, ULONG *chunk_list_base1){
/*
Copy a region of a bitmap to a region of another bitmap in bitwise reverse order, even if either region extends past end into implicit zeroes.

To reverse an entire biguint or its ULONG-granular hull, use biguint_reverse() or biguint_reverse_unsafe(), respectively.

Unlike some other Biguint functions, this one doesn't assume 64-bit granularity; ULONG granularity is assumed, with no overshoot allowed.

In:

  bit_count_minus_1 is the number of bits to copy, less one.

  bit_idx_min0 is the index of the first bit to write at chunk_list_base0, which may exceed the maximum defined bit as implied by chunk_idx_max0, but may not exceed the last readable bit.

  bit_idx_min1 is the index of the first bit to read at chunk_list_base1, which may exceed the maximum defined bit as implied by chunk_idx_max1, but may not exceed the last writable bit.

  chunk_idx_max0 is the index containing the MSB of *chunk_list_base0.

  chunk_idx_max1 is the index containing the MSB of *chunk_list_base1.

  *chunk_list_base0 is a bitmap consisting of at least (bit_count_minus_1+bit_idx_min0+1) undefined bits.

  *chunk_list_base1 is a bitmap consisting of at least (bit_count_minus_1+bit_idx_min1+1) bits to read, even if some of them have been zero-optimized away at the high end, as distinct from biguint_bitmap_copy_reverse_unsafe().

Out:

  Returns the index containing the MSB of *chunk_list_base0. Unlike biguint_bitmap_copy_unsafe(), this function performs trailing zero truncation, so this value may be less than, equal to, or greater than chunk_idx_max0.

  *chunk_list_base0 at bit indexes on [bit_idx_min0, bit_count_minus_1+bit_idx_min0] equals *chunk_list_base1 at bit indexes on [bit_idx_min1, bit_count_minus_1+bit_idx_min1], in bitwise reverse order, except that the former region is undefined beyond the returned ULONG index.
*/
  u64 bit_count_minus_1_partial;
  u64 bit_idx_copy_max0;
  u64 bit_idx_gap0;
  u64 bit_idx_max0;
  u64 bit_idx_max1;
  u64 bit_idx_min_reverse0;
  ULONG chunk_idx_copy_max0;

  bit_idx_max0=((u64)(chunk_idx_max0)<<ULONG_BITS_LOG2)+ULONG_BIT_MAX;
  bit_idx_max1=((u64)(chunk_idx_max1)<<ULONG_BITS_LOG2)+ULONG_BIT_MAX;
  if(bit_idx_min1<=bit_idx_max1){
    bit_count_minus_1_partial=bit_idx_max1-bit_idx_min1;
    bit_count_minus_1_partial=MIN(bit_count_minus_1, bit_count_minus_1_partial);
    bit_idx_copy_max0=bit_count_minus_1+bit_idx_min0;
    chunk_idx_copy_max0=(ULONG)(bit_idx_copy_max0>>ULONG_BITS_LOG2);
    if(chunk_idx_max0<chunk_idx_copy_max0){
      chunk_idx_max0=chunk_idx_copy_max0;
      chunk_list_base0[chunk_idx_copy_max0]=0;
    }
    bit_idx_min_reverse0=bit_idx_copy_max0-bit_count_minus_1_partial;
    biguint_bitmap_copy_reverse_unsafe(bit_count_minus_1_partial, bit_idx_min_reverse0, bit_idx_min1, chunk_list_base0, chunk_list_base1);
    BIGUINT_CANONIZE(chunk_idx_max0, chunk_list_base0);
    if(bit_count_minus_1!=bit_count_minus_1_partial){
      bit_count_minus_1_partial=bit_count_minus_1-bit_count_minus_1_partial-1;
      chunk_idx_max0=biguint_bitmap_clear(bit_count_minus_1_partial, bit_idx_min0, chunk_idx_max0, chunk_list_base0);
    }
    if(bit_idx_max0<bit_idx_min_reverse0){
      bit_idx_gap0=bit_idx_max0+1;
      if(bit_idx_gap0!=bit_idx_min_reverse0){
        bit_count_minus_1_partial=bit_idx_min_reverse0-bit_idx_gap0-1;
        chunk_idx_max0=biguint_bitmap_clear(bit_count_minus_1_partial, bit_idx_gap0, chunk_idx_max0, chunk_list_base0);
      }
    }
  }else if(bit_idx_min0<=bit_idx_max0){
    bit_count_minus_1_partial=bit_idx_max0-bit_idx_min0;
    bit_count_minus_1_partial=MIN(bit_count_minus_1, bit_count_minus_1_partial);
    chunk_idx_max0=biguint_bitmap_clear(bit_count_minus_1_partial, bit_idx_min0, chunk_idx_max0, chunk_list_base0);    
  }
  return chunk_idx_max0;
}

void
biguint_bitmap_copy_reverse_unsafe(u64 bit_count_minus_1, u64 bit_idx_min0, u64 bit_idx_min1, ULONG *chunk_list_base0, ULONG *chunk_list_base1){
/*
Copy a region of a bitmap in bitwise reverse order to a region of another bitmap, but don't truncate terminating zeros (hence "unsafe"), in order to facilitate straightforward bitstring copies within fixed sized structures.

To reverse an entire biguint or its ULONG-granular hull, use biguint_reverse() or biguint_reverse_unsafe(), respectively.

Unlike some other Biguint functions, this one doesn't assume 64-bit granularity; ULONG granularity is assumed, with no overshoot allowed.

In:

  bit_count_minus_1 is the number of bits to copy, less one.

  bit_idx_min0 is the index of the first bit to read at chunk_list_base0.

  bit_idx_min1 is the index of the first bit to write at chunk_list_base1.

  *chunk_list_base0 is a bitmap consisting of at least (bit_count_minus_1+bit_idx_min0+1) undefined bits.

  *chunk_list_base1 is a bitmap consisting of at least (bit_count_minus_1+bit_idx_min1+1) bits to read.

Out:

  See notes about high zero chunks in biguint_bitmap_copy_unsafe():Out.

  *chunk_list_base0 at bit indexes on [bit_idx_min0, bit_count_minus_1+bit_idx_min0] equals *chunk_list_base1 at bit indexes on [bit_idx_min1, bit_count_minus_1+bit_idx_min1], but in bitwise reverse order.
*/
  u64 bit_idx_max0;
  u64 bit_idx_max1;
  u8 chunk_first_lsb0;
  u8 chunk_first_lsb1;
  ULONG chunk_first_mask0;
  ULONG chunk_first_mask1;
  ULONG chunk_first0;
  ULONG chunk_first1;
  ULONG chunk_first_new1;
  ULONG chunk_idx_min0;
  ULONG chunk_idx_min1;
  ULONG chunk_last_mask0;
  u8 chunk_last_msb0;
  ULONG chunk_last0;
  ULONG chunk_last1;
  ULONG chunk_idx_max0;
  ULONG chunk_idx_max1;

  chunk_idx_min0=(ULONG)(bit_idx_min0>>ULONG_BITS_LOG2);
  chunk_idx_min1=(ULONG)(bit_idx_min1>>ULONG_BITS_LOG2);
  bit_idx_max0=bit_idx_min0+bit_count_minus_1;
  bit_idx_max1=bit_idx_min1+bit_count_minus_1;
  chunk_idx_max0=(ULONG)(bit_idx_max0>>ULONG_BITS_LOG2);
  chunk_idx_max1=(ULONG)(bit_idx_max1>>ULONG_BITS_LOG2);
  chunk_first_lsb0=bit_idx_min0&ULONG_BIT_MAX;
  chunk_first_lsb1=bit_idx_min1&ULONG_BIT_MAX;
  chunk_first_mask1=(ULONG)(ULONG_MAX<<chunk_first_lsb1);
  chunk_first_mask0=(ULONG)(~(ULONG_MAX<<chunk_first_lsb0));
  chunk_last_msb0=bit_idx_max0&ULONG_BIT_MAX;
  chunk_last_mask0=(ULONG)((ULONG_MAX<<1)<<chunk_last_msb0);
  chunk_first1=chunk_list_base1[chunk_idx_min1];
  chunk_last0=chunk_list_base0[chunk_idx_max0];
  chunk_first1&=chunk_first_mask1;
  chunk_last0&=chunk_last_mask0;
  while(ULONG_BIT_MAX<bit_count_minus_1){
    chunk_first_new1=chunk_list_base1[chunk_idx_min1+1];
    if(chunk_first_lsb1){
      chunk_first1>>=chunk_first_lsb1;
      chunk_first1|=chunk_first_new1<<(u8)(ULONG_BITS-chunk_first_lsb1);
    }
    REVERSE_ULONG(chunk_first1);
    if(chunk_last_msb0!=ULONG_BIT_MAX){
      chunk_list_base0[chunk_idx_max0]=chunk_last0|(chunk_first1>>(u8)(ULONG_BIT_MAX-chunk_last_msb0));
      chunk_last0=chunk_first1<<(u8)(chunk_last_msb0+1);
    }else{
      chunk_list_base0[chunk_idx_max0]=chunk_first1;
    }
    chunk_idx_max0--;
    chunk_idx_min1++;
    chunk_first1=chunk_first_new1;
    bit_count_minus_1-=ULONG_BITS;
  }
  chunk_first1=chunk_list_base1[chunk_idx_min1];
  if(chunk_idx_max1==chunk_idx_min1){
    chunk_first1>>=chunk_first_lsb1;
  }else{
    chunk_last1=chunk_list_base1[chunk_idx_max1];
    chunk_last1<<=(u8)(ULONG_BITS-chunk_first_lsb1);
    chunk_first1>>=chunk_first_lsb1;
    chunk_first1|=chunk_last1;
  }
  REVERSE_ULONG(chunk_first1);
  chunk_first0=chunk_list_base0[chunk_idx_min0];
  chunk_first0&=chunk_first_mask0;
  chunk_first1>>=(u8)(ULONG_BIT_MAX-bit_count_minus_1);
  if(chunk_idx_max0==chunk_idx_min0){
    chunk_first1<<=chunk_first_lsb0;
    chunk_first1|=chunk_first0|chunk_last0;
  }else{
    chunk_last1=(chunk_first1>>(u8)(ULONG_BITS-chunk_first_lsb0))|chunk_last0;
    chunk_first1=(chunk_first1<<chunk_first_lsb0)|chunk_first0;
    chunk_list_base0[chunk_idx_max0]=chunk_last1;
  }
  chunk_list_base0[chunk_idx_min0]=chunk_first1;
  return;
}

void
biguint_bitmap_copy_unsafe(u64 bit_count_minus_1, u64 bit_idx_min0, u64 bit_idx_min1, ULONG *chunk_list_base0, ULONG *chunk_list_base1){
/*
Copy a region of a bitmap to a region of another bitmap, but don't truncate terminating zeros (hence "unsafe"), in order to facilitate straightforward bitstring copies within fixed sized structures.

Unlike some other Biguint functions, this one doesn't assume 64-bit granularity; ULONG granularity is assumed, with no overshoot allowed.

In:

  bit_count_minus_1 is the number of bits to copy, less one.

  bit_idx_min0 is the index of the first bit to read at chunk_list_base0.

  bit_idx_min1 is the index of the first bit to write at chunk_list_base1.

  *chunk_list_base0 is a bitmap consisting of at least (bit_count_minus_1+bit_idx_min0+1) undefined bits.

  *chunk_list_base1 is a bitmap consisting of at least (bit_count_minus_1+bit_idx_min1+1) bits to read.

Out:

  Unlike most other Biguint functions, in the interest of speed, this one does not return the maximum chunk index of the result after clipping trailing zeroes, so it's possible that the return value is both more than one chunk long and contains a zero high chunk, which would violate In assumptions of those other functions.

  *chunk_list_base0 at bit indexes on [bit_idx_min0, bit_count_minus_1+bit_idx_min0] equals *chunk_list_base1 at bit indexes on [bit_idx_min1, bit_count_minus_1+bit_idx_min1].
*/
  u64 bit_idx_max0;
  u64 bit_idx_max1;
  u8 chunk_first_lsb0;
  u8 chunk_first_lsb1;
  ULONG chunk_first_mask0;
  ULONG chunk_first_mask1;
  ULONG chunk_idx_min0;
  ULONG chunk_idx_min1;
  ULONG chunk_idx0;
  ULONG chunk_idx1;
  ULONG chunk_last_mask0;
  ULONG chunk_last_mask1;
  ULONG chunk_idx_max0;
  ULONG chunk_idx_max1;
  u8 chunk_last_msb0;
  u8 chunk_last_msb1;
  u8 chunk_shift;
  UDOUBLE chunk0;
  UDOUBLE chunk1;
  ULONG list_size;

  chunk_idx_min0=(ULONG)(bit_idx_min0>>ULONG_BITS_LOG2);
  chunk_idx_min1=(ULONG)(bit_idx_min1>>ULONG_BITS_LOG2);
  chunk0=chunk_list_base0[chunk_idx_min0];
  chunk1=chunk_list_base1[chunk_idx_min1];
  bit_idx_max0=bit_idx_min0+bit_count_minus_1;
  bit_idx_max1=bit_idx_min1+bit_count_minus_1;
  chunk_idx_max0=(ULONG)(bit_idx_max0>>ULONG_BITS_LOG2);
  chunk_idx_max1=(ULONG)(bit_idx_max1>>ULONG_BITS_LOG2);
  chunk_first_lsb0=bit_idx_min0&ULONG_BIT_MAX;
  chunk_first_lsb1=bit_idx_min1&ULONG_BIT_MAX;
  chunk_first_mask0=(ULONG)(~(ULONG_MAX<<chunk_first_lsb0));
  chunk_first_mask1=(ULONG)(ULONG_MAX<<chunk_first_lsb1);
  chunk_last_msb0=bit_idx_max0&ULONG_BIT_MAX;
  chunk_last_msb1=bit_idx_max1&ULONG_BIT_MAX;
  chunk_last_mask0=(ULONG)((ULONG_MAX<<1)<<chunk_last_msb0);
  chunk_last_mask1=(ULONG)(~((ULONG_MAX<<1)<<chunk_last_msb1));
  if(chunk_idx_max1!=chunk_idx_min1){
    chunk1&=chunk_first_mask1;
    if(chunk_idx_max0!=chunk_idx_min0){
      chunk0&=chunk_first_mask0;
      if(chunk_first_lsb0==chunk_first_lsb1){
        if(!chunk_first_lsb1){
          list_size=(ULONG)(((bit_count_minus_1+1)>>ULONG_BITS_LOG2)<<ULONG_SIZE_LOG2);
          memcpy(&chunk_list_base0[chunk_idx_min0], &chunk_list_base1[chunk_idx_min1], (size_t)(list_size));
        }else{
          chunk_list_base0[chunk_idx_min0]=(ULONG)(chunk0|chunk1);
          chunk_idx0=chunk_idx_min0+1;
          chunk_idx1=chunk_idx_min1+1;
          list_size=(ULONG)(((bit_count_minus_1-ULONG_BIT_MAX+chunk_first_lsb1)>>ULONG_BITS_LOG2)<<ULONG_SIZE_LOG2);
          memcpy(&chunk_list_base0[chunk_idx0], &chunk_list_base1[chunk_idx1], (size_t)(list_size));
        }
        chunk0=chunk_list_base0[chunk_idx_max0];
        chunk1=chunk_list_base1[chunk_idx_max1];
        chunk0&=chunk_last_mask0;
        chunk1&=chunk_last_mask1;
        chunk_list_base0[chunk_idx_max0]=(ULONG)(chunk0|chunk1);
      }else{
        chunk_idx1=chunk_idx_min1+1;
        chunk1|=(UDOUBLE)(chunk_list_base1[chunk_idx1])<<ULONG_BITS;
        chunk_list_base0[chunk_idx_min0]=(ULONG)(((chunk1>>chunk_first_lsb1)<<chunk_first_lsb0)|chunk0);
        chunk_shift=(u8)(chunk_first_lsb1+ULONG_BITS-chunk_first_lsb0);
        if(ULONG_BIT_MAX<chunk_shift){
          chunk_idx1++;
          chunk_shift=(u8)(chunk_shift-ULONG_BITS);
          chunk1>>=ULONG_BITS;
        }
        for(chunk_idx0=chunk_idx_min0+1; chunk_idx0<chunk_idx_max0; chunk_idx0++){
          chunk1|=(UDOUBLE)(chunk_list_base1[chunk_idx1])<<ULONG_BITS;
          chunk_idx1++;
          chunk_list_base0[chunk_idx0]=(ULONG)(chunk1>>chunk_shift);
          chunk1>>=ULONG_BITS;
        }
        chunk0=chunk_list_base0[chunk_idx_max0];
        chunk1|=(UDOUBLE)(chunk_list_base1[chunk_idx_max1])<<ULONG_BITS;
        chunk0&=chunk_last_mask0;
        chunk1<<=ULONG_BIT_MAX-chunk_last_msb1;
        chunk_list_base0[chunk_idx_max0]=(ULONG)((chunk1>>(UDOUBLE_BIT_MAX-chunk_last_msb0))|chunk0);
      }
    }else{
      chunk1|=(UDOUBLE)(chunk_list_base1[chunk_idx_max1])<<ULONG_BITS;
      chunk0&=chunk_first_mask0|chunk_last_mask0;
      chunk1&=chunk_first_mask1|((UDOUBLE)(chunk_last_mask1)<<ULONG_BITS);
      chunk_list_base0[chunk_idx_min0]=(ULONG)(((chunk1>>chunk_first_lsb1)<<chunk_first_lsb0)|chunk0);
    }
  }else if(chunk_idx_max0!=chunk_idx_min0){
    chunk0|=(UDOUBLE)(chunk_list_base0[chunk_idx_max0])<<ULONG_BITS;
    chunk0&=chunk_first_mask0|((UDOUBLE)(chunk_last_mask0)<<ULONG_BITS);
    chunk1&=chunk_first_mask1&chunk_last_mask1;
    chunk0|=(chunk1>>chunk_first_lsb1)<<chunk_first_lsb0;
    chunk_list_base0[chunk_idx_min0]=(ULONG)(chunk0);
    chunk_list_base0[chunk_idx_min0+1]=(ULONG)(chunk0>>ULONG_BITS);
  }else{
    chunk0&=chunk_first_mask0|chunk_last_mask0;
    chunk1&=chunk_first_mask1&chunk_last_mask1;
    chunk_list_base0[chunk_idx_min0]=(ULONG)(((chunk1>>chunk_first_lsb1)<<chunk_first_lsb0)|chunk0);
  }
  return;
}

u8
biguint_bitmap_export(u64 bit_idx_max, ULONG chunk_idx_max, ULONG chunk_idx_max_max, ULONG *chunk_list_base){
/*
Literalize the notional high zeroes of a biguint in preparation for export.

In:

  bit_idx_max is one less than the number of bits in the notional bitmap based at chunk_list_base -- not necessarily its MSB.

  chunk_idx_max is the index containing the MSB of *chunk_list_base.

  chunk_idx_max_max is the number of (ULONG)s allocated at chunk_list_base, less one.

  *chunk_list_base is the bitmap.

Out:

  Returns one if bit_count_minus_1 implies a maximum ULONG index exceeding chunk_idx_max_max, else zero.

  *chunk_list_base has been modified such that, if bit_count_minus_1 does not align to the maximum bit index of a ULONG, then all higher bits within that ULONG are zero. This ensures that the biguint is all zeroes beyond the last bit of the bitmap. All chunks after that last ULONG remain undefined, as per normal for a biguint. All (ULONG)s strictly between chunk_idx_max and said last ULONG are literally zero, and not merely implied as such.
*/
  u8 bit_idx_max_tail;
  ULONG export_idx_max;
  ULONG tail_mask;
  u8 status;

  export_idx_max=(ULONG)(bit_idx_max>>ULONG_BITS_LOG2);
  status=1;
  if(export_idx_max<=chunk_idx_max_max){
    if(export_idx_max<=chunk_idx_max){
      bit_idx_max_tail=(u8)(bit_idx_max&ULONG_BIT_MAX);
      tail_mask=(ULONG)(1ULL<<1<<bit_idx_max_tail);
      tail_mask--;
      chunk_list_base[export_idx_max]=chunk_list_base[export_idx_max]&tail_mask;
    }else{
      do{
        chunk_idx_max++;
        chunk_list_base[chunk_idx_max]=0;
      }while(chunk_idx_max!=export_idx_max);
    }
    status=0;
  }
  return status;
}

u8
biguint_bitmap_import(u64 bit_idx_max, ULONG *chunk_idx_max_base, ULONG chunk_idx_max_max, ULONG *chunk_list_base){
/*
Convert a bitmap to canonical biguint form by truncating high zeroes.

In:

  bit_idx_max is one less than the number of bits in the notional bitmap based at chunk_list_base -- not necessarily its MSB.

  *chunk_idx_max_base is undefined.

  chunk_idx_max_max is the number of (ULONG)s allocated at chunk_list_base, less one.

  *chunk_list_base is the bitmap.

Out:

  Returns one if bit_count_minus_1 implies a maximum ULONG index exceeding chunk_idx_max_max, else zero.

  *chunk_idx_max_base is the index containing the MSB of *chunk_list_base.

  *chunk_list_base has been modified such that, if bit_count_minus_1 does not align to the maximum bit index of a ULONG, then all higher bits within that ULONG are zero. This ensures that postterminal one bits in the bitmap do not corrupt the resulting biguint. All chunks after that last ULONG are undefined, as per normal for a biguint.
*/
  ULONG import_idx_max;
  u8 status;

  import_idx_max=(ULONG)(bit_idx_max>>ULONG_BITS_LOG2);
  status=1;
  if(import_idx_max<=chunk_idx_max_max){
    *chunk_idx_max_base=biguint_truncate(bit_idx_max, chunk_idx_max_max, chunk_list_base);
    status=0;
  }
  return status;
}

ULONG
biguint_bitmap_set(u64 bit_count_minus_1, u64 bit_idx_min, ULONG chunk_idx_max, ULONG *chunk_list_base){
/*
Set a region of a bitmap to ones.

In:

  bit_count_minus_1 is the number of bits to clear, less one.

  bit_idx_min is the index of the first bit to clear.

  chunk_idx_max is the index containing the MSB of *chunk_list_base.

  *chunk_list_base is the bitmap containing the region to clear.

Out:

  Returns the updated value of chunk_idx_max.

  *chunk_list_base contains zero bits at bit indexes on [bit_idx_min, bit_count_minus_1+bit_idx_min], except that such region is undefined beyond the returned ULONG index.
*/
  u64 bit_idx_max;
  u64 bit_idx_post;
  ULONG chunk_idx_first;
  ULONG chunk_idx_last;
  ULONG chunk_mask_first;
  ULONG chunk_mask_last;
  ULONG list_size;
  u64 zero_count_minus_1;

  bit_idx_max=((u64)(chunk_idx_max)<<ULONG_BITS_LOG2)+ULONG_BIT_MAX;
  if(bit_idx_max<bit_idx_min){
    bit_idx_post=bit_idx_max+1;
    zero_count_minus_1=bit_idx_min-bit_idx_post;
    chunk_idx_max=biguint_bitmap_clear(zero_count_minus_1, bit_idx_post, chunk_idx_max, chunk_list_base);
  }
  bit_idx_max=bit_idx_min+bit_count_minus_1;
  chunk_mask_first=(ULONG)(~(((ULONG)(1)<<(bit_idx_min&ULONG_BIT_MAX))-1));
  chunk_mask_last=(ULONG)(((ULONG)(1)<<1<<(bit_idx_max&ULONG_BIT_MAX))-1);
  chunk_idx_first=(ULONG)(bit_idx_min>>ULONG_BITS_LOG2);
  chunk_idx_last=(ULONG)(bit_idx_max>>ULONG_BITS_LOG2);
  if(chunk_idx_max<chunk_idx_last){
    chunk_idx_max=chunk_idx_last;
    chunk_list_base[chunk_idx_last]=0;
  }
  if(chunk_idx_first!=chunk_idx_last){
    chunk_list_base[chunk_idx_first]=chunk_list_base[chunk_idx_first]|chunk_mask_first;
    list_size=(chunk_idx_last-chunk_idx_first-1)<<ULONG_SIZE_LOG2;
    memset(&chunk_list_base[chunk_idx_first+1], U8_MAX, (size_t)(list_size));
    chunk_list_base[chunk_idx_last]=chunk_list_base[chunk_idx_last]|chunk_mask_last;
  }else{
    chunk_list_base[chunk_idx_first]=chunk_list_base[chunk_idx_first]|(chunk_mask_first&chunk_mask_last);
  }
  return chunk_idx_max;
}

u8
biguint_chunk_idx_max_get(ULONG *biguint_chunk_idx_max_base, u64 *logplex_bit_idx_min_base, ULONG logplex_chunk_idx_max, ULONG *logplex_chunk_list_base){
/*
Compute the minimum maximum (sic) ULONG index required to decode a possibly truncated logplex of specified maximum size to a biguint.

In:

  *biguint_chunk_idx_max_base is undefined.

  *logplex_bit_idx_min_base is the base bit index of the logplex at logplex_chunk_list_base whose decoded maximum index to compute, on [0, ((logplex_chunk_idx_max+1)<<ULONG_BITS_LOG2)-1].

  logplex_chunk_idx_max is the index containing the MSB of *logplex_chunk_list_base.

  *logplex_chunk_list_base is the base of a bitmap containing the logplex whose maximum decoded chunk index to compute.

Out:

  Returns one if the size of the biguint could not be computed, due to logplex_chunk_idx_max being too small, else zero.

  *biguint_chunk_idx_max_base is the maximum index required to store the decoded biguint.

  *logplex_bit_idx_min_base has been incremented by the bit size of the logplex.
*/
  u64 logplex_bit_idx_min;
  u64 mantissa_bit_idx_min;
  u64 mantissa_msb;
  u8 status;

  logplex_bit_idx_min=*logplex_bit_idx_min_base;
  status=biguint_logplex_mantissa_get(&logplex_bit_idx_min, logplex_chunk_idx_max, logplex_chunk_list_base, &mantissa_bit_idx_min, &mantissa_msb);
  if(!status){
    *biguint_chunk_idx_max_base=(ULONG)(mantissa_msb>>ULONG_BITS_LOG2);
    *logplex_bit_idx_min_base=logplex_bit_idx_min;
  }
  return status;
}

u8
biguint_compare_biguint(ULONG chunk_idx_max0, ULONG chunk_idx_max1, ULONG *chunk_list_base0, ULONG *chunk_list_base1){
/*
Determine ordering status of a pair of biguints.

In:

  chunk_idx_max0 is the index containing the MSB of *chunk_list_base0.

  chunk_idx_max1 is the index containing the MSB of *chunk_list_base1.

  *chunk_list_base0 is a list of chunks of a biguint.

  *chunk_list_base1 is a list of chunks of a biguint.

Out:

  Returns BIGUINT_COMPARE_EQUAL if both biguints are equal, or BIGUINT_COMPARE_LESS if *chunk_list_base0 is less than *chunk_list_base1. Else BIGUINT_COMPARE_GREATER.
*/
  ULONG chunk0;
  ULONG chunk1;
  u8 status;

  status=BIGUINT_COMPARE_EQUAL;
  if(chunk_idx_max0==chunk_idx_max1){
    do{
      chunk0=chunk_list_base0[chunk_idx_max0];
      chunk1=chunk_list_base1[chunk_idx_max0];
      if(chunk0<chunk1){
        status=BIGUINT_COMPARE_LESS;
        break;
      }else if(chunk1<chunk0){
        status=BIGUINT_COMPARE_GREATER;
        break;
      }
    }while(chunk_idx_max0--);
  }else if(chunk_idx_max0<chunk_idx_max1){
    status=BIGUINT_COMPARE_LESS;
  }else{
    status=BIGUINT_COMPARE_GREATER;
  }
  return status;
}

u8
biguint_compare_u128(ULONG chunk_idx_max, ULONG *chunk_list_base, u128 uint){
/*
Determine ordering status of a biguint as compared to a u128.

In:

  chunk_idx_max is the index containing the MSB of *chunk_list_base.

  *chunk_list_base is a list of chunks of a biguint.

  uint is the u128 against which the biguint is to be compared.

Out:

  Returns BIGUINT_COMPARE_LESS, BIGUINT_COMPARE_EQUAL, or BIGUINT_COMPARE_GREATER if *chunk_list_base is less than, equal to, or greater than uint, respectively.
*/
  u128 biguint;
  int compare_status;
  ULONG list_size;
  u8 status;

  status=BIGUINT_COMPARE_GREATER;
  if(chunk_idx_max<=(U128_BYTE_MAX>>ULONG_SIZE_LOG2)){
    list_size=(chunk_idx_max+1)<<ULONG_SIZE_LOG2;
    memset(&biguint, 0, (size_t)(U128_SIZE));
    memcpy(&biguint, chunk_list_base, (size_t)(list_size));
    compare_status=memcmp(&biguint, &uint, (size_t)(list_size));
    if(compare_status<=0){
      status=compare_status?BIGUINT_COMPARE_LESS:BIGUINT_COMPARE_EQUAL;
    }
  }
  return status;
}

u8
biguint_compare_u64(ULONG chunk_idx_max, ULONG *chunk_list_base, u64 uint){
/*
Determine ordering status of a biguint as compared to a u64.

In:

  chunk_idx_max is the index containing the MSB of *chunk_list_base.

  *chunk_list_base is a list of chunks of a biguint.

  uint is the u64 against which the biguint is to be compared.

Out:

  Returns BIGUINT_COMPARE_LESS, BIGUINT_COMPARE_EQUAL, or BIGUINT_COMPARE_GREATER if *chunk_list_base is less than, equal to, or greater than uint, respectively.
*/
  u64 biguint;
  u8 status;

  status=BIGUINT_COMPARE_GREATER;
  if(chunk_idx_max<=(U64_BYTE_MAX>>ULONG_SIZE_LOG2)){
    biguint=chunk_list_base[0];
    #ifdef _32_
      if(chunk_idx_max){
        biguint|=(u64)(chunk_list_base[1])<<ULONG_BITS;
      }
    #endif
    if(biguint<=uint){
      status=(biguint<uint)?BIGUINT_COMPARE_LESS:BIGUINT_COMPARE_EQUAL;
    }
  }
  return status;
}

ULONG
biguint_copy(ULONG chunk_idx_max, ULONG *chunk_list_base0, ULONG *chunk_list_base1){
/*
Copy a list of (ULONG)s.

In:

  chunk_idx_max is the maximum index of *chunk_list_base0, which, unlike in most other Biguint functions, may be greater than the index containing the MSB.

  *chunk_list_base0 is the base of at least (chunk_idx_max+1) undefined chunks.

  *chunk_list_base1 is a list of chunks of the biguint to copy.

Out:

  Returns chunk_idx_max so it can be easily copied to the maximum index of *chunk_list_base1.

  *chunk_list_base0 is identical to *chunk_list_base1 for its first (chunk_idx_max+1) chunks.
*/
  ULONG list_size;

  list_size=(chunk_idx_max+1)<<ULONG_SIZE_LOG2;
  memcpy(chunk_list_base0, chunk_list_base1, (size_t)(list_size));
  return chunk_idx_max;
}

ULONG
biguint_decrement(ULONG chunk_idx_max, ULONG *chunk_list_base){
/*
Decrement a biguint.

In:

  chunk_idx_max is the index containing the MSB of *chunk_list_base.

  *chunk_list_base is list of chunks of a nonzero biguint.

Out:

  Returns the updated value of chunk_idx_max.

  *chunk_list_base has been decreased by one.
*/
  ULONG chunk;
  ULONG chunk_idx;

  chunk_idx=0;
  do{
    chunk=chunk_list_base[chunk_idx];
    chunk--;
    chunk_list_base[chunk_idx]=chunk;
    chunk_idx++;
  }while(chunk==ULONG_MAX);
  if((!chunk)&&chunk_idx_max&&(chunk_idx_max<chunk_idx)){
    chunk_idx_max--;
  }
  return chunk_idx_max;
}

ULONG
biguint_divide_biguint(ULONG *chunk_idx_max_base0, ULONG chunk_idx_max1, ULONG *chunk_list_base0, ULONG *chunk_list_base1, ULONG *chunk_list_base2){
/*
Divide one biguint by another.

In:

  *chunk_idx_max0_base is the index containing the MSB of *chunk_list_base0.

  chunk_idx_max1 is the index containing the MSB of *chunk_list_base1.

  *chunk_list_base0 is a list of chunks of the biguint dividend.

  *chunk_list_base1 is a list of chunks of the biguint divisor. Zero is allowed, as explained in (biguint_divide_u64():Out:*chunk_list_base).

  chunk_list_base2 is one of the following: (1) NULL if only the remainder is to be returned at chunk_list_base0; (2) equal to chunk_list_base0 if only the quotient is to be returned; or (3) the base, which may equal chunk_list_base1, of (chunk_idx_max1+1) undefined chunks to hold the returned remainder.

Out:

  Returns the index containing the MSB of *chunk_list_base2, or zero if not applicable.

  *chunk_idx_max_base0 is its updated value.

  *chunk_list_base0 is zero if (In:*chunk_list_base1) was zero. Else the remainder or quotient of ((In:*chunk_list_base0)/(In:*chunk_list_base1)) if chunk_list_base2 is NULL or nonnull, respectively.

  *chunk_list_base2 is unchanged if chunk_list_base2 is NULL, or ignored if it equals chunk_list_base0. Else zero if (In:*chunk_list_base1) was zero. Else ((In:*chunk_list_base0)%(In:*chunk_list_base1)).
*/
  u8 borrow;
  ULONG chunk;
  ULONG chunk_idx;
  ULONG chunk_idx_max0;
  ULONG chunk_idx_max2;
  ULONG chunk_old;
  u8 continue_status;
  u8 dividend_shift;
  UDOUBLE dividend_udouble;
  ULONG divisor;
  ULONG divisor_hi;
  ULONG divisor_plus_1;
  u8 divisor_shift;
  ULONG list_size;
  u8 msb;
  UDOUBLE product;
  ULONG quotient;
  ULONG quotient_idx_min;
  u8 quotient_shift;
  u8 quotient_shift_negative;
  ULONG quotient0;
  ULONG quotient1;
  ULONG remainder_chunk_count;
  u64 remainder_u64;
  ULONG remainder0;
  ULONG remainder1;
  ULONG subtract_chunk_idx_min;

  chunk_idx_max0=*chunk_idx_max_base0;
  divisor_hi=chunk_list_base1[chunk_idx_max1];
  chunk_idx_max2=0;
  if(chunk_idx_max1&&(chunk_idx_max1<=chunk_idx_max0)){
    divisor=divisor_hi;
    BITSCAN_MSB_ULONG_FLAT_GET(msb, divisor_hi);
    divisor_shift=(u8)(ULONG_BIT_MAX-msb);
    if(divisor_shift){
      divisor=(chunk_list_base1[chunk_idx_max1-1]>>(ULONG_BITS-divisor_shift))|(divisor_hi<<divisor_shift);
    }
    remainder_chunk_count=chunk_idx_max0-1;
    remainder0=chunk_list_base0[remainder_chunk_count];
    remainder1=chunk_list_base0[chunk_idx_max0];
    continue_status=1;
    divisor_plus_1=divisor+1;
/*
The MSB of divisor_plus_1 might be one greater than divisor, or it might even have wrapped, such that zero means (2^ULONG_BITS). It seems like we should increment divisor_shift in such cases, but in fact that would be wrong. The point is to generate a lower-bound quotient.
*/
    chunk_list_base0[remainder_chunk_count]=0;
    chunk_list_base0[chunk_idx_max0]=0;
    do{
      BITSCAN_MSB_ULONG_BIG_GET(msb, remainder1);
      dividend_shift=(u8)(ULONG_BIT_MAX-1-msb);
      dividend_udouble=((UDOUBLE)(remainder1)<<ULONG_BITS)|remainder0;
      if(dividend_shift!=U8_MAX){
        dividend_udouble<<=dividend_shift;
      }else{
        dividend_udouble>>=1;
      }
      if(divisor_plus_1){
        quotient=(ULONG)(dividend_udouble/divisor_plus_1);
      }else{
        quotient=(ULONG)(dividend_udouble>>ULONG_BITS);
      }
      quotient_idx_min=remainder_chunk_count;
      quotient_shift=(u8)(divisor_shift-dividend_shift);
      if(ULONG_BITS<quotient_shift){
        if((chunk_idx_max1+1)<=remainder_chunk_count){
          remainder_chunk_count--;
          remainder1=remainder0;
          remainder0=chunk_list_base0[remainder_chunk_count];
          quotient_idx_min--;
          quotient_shift=(u8)(quotient_shift+ULONG_BITS);
          subtract_chunk_idx_min=remainder_chunk_count-chunk_idx_max1;
          chunk_list_base0[remainder_chunk_count]=0;
        }else if(chunk_idx_max1==remainder_chunk_count){
          continue_status=0;
          quotient_shift_negative=(u8)(0U-quotient_shift);
          quotient>>=quotient_shift_negative;
          quotient_shift=0;
          subtract_chunk_idx_min=0;
        }else{
          continue_status=0;
          quotient=0;
        }
      }else if(!quotient_shift){
        if(chunk_idx_max1<=remainder_chunk_count){
          subtract_chunk_idx_min=remainder_chunk_count-chunk_idx_max1;
        }else{
          continue_status=0;
          quotient=0;
        }        
      }else if(quotient_shift!=ULONG_BITS){
        if(chunk_idx_max1<=remainder_chunk_count){
          subtract_chunk_idx_min=remainder_chunk_count-chunk_idx_max1;
        }else{
          continue_status=0;
          quotient_idx_min++;
          quotient_shift_negative=(u8)(ULONG_BITS-quotient_shift);
          quotient>>=quotient_shift_negative;
          quotient_shift=0;
          subtract_chunk_idx_min=0;
        }
      }else{
        quotient_idx_min++;
        quotient_shift=0;
        subtract_chunk_idx_min=remainder_chunk_count-chunk_idx_max1+1;
      }
      if(quotient){
        product=0;
        quotient_shift_negative=(u8)(ULONG_BITS-quotient_shift);
/*
Subtract the quotient times the divisor from the dividend after shifting the former as required.
*/
        for(chunk_idx=subtract_chunk_idx_min; chunk_idx<remainder_chunk_count; chunk_idx++){
          chunk=chunk_list_base0[chunk_idx];
          chunk_old=chunk;
          if(quotient_shift){
            chunk-=(ULONG)(product)>>quotient_shift_negative;
          }
          product>>=ULONG_BITS;
          product+=(UDOUBLE)(chunk_list_base1[chunk_idx-subtract_chunk_idx_min])*quotient;
          chunk-=(ULONG)(product)<<quotient_shift;
          product+=(UDOUBLE)(chunk_old<chunk)<<quotient_shift_negative;
          chunk_list_base0[chunk_idx]=chunk;
        }
        chunk_old=remainder0;
        if(quotient_shift){
          remainder0-=(ULONG)(product)>>quotient_shift_negative;
        }
        product>>=ULONG_BITS;
        if((remainder_chunk_count-subtract_chunk_idx_min)<=chunk_idx_max1){
          product+=(UDOUBLE)(chunk_list_base1[remainder_chunk_count-subtract_chunk_idx_min])*quotient;
        }
        remainder0-=(ULONG)(product)<<quotient_shift;
        product+=(UDOUBLE)(chunk_old<remainder0)<<quotient_shift_negative;
        chunk_old=remainder1;
        if(quotient_shift){
          remainder1-=(ULONG)(product)>>quotient_shift_negative;
        }
        product>>=ULONG_BITS;
        if((remainder_chunk_count-subtract_chunk_idx_min+1)<=chunk_idx_max1){
          product+=(UDOUBLE)(chunk_list_base1[remainder_chunk_count-subtract_chunk_idx_min+1])*quotient;
        }
        remainder1-=(ULONG)(product)<<quotient_shift;
        chunk=chunk_list_base0[quotient_idx_min];
        chunk_old=chunk;
        chunk+=quotient<<quotient_shift;
        chunk_list_base0[quotient_idx_min]=chunk;
        if(quotient_shift){
          quotient>>=quotient_shift_negative;
        }else{
          quotient=0;
        }
        quotient+=chunk<chunk_old;
/*
This "if" is not just a matter of efficiency. If you remove it, you could write past end.
*/
        if(quotient){
          chunk_idx=quotient_idx_min+1;
          chunk=chunk_list_base0[chunk_idx];
          chunk_old=chunk;
          chunk+=quotient;
          chunk_list_base0[chunk_idx]=chunk;
          while(chunk<chunk_old){
            chunk_idx++;
            chunk=chunk_list_base0[chunk_idx];
            chunk_old=chunk;
            chunk++;
            chunk_list_base0[chunk_idx]=chunk;
          }
        }
        while((!remainder1)&&remainder_chunk_count){
          remainder_chunk_count--;
          remainder1=remainder0;
          remainder0=chunk_list_base0[remainder_chunk_count];
          chunk_list_base0[remainder_chunk_count]=0;
        }
        continue_status=(u8)(continue_status&(chunk_idx_max1<=(remainder_chunk_count+1)));
      }
    }while(continue_status);
    quotient0=chunk_list_base0[chunk_idx_max1];
    quotient1=0;
    if(chunk_idx_max0!=chunk_idx_max1){
      quotient1=chunk_list_base0[chunk_idx_max1+1];
    }
    chunk_list_base0[remainder_chunk_count]=remainder0;
    chunk_list_base0[remainder_chunk_count+1]=remainder1;
    chunk_idx_max2=remainder_chunk_count+!!remainder1;
    while(chunk_idx_max1<=chunk_idx_max2){
      if(chunk_idx_max1==chunk_idx_max2){
        chunk=divisor_hi;
        chunk_idx=chunk_idx_max1;
        while((chunk==chunk_list_base0[chunk_idx])&&chunk_idx){
          chunk_idx--;
          chunk=chunk_list_base1[chunk_idx];
        }
        if(chunk_list_base0[chunk_idx]<chunk){
          break;
        }
      }
      borrow=0;
      for(chunk_idx=0; chunk_idx<=chunk_idx_max1; chunk_idx++){
        chunk=chunk_list_base0[chunk_idx];
        if(borrow){
          borrow=!chunk;
          chunk--;
        }
        chunk_old=chunk;
        chunk-=chunk_list_base1[chunk_idx];
        chunk_list_base0[chunk_idx]=chunk;
        borrow=(u8)(borrow|(chunk_old<chunk));
      }
      while(borrow){
        chunk=chunk_list_base0[chunk_idx];
        borrow=!chunk;
        chunk--;
        chunk_list_base0[chunk_idx]=chunk;
      }
      BIGUINT_CANONIZE(chunk_idx_max2, chunk_list_base0);
      quotient0++;
      if(!quotient0){
        quotient1++;
        if(!quotient1){
          chunk_idx=chunk_idx_max1+2;
          do{
            chunk=chunk_list_base0[chunk_idx];
            chunk++;
            chunk_list_base0[chunk_idx]=chunk;
            chunk_idx++;
          }while(!chunk);
        }
      }
    }
    if(chunk_list_base2){
      if(chunk_list_base0!=chunk_list_base2){
        for(chunk_idx=0; chunk_idx<=chunk_idx_max2; chunk_idx++){
          chunk=chunk_list_base0[chunk_idx];
          chunk_list_base2[chunk_idx]=chunk;
        }
      }
      chunk_list_base0[0]=quotient0;
      if(chunk_idx_max0!=chunk_idx_max1){
        chunk=quotient1;
        chunk_list_base0[1]=quotient1;
        for(chunk_idx=chunk_idx_max1+2; chunk_idx<=chunk_idx_max0; chunk_idx++){
          chunk=chunk_list_base0[chunk_idx];
          chunk_list_base0[chunk_idx-chunk_idx_max1]=chunk;
        }
        chunk_idx_max0-=!chunk;
      }
      chunk_idx_max0-=chunk_idx_max1;
    }else{
      chunk_idx_max0=remainder_chunk_count-1;
      chunk_idx_max2=0;
      BIGUINT_CANONIZE(chunk_idx_max0, chunk_list_base0);
    }
  }else{
    remainder_u64=0;
    if(!chunk_idx_max1){
      chunk_idx_max0=biguint_divide_u64(chunk_idx_max0, chunk_list_base0, &remainder_u64, divisor_hi);
      if(chunk_list_base2){
        if(chunk_list_base0!=chunk_list_base2){
          chunk_list_base2[0]=(ULONG)(remainder_u64);
        }
      }else{
        chunk_idx_max0=0;
        chunk_list_base0[0]=(ULONG)(remainder_u64);
      }
    }else{
      if(chunk_list_base2){
        if(chunk_list_base0!=chunk_list_base2){
          list_size=(chunk_idx_max0+1)<<ULONG_SIZE_LOG2;
          memcpy(chunk_list_base2, chunk_list_base0, (size_t)(list_size));
          chunk_idx_max2=chunk_idx_max0;
        }
        BIGUINT_SET_ZERO(chunk_idx_max0, chunk_list_base0);
      }
    }
  }
  *chunk_idx_max_base0=chunk_idx_max0;
  return chunk_idx_max2;
}

ULONG
biguint_divide_u64(ULONG chunk_idx_max, ULONG *chunk_list_base, u64 *remainder_base, u64 uint){
/*
Divide a biguint by a u64.

In:

  chunk_idx_max is the index containing the MSB of *chunk_list_base.

  *chunk_list_base is a list of chunks of a biguint.

  *remainder_base is undefined.

  uint is any value, even zero.

Out:

  Returns the updated value of chunk_idx_max.

  *chunk_list_base has been divided by uint and rounded toward zero unless uint was zero, in which case this quotient is zero. The reason is that zero is the least reasonable possible quotient, which will hopefully get the attention of the programmer. We don't want to invite problems by attempting to make OS calls in order to report a divide overflow. Nor do we want to define infinity as a special integer, which would create more overhead in every operation.

  *remainder_base is the remainder of the division, which is zero if uint is zero. The justification for this behavior set forth in *chunk_list_base still applies.
*/
  ULONG chunk_idx;
  UDOUBLE dividend;
  ULONG quotient;
  #ifdef _32_
    ULONG remainder;
    ULONG temp_chunk_idx_max;
    ULONG temp_chunk_list_base[U64_SIZE>>ULONG_SIZE_LOG2];
  #endif
  u64 remainder_u64;
  #ifdef _32_
    ULONG uint_ulong;
  #endif

  remainder_u64=0;
  if(2<uint){
    #ifdef _64_
      for(chunk_idx=chunk_idx_max; chunk_idx<=chunk_idx_max; chunk_idx--){
        dividend=chunk_list_base[chunk_idx]|((UDOUBLE)(remainder_u64)<<ULONG_BITS);
        quotient=(ULONG)(dividend/uint);
        remainder_u64=(ULONG)(dividend%uint);
        chunk_list_base[chunk_idx]=quotient;
      }
      if((!chunk_list_base[chunk_idx_max])&&chunk_idx_max){
        chunk_idx_max--;
      }
    #else
      remainder=0;
      uint_ulong=(ULONG)(uint);
      if(uint==uint_ulong){
        for(chunk_idx=chunk_idx_max; chunk_idx<=chunk_idx_max; chunk_idx--){
          dividend=chunk_list_base[chunk_idx]|((UDOUBLE)(remainder)<<ULONG_BITS);
          quotient=(ULONG)(dividend/uint);
          remainder=(ULONG)(dividend%uint);
          chunk_list_base[chunk_idx]=quotient;
        }
        if((!chunk_list_base[chunk_idx_max])&&chunk_idx_max){
          chunk_idx_max--;
        }
        remainder_u64=remainder;
      }else{
        temp_chunk_idx_max=1;
        temp_chunk_list_base[0]=uint_ulong;
        temp_chunk_list_base[1]=(ULONG)(uint>>ULONG_BITS);
        temp_chunk_idx_max=biguint_divide_biguint(&chunk_idx_max, temp_chunk_idx_max, chunk_list_base, temp_chunk_list_base, temp_chunk_list_base);
        remainder_u64=temp_chunk_list_base[0];
        if(temp_chunk_idx_max){
          remainder_u64|=(u64)(temp_chunk_list_base[1])<<ULONG_BITS;
        }
      }
    #endif
  }else if(uint!=1){
    uint--;
    remainder_u64=chunk_list_base[0]&uint;
    chunk_idx_max=biguint_shift_right(uint, chunk_idx_max, chunk_list_base);
  }else if(!uint){
/*
Division by zero is defined in Out to be zero remainder zero.
*/
    BIGUINT_SET_ZERO(chunk_idx_max, chunk_list_base);
  }
  *remainder_base=remainder_u64;
  return chunk_idx_max;
}

void *
biguint_free(void *base){
/*
To maximize portability and debuggability, this is the only function in which Biguint calls free().

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
biguint_from_ascii_decimal(ULONG *chunk_idx_max_base, ULONG chunk_idx_max_max, ULONG *chunk_list_base, const char *digit_list_base){
/*
Convert a null-terminated big endian ASCII string of decimal digits to a biguint.

In:

  *chunk_idx_max_base is undefined.

  chunk_idx_max_max was passed to biguint_malloc() to allocate *chunk_list_base.

  *chunk_list_base is undefined.

  *digit_list_base is the null-terminated ASCII string, which may originate from an untrusted source. Leading zeroes shall be ignored, but a whole block of zeros shall be considered as a valid zero. A null string shall return an error.

Out:

  Returns one if the string was null or contained characters other than digits, or if *chunk_list_base did not have enough space to store the binary result. Else zero.

  *chunk_idx_max_base is the index containing the MSB of *chunk_list_base.

  *chunk_list_base is the biguint corresponding to *digit_list_base.
*/
  ULONG chunk_idx_max;
  u8 digit;
  ULONG digit_idx;
  u8 status;

  status=1;
  if(digit_list_base[0]){
    BIGUINT_SET_ZERO(chunk_idx_max, chunk_list_base);
    digit_idx=0;
    do{
      digit=(u8)(digit_list_base[digit_idx]);
      digit_idx++;
      if((digit<'0')||('9'<digit)){
        status=!!digit;
        break;
      }
      digit=(u8)(digit-'0');
/*
Multiply the biguint by 10, then add the digit. If the result exceeds chunk_idx_max_max (ULONG)s in size, then fail. (Exceeding by one ULONG is safe, per biguint_malloc().)
*/
      chunk_idx_max=biguint_multiply_u64(chunk_idx_max, chunk_list_base, 0xA);
      chunk_idx_max=biguint_add_u64(chunk_idx_max, chunk_list_base, digit);
      status=(chunk_idx_max_max<chunk_idx_max);
    }while(!status);
  }
  if(status){
    BIGUINT_SET_ZERO(chunk_idx_max, chunk_list_base);
  }
  *chunk_idx_max_base=chunk_idx_max;
  return status;
}

u8
biguint_from_ascii_hex(ULONG *chunk_idx_max_base, ULONG chunk_idx_max_max, ULONG *chunk_list_base, const char *digit_list_base){
/*
Convert a null-terminated big endian ASCII string of hex digits to a biguint.

In:

  *chunk_idx_max_base is undefined.

  chunk_idx_max_max was passed to biguint_malloc() to allocate *chunk_list_base.

  *chunk_list_base is undefined.

  *digit_list_base is the null-terminated ASCII string, which may originate from an untrusted source. Leading zeroes shall be ignored, but a whole block of zeros shall be considered as a valid zero. A null string shall return an error. Upper and lower case digits are acceptable.

Out:

  Returns one if the string was null or contained characters other than digits, or if *chunk_list_base did not have enough space to store the binary result. Else zero.

  *chunk_idx_max_base is the index containing the MSB of *chunk_list_base.

  *chunk_list_base is the biguint corresponding to *digit_list_base.
*/
  ULONG chunk_idx_max;
  u8 digit;
  ULONG digit_idx;
  ULONG digit_idx_max;
  ULONG digit_idx_min;
  u8 shift;
  u8 status;

  chunk_idx_max=0;
  digit_idx_min=0;
  do{
    digit=(u8)(digit_list_base[digit_idx_min]);
    digit_idx_min++;
  }while(digit=='0');
  digit_idx_min--;
  if(digit){
    digit_idx_max=digit_idx_min;
    do{
      digit_idx_max++;
      digit=(u8)(digit_list_base[digit_idx_max]);
    }while(digit);
    digit_idx=digit_idx_max;
    shift=0;
    status=0;
    do{
      if(!shift){
        chunk_list_base[chunk_idx_max]=0;
      }
      digit_idx--;
      digit=(u8)(digit_list_base[digit_idx]);
      if(('0'<=digit)&&(digit<='9')){
        digit=(u8)(digit-'0');
      }else{
        digit=(u8)(digit|('a'-'A'));
        if(('a'<=digit)&&(digit<='f')){
          digit=(u8)(digit-'a'+0xA);
        }else{
          status=1;
          break;
        }
      }
      chunk_list_base[chunk_idx_max]=chunk_list_base[chunk_idx_max]|((ULONG)(digit)<<shift);
      shift=(u8)(shift+4);
      if(ULONG_BIT_MAX<shift){
        chunk_idx_max++;
        shift=0;
      }
    }while((digit_idx!=digit_idx_min)&&(chunk_idx_max<=chunk_idx_max_max));
    chunk_idx_max-=!shift;
    status=(u8)(status|(digit_idx!=digit_idx_min));
  }else{
    status=!digit_idx_min;
    chunk_list_base[0]=0;
  }
  if(status){
    BIGUINT_SET_ZERO(chunk_idx_max, chunk_list_base);
  }
  *chunk_idx_max_base=chunk_idx_max;
  return status;
}

ULONG
biguint_from_u128(ULONG *chunk_list_base, u128 uint){
/*
Copy a u128 to a biguint.

In:

  *chunk_list_base is undefined and writable for as many (ULONG)s as required to store uint.

  uint is the u128 to convert to a biguint.

Out:

  Returns the index containing the MSB of *chunk_list_base.

  *chunk_list_base equals uint.
*/
  ULONG chunk_idx_max;
  ULONG chunk_list_base_copy[U128_SIZE>>ULONG_SIZE_LOG2];
  ULONG list_size;

  memcpy(chunk_list_base_copy, &uint, (size_t)(U128_SIZE));
  chunk_idx_max=U128_BYTE_MAX>>ULONG_SIZE_LOG2;
  BIGUINT_CANONIZE(chunk_idx_max, chunk_list_base_copy);
  list_size=(chunk_idx_max+1)<<ULONG_SIZE_LOG2;
  memcpy(chunk_list_base, &uint, (size_t)(list_size));
  return chunk_idx_max;
}

ULONG
biguint_from_u64(ULONG *chunk_list_base, u64 uint){
/*
Copy a u64 to a biguint.

In:

  *chunk_list_base is undefined and writable for as many (ULONG)s as required to store uint.

  uint is the u64 to convert to a biguint.

Out:

  Returns the index containing the MSB of *chunk_list_base.

  *chunk_list_base equals uint.
*/
  ULONG chunk_idx_max;
  ULONG uint_ulong;

  chunk_idx_max=0;
  uint_ulong=(ULONG)(uint);
  chunk_list_base[0]=uint_ulong;
  #ifdef _32_
    if(uint!=uint_ulong){
      chunk_idx_max++;
      chunk_list_base[1]=(ULONG)(uint>>ULONG_BITS);
    }
  #endif
  return chunk_idx_max;
}

ULONG
biguint_increment(ULONG chunk_idx_max, ULONG *chunk_list_base){
/*
Increment a biguint.

In:

  chunk_idx_max is the index containing the MSB of *chunk_list_base.

  *chunk_list_base is list of chunks of a biguint, with sufficient space to store ((In:*chunk_list_base)+1).

Out:

  Returns the updated value of chunk_idx_max.

  *chunk_list_base has been increased by one.
*/
  ULONG chunk;
  ULONG chunk_idx;

  chunk_idx=0;
  do{
    chunk=0;
    if(chunk_idx<=chunk_idx_max){
      chunk=chunk_list_base[chunk_idx];
    }
    chunk++;
    chunk_list_base[chunk_idx]=chunk;
    chunk_idx++;
  }while(!chunk);
  chunk_idx--;
  chunk_idx_max=MAX(chunk_idx, chunk_idx_max);
  return chunk_idx_max;
}

u8
biguint_init(u32 build_break_count, u32 build_feature_count){
/*
Verify that the source code is sufficiently updated.

In:

  build_break_count is the caller's most recent knowledge of BIGUINT_BUILD_BREAK_COUNT, which will fail if the caller is unaware of all critical updates.

  build_feature_count is the caller's most recent knowledge of BIGUINT_BUILD_FEATURE_COUNT, which will fail if this library is not up to date with the caller's expectations.

Out:

  Returns one if (build_break_count!=BIGUINT_BUILD_BREAK_COUNT) or (build_feature_count>BIGUINT_BUILD_FEATURE_COUNT). Otherwise, returns zero.
*/
  u8 status;

  status=(u8)(build_break_count!=BIGUINT_BUILD_BREAK_COUNT);
  status=(u8)(status|(BIGUINT_BUILD_FEATURE_COUNT<build_feature_count));
  return status;
}

u8
biguint_is_power_of_2(ULONG chunk_idx_max, ULONG *chunk_list_base){
/*
Determine whether or not a biguint is a power of 2.

In:

  chunk_idx_max is the index containing the MSB of *chunk_list_base.

  *chunk_list_base is a list of chunks of a biguint.

Out:

 Returns one if *chunk_list_base is either zero or a power of 2, else zero.
*/
  ULONG chunk;
  ULONG chunk_idx;
  u8 status;

  chunk=chunk_list_base[chunk_idx_max];
  chunk&=chunk-1;
  chunk_idx=chunk_idx_max;
  while((chunk_idx--)&&!chunk){
    chunk=chunk_list_base[chunk_idx];
  }
  status=!chunk;
  return status;
}

u8
biguint_is_power_of_2_minus_1(ULONG chunk_idx_max, ULONG *chunk_list_base){
/*
Determine whether or not a biguint is one less than a power of 2.

In:

  chunk_idx_max is the index containing the MSB of *chunk_list_base.

  *chunk_list_base is a list of chunks of a biguint.

Out:

 Returns one if *chunk_list_base is one less than a power of 2, else zero.
*/
  ULONG chunk;
  ULONG chunk_idx;
  u8 status;

  chunk=chunk_list_base[chunk_idx_max];
  chunk&=chunk+1;
  chunk_idx=chunk_idx_max;
  while((chunk_idx--)&&!chunk){
    chunk=~chunk_list_base[chunk_idx];
  }
  status=!chunk;
  return status;
}

ULONG
biguint_logplex_chunk_idx_max_get(ULONG biguint_chunk_idx_max, ULONG *biguint_chunk_list_base, u64 logplex_bit_idx_min){
/*
Compute the minimum maximum (sic) ULONG index required to encode a biguint of known size to a logplex.

In:

  biguint_chunk_idx_max is the index containing the MSB of *biguint_chunk_list_base.

  *biguint_chunk_list_base is a list of chunks of a biguint, the maximum index of the logplex of which to compute.

  logplex_bit_idx_min is the base bit index at logplex_chunk_list_base at which to begin encoding.

Out:

  Returns the maximum index requried to store the logplex encoding of *biguint_chunk_list_base starting at bit index logplex_bit_idx_min.
*/
  u64 logplex_msb;
  ULONG logplex_chunk_idx_max;

  logplex_msb=biguint_logplex_msb_get(biguint_chunk_idx_max, biguint_chunk_list_base);
  logplex_msb+=logplex_bit_idx_min;
  logplex_chunk_idx_max=(ULONG)(logplex_msb>>ULONG_BITS_LOG2);
  return logplex_chunk_idx_max;
}

u8
biguint_logplex_decode(ULONG *biguint_chunk_idx_max_base, ULONG *biguint_chunk_list_base, u64 *logplex_bit_idx_min_base, ULONG logplex_chunk_idx_max, ULONG *logplex_chunk_list_base){
/*
Decode a logplex to a biguint.

In:

  *biguint_chunk_idx_max_base is the index containing the MSB of biguint_chunk_list_base. biguint_chunk_idx_max_get() can be used to find the minimum required value, but unlike most other Biguint functions, this one actually checks for adequate size, so speculative decoding is safe.

  biguint_chunk_list_base is the base of a list of undefined (ULONG)s.

  *logplex_bit_idx_min_base is the base bit index of the logplex at logplex_chunk_list_base at which to begin decoding, on [0, ((logplex_chunk_idx_max+1)<<ULONG_BITS_LOG2)-1].

  logplex_chunk_idx_max is the index containing the MSB of *logplex_chunk_list_base.

  *logplex_chunk_list_base is the base of a bitmap containing the logplex to decode.

Out:

  Returns one if the size of the biguint could not be computed, due to logplex_chunk_idx_max being too small, else zero.

  *biguint_chunk_idx_max_base is the index containing the MSB of *biguint_chunk_list_base.

  *biguint_chunk_list_base is the decoded biguint.

  *logplex_bit_idx_min_base has been incremented by the bit size of the decoded logplex.
*/
  ULONG biguint_chunk_idx_max;
  u64 logplex_bit_idx_min;
  u64 mantissa_bit_idx_min;
  u64 mantissa_msb;
  u8 status;

  logplex_bit_idx_min=*logplex_bit_idx_min_base;
  status=biguint_logplex_mantissa_get(&logplex_bit_idx_min, logplex_chunk_idx_max, logplex_chunk_list_base, &mantissa_bit_idx_min, &mantissa_msb);
  if(!status){
    status=1;
    biguint_chunk_idx_max=(ULONG)(mantissa_msb>>ULONG_BITS_LOG2);
    if(biguint_chunk_idx_max<=*biguint_chunk_idx_max_base){
      status=0;
      biguint_chunk_list_base[biguint_chunk_idx_max]=0;
      biguint_bitmap_copy_unsafe(mantissa_msb, 0, mantissa_bit_idx_min, biguint_chunk_list_base, logplex_chunk_list_base);
      *biguint_chunk_idx_max_base=biguint_chunk_idx_max;
      *logplex_bit_idx_min_base=logplex_bit_idx_min;
    }
  }
  return status;
}

u8
biguint_logplex_decode_u64(u64 *logplex_bit_idx_min_base, ULONG logplex_chunk_idx_max, ULONG *logplex_chunk_list_base, u64 *uint_base){
/*
Decode a u64 from its logplex encoding.

In:

  *logplex_bit_idx_min_base is the base bit index of the logplex at logplex_chunk_list_base at which to begin decoding, on [0, ((logplex_chunk_idx_max+1)<<ULONG_BITS_LOG2)-1].

  logplex_chunk_idx_max is the index containing the MSB of *logplex_chunk_list_base.

  *logplex_chunk_list_base is the base of a bitmap containing the logplex to decode.

  *uint_base is undefined.

Out:

  Returns zero on success, else one if either (1) the size of the biguint could not be computed, due to logplex_chunk_idx_max being too small; or (2) the decoded biguint would exceed U64_MAX.

  *logplex_bit_idx_min_base has been incremented by the bit size of the decoded logplex.

  *uint_base is the decoded u64.
*/
  ULONG chunk_idx_max;
  ULONG chunk_list_base[U64_SIZE>>ULONG_SIZE_LOG2];
  u64 logplex_bit_idx_min;
  u8 status;
  u64 uint;

  logplex_bit_idx_min=*logplex_bit_idx_min_base;
  chunk_idx_max=(U64_SIZE>>ULONG_SIZE_LOG2)-1;
  status=biguint_logplex_decode(&chunk_idx_max, chunk_list_base, &logplex_bit_idx_min, logplex_chunk_idx_max, logplex_chunk_list_base);
  if(!status){
    #ifdef _64_
      uint=chunk_list_base[0];
    #else
      memcpy(&uint, chunk_list_base, (size_t)(U64_SIZE));
      uint&=(1ULL<<(chunk_idx_max<<ULONG_BITS_LOG2)<<ULONG_BITS)-1;
    #endif
    *logplex_bit_idx_min_base=logplex_bit_idx_min;
    *uint_base=uint;
  }
  return status;
}

u8
biguint_logplex_encode(ULONG biguint_chunk_idx_max, ULONG *biguint_chunk_list_base, u64 *logplex_bit_idx_min_base, ULONG *logplex_chunk_idx_max_base, ULONG logplex_chunk_idx_max_max, ULONG *logplex_chunk_list_base){
/*
Encode a logplex from a biguint.

In:

  biguint_chunk_idx_max is the index containing the MSB of *biguint_chunk_list_base.

  *biguint_chunk_list_base is the biguint to encode.

  *logplex_bit_idx_min_base is the base bit index of the logplex at logplex_chunk_list_base at which to begin encoding, on [0, ((logplex_chunk_idx_max_max+1)<<ULONG_BITS_LOG2)-1].

  *logplex_chunk_idx_max_base is the index containing the MSB of *logplex_chunk_list_base.

  logplex_chunk_idx_max_max is the maximum writable index of *logplex_chunk_list_base. biguint_logplex_chunk_idx_max_get() can be used to find the minimum required value, but unlike most other Biguint functions, this one actually checks for adequate size, so speculation is safe.

  *logplex_chunk_list_base is undefined.

Out:

  Returns one if the biguint could not be encoded, due to logplex_chunk_idx_max_max being too small, else zero.

  *logplex_bit_idx_min_base has been incremented by the bit size of the encoded logplex.

  *logplex_chunk_idx_max_base is updated.

  *logplex_chunk_list_base contains the logplex corresponding to *biguint_chunk_list_base, starting at bit index *logplex_bit_idx_min_base.
*/
  u64 biguint_msb;
  u8 biguint_msb_msb;
  u8 bit;
  ULONG chunk_list_base[U64_SIZE>>ULONG_SIZE_LOG2];
  u64 logplex_bit_idx;
  ULONG logplex_chunk_idx_max;
  u64 logplex_msb;
  u64 logplex_msb_max;
  u8 status;

  logplex_msb=biguint_logplex_msb_get(biguint_chunk_idx_max, biguint_chunk_list_base);
  logplex_bit_idx=*logplex_bit_idx_min_base;
  logplex_msb_max=((u64)(logplex_chunk_idx_max_max)<<ULONG_BITS_LOG2)+ULONG_BIT_MAX-logplex_bit_idx;
  status=1;
  if(logplex_msb<=logplex_msb_max){
    biguint_msb=biguint_msb_get(biguint_chunk_idx_max, biguint_chunk_list_base);
    logplex_bit_idx+=logplex_msb-biguint_msb;
    biguint_bitmap_copy_unsafe(biguint_msb, logplex_bit_idx, 0, logplex_chunk_list_base, biguint_chunk_list_base);
    if(biguint_msb){
      biguint_msb--;
      BITSCAN_MSB64_SMALL_GET(biguint_msb_msb, biguint_msb);
      while(biguint_msb_msb){
        biguint_msb^=1ULL<<biguint_msb_msb;
        memcpy(chunk_list_base, &biguint_msb, (size_t)(U64_SIZE));
        logplex_bit_idx-=(u8)(biguint_msb_msb+1);
        biguint_bitmap_copy_unsafe(biguint_msb_msb, logplex_bit_idx, 0, logplex_chunk_list_base, chunk_list_base);
        biguint_msb=(u8)(biguint_msb_msb-1);
        BITSCAN_MSB8_SMALL_GET(biguint_msb_msb, biguint_msb);
      }
      logplex_bit_idx--;
      BIT_CLEAR(logplex_chunk_list_base, logplex_bit_idx);
      bit=(u8)(biguint_msb);
    }else{
      BIT_SET(logplex_chunk_list_base, logplex_bit_idx);
      bit=(u8)(biguint_chunk_list_base[0]);
    }
    logplex_bit_idx--;
    if(!bit){
      BIT_CLEAR(logplex_chunk_list_base, logplex_bit_idx);
    }else{
      BIT_SET(logplex_chunk_list_base, logplex_bit_idx);
    }
    logplex_bit_idx=*logplex_bit_idx_min_base+logplex_msb+1;
    *logplex_bit_idx_min_base=logplex_bit_idx;
    logplex_chunk_idx_max=(ULONG)(logplex_bit_idx>>ULONG_BITS_LOG2);
    if(*logplex_chunk_idx_max_base<logplex_chunk_idx_max){
      *logplex_chunk_idx_max_base=logplex_chunk_idx_max;
    }
    status=0;
  }
  return status;
}

u8
biguint_logplex_encode_u64(u64 *logplex_bit_idx_min_base, ULONG *logplex_chunk_idx_max_base, ULONG logplex_chunk_idx_max_max, ULONG *logplex_chunk_list_base, u64 uint){
/*
Encode a logplex from a u64.

In:

  *logplex_bit_idx_min_base is the base bit index of the logplex at logplex_chunk_list_base at which to begin encoding, on [0, ((logplex_chunk_idx_max_max+1)<<ULONG_BITS_LOG2)-1].

  logplex_chunk_idx_max_max is the maximum writable index of *logplex_chunk_list_base. biguint_logplex_chunk_idx_max_get() can be used to find the minimum required value, but unlike with most other Biguint functions, this one actually checks for adequate size, so speculation is safe.

  *logplex_chunk_idx_max_base is the index containing the MSB of *logplex_chunk_list_base.

  *logplex_chunk_list_base is undefined.

  uint is the u64 to encode.

Out:

  Returns one if the u64 could not be encoded, due to logplex_chunk_idx_max_max being too small, else zero.

  *logplex_bit_idx_min_base has been incremented by the bit size of the encoded logplex.

  *logplex_chunk_idx_max_base is updated.

  *logplex_chunk_list_base contains the logplex corresponding to uint, starting at bit index *logplex_bit_idx_min_base.
*/
  #ifdef _32_
    ULONG chunk_idx_max;
    ULONG chunk_list_base[U64_SIZE>>ULONG_SIZE_LOG2];
  #endif
  u8 status;

  #ifdef _64_
    status=biguint_logplex_encode(0, &uint, logplex_bit_idx_min_base, logplex_chunk_idx_max_base, logplex_chunk_idx_max_max, logplex_chunk_list_base);
  #else
    memcpy(chunk_list_base, &uint, (size_t)(U64_SIZE));
    chunk_idx_max=U64_BYTE_MAX>>ULONG_SIZE_LOG2;
    BIGUINT_CANONIZE(chunk_idx_max, chunk_list_base);
    status=biguint_logplex_encode(chunk_idx_max, chunk_list_base, logplex_bit_idx_min_base, logplex_chunk_idx_max_base, logplex_chunk_idx_max_max, logplex_chunk_list_base);
  #endif
  return status;
}

u8
biguint_logplex_mantissa_get(u64 *logplex_bit_idx_min_base, ULONG logplex_chunk_idx_max, ULONG *logplex_chunk_list_base, u64 *mantissa_bit_idx_min_base, u64 *mantissa_msb_base){
/*
Find the base bit index and MSB of the mantissa of a logplex. Note that MSB means "maximum zero-based bit index at which a one is present, or zero if no such index exists". It is not a bit, despite the name.

In:

  *logplex_bit_idx_min_base is the base bit index of the logplex at logplex_chunk_list_base at which to begin decoding, on [0, ((logplex_chunk_idx_max+1)<<ULONG_BITS_LOG2)-1].

  logplex_chunk_idx_max is the index containing the MSB of *logplex_chunk_list_base. Unlike most other Biguint functions, this one actually checks for adequate size during decoding, so speculation is safe.

  *logplex_chunk_list_base is the base of a bitmap containing the logplex whose mantissa to extract.

  *mantissa_bit_idx_min_base is undefined.

  *mantissa_msb_base is undefined.

Out:

  Returns one if the mantissa is not present in its entirity, due to logplex_chunk_idx_max being too small, else zero.

  *logplex_bit_idx_min_base has been incremented by the bit size of the logplex containing the mantissa.

  *mantissa_bit_idx_min_base is the minimum bit index of the mantissa.

  *mantissa_msb_base is the MSB of the mantissa, as measured relative to *mantissa_bit_idx_min_base. The bit at the MSB is one unless the logplex is zero.
*/
  ULONG chunk_list_base[U64_SIZE>>ULONG_SIZE_LOG2];
  u8 continue_status;
  u8 first_loop_status;
  u64 logplex_bit_idx;
  u64 logplex_bit_idx_max;
  u64 logplex_bit_idx_new;
  u64 msb;
  u8 msb_msb;
  u8 status;

  memset(chunk_list_base, 0, (size_t)(U64_SIZE));
  logplex_bit_idx=*logplex_bit_idx_min_base;
  logplex_bit_idx_max=((u64)(logplex_chunk_idx_max)<<ULONG_BITS_LOG2)+ULONG_BIT_MAX;
  first_loop_status=1;
  msb=0;
  msb_msb=1;
  status=1;
  do{
    continue_status=0;
    logplex_bit_idx_new=logplex_bit_idx+msb_msb;
    if(logplex_bit_idx_new<=logplex_bit_idx_max){
      biguint_bitmap_copy_unsafe(msb_msb, 0, logplex_bit_idx, chunk_list_base, logplex_chunk_list_base);
      memcpy(&msb, chunk_list_base, (size_t)(U64_SIZE));
      if(!(msb>>msb_msb)){
        logplex_bit_idx=logplex_bit_idx_new;
        logplex_bit_idx++;
        if(!first_loop_status){
          msb|=1ULL<<msb_msb;
        }
        msb++;
        continue_status=!!msb;
        if(U64_BIT_MAX<msb){
          continue_status=0;
          logplex_bit_idx_new=logplex_bit_idx+msb;
          if((logplex_bit_idx<logplex_bit_idx_new)&&(logplex_bit_idx_new<=logplex_bit_idx_max)){
            status=!BIT_GET(logplex_chunk_list_base, logplex_bit_idx_new);
          }
        }
        msb_msb=(u8)(msb);
      }else{
        msb=(u8)(msb_msb-first_loop_status);
        status=0;
      }
    }
    first_loop_status=0;
  }while(continue_status);
  if(!status){
    logplex_bit_idx_new++;
    *logplex_bit_idx_min_base=logplex_bit_idx_new;
    *mantissa_bit_idx_min_base=logplex_bit_idx;
    *mantissa_msb_base=msb;
  }
  return status;
}

u64
biguint_logplex_msb_get(ULONG chunk_idx_max, ULONG *chunk_list_base){
/*
Find the MSB of a logplex. Note that MSB means "maximum zero-based bit index at which a one is present, or zero if no such index exists". It is not a bit, despite the name.

In:

  chunk_idx_max is the index containing the MSB of *chunk_list_base.

  *chunk_list_base is the biguint, the MSB of the logplex of which to compute.

Out:

  Returns the MSB of the logplex of chunk_list_base. After encoding the logplex with biguint_logplex_encode(), this value will match (biguint_logplex_mantissa_get():Out:*mantissa_msb_max_base).
*/
  u64 logplex_msb;
  u8 msb;
  u8 msb_msb;

  logplex_msb=biguint_msb_get(chunk_idx_max, chunk_list_base);
  if(logplex_msb){
    logplex_msb--;
    BITSCAN_MSB64_SMALL_GET(msb_msb, logplex_msb);
    while(msb_msb){
      logplex_msb++;
      logplex_msb+=msb_msb;
      msb=(u8)(msb_msb-1);
      BITSCAN_MSB8_SMALL_GET(msb_msb, msb);
    }
    logplex_msb+=2;
  }
  logplex_msb++;
  return logplex_msb;
}

u64
biguint_lsb_get(ULONG chunk_idx_max, ULONG *chunk_list_base){
/*
Find the LSB of a biguint. Note that LSB means "minimum zero-based bit index at which a one is present, or zero if no such index exists". It is not a bit, despite the name.

In:

  chunk_idx_max is the index containing the MSB of *chunk_list_base.

  *chunk_list_base is the biguint the LSB of which to find.

Out:

  Returns the LSB of *chunk_list_base.
*/
  ULONG chunk;
  ULONG chunk_idx;
  u64 lsb;
  u8 lsb_u8;

  chunk_idx=0;
  while((!chunk_list_base[chunk_idx])&&(chunk_idx!=chunk_idx_max)){
    chunk_idx++;
  }
  chunk=chunk_list_base[chunk_idx];
  BITSCAN_LSB_ULONG_SMALL_GET(lsb_u8, chunk);
  lsb=((u64)(chunk_idx)<<ULONG_BITS_LOG2)+lsb_u8;
  return lsb;
}

ULONG *
biguint_malloc(ULONG ulong_idx_max){
/*
Allocate a list of undefined (ULONG)s then zero the first one in preparation for lazy expansion.

To maximize portability and debuggability, this is the only function in which Biguint calls malloc().

In:

  ulong_idx_max is the number of (ULONG)s to allocate, less one. Any value is allowed, but the following attempted allocations will fail: (1) any allocation of at least 2^64 bits, which could cause u64 bit counts to wrap and (2) any allocation whose size (in bytes) could not be represented in the presumptive machine word size (specified as _32_ or _64_, as defined at the compiler command line), which would cause list size computations (for memcpy(), fread(), etc.) to wrap. Certain optimizations can be had by exploiting the fact that the foregoing requested allocations would have failed, thereby obviating some limit checking overhead. Note that specifying _32_ means that no individual allocation can exceed U32_MAX bytes, but the sum of all allocation sizes could exceed that. Furthermore, the presumptive machine word size may be less than or equal to the actual machine word size, which is handy for cross-platform simulation on a single 64-bit platform.

Out:

  Returns NULL on failure, else the base of a zero followed by ulong_idx_max undefined items, thus implying the value zero with an initial maximum index of zero, which should eventually be freed via biguint_free(). In any event, at least one more undefined ULONG has been allocated than was requested, which can be used for optimization, for example by allowing carries to overflow safely.
*/
  ULONG *list_base;
  u64 list_bit_count;
  ULONG list_size;

  list_base=NULL;
  list_bit_count=((u64)(ulong_idx_max)+2)<<ULONG_BITS_LOG2;
  list_size=(ULONG)(list_bit_count>>U8_BITS_LOG2);
  if(ulong_idx_max<(list_size>>ULONG_SIZE_LOG2)){
    list_base=DEBUG_MALLOC_PARANOID(list_size);
    if(list_base){
      list_base[0]=0;
    }
  }
  return list_base;
}

u64
biguint_msb_get(ULONG chunk_idx_max, ULONG *chunk_list_base){
/*
Find the MSB of a biguint. Note that MSB means "maximum zero-based bit index at which a one is present, or zero if no such index exists". It is not a bit, despite the name.

In:

  chunk_idx_max is the index containing the MSB of *chunk_list_base.

  *chunk_list_base is the biguint the MSB of which to find.

Out:

  Returns the MSB of *chunk_list_base.
*/
  ULONG chunk;
  u64 msb;
  u8 msb_u8;

  chunk=chunk_list_base[chunk_idx_max];
  BITSCAN_MSB_ULONG_FLAT_GET(msb_u8, chunk);
  msb=((u64)(chunk_idx_max)<<ULONG_BITS_LOG2)+msb_u8;
  return msb;
}

ULONG
biguint_multiply_add_biguint(ULONG chunk_idx_max0, ULONG chunk_idx_max1, ULONG chunk_idx_max2, ULONG *chunk_list_base0, ULONG *chunk_list_base1, ULONG *chunk_list_base2){
/*
Multiply one biguint by another then add it to a third one.

In:

  chunk_idx_max0 is the index containing the MSB of *chunk_list_base0.

  chunk_idx_max1 is the index containing the MSB of *chunk_list_base1. For optimal performance, it should not exceed chunk_idx_max0.

  chunk_idx_max2 is the index containing the MSB of *chunk_list_base2.

  *chunk_list_base0 is a list of (chunk_idx_max0+1) chunks of a biguint factor. It must be writable for MAX((chunk_idx_max0+chunk_idx_max1+3), (chunk_idx_max2+2)) chunks.

  *chunk_list_base1 is a list of chunks of the other biguint factor.

  *chunk_list_base2 is a biguint to add to ((*chunk_list_base0)*(*chunk_list_base1)).

Out:

  Returns the updated value of chunk_idx_max0.

  *chunk_list_base0 is (((In:*chunk_list_base0)*(*chunk_list_base1))+(*chunk_list_base2)).
*/
  u8 carry;
  ULONG chunk_idx_max;
  ULONG chunk_idx0;
  ULONG chunk_idx1;
  ULONG chunk_idx2;
  ULONG chunk0;
  ULONG chunk1;
  ULONG chunk2;
  UDOUBLE product;

  chunk0=chunk_list_base0[chunk_idx_max0];
  chunk_idx1=0;
  product=0;
  do{
    chunk1=chunk_list_base1[chunk_idx1];
    product+=(UDOUBLE)(chunk0)*chunk1;
    chunk_list_base0[chunk_idx_max0+chunk_idx1]=(ULONG)(product);
    product>>=ULONG_BITS;
  }while((chunk_idx1++)!=chunk_idx_max1);
  chunk_idx0=chunk_idx_max0;
  chunk_idx_max0+=chunk_idx_max1+1;
  chunk_list_base0[chunk_idx_max0]=(ULONG)(product);
  while(chunk_idx0--){
    chunk0=chunk_list_base0[chunk_idx0];
    chunk1=chunk_list_base1[0];
    chunk_idx1=0;
    product=(UDOUBLE)(chunk0)*chunk1;
    chunk_list_base0[chunk_idx0]=(ULONG)(product);
    product>>=ULONG_BITS;
    while((chunk_idx1++)!=chunk_idx_max1){
      chunk1=chunk_list_base1[chunk_idx1];
      chunk_idx2=chunk_idx0+chunk_idx1;
      chunk2=chunk_list_base0[chunk_idx2];
      product+=((UDOUBLE)(chunk0)*chunk1)+chunk2;
      chunk_list_base0[chunk_idx2]=(ULONG)(product);
      product>>=ULONG_BITS;
    }
    chunk_idx2=chunk_idx0+chunk_idx_max1+1;
    while(product){
      product+=chunk_list_base0[chunk_idx2];
      chunk_list_base0[chunk_idx2]=(ULONG)(product);
      chunk_idx2++;
      product>>=ULONG_BITS;
    }
  }
  while(!chunk_list_base0[chunk_idx_max0]&&chunk_idx_max0){
    chunk_idx_max0--;
  }
  carry=0;
  chunk_idx_max=MIN(chunk_idx_max0, chunk_idx_max2);
  chunk_idx2=0;
  do{
    chunk0=chunk_list_base0[chunk_idx2];
    chunk2=chunk_list_base2[chunk_idx2];
    chunk0+=carry;
    carry=(u8)(carry&!chunk0);
    chunk0+=chunk2;
    carry=(u8)(carry|(chunk0<chunk2));
    chunk_list_base0[chunk_idx2]=chunk0;
  }while((chunk_idx2++)!=chunk_idx_max);
  while(chunk_idx_max<chunk_idx_max2){
    chunk_idx_max++;
    chunk2=chunk_list_base2[chunk_idx_max];
    chunk2+=carry;
    carry=(u8)(carry&!chunk2);
    chunk_list_base0[chunk_idx_max]=chunk2;
  }
  while(carry){
    chunk_idx_max++;
    chunk0=0;
    if(chunk_idx_max<=chunk_idx_max0){
      chunk0=chunk_list_base0[chunk_idx_max];
    }
    chunk0++;
    carry=!chunk0;
    chunk_list_base0[chunk_idx_max]=chunk0;
  }
  chunk_idx_max0=MAX(chunk_idx_max, chunk_idx_max0);
  return chunk_idx_max0;
}

ULONG
biguint_multiply_biguint(ULONG chunk_idx_max0, ULONG chunk_idx_max1, ULONG *chunk_list_base0, ULONG *chunk_list_base1){
/*
Multiply one biguint by another.

In:

  chunk_idx_max0 is the index containing the MSB of *chunk_list_base0.

  chunk_idx_max1 is the index containing the MSB of *chunk_list_base1. For optimal performance, it should not exceed chunk_idx_max0.

  *chunk_list_base0 is a list of (chunk_idx_max0+1) chunks of a biguint factor followed by (chunk_idx_max1+1) undefined chunks.

  *chunk_list_base1 is a list of chunks of the other biguint factor.

Out:

  Returns the updated value of chunk_idx_max0.

  *chunk_list_base0 is ((In:*chunk_list_base0)*(*chunk_list_base1)).
*/
  ULONG chunk_idx0;
  ULONG chunk_idx1;
  ULONG chunk_idx2;
  ULONG chunk0;
  ULONG chunk1;
  ULONG chunk2;
  UDOUBLE product;

  chunk0=chunk_list_base0[chunk_idx_max0];
  chunk_idx1=0;
  product=0;
  do{
    chunk1=chunk_list_base1[chunk_idx1];
    product+=(UDOUBLE)(chunk0)*chunk1;
    chunk_list_base0[chunk_idx_max0+chunk_idx1]=(ULONG)(product);
    product>>=ULONG_BITS;
  }while((chunk_idx1++)!=chunk_idx_max1);
  chunk_idx0=chunk_idx_max0;
  chunk_idx_max0+=chunk_idx_max1+1;
  chunk_list_base0[chunk_idx_max0]=(ULONG)(product);
  while(chunk_idx0--){
    chunk0=chunk_list_base0[chunk_idx0];
    chunk1=chunk_list_base1[0];
    chunk_idx1=0;
    product=(UDOUBLE)(chunk0)*chunk1;
    chunk_list_base0[chunk_idx0]=(ULONG)(product);
    product>>=ULONG_BITS;
    while((chunk_idx1++)!=chunk_idx_max1){
      chunk1=chunk_list_base1[chunk_idx1];
      chunk_idx2=chunk_idx0+chunk_idx1;
      chunk2=chunk_list_base0[chunk_idx2];
      product+=((UDOUBLE)(chunk0)*chunk1)+chunk2;
      chunk_list_base0[chunk_idx2]=(ULONG)(product);
      product>>=ULONG_BITS;
    }
    chunk_idx2=chunk_idx0+chunk_idx_max1+1;
    while(product){
      product+=chunk_list_base0[chunk_idx2];
      chunk_list_base0[chunk_idx2]=(ULONG)(product);
      chunk_idx2++;
      product>>=ULONG_BITS;
    }
  }
  while(!chunk_list_base0[chunk_idx_max0]&&chunk_idx_max0){
    chunk_idx_max0--;
  }
  return chunk_idx_max0;
}

ULONG
biguint_multiply_u64(ULONG chunk_idx_max, ULONG *chunk_list_base, u64 uint){
/*
Multiply a biguint by a u64.

In:

  chunk_idx_max is the index containing the MSB of *chunk_list_base.

  *chunk_list_base is a list of chunks of a biguint, the allocated hull of which is big enough to accomodate the product of itself and uint. Furthermore it must have been allocated by biguint_malloc(), which provides a certain write-past-end guarantee exploited by this function.

  uint is the factor by which to multiply *chunk_list_base.

Out:

  Returns the updated value of chunk_idx_max.

  *chunk_list_base has been multiplied by uint.
*/
  ULONG chunk_idx;
  #ifdef _32_
    u32 chunk_u32;
  #endif
  u64 product;
  #ifdef _64_
    UDOUBLE product_udouble;
  #else
    u64 product_lo;
    u32 product_u32;
    u32 uint_u32;
  #endif

  if(1<uint){
    #ifdef _64_
      product_udouble=0;
      for(chunk_idx=0; chunk_idx<=chunk_idx_max; chunk_idx++){
        product_udouble+=(chunk_list_base[chunk_idx]*(UDOUBLE)(uint));
        chunk_list_base[chunk_idx]=(ULONG)(product_udouble);
        product_udouble>>=ULONG_BITS;
      }
      product=(ULONG)(product_udouble);
    #else
      product=0;
/*
We can write past end in this case due to the guarantee from biguint_malloc().
*/
      uint_u32=(u32)(uint);
      if(uint==uint_u32){
        for(chunk_idx=0; chunk_idx<=chunk_idx_max; chunk_idx+=(U32_SIZE>>ULONG_SIZE_LOG2)){
          #ifdef _32_
            product+=chunk_list_base[chunk_idx]*(u64)(uint_u32);
          #else
            product+=((((u32)(chunk_list_base[chunk_idx+1])<<U16_BITS)|chunk_list_base[chunk_idx])*(u64)(uint_u32));
          #endif
          chunk_list_base[chunk_idx]=(ULONG)(product);
          product>>=U32_BITS;
        }
      }else{
        product_u32=0;
        for(chunk_idx=0; chunk_idx<=chunk_idx_max; chunk_idx+=(U32_SIZE>>ULONG_SIZE_LOG2)){
          chunk_u32=chunk_list_base[chunk_idx];
          product_lo=(u64)(chunk_u32)*(u32)(uint);
          product_u32+=(u32)(product_lo);
          chunk_list_base[chunk_idx]=product_u32;
          product+=(chunk_u32*(uint>>U32_BITS))+(product_lo>>U32_BITS)+(product_u32<(u32)(product_lo));
          product_u32=(u32)(product);
          product>>=U32_BITS;
        }
        product=(product<<U32_BITS)|product_u32;
      }
    #endif
    if(product){
      #ifdef _64_
        chunk_idx_max++;
        chunk_list_base[chunk_idx_max]=product;
      #else
        #ifdef _32_
          chunk_idx_max++;
          chunk_list_base[chunk_idx_max]=(ULONG)(product);
          product>>=ULONG_BITS;
          if(product){
            chunk_idx_max++;
            chunk_list_base[chunk_idx_max]=(ULONG)(product);
          }
        #else
          while(product){
            chunk_idx_max++;
            chunk_list_base[chunk_idx_max]=(ULONG)(product);
            product>>=ULONG_BITS;
          }
        #endif
      #endif
    }else{
      BIGUINT_CANONIZE(chunk_idx_max, chunk_list_base);
    }
  }else if(!uint){
    BIGUINT_SET_ZERO(chunk_idx_max, chunk_list_base);
  }
  return chunk_idx_max;
}

ULONG
biguint_pochhammer_multiply(ULONG chunk_idx_max, ULONG *chunk_list_base, ULONG factor_count, ULONG factor_min){
/*
Multiply a biguint by (((factor_min+factor_count-1)!)/((factor_min-1)!)), which is equivalent to multiplying by Pochhammer[factor_min, factor_count] at WolframAlpha.

In:

  chunk_idx_max is the index containing the MSB of *chunk_list_base.

  *chunk_list_base is a list of (ULONG)s long enough to accomodate itself multiplied by ((factor_min+factor_count-1!)/((factor_min-1)!)).

  factor_count is small enough such that (factor_min+factor_count-1) does not exceed ULONG_MAX. Zero will result in no changes.

  factor_min is nonzero.

Out:

  Returns the updated value of chunk_idx_max.

  *chunk_list_base has been multiplied by (((factor_min+factor_count-1)!)/((factor_min-1)!)).
*/
  ULONG factor;
  u64 factor_product;
  u64 factor_product_next;

  if(factor_count){
    factor=factor_count+factor_min-1;
    do{
      factor_product=factor;
      if(factor_min<factor){
        do{
          factor--;
          factor_product_next=factor*factor_product;
          if(factor_product!=(factor_product_next/factor)){
            factor++;
            break;
          }
          factor_product=factor_product_next;
        }while(factor_min<factor);
      }
      factor--;
      chunk_idx_max=biguint_multiply_u64(chunk_idx_max, chunk_list_base, factor_product);
    }while(factor_min<=factor);
  }
  return chunk_idx_max;
}

ULONG
biguint_power_of_2_get(u64 bit_idx_max, ULONG *chunk_list_base){
/*
Set a biguint to a power of 2.

In:

  bit_idx_max is MSB of the desired power of 2.

  *chunk_list_base is a list of chunks of a biguint sufficient to store (2^bit_idx_max).

Out:

  Returns the index containing the MSB of *chunk_list_base.
*/
  ULONG chunk_idx_max;
  ULONG list_size;

  chunk_idx_max=(ULONG)(bit_idx_max>>ULONG_BITS_LOG2);
  list_size=chunk_idx_max<<ULONG_SIZE_LOG2;
  memset(chunk_list_base, 0, (size_t)(list_size));
  chunk_list_base[chunk_idx_max]=(ULONG)(1ULL<<(bit_idx_max&ULONG_BIT_MAX));
  return chunk_idx_max;
}

ULONG
biguint_power_of_2_minus_1_get(u64 bit_count, ULONG *chunk_list_base){
/*
Set a biguint to one less than a power of 2.

In:

  bit_count is the number of ones in the desired value.

  *chunk_list_base is a list of chunks of a biguint sufficient to store ((2^bit_count)-1).

Out:

  Returns the index containing the MSB of *chunk_list_base.
*/
  ULONG chunk_idx_max;
  ULONG list_size;

  chunk_idx_max=(ULONG)(bit_count>>ULONG_BITS_LOG2);
  list_size=chunk_idx_max<<ULONG_SIZE_LOG2;
  memset(chunk_list_base, U8_MAX, (size_t)(list_size));
  chunk_list_base[chunk_idx_max]=(ULONG)((1ULL<<(bit_count&ULONG_BIT_MAX))-1);
  return chunk_idx_max;
}

ULONG
biguint_reverse(ULONG chunk_idx_max, ULONG *chunk_list_base){
/*
Reverse the bits of a biguint.

In:

  chunk_idx_max is the index containing the MSB of *chunk_list_base.

  *chunk_list_base is a list of chunks of the biguint to reverse.

Out:

  Returns the updated value of chunk_idx_max.

  *chunk_list_base is bitwise reversed, in the sense that bit zero is swapped with the former MSB, and so on.
*/
  u64 lsb;

  biguint_reverse_unsafe(chunk_idx_max, chunk_list_base);
  BIGUINT_CANONIZE(chunk_idx_max, chunk_list_base);
  lsb=biguint_lsb_get(chunk_idx_max, chunk_list_base);
  chunk_idx_max=biguint_shift_right(lsb, chunk_idx_max, chunk_list_base);
  return chunk_idx_max;
}

void
biguint_reverse_unsafe(ULONG chunk_idx_max, ULONG *chunk_list_base){
/*
Reverse the bits of the ULONG hull of a biguint, with no regard for maintaining canonical form or stopping at the MSB.

In:

  chunk_idx_max is the index containing the MSB of *chunk_list_base.

  *chunk_list_base is a list of chunks of the biguint to reverse.

Out:

  *chunk_list_base is bitwise reversed, in the sense that the high bit (not necessarily the high one) of the high ULONG has been swapped with bit zero, and so on.
*/
  ULONG chunk_idx;
  ULONG chunk_idx_max_half;
  ULONG chunk0;
  ULONG chunk1;

  chunk_idx=0;
  chunk_idx_max_half=chunk_idx_max>>1;
  do{
    chunk0=chunk_list_base[chunk_idx];
    chunk1=chunk_list_base[chunk_idx_max-chunk_idx];
    REVERSE_ULONG(chunk0);
    REVERSE_ULONG(chunk1);
    chunk_list_base[chunk_idx]=chunk1;
    chunk_list_base[chunk_idx_max-chunk_idx]=chunk0;
  }while((chunk_idx++)!=chunk_idx_max_half);
  return;
}

ULONG
biguint_shift_left(u64 bit_count, ULONG chunk_idx_max, ULONG *chunk_list_base){
/*
Shift a biguint to the left.

In:

  bit_count is the number of bits by which to shift.

  chunk_idx_max is the index containing the MSB of *chunk_list_base.

  *chunk_list_base0 is a list of chunks of a biguint. It must be writable for enough chunks to allow itself to be shifted left by bit_count.

Out:

  Returns the updated value of chunk_idx_max.

  *chunk_list_base has been shifted left by bit_count.
*/
  ULONG chunk0;
  ULONG chunk1;
  ULONG chunk_idx_max_new;
  ULONG chunk_idx0;
  ULONG chunk_idx1;
  u8 shift;
  u8 shift_negative;

  chunk_idx_max_new=0;
  if(chunk_idx_max||chunk_list_base[chunk_idx_max]){
    chunk_idx0=(ULONG)((bit_count>>ULONG_BITS_LOG2)+chunk_idx_max);
    chunk_idx_max_new=chunk_idx0;
    shift=bit_count&ULONG_BIT_MAX;
    if(shift){
      chunk0=chunk_list_base[chunk_idx_max];
      shift_negative=(u8)(ULONG_BITS-shift);
      chunk1=chunk0>>shift_negative;
      if(chunk1){
        chunk_idx_max_new++;
        chunk_list_base[chunk_idx_max_new]=chunk1;
      }
      for(chunk_idx1=chunk_idx_max-1; chunk_idx1<chunk_idx_max; chunk_idx1--){
        chunk1=chunk_list_base[chunk_idx1];
        chunk_list_base[chunk_idx0]=(chunk0<<shift)|(chunk1>>shift_negative);
        chunk_idx0--;
        chunk0=chunk1;
      }
      chunk_list_base[chunk_idx0]=chunk0<<shift;
      chunk_idx0--;
    }else{
      if(bit_count){
        for(chunk_idx1=chunk_idx_max; chunk_idx1<=chunk_idx_max; chunk_idx1--){
          chunk1=chunk_list_base[chunk_idx1];
          chunk_list_base[chunk_idx0]=chunk1;
          chunk_idx0--;
        }
      }else{
        chunk_idx0=ULONG_MAX;
      }
    }
    for(; chunk_idx0<chunk_idx_max_new; chunk_idx0--){
      chunk_list_base[chunk_idx0]=0;
    }
  }
  return chunk_idx_max_new;
}

ULONG
biguint_shift_right(u64 bit_count, ULONG chunk_idx_max, ULONG *chunk_list_base){
/*
Shift a biguint to the right.

In:

  bit_count is the number of bits by which to shift.

  chunk_idx_max is the index containing the MSB of *chunk_list_base.

  *chunk_list_base is a list of chunks of a biguint.

Out:

  Returns the updated value of chunk_idx_max.

  *chunk_list_base has been shifted right by bit_count.
*/
  ULONG chunk;
  ULONG chunk_idx;
  ULONG chunk_idx_max_new;
  ULONG chunk_idx_old;
  ULONG chunk_old;
  u8 shift;
  u8 shift_negative;

  chunk_idx_old=(ULONG)(bit_count>>ULONG_BITS_LOG2);
  chunk_idx_max_new=chunk_idx_max-chunk_idx_old;
  if(chunk_idx_old<=chunk_idx_max){
    shift=bit_count&ULONG_BIT_MAX;
    if(shift){
      chunk_old=chunk_list_base[chunk_idx_old];
      chunk_idx_old++;
      shift_negative=(u8)(ULONG_BITS-shift);
      for(chunk_idx=0; chunk_idx<chunk_idx_max_new; chunk_idx++){
        chunk=chunk_list_base[chunk_idx_old];
        chunk_idx_old++;
        chunk_old>>=shift;
        chunk_list_base[chunk_idx]=(chunk<<shift_negative)|chunk_old;
        chunk_old=chunk;
      }
      chunk_old>>=shift;
      chunk_list_base[chunk_idx_max_new]=chunk_old;
      if((!chunk_old)&&chunk_idx_max_new){
        chunk_idx_max_new--;
      }
    }else{
      if(chunk_idx_old){
        for(chunk_idx=0; chunk_idx<=chunk_idx_max_new; chunk_idx++){
          chunk_list_base[chunk_idx]=chunk_list_base[chunk_idx_old];
          chunk_idx_old++;
        }
      }
    }
  }else{
    BIGUINT_SET_ZERO(chunk_idx_max_new, chunk_list_base);
  }
  return chunk_idx_max_new;
}

ULONG
biguint_subtract_biguint(ULONG chunk_idx_max0, ULONG chunk_idx_max1, ULONG *chunk_list_base0, ULONG *chunk_list_base1){
/*
Subtract one biguint from another.

In:

  chunk_idx_max0 is the index containing the MSB of *chunk_list_base0.

  chunk_idx_max1 is the index containing the MSB of *chunk_list_base1, on [0, chunk_idx_max0].

  *chunk_list_base0 is a list of chunks of a biguint.

  *chunk_list_base1 is a list of chunks of a biguint to be subtracted from, and which must not exceed, *chunk_list_base0.

Out:

  Returns the updated value of chunk_idx_max0.

  *chunk_list_base0 is ((In:*chunk_list_base0)-(*chunk_list_base1)).
*/
  u8 borrow;
  ULONG chunk;
  ULONG chunk_idx;
  ULONG chunk_old;

  borrow=0;
  for(chunk_idx=0; chunk_idx<=chunk_idx_max1; chunk_idx++){
    chunk=chunk_list_base0[chunk_idx];
    if(borrow){
      borrow=!chunk;
      chunk--;
    }
    chunk_old=chunk;
    chunk-=chunk_list_base1[chunk_idx];
    borrow=(u8)(borrow|(chunk_old<chunk));
    chunk_list_base0[chunk_idx]=chunk;
  }
  chunk_idx=chunk_idx_max1+1;
  while(borrow){
    chunk=chunk_list_base0[chunk_idx];
    borrow=!chunk;
    chunk--;
    chunk_list_base0[chunk_idx]=chunk;
    chunk_idx++;
  }
  BIGUINT_CANONIZE(chunk_idx_max0, chunk_list_base0);
  return chunk_idx_max0;
}

ULONG
biguint_subtract_from_biguint(ULONG chunk_idx_max0, ULONG chunk_idx_max1, ULONG *chunk_list_base0, ULONG *chunk_list_base1){
/*
Subtract one biguint from another, then write the result to the former.

In:

  chunk_idx_max0 is the index containing the MSB of *chunk_list_base0.

  chunk_idx_max1 is the index containing the MSB of *chunk_list_base1, on [0, chunk_idx_max0].

  *chunk_list_base0 is a list of chunks of a biguint to be subtracted from, and which must not exceed, *chunk_list_base1. It must be large enough to store *chunk_list_base1.

  *chunk_list_base1 is a list of chunks of a biguint.

Out:

  Returns the updated value of chunk_idx_max0.

  *chunk_list_base0 is ((In:*chunk_list_base1)-(*chunk_list_base0)).
*/
  u8 borrow;
  ULONG chunk;
  ULONG chunk_idx;
  ULONG chunk_old;

  borrow=0;
  for(chunk_idx=0; chunk_idx<=chunk_idx_max0; chunk_idx++){
    chunk=chunk_list_base1[chunk_idx];
    if(borrow){
      borrow=!chunk;
      chunk--;
    }
    chunk_old=chunk;
    chunk-=chunk_list_base0[chunk_idx];
    borrow=(u8)(borrow|(chunk_old<chunk));
    chunk_list_base0[chunk_idx]=chunk;
  }
  for(chunk_idx=chunk_idx_max0+1; chunk_idx<=chunk_idx_max1; chunk_idx++){
    chunk=chunk_list_base1[chunk_idx];
    if(borrow){
      borrow=!chunk;
      chunk--;
    }
    chunk_list_base0[chunk_idx]=chunk;
  }
  chunk_idx_max0=chunk_idx_max1;
  BIGUINT_CANONIZE(chunk_idx_max0, chunk_list_base0);
  return chunk_idx_max0;
}

ULONG
biguint_subtract_u64(ULONG chunk_idx_max, ULONG *chunk_list_base, u64 uint){
/*
Subtract a u64 from a biguint.

In:

  chunk_idx_max is the index containing the MSB of *chunk_list_base.

  *chunk_list_base is a list of chunks of a biguint whose value is at least uint.

  uint is the value to subtract from *chunk_list_base.

Out:
  
  Returns the updated value of chunk_idx_max.

  *chunk_list_base has been decreased by uint.
*/
  ULONG chunk;
  ULONG chunk_idx;
  ULONG delta;

  chunk_idx=0;
  do{
    chunk=chunk_list_base[chunk_idx];
    delta=(ULONG)(chunk-uint);
    #ifdef _64_
      uint=(chunk<delta);
    #else
      uint>>=ULONG_BITS;
      uint+=(chunk<delta);
    #endif
    chunk_list_base[chunk_idx]=delta;
    chunk_idx++;
  }while(uint);
  #ifdef _64_
    if((!chunk_list_base[chunk_idx_max])&&chunk_idx_max){
      chunk_idx_max--;
    }
  #else
    BIGUINT_CANONIZE(chunk_idx_max, chunk_list_base);
  #endif
  return chunk_idx_max;
}

ULONG
biguint_subtract_u64_shifted(u64 bit_count, ULONG chunk_idx_max, ULONG *chunk_list_base, u64 uint){
/*
Shift a u64 to the left, then subtract it from a biguint.

In:

  bit_count is the number of bits by which to shift uint to the left before subtracting it.

  chunk_idx_max is the index containing the MSB of *chunk_list_base.

  *chunk_list_base is a list of chunks of a biguint whose value is at least (uint<<bit_count).

  uint is the value to shift left by bit_count and then  subtract from *chunk_list_base.

Out:
  
  Returns the updated value of chunk_idx_max.

  *chunk_list_base has been decreased by (uint<<bit_count).
*/
  ULONG chunk_idx;
  u64 chunk;
  u64 delta;
  u64 uint_partial;
  u8 uint_shift;
/*
Techically the caller could legally ask to subtract zero shifted by an amount which would overflow chunk_idx_max by an arbitrary amount. Prevent this from wreaking havoc.
*/
  if(uint){
    chunk_idx=(ULONG)((bit_count>>U64_BITS_LOG2)<<(U64_SIZE_LOG2-ULONG_SIZE_LOG2));
    chunk=chunk_list_base[chunk_idx];
    #ifdef _32_
      if(chunk_idx!=chunk_idx_max){
        chunk|=(u64)(chunk_list_base[chunk_idx+1])<<ULONG_BITS;
      }
    #endif
    uint_shift=(u8)(bit_count&U64_BIT_MAX);
    uint_partial=uint<<uint_shift;
    delta=chunk-uint_partial;
    chunk_list_base[chunk_idx]=(ULONG)(delta);
    #ifdef _32_
      if(chunk_idx!=chunk_idx_max){
        chunk_list_base[chunk_idx+1]=(ULONG)(delta>>ULONG_BITS);
      }
    #endif
    uint_partial=(chunk<delta);
    if(uint_shift){
      uint_partial+=uint>>(U64_BITS-uint_shift);
    }
    while(uint_partial&&((chunk_idx+(U64_BYTE_MAX>>ULONG_SIZE_LOG2))<=chunk_idx_max)){
      chunk_idx+=U64_SIZE>>ULONG_SIZE_LOG2;
      chunk=chunk_list_base[chunk_idx];
      #ifdef _32_
        if(chunk_idx!=chunk_idx_max){
          chunk|=(u64)(chunk_list_base[chunk_idx+1])<<ULONG_BITS;
        }
      #endif
      delta=chunk-uint_partial;
      uint_partial=(chunk<delta);
      chunk_list_base[chunk_idx]=(ULONG)(delta);
      #ifdef _32_
        if(chunk_idx!=chunk_idx_max){
          chunk_list_base[chunk_idx+1]=(ULONG)(delta>>ULONG_BITS);
        }
      #endif
    }
    BIGUINT_CANONIZE(chunk_idx_max, chunk_list_base);
  }
  return chunk_idx_max;
}

ULONG
biguint_swap_biguint(ULONG *chunk_idx_max0_base, ULONG chunk_idx_max1, ULONG *chunk_list_base0, ULONG *chunk_list_base1){
/*
Exchange a pair of biguints.

In:

  *chunk_idx_max0_base is the index containing the MSB of *chunk_list_base0.

  chunk_idx_max1 is the index containing the MSB of *chunk_list_base1.

  *chunk_list_base0 is a list of chunks of a biguint to swap with *chunk_list_base1, which must be writable for MAX(chunk_idx_max0, chunk_idx_max1) chunks.

  *chunk_list_base1 is a list of chunks of a biguint to swap with *chunk_list_base0, which must be writable for MAX(chunk_idx_max0, chunk_idx_max1) chunks.

Out:

  Returns (In:chunk_idx_max0) for convenience.

  *chunk_idx_max0_base=(In:chunk_idx_max1).

  *chunk_list_base0 is (In:*chunk_list_base1) for (In:chunk_idx_max0) chunks.

  *chunk_list_base1 is (In:*chunk_list_base0) for (In:chunk_idx_max1) chunks.
*/
  ULONG chunk0;
  ULONG chunk1;
  ULONG chunk_idx;
  ULONG chunk_idx_max;
  ULONG chunk_idx_max0;

  chunk_idx_max0=*chunk_idx_max0_base;
/*
Most swapped variables are of nearly if not exactly the same number of chunks, so it's not worth the overhead to optimize the swap for asymmetric sizes.
*/
  chunk_idx_max=MAX(chunk_idx_max0, chunk_idx_max1);
  for(chunk_idx=0; chunk_idx<=chunk_idx_max; chunk_idx++){
    chunk0=chunk_list_base0[chunk_idx];
    chunk1=chunk_list_base1[chunk_idx];
    chunk_list_base0[chunk_idx]=chunk1;
    chunk_list_base1[chunk_idx]=chunk0;
  }
  *chunk_idx_max0_base=chunk_idx_max1;
  return chunk_idx_max0;
}

u8
biguint_to_ascii_decimal(ULONG chunk_idx_max, ULONG *chunk_list_base, ULONG digit_idx_max_max, char *digit_list_base){
/*
Convert a biguint to a null-terminated big endian ASCII string of decimal digits.

In:

  chunk_idx_max is the index containing the MSB of *chunk_list_base.

  *chunk_list_base is a list of chunks of a biguint to convert to ASCII. It must be writable because it's transparently modified by this function.

  digit_idx_max_max is the maximum index at which the terminating null character may be stored.

  *digit_list_base is undefined.

Out:

  Returns one if the biguint could not be completely stored at *digit_list_base, else zero.

  *chunk_idx_max_base is the index containing the MSB of *chunk_list_base.

  *digit_list_base is the null-terminated ASCII string corresponding to *chunk_list_base. This is always a null character if the return value is one.
*/
  u8 digit;
  ULONG digit_idx;
  u64 remainder;
  u8 status;

  status=1;
  if(digit_idx_max_max){
    digit_idx=digit_idx_max_max;
    do{
      chunk_idx_max=biguint_divide_u64(chunk_idx_max, chunk_list_base, &remainder, 0xA);
      digit=(u8)(remainder+'0');
      digit_idx--;
      digit_list_base[digit_idx]=(char)(digit);
    }while((chunk_list_base[chunk_idx_max]||chunk_idx_max)&&digit_idx);
    status=(chunk_list_base[chunk_idx_max]||chunk_idx_max);
    if(digit_idx){
      digit_idx_max_max-=digit_idx;
      memmove(&digit_list_base[0], &digit_list_base[digit_idx], (size_t)(digit_idx_max_max));
    }
    digit_idx=digit_idx_max_max;
    do{
      chunk_idx_max=biguint_multiply_u64(chunk_idx_max, chunk_list_base, 0xA);
      digit_idx--;
      digit=(u8)(digit_list_base[digit_idx]-'0');
      chunk_idx_max=biguint_add_u64(chunk_idx_max, chunk_list_base, digit);
    }while(digit_idx);
    if(status){
      digit_idx_max_max=0;
    }
  }
  digit_list_base[digit_idx_max_max]=0;
  return status;
}

u8
biguint_to_ascii_hex(ULONG chunk_idx_max, ULONG *chunk_list_base, ULONG digit_idx_max_max, char *digit_list_base){
/*
Convert a biguint to a null-terminated big endian ASCII string of hex digits.

In:

  chunk_idx_max is the index containing the MSB of *chunk_list_base.

  *chunk_list_base is a list of chunks of a biguint to convert to ASCII. Unlike with biguint_to_ascii_decimal(), it can be readonly.

  digit_idx_max_max is the maximum index at which the terminating null character may be stored.

  *digit_list_base is undefined.

Out:

  Returns one if the biguint could not be completely stored at *digit_list_base, else zero.

  *chunk_idx_max_base is the index containing the MSB of *chunk_list_base.

  *digit_list_base is the null-terminated ASCII string corresponding to *chunk_list_base. This is always a null character if the return value is one.
*/
  ULONG chunk;
  ULONG chunk_idx;
  ULONG chunk_shifted;
  u8 digit;
  ULONG digit_idx;
  u8 shift;
  u8 status;

  status=1;
  if(digit_idx_max_max){
    chunk=0;
    chunk_idx=ULONG_MAX;
    digit_idx=digit_idx_max_max;
    shift=ULONG_BIT_MAX;
    do{
      shift=(u8)(shift+4);
      if(ULONG_BIT_MAX<shift){
        chunk_idx++;
        chunk=chunk_list_base[chunk_idx];
        shift=0;
      }
      chunk_shifted=chunk>>shift;
      digit=(u8)(chunk_shifted&0xF);
      chunk_shifted>>=4;
      if(digit<=9){
        digit=(u8)(digit+'0');
      }else{
        digit=(u8)(digit+'A'-0xA);
      }
      digit_idx--;
      digit_list_base[digit_idx]=(char)(digit);
    }while(((chunk_idx<chunk_idx_max)||chunk_shifted)&&digit_idx);
    if(digit_idx){
      digit_idx_max_max-=digit_idx;
      memmove(&digit_list_base[0], &digit_list_base[digit_idx], (size_t)(digit_idx_max_max));
    }
    if((chunk_idx_max<chunk_idx)||((chunk_idx==chunk_idx_max)&&!chunk_shifted)){
      status=0;
    }else if(status){
      digit_idx_max_max=0;
    }
  }
  digit_list_base[digit_idx_max_max]=0;
  return status;
}

u128
biguint_to_u128_saturate(ULONG chunk_idx_max, ULONG *chunk_list_base, u8 *overflow_status_base){
/*
Copy a biguint to a u128, saturating to (2^128-1).

In:

  chunk_idx_max is the index containing the MSB of *chunk_list_base.

  *chunk_list_base is a list of chunks of a biguint to convert to a u128.

Out:

  Returns the u128 closest to *chunk_list_base.

  *overflow_status_base is one if *chunk_list_base exceeded (but did not equal) (2^128-1), else unchanged.
*/
  ULONG list_size;
  u128 uint;

  if(chunk_idx_max<=(U128_BYTE_MAX>>ULONG_SIZE_LOG2)){
    list_size=(chunk_idx_max+1)<<ULONG_SIZE_LOG2;
    memset(&uint, 0, (size_t)(U128_SIZE));
    memcpy(&uint, chunk_list_base, (size_t)(list_size));
  }else{
    memset(&uint, U8_MAX, (size_t)(U128_SIZE));
    *overflow_status_base=1;
  }
  return uint;
}

u128
biguint_to_u128_wrap(ULONG chunk_idx_max, ULONG *chunk_list_base){
/*
Find the remainder of a biguint divided by (2^128).

In:

  chunk_idx_max is the index containing the MSB of *chunk_list_base.

  *chunk_list_base is a list of chunks of a biguint to convert to a u128.

Out:

  Returns the remainder of a *chunk_list_base divided by (2^128).
*/
  ULONG list_size;
  u128 uint;

  chunk_idx_max=MIN(chunk_idx_max, U128_BYTE_MAX>>ULONG_SIZE_LOG2);
  list_size=(chunk_idx_max+1)<<ULONG_SIZE_LOG2;
  memset(&uint, 0, (size_t)(U128_SIZE));
  memcpy(&uint, chunk_list_base, (size_t)(list_size));
  return uint;
}

u64
biguint_to_u64_saturate(ULONG chunk_idx_max, ULONG *chunk_list_base, u8 *overflow_status_base){
/*
Copy a biguint to a u64, saturating to U64_MAX.

In:

  chunk_idx_max is the index containing the MSB of *chunk_list_base.

  *chunk_list_base is a list of chunks of a biguint to convert to a u64.

Out:

  Returns the u64 closest to *chunk_list_base.

  *overflow_status_base is one if *chunk_list_base exceeded (but did not equal) U64_MAX, else unchanged.
*/
  #ifdef _32_
    ULONG list_size;
  #endif
  u64 uint;

  uint=U64_MAX;
  if(chunk_idx_max<=(U64_BYTE_MAX>>ULONG_SIZE_LOG2)){
    #ifdef _64_
      uint=chunk_list_base[0];
    #else
      list_size=(chunk_idx_max+1)<<ULONG_SIZE_LOG2;
      uint=0;
      memcpy(&uint, chunk_list_base, (size_t)(list_size));
    #endif
  }else{
    *overflow_status_base=1;
  }
  return uint;
}

u64
biguint_to_u64_wrap(ULONG chunk_idx_max, ULONG *chunk_list_base){
/*
Find the remainder of a biguint divided by (2^64).

In:

  chunk_idx_max is the index containing the MSB of *chunk_list_base.

  *chunk_list_base is a list of chunks of a biguint to convert to a u64.

Out:

  Returns the remainder of a *chunk_list_base divided by (2^64).
*/
  #ifdef _32_
    ULONG list_size;
  #endif
  u64 uint;

  #ifdef _64_
/*
Prevent the compiler from complaining that chunk_idx_max is an unused parameter.
*/
    chunk_idx_max=0;
    uint=chunk_list_base[chunk_idx_max];
  #else
    list_size=(MIN(chunk_idx_max, U64_BYTE_MAX>>ULONG_SIZE_LOG2)+1)<<ULONG_SIZE_LOG2;
    uint=0;
    memcpy(&uint, chunk_list_base, (size_t)(list_size));
  #endif
  return uint;
}

ULONG
biguint_truncate(u64 bit_idx_max, ULONG chunk_idx_max, ULONG *chunk_list_base){
/*
Truncate a biguint to at most a particular number of bits.

In:

  bit_idx_max is the desired maximum bit index of *chunk_list_base -- not necessarily its desired MSB.

  chunk_idx_max is the index containing the MSB of *chunk_list_base.

  *chunk_list_base is a list of chunks of a biguint to truncate.

Out:

  Returns the index containing the MSB of *chunk_list_base, which may be less than the index containing bit index bit_idx_max.

  *chunk_list_base has been modified so as to guarantee that its MSB is at most bit_idx_max.
*/
  ULONG chunk_idx_max_new;
  ULONG tail_mask;

  chunk_idx_max_new=(ULONG)(bit_idx_max>>ULONG_BITS_LOG2);
  if(chunk_idx_max_new<=chunk_idx_max){
    tail_mask=(ULONG)((1ULL<<1<<(u8)(bit_idx_max&ULONG_BIT_MAX))-1);
    tail_mask&=chunk_list_base[chunk_idx_max_new];
    chunk_list_base[chunk_idx_max_new]=tail_mask;
    chunk_idx_max=chunk_idx_max_new;
    BIGUINT_CANONIZE(chunk_idx_max, chunk_list_base);
  }
  return chunk_idx_max;
}
