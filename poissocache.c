/*
Poissocache
Copyright 2017 Russell Leidich

This collection of files constitutes the Poissocache Library. (This is a
library in the abstact sense; it's not intended to compile to a ".lib"
file.)

The Poissocache Library is free software: you can redistribute it and/or
modify it under the terms of the GNU Limited General Public License as
published by the Free Software Foundation, version 3.

The Poissocache Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
Limited General Public License version 3 for more details.

You should have received a copy of the GNU Limited General Public
License version 3 along with the Poissocache Library (filename
"COPYING"). If not, see http://www.gnu.org/licenses/ .
*/
/*
Poisson Cache Hierarchy Kernel
*/
#include "flag.h"
#include "flag_poissocache.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "constant.h"
#include "debug.h"
#include "debug_xtrn.h"
#include "poissocache.h"
#include "poissocache_xtrn.h"

void *
poissocache_free(void *base){
/*
To maximize portability and debuggability, this is the only function in which Poissocache calls free().

In:

  base is the base of a memory region to free. May be NULL.

Out:

  Returns NULL so that the caller can easily maintain the good practice of NULLing out invalid pointers.

  *base is freed.
*/
  DEBUG_FREE_PARANOID(base);
  return NULL;
}

poissocache_t *
poissocache_free_all(poissocache_t *poissocache_base){
/*
Free all private storage.

In:

  poissocache_base is the return value of poissocache_init().

Out:

  Returns NULL so that the caller can easily maintain the good practice of NULLing out invalid pointers.

  *poissocache_base and all its child allocations are freed.
*/
  u8 cache_idx;
  u8 cache_idx_max;
  ULONG *cache_item_list_base;

  if(poissocache_base){
    cache_idx_max=poissocache_base->cache_idx_max;
    cache_idx=cache_idx_max;
    do{
      cache_item_list_base=poissocache_base->cache_item_list_base_list_base1[cache_idx];
      poissocache_free(cache_item_list_base);
      cache_item_list_base=poissocache_base->cache_item_list_base_list_base0[cache_idx];
      poissocache_free(cache_item_list_base);
      cache_idx--;
    }while(cache_idx<cache_idx_max);
    poissocache_base=poissocache_free(poissocache_base);
  }
  return poissocache_base;
}

