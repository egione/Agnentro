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
extern void *poissocache_free(void *base);
extern poissocache_t *poissocache_free_all(poissocache_t *poissocache_base);
extern poissocache_t *poissocache_init(u32 build_break_count, u32 build_feature_count, ULONG cache_item_idx_max);
extern u8 poissocache_item_get_serialized(u8 *cache_idx_base, ULONG *cache_item_ulong_idx_base, ULONG *key_base, poissocache_t *poissocache_base, ULONG *value_base);
extern ULONG poissocache_item_ulong_idx_get(ULONG cache_item_idx_max, ULONG **cache_item_list_base_base, ULONG key, poissocache_t *poissocache_base);
extern ULONG poissocache_parameters_get(ULONG **cache_item_list_base_base, poissocache_t *poissocache_base);
extern void poissocache_reset(poissocache_t *poissocache_base);
