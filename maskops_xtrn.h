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
extern ULONG *maskops_bitmap_malloc(u64 msb);
extern void maskops_deltafy(u8 channel_status, u8 direction_status, u8 granularity, ULONG mask_idx_max, u8 *mask_list_base);
extern void maskops_densify(u8 direction_status, u8 granularity, ULONG mask_idx_max, u8 *mask_list_base, u32 mask_min, u32 *remask_list_base);
extern void maskops_densify_bitmap_prepare(ULONG *bitmap_base, u8 granularity, ULONG mask_idx_max, u8 *mask_list_base, u32 mask_max, u32 mask_min, u8 reset_status);
extern u32 maskops_densify_remask_prepare(ULONG *bitmap_base, u8 direction_status, u32 mask_max, u32 mask_min, u32 *remask_list_base);
extern void *maskops_free(void *base);
extern u8 maskops_init(u32 build_break_count, u32 build_feature_count);
extern u8 *maskops_mask_list_malloc(u8 granularity, ULONG mask_idx_max);
extern u32 maskops_max_min_get(u8 granularity, ULONG mask_idx_max, u8 *mask_list_base, u32 *mask_max_base, u8 sign_status);
extern u32 maskops_negate(u8 granularity, ULONG mask_idx_max, u8 *mask_list_base, u32 *mask_max_base);
extern u32 maskops_surroundify(u8 channel_status, u8 direction_status, u8 granularity, ULONG mask_idx_max, u8 *mask_list_base, u32 mask_max, u32 mask_min);
extern u32 *maskops_u32_list_malloc(ULONG u32_idx_max);
extern void maskops_u32_list_zero(ULONG u32_idx_max, u32 *u32_list_base);
extern u8 maskops_unsign(u8 granularity, ULONG mask_idx_max, u8 *mask_list_base, u32 *mask_max_base, u32 *mask_min_base);
