/*
Zorb
Copyright 2017 Russell Leidich

This collection of files constitutes the Zorb Library. (This is a
library in the abstact sense; it's not intended to compile to a ".lib"
file.)

The Zorb Library is free software: you can redistribute it and/or
modify it under the terms of the GNU Limited General Public License as
published by the Free Software Foundation, version 3.

The Zorb Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
Limited General Public License version 3 for more details.

You should have received a copy of the GNU Limited General Public
License version 3 along with the Zorb Library (filename
"COPYING"). If not, see http://www.gnu.org/licenses/ .
*/
/*
Frequency List Absorption Functions
*/
#include "flag.h"
#include "flag_fracterval_u128.h"
#include "flag_fracterval_u64.h"
#include "flag_biguint.h"
#include "flag_loggamma.h"
#include "flag_poissocache.h"
#include "flag_agnentroprox.h"
#include "flag_zorb.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "constant.h"
#include "debug.h"
#include "debug_xtrn.h"
#include "fracterval_u128.h"
#include "fracterval_u64.h"
#include "loggamma.h"
#include "poissocache.h"
#include "agnentroprox.h"
#include "agnentroprox_xtrn.h"
#include "lmd2.h"
#include "zorb.h"
#include "zorb_xtrn.h"

u8
zorb_check(u32 mask_max, zorb_t *zorb_base, ULONG zorb_size){
/*
Verify the size and integrity of a Zorb (ZRB) file.

In:

  mask_max is the the expected value of zorb_base->mask_max.

  zorb_base is the return value of zorb_init().

  zorb_size is the size of the Zorb file.

Out:

  Returns one on failure, else zero.
*/
  u64 lmd2;
  u8 status;
  u64 zorb_size_u64;

  status=1;
  if(zorb_base->signature==ZORB_SIGNATURE){
    if(mask_max==zorb_base->mask_max){
      if(!zorb_base->zero){
        zorb_size_u64=(u64)(sizeof(zorb_t))+(((u64)(mask_max)+1)<<U64_SIZE_LOG2);
        if(zorb_size==zorb_size_u64){
          status=zorb_lmd2_get(zorb_base, &lmd2);
          status=(u8)(status|(lmd2!=zorb_base->lmd2_following));
        }
      }
    }     
  }
  return status;
}

void
zorb_finalize(agnentroprox_t *agnentroprox_base, zorb_t *zorb_base){
/*
Prepare a Zorb file header for writing to storage.

In:

  agnentroprox_base is the return value of agnentroprox_init(), which has successfully passed through zorb_freq_list_import().

  zorb_base is the return value of zorb_init().

Out:

  *zorb_base is ready for writing to storage.
*/
  u64 lmd2;
  ULONG mask_idx_max;
  u32 mask_max;

  mask_idx_max=agnentroprox_base->mask_count0-1;
  mask_max=agnentroprox_base->mask_max;
  zorb_base->mask_idx_max=mask_idx_max;
  zorb_base->mask_max=mask_max;
  zorb_base->signature=ZORB_SIGNATURE;
  zorb_base->zero=0;
  zorb_lmd2_get(zorb_base, &lmd2);
  zorb_base->lmd2_following=lmd2;
  return;
}

void *
zorb_free(void *base){
/*
To maximize portability and debuggability, this is the only function in which Zorb calls free().

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
zorb_freq_list_add(agnentroprox_t *agnentroprox_base){
/*
After verifying that it's safe to do so, add Agnentroprox's frequency list one to its frequency list zero.

In:

  agnentroprox_base is the return value of agnentroprox_init(), having passed through zorb_freq_list_export() and zorb_mask_list_load().

Out:

  agnentroprox_base has been referenced for the sake of augmenting its frequency list zero as described in the summary.
*/
  ULONG mask_idx_max;
  u8 status;

  mask_idx_max=agnentroprox_base->mask_count1;
  status=1;
  if(mask_idx_max){
    mask_idx_max--;
    status=agnentroprox_capacity_check(agnentroprox_base, 0, mask_idx_max, 1);
    if(!status){
      agnentroprox_freq_list_add(agnentroprox_base, 1);
    }
  }
  return status;
}

