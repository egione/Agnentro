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
#define POISSOCACHE_CACHE_LEVEL_COUNT_MAX 20

#define POISSOCACHE_ITEM_ULONG_IDX_GET(cache_item_ulong_idx, cache_item_idx_max, key, cache_item_list_base0, cache_item_list_base1, poissocache_base) \
  cache_item_ulong_idx=(ULONG)((key&cache_item_idx_max)<<1); \
  cache_item_list_base1=cache_item_list_base0; \
  if(key!=cache_item_list_base0[cache_item_ulong_idx]){ \
    cache_item_ulong_idx=poissocache_item_ulong_idx_get(cache_item_idx_max, &cache_item_list_base1, key, poissocache_base); \
  }

TYPEDEF_START
  u64 seed;
  ULONG *cache_item_list_base_list_base0[POISSOCACHE_CACHE_LEVEL_COUNT_MAX];
  ULONG *cache_item_list_base_list_base1[POISSOCACHE_CACHE_LEVEL_COUNT_MAX];
  ULONG cache_item_idx_max;
  ULONG cache_item_ulong_idx_max;
  ULONG cache_size;
  u8 cache_idx_max;
TYPEDEF_END(poissocache_t)