poissocache_t *
poissocache_init(u32 build_break_count, u32 build_feature_count, ULONG cache_item_idx_max){
/*
Verify that the source code is sufficiently updated and initialize private storage.

In:

  build_break_count is the caller's most recent knowledge of POISSOCACHE_BUILD_BREAK_COUNT, which will fail if the caller is unaware of all critical updates.

  build_feature_count is the caller's most recent knowledge of POISSOCACHE_BUILD_FEATURE_COUNT, which will fail if this library is not up to date with the caller's expectations.

  cache_item_idx_max is one less than the maximum number of (key, value) pairs which would need to be stored in the entire Poisson cache at any given time. It must be one less than a power of 2. The entire Poisson cache will consist of POISSOCACHE_CACHE_LEVEL_COUNT_MAX caches, each containing (cache_item_idx_max+1) such pairs. Furthermore there will be 2 such Poisson caches allocated, in order to facilitate double-buffered refactoring in the event of excessive collisions.

Out:

  Returns NULL if (build_break_count!=POISSOCACHE_BUILD_BREAK_COUNT); (build_feature_count>POISSOCACHE_BUILD_FEATURE_COUNT); there is insufficient memory; or one of the input parameters falls outside its valid range. Else, returns the base of a poissocache_t to be used with other Poissocache functions. It must be freed with poissocache_free_all().
*/
  u8 cache_idx;
  u8 cache_idx_max;
  ULONG cache_item_idx_mask;
  ULONG *cache_item_list_base;
  ULONG cache_size;
  u64 k;
  u64 kn;
  u8 n;
  u64 n_factorial;
  poissocache_t *poissocache_base;
  u8 status;

  poissocache_base=NULL;
  status=(build_break_count!=POISSOCACHE_BUILD_BREAK_COUNT);
  status=(u8)(status|(POISSOCACHE_BUILD_FEATURE_COUNT<build_feature_count));
/*
Compute N, the number of levels in the Poisson cache. This must be at least 2 due to expectations in the code. Beyond that, we must ensure that N is large enough to keep the frequency of refactoring down to a minimum level. Refactoring occurs in response to excessive collision. Excessive collision is defined as failing to locate a free cache item in any level of the cache for a given key. This occurs because each of N different hashes of the key overlap with existing cache items, which results in the need to change the hash seeds throughout the entire hiererchy, until such a seed is found which resolves all excessive collisions. The complexity of this refactoring operation is O(KN) where K is the maximum number of unique keys which could possibly need to be stored in the cache at any given time, namely (cache_item_idx_max+1). If N is too small, then refactoring will occur too often, resulting in poor performance. If it's too large, then we could potentially waste a lot of memory and increase cache latency. The optimum is roughly when N is just large enough so that (N!) exceeds (KN). The reason derives from the probability of getting N collisions in a row for any given attempted cache item insertion, which is one in ((N!)e).
*/
  k=(u64)(cache_item_idx_max)+1;
  kn=k;
  n=1;
  n_factorial=n;
  do{
    n++;
    kn+=k;
    if(kn<k){
      kn=U64_MAX;
    }
    n_factorial*=n;
  }while((n_factorial<kn)&&(n!=POISSOCACHE_CACHE_LEVEL_COUNT_MAX));
  cache_idx_max=(u8)(n-1);
/*
cache_item_idx_mask to one less than the least power of 2 which exceeds cache_item_idx_max, then return it below as the new cache_item_idx_max. This is necessary because each cache in the Poisson cache uses masked hashes of the key value as indexes, which can be anything less than a particular power of 2.
*/
  cache_item_idx_mask=0;
  while(cache_item_idx_mask<cache_item_idx_max){
    cache_item_idx_mask=(cache_item_idx_mask<<1)+1;
  }
  cache_size=(cache_item_idx_mask+1)<<(ULONG_SIZE_LOG2+1);
  status=(u8)(status|((cache_size>>(ULONG_SIZE_LOG2+1))<=cache_item_idx_mask));
  if(!status){
    poissocache_base=DEBUG_CALLOC_PARANOID(sizeof(poissocache_t));
    if(poissocache_base){
      poissocache_base->cache_idx_max=cache_idx_max;
      poissocache_base->cache_item_idx_max=cache_item_idx_mask;
      poissocache_base->cache_item_ulong_idx_max=(cache_item_idx_mask<<1)+1;
      poissocache_base->cache_size=cache_size;
      poissocache_base->seed=1;
      cache_idx=0;
      do{
        cache_item_list_base=DEBUG_MALLOC_PARANOID(cache_size);
        status=(u8)(status|!cache_item_list_base);
        poissocache_base->cache_item_list_base_list_base0[cache_idx]=cache_item_list_base;
/*
The secondary Poisson cache cache lacks level zero because its content is unaffected by seed changes and therefore equivalent to level zero of the primary Poisson cache, so don't allocate it.
*/
        if(cache_idx){
          cache_item_list_base=DEBUG_MALLOC_PARANOID(cache_size);
          poissocache_base->cache_item_list_base_list_base1[cache_idx]=cache_item_list_base;
          status=(u8)(status|!cache_item_list_base);
        }
        cache_idx++;
      }while(cache_idx<=cache_idx_max);
      if(!status){
        poissocache_reset(poissocache_base);
      }else{
        poissocache_base=poissocache_free_all(poissocache_base);
      }
    }
  }
  return poissocache_base;
}

