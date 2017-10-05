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
extern fru128 agnentroprox_compressivity_get(fru128 entropy, fru128 entropy_raw);
extern fru128 agnentroprox_diventropy_get(agnentroprox_t *agnentroprox_base, ULONG haystack_mask_idx_max, u8 *haystack_mask_list_base, u8 *overflow_status_base);
extern ULONG agnentroprox_diventropy_transform(agnentroprox_t *agnentroprox_base, u8 append_mode, fru128 *diventropy_list_base, ULONG haystack_mask_idx_max, u8 *haystack_mask_list_base, ULONG match_idx_max_max, ULONG *match_u8_idx_list_base, u8 *overflow_status_base, ULONG sweep_mask_idx_max);
extern fru128 agnentroprox_dyspoissonism_get(fru128 entropy, fru128 entropy_raw);
extern fru128 agnentroprox_entropy_delta_get(agnentroprox_t *agnentroprox_base, ULONG mask_idx_max, u8 *mask_list_base, u16 mode, u8 new_status, u8 *overflow_status_base, u8 rollback_status);
extern fru128 agnentroprox_entropy_raw_get(agnentroprox_t *agnentroprox_base, ULONG mask_idx_max, u8 *overflow_status_base);
extern ULONG agnentroprox_entropy_transform(agnentroprox_t *agnentroprox_base, u8 append_mode, fru128 *entropy_list_base, ULONG mask_idx_max, u8 *mask_list_base, ULONG match_idx_max_max, ULONG *match_u8_idx_list_base, u16 mode, u8 *overflow_status_base, ULONG sweep_mask_idx_max);
extern ULONG agnentroprox_exoelasticity_transform(agnentroprox_t *agnentroprox_base, u8 append_mode, fru128 *exoelasticity_list_base, ULONG mask_idx_max, u8 *mask_list_base, ULONG match_idx_max_max, ULONG *match_u8_idx_list_base, u8 *overflow_status_base, ULONG sweep_mask_idx_max);
extern void *agnentroprox_free(void *base);
extern agnentroprox_t *agnentroprox_free_all(agnentroprox_t *agnentroprox_base);
extern agnentroprox_t *agnentroprox_init(u32 build_break_count, u32 build_feature_count, u8 granularity, loggamma_t *loggamma_base, ULONG mask_idx_max_max, u32 mask_max, u16 mode_bitmap, u8 overlap_status, ULONG sweep_mask_idx_max_max);
extern fru128 agnentroprox_jsd_get(agnentroprox_t *agnentroprox_base, u8 *haystack_mask_list_base, u8 *overflow_status_base);
extern ULONG agnentroprox_jsd_transform(agnentroprox_t *agnentroprox_base, u8 append_mode, ULONG haystack_mask_idx_max, u8 *haystack_mask_list_base, fru128 *jsd_list_base, ULONG match_idx_max_max, ULONG *match_u8_idx_list_base, u8 *overflow_status_base);
extern ULONG agnentroprox_mask_idx_max_get(u8 granularity, u8 *granularity_status_base, ULONG mask_list_size, u8 overlap_status);
extern void agnentroprox_mask_list_accrue(agnentroprox_t *agnentroprox_base, u8 freq_list_idx, ULONG mask_idx_max, u8 *mask_list_base);
extern void agnentroprox_mask_list_deltafy(u8 channel_status, u8 delta_status, u8 granularity, ULONG mask_idx_max, u8 *mask_list_base);
extern u8 *agnentroprox_mask_list_malloc(u8 granularity, ULONG mask_idx_max, u8 overlap_status);
extern u128 agnentroprox_mask_list_mean_get(agnentroprox_t *agnentroprox_base, ULONG mask_idx_max, u8 *mask_list_base, u8 *sign_status_base);
extern u8 *agnentroprox_mask_list_realloc(u8 granularity, ULONG mask_idx_max, u8 *mask_list_base, u8 overlap_status);
extern void agnentroprox_mask_list_subtract(agnentroprox_t *agnentroprox_base);
extern void agnentroprox_mask_list_unaccrue(agnentroprox_t *agnentroprox_base, u8 freq_list_idx, ULONG mask_idx_max, u8 *mask_list_base);
extern ULONG agnentroprox_match_find(u8 ascending_status, u8 case_insensitive_status, u8 granularity, ULONG haystack_mask_idx_max, u8 *haystack_mask_list_base, ULONG match_idx_max_max, ULONG *match_u8_idx_list_base, ULONG needle_mask_idx_max, u8 *needle_mask_list_base, u8 overlap_status);
extern void agnentroprox_needle_freq_list_copy(agnentroprox_t *agnentroprox_base, u8 restore_status);
extern void agnentroprox_needle_freq_list_equalize(agnentroprox_t *agnentroprox_base, ULONG haystack_mask_idx_max);
extern void agnentroprox_needle_mask_list_load(agnentroprox_t *agnentroprox_base, ULONG mask_idx_max, u8 *mask_list_base);
extern fru128 agnentroprox_shannon_entropy_get(agnentroprox_t *agnentroprox_base, u8 freq_list_idx, u8 *overflow_status_base);
extern u128 *agnentroprox_u128_list_malloc(ULONG u128_idx_max);
extern u32 *agnentroprox_u32_list_malloc(ULONG u32_idx_max);
extern void agnentroprox_u32_list_zero(ULONG u32_idx_max, u32 *u32_list_base);
extern u64 *agnentroprox_u64_list_malloc(ULONG u64_idx_max);
extern void agnentroprox_u64_list_zero(ULONG u64_idx_max, u64 *u64_list_base);
#ifdef _64_
  #define agnentroprox_ulong_list_malloc agnentroprox_u64_list_malloc
  #define agnentroprox_ulong_list_zero agnentroprox_u64_list_zero
#else
  #define agnentroprox_ulong_list_malloc agnentroprox_u32_list_malloc
  #define agnentroprox_ulong_list_zero agnentroprox_u32_list_zero
#endif
extern void agnentroprox_ulong_list_copy(ULONG ulong_idx_max, ULONG *ulong_list_base0, ULONG *ulong_list_base1);