u8
zorb_freq_list_export(agnentroprox_t *agnentroprox_base, zorb_t *zorb_base){
/*
After verifying that it's safe to do so, copy the frequency list from a Zorb file to an Agnentroprox instance.

In:

  agnentroprox_base is the return value of agnentroprox_init(), called with the same mask_max as zorb_init().

  zorb_base is the return value of zorb_init().

Out:

  agnentroprox_base has been referenced for the sake of copying its frequency list zero from the frequency list in the Zorb file.
*/
  ULONG *freq_list_base;
  u32 mask;
  ULONG mask_idx_max;
  u32 mask_max;
  u8 status;

  mask_max=agnentroprox_base->mask_max;
  status=1;
  if(mask_max==zorb_base->mask_max){
    status=zorb_mask_idx_max_get(&mask_idx_max, zorb_base);
    if(!status){
      status=agnentroprox_capacity_check(agnentroprox_base, 0, mask_idx_max, 0);
      if(!status){
        freq_list_base=agnentroprox_base->freq_list_base0;
        mask=0;
        do{
          freq_list_base[mask]=(ULONG)(zorb_base->freq_list[mask]);
        }while((mask++)!=mask_max);
        agnentroprox_base->mask_count0=mask_idx_max+1;
      }
    }
  }
  return status;
}

u8
zorb_freq_list_import(agnentroprox_t *agnentroprox_base, zorb_t *zorb_base){
/*
Copy frequency list zero from an Agnentroprox instance to a Zorb file.

In:

  agnentroprox_base is the return value of agnentroprox_init(), called with the same mask_max as zorb_init().

  zorb_base is the return value of zorb_init().

Out:

  agnentroprox_base has been referenced for the sake of copying its frequency list zero to the frequency list in the Zorb file.
*/
  ULONG *freq_list_base;
  u32 mask;
  u32 mask_max;
  u8 status;

  mask_max=agnentroprox_base->mask_max;
  status=1;
  if(mask_max==zorb_base->mask_max){
    status=0;
    freq_list_base=agnentroprox_base->freq_list_base0;
    mask=0;
    do{
      zorb_base->freq_list[mask]=freq_list_base[mask];
    }while((mask++)!=mask_max);
  }
  return status;
}

u8
zorb_freq_list_subtract(agnentroprox_t *agnentroprox_base){
/*
After verifying that it's safe to do so, subtract Agnentroprox's frequency list one from its frequency list zero.

In:

  agnentroprox_base is the return value of agnentroprox_init(), having passed through zorb_freq_list_export() and zorb_mask_list_load().

Out:

  agnentroprox_base has been referenced for the sake of reducing its frequency list zero as described in the summary.
*/
  ULONG mask_idx_max;
  u8 status;

  mask_idx_max=agnentroprox_base->mask_count1;
  status=1;
  if(mask_idx_max){
    mask_idx_max--;
    status=agnentroprox_capacity_check(agnentroprox_base, 0, mask_idx_max, 2);
    if(!status){
      agnentroprox_freq_list_subtract(agnentroprox_base, 1);
    }
  }
  return status;
}

zorb_t *
zorb_init(u32 build_break_count, u32 build_feature_count, u32 mask_max){
/*
Verify that the source code is sufficiently updated and initialize private storage.

In:

  build_break_count is the caller's most recent knowledge of ZORB_BUILD_BREAK_COUNT, which will fail if the caller is unaware of all critical updates.

  build_feature_count is the caller's most recent knowledge of ZORB_BUILD_FEATURE_COUNT, which will fail if this library is not up to date with the caller's expectations.

  mask_max_max is the maximum possible mask the frequency of which might need to be stored in the returned zorb_t.

Out:

  Returns NULL if (build_break_count!=ZORB_BUILD_BREAK_COUNT); (build_feature_count>ZORB_BUILD_FEATURE_COUNT); or there is insufficient memory. Else, returns the base of an undefined zorb_t which can then be passed to zorb_reset() or zorb_freq_list_import(), and/or read from storage. It must be freed with zorb_free().
*/
  u8 status;
  zorb_t *zorb_base;
  ULONG zorb_size;

  zorb_base=NULL;
  status=(u8)(build_break_count!=ZORB_BUILD_BREAK_COUNT);
  status=(u8)(status|(ZORB_BUILD_FEATURE_COUNT<build_feature_count));
  if(!status){
    status=zorb_size_ulong_get(mask_max, &zorb_size);
    if(!status){
      zorb_base=(zorb_t *)(DEBUG_MALLOC_PARANOID(zorb_size));
    }
  }
  return zorb_base;
}