u8
poissocache_item_get_serialized(u8 *cache_idx_base, ULONG *cache_item_ulong_idx_base, ULONG *key_base, poissocache_t *poissocache_base, ULONG *value_base){
/*
Dump the entire contents of each cache in a Poisson cache one item at a time, cycling through items before cycling through caches.

In:

  *cache_idx_base is initially zero, then fed back thereafter.

  *cache_item_ulong_idx_base is initially zero, then fed back thereafter.

  *key_base is undefined.

  *poissocache_base is the return value of poissocache_init().

  *value_base is undefined.

Out:

  Returns zero if (*key_base, *value_base) contains a valid (key, value) pair from the Poisson cache, else one. In the latter case, all such pairs, if any, have already been returned on previous calls.

  *cache_idx_base is updated.

  *cache_item_ulong_idx_base is updated.

  *key_base is the key from the next (key, value) pair found. Ordering of results is undefined.

  *value_base is the value from the next (key, value) pair found. Ordering of results is undefined.
*/
  u8 cache_idx;
  u8 cache_idx_max;
  ULONG *cache_item_list_base;
  ULONG cache_item_ulong_idx;
  ULONG cache_item_ulong_idx_max;
  ULONG key;
  u8 status;
  ULONG value;

  cache_idx=*cache_idx_base;
  cache_idx_max=poissocache_base->cache_idx_max;
  cache_item_ulong_idx=*cache_item_ulong_idx_base;
  cache_item_ulong_idx_max=poissocache_base->cache_item_ulong_idx_max;
  status=1;
  while(cache_idx<=cache_idx_max){
    cache_item_list_base=poissocache_base->cache_item_list_base_list_base0[cache_idx];
    while(cache_item_ulong_idx<=cache_item_ulong_idx_max){
      key=cache_item_list_base[cache_item_ulong_idx];
      if(key!=ULONG_MAX){
        value=cache_item_list_base[cache_item_ulong_idx+1];
        cache_item_ulong_idx+=2;
        status=0;
        *key_base=key;
        *value_base=value;
        break;
      }
      cache_item_ulong_idx+=2;
    }
    if(!status){
      break;
    }
    cache_idx++;
    cache_item_ulong_idx=0;
  }
  if(cache_item_ulong_idx_max<cache_item_ulong_idx){
    cache_item_ulong_idx=0;
    cache_idx++;
  }
  if(status|(cache_idx_max<cache_idx)){
/*
Return cache_idx and cache_item_ulong_idx as their respective postterminal values in order to force failure on any subsequent calls.
*/
    cache_idx=(u8)(cache_idx_max+1);
    cache_item_ulong_idx=cache_item_ulong_idx_max+2;
  }
  *cache_idx_base=cache_idx;
  *cache_item_ulong_idx_base=cache_item_ulong_idx;
  return status;
}

