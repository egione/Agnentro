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
extern u64 agnentrocodec_code_bit_idx_max_max_get(loggamma_t *loggamma_base, ULONG mask_idx_max_max, u32 mask_max);
extern ULONG agnentrocodec_code_chunk_idx_max_max_get(loggamma_t *loggamma_base, ULONG mask_idx_max_max, u32 mask_max);
extern u8 agnentrocodec_code_export(agnentrocodec_t *agnentrocodec_base, u64 bit_idx_min, ULONG *chunk_idx_max_base, ULONG chunk_idx_max_max, ULONG *chunk_list_base);
extern u8 agnentrocodec_code_import(agnentrocodec_t *agnentrocodec_base, u64 bit_count_minus_1, u64 bit_idx_min, ULONG chunk_idx_max, ULONG *chunk_list_base);
extern void agnentrocodec_decode(agnentrocodec_t *agnentrocodec_base, ULONG mask_idx_max, u8 *mask_list_base);
extern u64 agnentrocodec_encode(agnentrocodec_t *agnentrocodec_base, ULONG mask_idx_max, u8 *mask_list_base);
extern void *agnentrocodec_free(void *base);
extern agnentrocodec_t *agnentrocodec_free_all(agnentrocodec_t *agnentrocodec_base);
extern void agnentrocodec_freq_tree_sync(agnentrocodec_t *agnentrocodec_base);
extern agnentrocodec_t *agnentrocodec_init(u32 build_break_count, u32 build_feature_count, u8 granularity, loggamma_t *loggamma_base, ULONG mask_idx_max_max, u32 mask_max);
extern u8 *agnentrocodec_mask_list_malloc(u8 granularity, ULONG mask_idx_max);
extern u32 agnentrocodec_mask_max_get(u8 granularity, ULONG mask_idx_max, u8 *mask_list_base);
extern u64 agnentrocodec_protoagnentropic_get(agnentrocodec_t *agnentrocodec_base, ULONG mask_idx_max, u8 *mask_list_base);
extern void agnentrocodec_reset(agnentrocodec_t *agnentrocodec_base);