u8
zorb_lmd2_get(zorb_t *zorb_base, u64 *lmd2_base){
/*
Get the LMD2 error detection code of a zorb_t, and return it along with a status indicating whether the mask count implied by its header is consistent with the sum of its constituent frequencies.

In:

  *zorb_base is the zorb_t for which to compute (but not overwrite) the value of lmd2_following (which is not the LMD2 of the entire structure).

  *lmd2_base is undefined.

Out:

  Returns zero if zorb_base->mask_idx_max is consistent with the sum of all (zorb_base->mask_max+1) items at zorb_base->freq_list, else one.

  *lmd2_base is the correct value of zorb_base->lmd2_following.
*/
  u64 freq;
  u64 lmd2;
  u32 lmd2_c0;
  u64 lmd2_iterand;
  u32 lmd2_x0;
  u32 mask;
  u64 mask_count;
  u64 mask_idx_max;
  u32 mask_max;
  u8 status;
  u32 uint;

  LMD_SEED_INIT(LMD2_C0, lmd2_c0, LMD2_X0, lmd2_x0)
  LMD_ACCUMULATOR_INIT(lmd2)
  mask_idx_max=zorb_base->mask_idx_max;
  uint=(u32)(mask_idx_max);
  LMD_ITERATE_NO_ZERO_CHECK(LMD2_A, lmd2_c0, lmd2_x0, lmd2_iterand)
  LMD_ACCUMULATE(uint, lmd2_x0, lmd2)
  uint=(u32)(mask_idx_max>>U32_BITS);
  LMD_ITERATE_NO_ZERO_CHECK(LMD2_A, lmd2_c0, lmd2_x0, lmd2_iterand)
  LMD_ACCUMULATE(uint, lmd2_x0, lmd2)
  mask_max=zorb_base->mask_max;
  LMD_ITERATE_NO_ZERO_CHECK(LMD2_A, lmd2_c0, lmd2_x0, lmd2_iterand)
  LMD_ACCUMULATE(mask_max, lmd2_x0, lmd2)
  uint=zorb_base->zero;
  LMD_ITERATE_NO_ZERO_CHECK(LMD2_A, lmd2_c0, lmd2_x0, lmd2_iterand)
  LMD_ACCUMULATE(uint, lmd2_x0, lmd2)
  mask=0;
  mask_count=mask_idx_max+1;
  status=0;
  do{
    freq=zorb_base->freq_list[mask];
    status=(u8)(status|(mask_count<freq));
    mask_count-=freq;
    uint=(u32)(freq);
    LMD_ITERATE_NO_ZERO_CHECK(LMD2_A, lmd2_c0, lmd2_x0, lmd2_iterand)
    LMD_ACCUMULATE(uint, lmd2_x0, lmd2)
    uint=(u32)(freq>>U32_BITS);
    LMD_ITERATE_NO_ZERO_CHECK(LMD2_A, lmd2_c0, lmd2_x0, lmd2_iterand)
    LMD_ACCUMULATE(uint, lmd2_x0, lmd2)
  }while((mask++)!=mask_max);
  status=(u8)(status|!!mask_count);
  LMD_FINALIZE(LMD2_A, lmd2_c0, lmd2_x0, lmd2_iterand, lmd2)
  *lmd2_base=lmd2;
  return status;
}

u8
zorb_mask_idx_max_get(ULONG *mask_idx_max_base, zorb_t *zorb_base){
/*
If possible, return zorb_base->mask_idx_max as a ULONG, which is one less than the total of all items in its frequency list.

In:

  *mask_idx_max_base is undefined.

  zorb_base is the return value of zorb_init(), generally having been overwritten by a Zorb file read from storage.

Out:

  Returns one on failure, else zero.

  *mask_idx_max_base is zorb_base->mask_idx_max.
*/
  #ifndef _64_
    ULONG mask_idx_max;
  #endif
  u64 mask_idx_max_u64;
  u8 status;

  mask_idx_max_u64=zorb_base->mask_idx_max;
  #ifdef _64_
    status=0;
    *mask_idx_max_base=mask_idx_max_u64;
  #else
    mask_idx_max=(ULONG)(mask_idx_max_u64);
    status=1;
    if((mask_idx_max==mask_idx_max_u64)&&(mask_idx_max!=ULONG_MAX)){
      status=0;
      *mask_idx_max_base=mask_idx_max;
    }
  #endif
  return status;
}