ULONG
poissocache_item_ulong_idx_get(ULONG cache_item_idx_max, ULONG **cache_item_list_base_base, ULONG key, poissocache_t *poissocache_base){
/*
Find a (key, value) pair in a Poisson cache whose key matches the specified value. If it doesn't exist, try to create it. If the the hash tags in every cache level collide with one another, refactor the cache pseudorandomly until no such excessive collisions occur.

Don't call here directly. Instead, use  POISSOCACHE_ITEM_ULONG_IDX_GET() as follows:

  key_value_idx_max=poissocache_parameters_get(&key_value_list_base0, poissocache_base);
  POISSOCACHE_ITEM_ULONG_IDX_GET(key_value_ulong_idx, key_value_idx_max, key, key_value_list_base0, key_value_list_base1, poissocache_base);

where key_value_list_base1[key_value_ulong_idx] will then point to the key (a ULONG), and key_value_list_base[key_value_ulong_idx+1] will then point to the value (a ULONG). key_value_idx_max and key_value_list_base0 can then be reused for many calls to POISSOCACHE_ITEM_ULONG_IDX_GET(), even across poissocache_reset(). Note that the granularity of key_value_idx_max is 2 (ULONG)s, whereas that of key_value_ulong_idx is just one ULONG.

In:

  cache_item_idx_max is (poissocache_init():In:cache_item_idx_max).

  **cache_item_list_base_base is undefined.

  key is the key to find, which may or may not be present in the entire Poisson cache. Must not be ULONG_MAX.

  *poissocache_base is the return value of poissocache_init().

Out:

  Returns the ULONG index of the (key, value) pair where the key matches (In:key). If no such pair existed, then it will have been initialized to (key, 0). The caller may then modify the value, but not the key.

  **cache_item_list_base_base is the base of the particular cache within the Poisson cache which contains (key, value) (at the ULONG index given by (return value)).
*/
  u8 cache_idx_max;
  ULONG cache_idx0;
  ULONG cache_idx1;
  ULONG cache_item_ulong_idx_max;
  ULONG cache_item_ulong_idx1;
  ULONG cache_item_ulong_idx0;
  ULONG *cache_item_list_base0;
  ULONG *cache_item_list_base1;
  ULONG cache_size;
  ULONG key0;
  ULONG key1;
  u32 marsaglia_c;
  u32 marsaglia_x;
  u8 max_collision_status;
  u64 random;
  u64 seed;
  ULONG value0;

  cache_idx_max=poissocache_base->cache_idx_max;
  seed=poissocache_base->seed;
  do{
    cache_idx0=1;
    random=key+seed;
    do{
      MARSAGLIA_ITERATE(marsaglia_c, marsaglia_x, random);
      cache_item_list_base0=poissocache_base->cache_item_list_base_list_base0[cache_idx0];
      cache_item_ulong_idx0=(ULONG)((random&cache_item_idx_max)<<1);
      key0=cache_item_list_base0[cache_item_ulong_idx0];
      if(key==key0){
        break;
      }else if(key0==ULONG_MAX){
        cache_item_list_base0[cache_item_ulong_idx0]=key;
        cache_item_list_base0[cache_item_ulong_idx0+1]=0;
        break;
      }
      cache_idx0++;
    }while(cache_idx0<=cache_idx_max);
    if(cache_idx_max<cache_idx0){
      cache_idx0=0;
      cache_item_ulong_idx0=(key&cache_item_idx_max)<<1;
      random=key+seed;
      do{
        cache_item_list_base0=poissocache_base->cache_item_list_base_list_base0[cache_idx0];
        value0=cache_item_list_base0[cache_item_ulong_idx0+1];
        if(!value0){
          cache_item_list_base0[cache_item_ulong_idx0]=key;
          break;
        }
        MARSAGLIA_ITERATE(marsaglia_c, marsaglia_x, random);
        cache_idx0++;
        cache_item_ulong_idx0=(ULONG)((random&cache_item_idx_max)<<1);
      }while(cache_idx0<=cache_idx_max);
    }
    max_collision_status=0;
    if(cache_idx_max<cache_idx0){
      cache_size=poissocache_base->cache_size;
      cache_item_ulong_idx_max=poissocache_base->cache_item_ulong_idx_max;
      do{
        MARSAGLIA_ITERATE(marsaglia_c, marsaglia_x, seed);
        cache_idx1=1;
        do{
          cache_item_list_base1=poissocache_base->cache_item_list_base_list_base1[cache_idx1];
          memset(cache_item_list_base1, U8_MAX, (size_t)(cache_size));
          cache_idx1++;
        }while(cache_idx1<=cache_idx_max);
        cache_idx0=1;
        max_collision_status=0;
        do{
          cache_item_list_base0=poissocache_base->cache_item_list_base_list_base0[cache_idx0];
          cache_item_ulong_idx0=0;
          do{
            key0=cache_item_list_base0[cache_item_ulong_idx0];
            if(key0!=ULONG_MAX){
              value0=cache_item_list_base0[cache_item_ulong_idx0+1];
              if(value0){
                cache_idx1=1;
                random=key0+seed;
                do{
                  MARSAGLIA_ITERATE(marsaglia_c, marsaglia_x, random);
                  cache_item_list_base1=poissocache_base->cache_item_list_base_list_base1[cache_idx1];
                  cache_item_ulong_idx1=(ULONG)((random&cache_item_idx_max)<<1);
                  key1=cache_item_list_base1[cache_item_ulong_idx1];
                  if(key1==ULONG_MAX){
                    cache_item_list_base1[cache_item_ulong_idx1]=key0;
                    cache_item_list_base1[cache_item_ulong_idx1+1]=value0;
                    break;
                  }
                  cache_idx1++;
                }while(cache_idx1<=cache_idx_max);
                max_collision_status=(cache_idx_max<cache_idx1);
              }
            }
            cache_item_ulong_idx0+=2;
          }while((cache_item_ulong_idx0<=cache_item_ulong_idx_max)&&!max_collision_status);
          cache_idx0++;
        }while((cache_idx0<=cache_idx_max)&&!max_collision_status);
      }while(max_collision_status);
      cache_idx0=1;
      do{
        cache_item_list_base0=poissocache_base->cache_item_list_base_list_base0[cache_idx0];
        cache_item_list_base1=poissocache_base->cache_item_list_base_list_base1[cache_idx0];
        memcpy(cache_item_list_base0, cache_item_list_base1, (size_t)(cache_size));
        cache_idx0++;
      }while(cache_idx0<=cache_idx_max);
      poissocache_base->seed=seed;
      max_collision_status=1;
    }
  }while(max_collision_status);
  *cache_item_list_base_base=cache_item_list_base0;
  return cache_item_ulong_idx0;
}

