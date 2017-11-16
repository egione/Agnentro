/*
MaskOps
Copyright 2017 Russell Leidich

This collection of files constitutes the MaskOps Library. (This is a
library in the abstact sense; it's not intended to compile to a ".lib"
file.)

The MaskOps Library is free software: you can redistribute it and/or
modify it under the terms of the GNU Limited General Public License as
published by the Free Software Foundation, version 3.

The MaskOps Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
Limited General Public License version 3 for more details.

You should have received a copy of the GNU Limited General Public
License version 3 along with the MaskOps Library (filename
"COPYING"). If not, see http://www.gnu.org/licenses/ .
*/
/*
Mask List Functions
*/
#include "flag.h"
#include "flag_maskops.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "constant.h"
#include "debug.h"
#include "debug_xtrn.h"
#include "maskops.h"
#include "maskops_xtrn.h"

ULONG *
maskops_bitmap_malloc(u64 msb){
/*
Allocate a bitmap.

To maximize portability and debuggability, this is one of the few functions in which MaskOps calls malloc().

In:

  msb is one less than the number of bits in the bitmap.

Out:

  Returns NULL on failure, else the base of (msb+1) undefined bits (with a ULONG-granular hull) which should eventually be freed via maskops_free().
*/
  ULONG *bitmap_base;
  #ifndef _64_
    ULONG bitmap_size;
  #endif
  u64 bitmap_size_u64;

  bitmap_base=NULL;
  bitmap_size_u64=((msb>>ULONG_BITS_LOG2)+1)<<ULONG_SIZE_LOG2;
  #ifdef _64_
    bitmap_base=DEBUG_MALLOC_PARANOID(bitmap_size_u64);
  #else
    bitmap_size=(ULONG)(bitmap_size_u64);
    if(bitmap_size==bitmap_size_u64){
      bitmap_base=DEBUG_MALLOC_PARANOID(bitmap_size);
    }
  #endif
  return bitmap_base;
}