u8
zorb_mask_list_load(agnentroprox_t *agnentroprox_base, u8 *mask_list_base, ULONG mask_list_size){
/*
After checking for sufficient capacity, copy the frequencies of masks in a mask list to the internal haystack frequency list of an Agnentroprox instance.

In:

  agnentroprox_base is the return value of agenentroprox_init().

  mask_list_size is the size (in bytes) of *mask_list_base.

  mask_idx_max is one less than the number of masks in the mask list, such that if agnentroprox_init():In:overlap_status was one, then this value would need to be increased in order to account for mask overlap. For example, if the sweep contains 5 of 3-byte masks, then this value would be 4 _without_ overlap, or 12 _with_ overlap. Must not exceed agnentroprox_init():In:mask_idx_max_max. See also agnentroprox_mask_idx_max_get().

  *mask_list_base is the needle.

Out:

  Returns one if: (1) agnentroprox_init() was called with overlap_status of zero, but mask_list_size is not a nonzero multiple of the mask size; (2) agnentroprox_init() was called with a mask_idx_max_max which is less than the mask_idx_max implied by mask_list_size. Else zero.

  The mask frequencies implied by *mask_list_base have been copied to frequency list one (the haystack frequency list) of the indicated Agnentroprox instance.
*/
  ULONG mask_idx_max;
  u8 status;

  mask_idx_max=agnentroprox_mask_idx_max_get(agnentroprox_base->granularity, &status, mask_list_size, agnentroprox_base->overlap_status);
  if(!status){
    status=agnentroprox_capacity_check(agnentroprox_base, 1, mask_idx_max, 0);
    if(!status){
      agnentroprox_mask_list_load(agnentroprox_base, 1, mask_idx_max, mask_list_base);
    }
  }
  return status;
}

void
zorb_reset(agnentroprox_t *agnentroprox_base, zorb_t *zorb_base){
/*
Reset a Zorb header (but not its frequency list) to be consistent with an Agnentroprox instance.

In:

  agnentroprox_base is the return value of agnentroprox_init().

  zorb_base is the return value of zorb_init().

Out:

  *zorb_base is consistent with *agnentroprox_base, but its frequency list and LMD2 hash are unaffected.
*/
  ULONG mask_idx_max;
  u32 mask_max;
  ULONG zorb_size;

  mask_idx_max=agnentroprox_base->mask_count0-1;
  mask_max=agnentroprox_base->mask_max;
  zorb_size=(ULONG)(sizeof(zorb_t))+(((ULONG)(mask_max)+1)<<U64_SIZE_LOG2);
  memset(zorb_base, 0, (size_t)(zorb_size));
  zorb_base->mask_idx_max=mask_idx_max;
  zorb_base->mask_max=mask_max;
  zorb_base->signature=ZORB_SIGNATURE;
  return;
}

u8
zorb_size_ulong_get(u32 mask_max, ULONG *zorb_size_base){
/*
Get the size required for a Zorb file (header and frequency list) as a ULONG.

In:

  mask_max is the maximum mask value, such that all mask frequencies need to be stored in the Zorb file to be created.

  *zorb_size_base is undefined.

Out:

  Returns one if the required Zorb file size cannot be stored in a ULONG, else zero.

  *zorb_size_base is the required Zorb file size, assuming a frequency list consisting of (mask_max+1) u64 frequencies.
*/
  u8 status;
  #ifndef _64_
    ULONG zorb_size;
  #endif
  u64 zorb_size_u64;

  zorb_size_u64=(u64)(sizeof(zorb_t))+(((u64)(mask_max)+1)<<U64_SIZE_LOG2);
  status=0;
  #ifndef _64_
    zorb_size=(ULONG)(zorb_size_u64);
    status=(zorb_size!=zorb_size_u64);
    if(!status){
      *zorb_size_base=zorb_size;
    }
  #else
    *zorb_size_base=zorb_size_u64;
  #endif
  return status;
}