ULONG
poissocache_parameters_get(ULONG **cache_item_list_base_base, poissocache_t *poissocache_base){
/*
Prepare to find (key, value) pairs in the Poisson cache via POISSOCACHE_ITEM_ULONG_IDX_GET(). The whole point is to allow rapid level zero cache lookup, as level zero usually hits.

In:
 
  **cache_item_list_base_base is undefined.

  *poissocache_base is the return value of poissocache_init().

Out:

  Returns the cache_item_idx_max input to POISSOCACHE_ITEM_ULONG_IDX_GET(), which is just a copy of (poissocache_init():In:cache_item_idx_max).

  **cache_item_list_base_base is the base of the first cache within the Poisson cache (level zero), which can then be passed to POISSOCACHE_ITEM_ULONG_IDX_GET() as cache_item_list_base0 as shown in the summary for poissocache_item_ulong_idx_get().
*/
  ULONG cache_item_idx_max;

  *cache_item_list_base_base=poissocache_base->cache_item_list_base_list_base0[0];
  cache_item_idx_max=poissocache_base->cache_item_idx_max;
  return cache_item_idx_max;
}

void
poissocache_reset(poissocache_t *poissocache_base){
/*
Invalidate all (key, value) pairs in the Poisson cache.

In:

  *poissocache_base is the return value of poissocache_init().

Out:

  The Poisson cache associated with *poissocache_base is empty.
*/
  u8 cache_idx;
  u8 cache_idx_max;
  ULONG *cache_item_list_base;
  ULONG cache_size;
/*
Initialize all cache entries in the Poisson cache zero to invalid (free) values. End up with the hardware cache is primed with the contents of cache level zero.
*/
  cache_idx_max=poissocache_base->cache_idx_max;
  cache_size=poissocache_base->cache_size;
  cache_idx=cache_idx_max;
  do{
    cache_item_list_base=poissocache_base->cache_item_list_base_list_base0[cache_idx];
    memset(cache_item_list_base, U8_MAX, (size_t)(cache_size));
    cache_idx--;
  }while(cache_idx<cache_idx_max);
/*
There's no need to reset the caches pointed to by cache_item_list_base_list_base1 because that gets done automatically upon refactoring. 
*/
  return;
}