void
maskops_deltafy(u8 channel_status, u8 direction_status, u8 granularity, ULONG mask_idx_max, u8 *mask_list_base){
/*
Perform reversible (un)differencing on a mask list in order to enhance signal detection or comparison. maskops_surroundify() will probably result in superior entropy contrast, but this function is provided because people tend to think in terms of deltas as opposed to surround functions.

In:

  channel_status is zero if each mask is a unified integer, else one if each mask consists of (granularity+1) parallel byte channels. For example, this value should be one for pixels consisting of red, green, and blue bytes.

  direction_status is one if and only if *mask_list_base should be converted to the first delta of its original state. If the latter is {A, B, C...}, then its first delta is {A, (B-A), (C-B)...}, ANDed with ((1U<<(granularity<<U8_BITS_LOG2)<<U8_BITS)-1). Otherwise zero to reverse the process.

  granularity is one less than the number of bytes per mask, on [0, U32_BYTE_MAX].

  mask_idx_max is one less than the number of masks of size (granularity+1) at *mask_list_base.

  *mask_list_base is the mask list.

Out:

  *mask_list_base is as described in In:direction_status.
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
    if(direction_status){
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

void
maskops_densify(u8 direction_status, u8 granularity, ULONG mask_idx_max, u8 *mask_list_base, u32 mask_min, u32 *remask_list_base){
/*
Minimize the mask span while preserving mask order. Thus zero will be the minmum mask actually used; and the return value will be the maximum. Or invert a previous such densification. See http://viXra.org/abs/1711.0277 .

In:

  *bitmap_base has passed through maskops_densify_bitmap_prepare() with the same granularity, mask_max, and mask_min.

  direction_status is maskops_densify_remask_prepare():In:direction_status.

  granularity is one less than the number of bytes per mask, on [0, U32_BYTE_MAX].

  mask_idx_max is one less than the number of masks of size (granularity+1) at *mask_list_base.

  *mask_list_base is the mask list.

  mask_min is maskops_densify_bitmap_prepare():In:mask_min, regardless of direction_status.

  *remask_list_base has passed through maskops_densify_remask_prepare(bitmap_base, mask_max, mask_min, remask_list_base).

Out:

  *mask_list_base contains masks which have been compressed so as to have the minimum possible mask span while preserving order. For example. {9, 8, 8, 1, 3, 1} would densify to {3, 2, 2, 0, 1, 0}. Note that densification occurs on an entire mask list all at once, so there will be subsets which contain sparse (nondense) mask utilization footprints.
*/
  u32 mask;
  u8 mask_u8;
  ULONG u8_idx;
  u32 u8_idx_delta;
  ULONG u8_idx_max;

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
    if(direction_status){
      mask=remask_list_base[mask-mask_min];
    }else{
      mask=remask_list_base[mask];
      mask+=mask_min;
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

void
maskops_densify_bitmap_prepare(ULONG *bitmap_base, u8 granularity, ULONG mask_idx_max, u8 *mask_list_base, u32 mask_max, u32 mask_min, u8 reset_status){
/*
Include all the masks in a list in an evolving mask utilization footprint, in preparation for maskops_densify_remask_prepare().

In:

  *bitmap_base is the nonnull return value of maskops_bitmap_malloc((u64)(mask_max-mask_min)) if reset_status is one, else unchanged from the previous return value.

  granularity is one less than the number of bytes per mask, on [0, U32_BYTE_MAX].

  mask_idx_max is one less than the number of masks of size (granularity+1) at *mask_list_base.

  *mask_list_base is the mask list.

  mask_max is at least the maximum mask present at mask_list_base. If reset_status is zero, then this must be the same value as that was passed into this function previously.

  mask_min is at most the minimum mask value present at mask_list_base. The same constraints apply with respect to reset_status as with mask_max.

  *reset_status must be one on the first call, or at any time in order to zero the mask utilization footprint; else zero to accrue an existing footprint by ORing it with the new one implied by *mask_list_base.

Out:

  *bitmap_base has ones corresponding to masks which are present at *mask_list_base, and has been ORed with all previous such outputs since reset_status was last set to one.
*/
  ULONG bitmap_size;
  ULONG chunk;
  ULONG chunk_idx;
  ULONG chunk_idx_max;
  ULONG chunk_mask;
  u32 mask;
  u8 mask_u8;
  ULONG u8_idx;
  u32 u8_idx_delta;
  ULONG u8_idx_max;

  chunk_idx_max=(mask_max-mask_min)>>ULONG_BITS_LOG2;
  if(reset_status){
    bitmap_size=(chunk_idx_max+1)<<ULONG_SIZE_LOG2;
/*
Clear the bitmap in preparation for setting bits corresponding to masks which are actually present.
*/
    memset(bitmap_base, 0, (size_t)(bitmap_size));
  }
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
    mask-=mask_min;
    chunk_idx=mask>>ULONG_BITS_LOG2;
    chunk=bitmap_base[chunk_idx];
/*
Most of the time we can improve performance by not writing back unnecessarily.
*/
    chunk_mask=(ULONG)(1)<<(mask&ULONG_BIT_MAX);
    if(!(chunk&chunk_mask)){
      chunk|=chunk_mask;
      bitmap_base[chunk_idx]=chunk;
    }
    u8_idx+=u8_idx_delta;
  }while(u8_idx<=u8_idx_max);
  return;
}

u32
maskops_densify_remask_prepare(ULONG *bitmap_base, u8 direction_status, u32 mask_max, u32 mask_min, u32 *remask_list_base){
/*
Convert a mask utilization footprint created by maskops_densify_bitmap_prepare() into a mask translation list, in preparation for maskops_densify() with direction_status set to one.

In:

  *bitmap_base has passed through maskops_densify_bitmap_prepare() with the same mask_max, and mask_min.

  direction_status is one if and only if *mask_list_base should be converted its densification on the next call to maskops_densify(). Otherwise zero to reverse the process.

  mask_max is maskops_densify_bitmap_prepare():In:mask_max, regardless of direction_status.

  mask_min is maskops_densify_bitmap_prepare():In:mask_min, regardless of direction_status.

  *remask_list_base is the nonnull return value of maskops_u32_list_malloc((ULONG)(mask_max-mask_min)).

Out:

  Returns mask_max, which will be the greatest mask at mask_list_base after maskops_densify() has been called with direction_status set to one. After doing so, the minimum mask is guaranteed to be zero. The output is irrelevant if the next call to maskops_densify() will have direction_status set to zero, because in that case the output mask range is guaranteed to be [mask_min, mask_max].

  *remask_list_base is a mask translation table for use by maskops_densify().
*/
  ULONG chunk;
  ULONG chunk_idx;
  ULONG chunk_idx_max;
  u32 mask;

  chunk_idx=0;
  chunk_idx_max=(mask_max-mask_min)>>ULONG_BITS_LOG2;
/*
Convert the mask utilization footprint into a mask translation list.
*/
  mask=0;
  mask_max=0;
  do{
    chunk=bitmap_base[chunk_idx];
    do{
      if(chunk&1){
        if(direction_status){
          remask_list_base[mask]=mask_max;
        }else{
          remask_list_base[mask_max]=mask;
        }
        mask_max++;
      }
      chunk>>=1;
      mask++;
    }while(chunk);
    mask+=ULONG_BIT_MAX;
    mask&=~ULONG_BIT_MAX;
  }while((chunk_idx++)!=chunk_idx_max);
  mask_max--;
  return mask_max;
}

void *
maskops_free(void *base){
/*
To maximize portability and debuggability, this is the only function in which MaskOps calls free().

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
maskops_init(u32 build_break_count, u32 build_feature_count){
/*
Verify that the source code is sufficiently updated.

In:

  build_break_count is the caller's most recent knowledge of MASKOPS_BUILD_BREAK_COUNT, which will fail if the caller is unaware of all critical updates.

  build_feature_count is the caller's most recent knowledge of MASKOPS_BUILD_FEATURE_COUNT, which will fail if this library is not up to date with the caller's expectations.

Out:

  Returns one if (build_break_count!=MASKOPS_BUILD_BREAK_COUNT) or (build_feature_count>MASKOPS_BUILD_FEATURE_COUNT). Otherwise, returns zero.
*/
  u8 status;

  status=(u8)(build_break_count!=MASKOPS_BUILD_BREAK_COUNT);
  status=(u8)(status|(MASKOPS_BUILD_FEATURE_COUNT<build_feature_count));
  return status;
}

u8 *
maskops_mask_list_malloc(u8 granularity, ULONG mask_idx_max){
/*
Allocate a list of masks as (u8)s regardless of their actual size.

To maximize portability and debuggability, this is one of the few functions in which MaskOps calls malloc().

In:

  granularity is one less than the number of bytes per mask, on [0, U32_BYTE_MAX].

  mask_idx_max is one less than the number of masks in the mask list.

Out:

  Returns NULL on failure, else the base of (((mask_idx_max+1)*(granularity+1))+granularity) undefined (u8)s, which should eventually be freed via maskops_free(). Note that this is enough to allow for the maximum possible number of remainder bytes in case overlap_status is zero. This is important because we might want to read a file which is not a multiple of (granularity+1), without causing a buffer overflow.
*/
  u8 *list_base;
  u64 list_bit_count;
  ULONG list_size;
  ULONG mask_count;
  u8 mask_size;

  list_base=NULL;
  mask_count=mask_idx_max+1;
  list_size=mask_count;
  mask_size=(u8)(granularity+1);
  list_size*=mask_size;
  if((list_size/mask_size)!=mask_count){
    list_size=0;
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

u32
maskops_max_min_get(u8 granularity, ULONG mask_idx_max, u8 *mask_list_base, u32 *mask_max_base, u8 sign_status){
/*
Find the minimum and maximum masks in a list of (un)signed masks.

In:

  granularity is one less than the number of bytes per mask, on [0, U32_BYTE_MAX].

  *mask_max_base is undefined.

  sign_status is zero if the masks are unsigned, else one.

Out:

  Returns the minimum signed or unsigned mask at mask_list_base, if sign_status is one or zero, respectively.

  *mask_max_base is the maximum signed or unsigned mask at mask_list_base, if sign_status is one or zero, respectively.
*/
  u32 mask;
  u32 mask_max;
  u32 mask_min;
  u32 mask_sign_mask;
  u8 mask_u8;
  ULONG u8_idx;
  ULONG u8_idx_delta;
  ULONG u8_idx_max;

  mask_max=0;
  mask_min=mask_max-1;
  mask_sign_mask=(u32)(sign_status)<<(granularity<<U8_BITS_LOG2)<<U8_BIT_MAX;
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
    mask^=mask_sign_mask;
    mask_max=MAX(mask, mask_max);
    mask_min=MIN(mask, mask_min);
    u8_idx+=u8_idx_delta;
  }while(u8_idx<=u8_idx_max);
  mask_max^=mask_sign_mask;
  mask_min^=mask_sign_mask;
  *mask_max_base=mask_max;
  return mask_min;
}

u32
maskops_negate(u8 granularity, ULONG mask_idx_max, u8 *mask_list_base, u32 *mask_max_base){
/*
Add the most negative number consistent with (granularity) to each mask.

In:

  granularity is one less than the number of bytes per mask, on [0, U32_BYTE_MAX].

  mask_idx_max is one less than the number of masks of size (granularity+1) at *mask_list_base.

  *mask_list_base is the mask list.

  *mask_max_base is undefined.

Out:

  Returns the minimum (unsigned) mask at mask_list_base.

  *mask_list_base is modified in a manner consistent with the summary above.

  *mask_max_base is the maximum (unsigned) mask at mask_list_base.
*/
  u32 mask;
  u32 mask_max;
  u32 mask_min;
  u32 mask_sign_mask;
  u8 mask_u8;
  ULONG u8_idx;
  u32 u8_idx_delta;
  ULONG u8_idx_max;

  mask_max=0;
  mask_min=~mask_max;
  mask_sign_mask=1U<<(granularity<<U8_BITS_LOG2)<<U8_BIT_MAX;
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
    mask^=mask_sign_mask;
    mask_max=MAX(mask, mask_max);
    mask_min=MIN(mask, mask_min);
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
  *mask_max_base=mask_max;
  return mask_min;
}

u32
maskops_surroundify(u8 channel_status, u8 direction_status, u8 granularity, ULONG mask_idx_max, u8 *mask_list_base, u32 mask_max, u32 mask_min){
/*
Perform reversible (un)surrounding on a mask list in order to enhance signal detection or comparison. See http://viXra.org/abs/1711.0277 .

In:

  channel_status is zero if each mask is a unified integer, else one if each mask consists of (granularity+1) parallel byte channels. For example, this value should be one for pixels consisting of red, green, and blue bytes.

  direction_status is one if and only if *mask_list_base should be converted to the first surround of its original state. If the latter is {A, B...}, then its first surround is {F((A-mask_min), (K-mask_min), (mask_max-mask_min)), F((B-mask_min), (A-mask_min), (mask_max-mask_min))...}, ANDed with ((1U<<(granularity<<U8_BITS_LOG2)<<U8_BITS)-1); where K is the integer ceiling of (((mask_max-mask_min)/2)+mask_min) and (S=F(X, Y, Z)) is equivalent to (SURROUND_U(8/16/24/32)(X, Y, Z, S)). (The terminating zero can be ignored because it adds no information; it's only present in order to provide for the sake of defined behavior and is guaranteed to be a valid surround code; it could perhaps be overwritten with (mask_max-mask_min), which is also a valid surround code and would facilitate invertibility in the event that mask_min is known.) Otherwise zero to reverse the process.

  granularity is one less than the number of bytes per mask, on [0, U32_BYTE_MAX].

  mask_idx_max is one less than the number of masks of size (granularity+1) at *mask_list_base.

  *mask_list_base is the mask list.

  mask_max is at least the maximum mask present at mask_list_base. The tighter this value, the more compressible the resulting list of surround codes will be. On the other hand, tightening beyond what granularity already implies would then implicitly require additional information overhead for the sake of achieving said compression, for which the caller must then account in estimates of total entropy. If channel_status is one, then this value must not exceed U8_MAX. If direction_status is zero, then this must be the same value as that was passed into this function in order to surroundify *mask_list_base; otherwise the operation will not invert correctly.

  mask_min is at most the minimum mask value present at mask_list_base. The same constraints apply with respect to tightness, channel_status, and direction_status, as with mask_max.

Out:

  Returns zero if direction_status is zero, else the maximum mask at *mask_list_base (which will not exceed U8_MAX if channel_status is one). The minimum mask is likely to be zero, but is not guaranteed to be.

  *mask_list_base is as described in In:direction_status.
*/
  u8 channel_shift;
  u32 mask;
  u32 mask_max_new;
  u8 mask_max_u8;
  u32 mask_new;
  u8 mask_new_u8;
  u32 mask_old;
  u8 mask_old_u8;
  u32 mask_out;
  u8 mask_out_u8;
  u8 mask_u8;
  ULONG u8_idx;
  u32 u8_idx_delta;
  ULONG u8_idx_max;

  mask_max-=mask_min;
  if(channel_status){
    mask_max=(u8)(mask_max);
  }
  mask_old=(mask_max>>1)+(mask_max&1);
  if(channel_status){
    mask_old|=(mask_old<<U8_BITS);
    mask_old|=(mask_old<<U16_BITS);
  }
  mask_max_new=0;
  mask_new=0;
  mask_out=0;
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
    if(direction_status){
      if(!channel_status){
        mask-=mask_min;
        mask_new=mask;
        SURROUND_U32(mask, mask_max, mask_old, mask_out);
        mask_max_new=MAX(mask_max_new, mask_out);
      }else{
        mask_max_u8=(u8)(mask_max);
        mask_new=0;
        mask_out=0;
        for(channel_shift=(u8)(granularity<<U8_BITS_LOG2); channel_shift<=U32_BIT_MAX; channel_shift=(u8)(channel_shift-U8_BITS)){
          mask_old_u8=(u8)(mask_old>>channel_shift);
          mask_u8=(u8)((mask>>channel_shift)-mask_min);
          mask_new|=(u32)(mask_u8)<<channel_shift;
          SURROUND_U8(mask_u8, mask_max_u8, mask_old_u8, mask_out_u8);
          mask_max_new=MAX(mask_max_new, mask_out_u8);
          mask_out|=(u32)(mask_out_u8)<<channel_shift;
        }
      }
    }else{
      if(!channel_status){
        UNSURROUND_U32(mask_max, mask_old, mask, mask_new);
        mask_out=mask_min+mask_new;
      }else{
        mask_max_u8=(u8)(mask_max);
        mask_new=0;
        mask_out=0;
        for(channel_shift=(u8)(granularity<<U8_BITS_LOG2); channel_shift<=U32_BIT_MAX; channel_shift=(u8)(channel_shift-U8_BITS)){
          mask_old_u8=(u8)(mask_old>>channel_shift);
          mask_u8=(u8)(mask>>channel_shift);
          UNSURROUND_U8(mask_max_u8, mask_old_u8, mask_u8, mask_new_u8);
          mask_new|=(u32)(mask_new_u8)<<channel_shift;
          mask_out|=(u32)((u8)(mask_min+mask_new_u8))<<channel_shift;
        }
      }
    }
    mask_old=mask_new;
    mask_u8=(u8)(mask_out);
    mask_list_base[u8_idx]=mask_u8;
    if(granularity){
      mask_u8=(u8)(mask_out>>U8_BITS);
      mask_list_base[u8_idx+U16_BYTE_MAX]=mask_u8;
      if(U16_BYTE_MAX<granularity){
        mask_u8=(u8)(mask_out>>U16_BITS);
        mask_list_base[u8_idx+U24_BYTE_MAX]=mask_u8;
        if(U24_BYTE_MAX<granularity){
          mask_u8=(u8)(mask_out>>U24_BITS);
          mask_list_base[u8_idx+U32_BYTE_MAX]=mask_u8;
        }
      }
    }
    u8_idx+=u8_idx_delta;
  }while(u8_idx<=u8_idx_max);
  return mask_max_new;
}

u32 *
maskops_u32_list_malloc(ULONG u32_idx_max){
/*
Allocate a list of undefined (u32)s.

To maximize portability and debuggability, this is one of the few functions in which MaskOps calls malloc().

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
maskops_u32_list_zero(ULONG u32_idx_max, u32 *u32_list_base){
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

u8
maskops_unsign(u8 granularity, ULONG mask_idx_max, u8 *mask_list_base, u32 *mask_max_base, u32 *mask_min_base){
/*
Add the most negative number consistent with (granularity) to each mask, if and only if doing so would reduce the (unsigned) mask span.

In:

  granularity is one less than the number of bytes per mask, on [0, U32_BYTE_MAX].

  mask_idx_max is one less than the number of masks of size (granularity+1) at *mask_list_base.

  *mask_list_base is the mask list.

  *mask_max_base is undefined.

  *mask_min_base is undefined.

Out:

  Returns one if the masks were signed and therefore *mask_list_base was (reversibly) negated, else zero.

  *mask_list_base is modified (or not) in a manner consistent with the summary above.

  *mask_max_base is the maximum (unsigned) mask at mask_list_base.

  *mask_min_base is the minimum (unsigned) mask at mask_list_base.
*/
  u32 mask;
  u32 mask_max_signed;
  u32 mask_max_unsigned;
  u32 mask_min_signed;
  u32 mask_min_unsigned;
  u32 mask_sign_mask;
  u8 mask_u8;
  u8 status;
  ULONG u8_idx;
  ULONG u8_idx_delta;
  ULONG u8_idx_max;

  mask_max_signed=0;
  mask_max_unsigned=0;
  mask_min_signed=~mask_max_signed;
  mask_min_unsigned=mask_min_signed;
  mask_sign_mask=1U<<(granularity<<U8_BITS_LOG2)<<U8_BIT_MAX;
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
    mask_max_unsigned=MAX(mask, mask_max_unsigned);
    mask_min_unsigned=MIN(mask, mask_min_unsigned);
    mask^=mask_sign_mask;
    mask_max_signed=MAX(mask, mask_max_signed);
    mask_min_signed=MIN(mask, mask_min_signed);
    u8_idx+=u8_idx_delta;
  }while(u8_idx<=u8_idx_max);
  status=0;
  if((mask_max_signed-mask_min_signed)<(mask_max_unsigned-mask_min_unsigned)){
    mask_min_unsigned=maskops_negate(granularity, mask_idx_max, mask_list_base, &mask_max_unsigned);
    status=1;
  }
  *mask_max_base=mask_max_unsigned;
  *mask_min_base=mask_min_unsigned;
  return status;
}
