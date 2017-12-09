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
extern u8 zorb_check(u32 mask_max, zorb_t *zorb_base, ULONG zorb_size);
extern void zorb_finalize(agnentroprox_t *agnentroprox_base, zorb_t *zorb_base);
extern void *zorb_free(void *base);
extern u8 zorb_freq_list_add(agnentroprox_t *agnentroprox_base);
extern u8 zorb_freq_list_export(agnentroprox_t *agnentroprox_base, zorb_t *zorb_base);
extern u8 zorb_freq_list_import(agnentroprox_t *agnentroprox_base, zorb_t *zorb_base);
extern u8 zorb_freq_list_subtract(agnentroprox_t *agnentroprox_base);
extern zorb_t *zorb_init(u32 build_break_count, u32 build_feature_count, u32 mask_max);
extern u8 zorb_lmd2_get(zorb_t *zorb_base, u64 *lmd2_base);
extern u8 zorb_mask_idx_max_get(ULONG *mask_idx_max_base, zorb_t *zorb_base);
extern u8 zorb_mask_list_load(agnentroprox_t *agnentroprox_base, u8 *mask_list_base, ULONG mask_list_size);
extern void zorb_reset(agnentroprox_t *agnentroprox_base, zorb_t *zorb_base);
extern u8 zorb_size_ulong_get(u32 mask_max, ULONG *zorb_size_base);
