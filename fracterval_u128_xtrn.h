/*
Fracterval U128
Copyright 2017 Russell Leidich

This collection of files constitutes the Fracterval U128 Library. (This is a
library in the abstact sense; it's not intended to compile to a ".lib"
file.)

The Fracterval U128 Library is free software: you can redistribute it and/or
modify it under the terms of the GNU Limited General Public License as
published by the Free Software Foundation, version 3.

The Fracterval U128 Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
Limited General Public License version 3 for more details.

You should have received a copy of the GNU Limited General Public
License version 3 along with the Fracterval U128 Library (filename
"COPYING"). If not, see http://www.gnu.org/licenses/ .
*/
extern u8 fracterval_u128_divide_fracterval_u128(fru128 *a_base, fru128 p, fru128 q);
extern u8 fracterval_u128_divide_u128(fru128 *a_base, fru128 p, u128 q);
extern u8 fracterval_u128_divide_u64(fru128 *a_base, fru128 p, u64 q);
extern void *fracterval_u128_free(void *base);
extern void fracterval_u128_from_fractoid_u128_mantissa_u128_product(fru128 *a_base, u128 p, u128 q);
extern void fracterval_u128_from_fractoid_u128_mantissa_u64_product(fru128 *a_base, u128 p, u64 v);
extern u8 fracterval_u128_from_fractoid_u128_u64_product(fru128 *a_base, u128 p, u64 v);
extern u8 fracterval_u128_init(u32 build_break_count, u32 build_feature_count);
extern fru128 *fracterval_u128_list_malloc(ULONG fru128_idx_max);
extern u8 fracterval_u128_log_delta_u64(fru128 *a_base, u64 v);
extern u8 fracterval_u128_log_delta_u64_cached(fru128 *a_base, ULONG log_delta_idx_max, fru128 *log_delta_list_base, u64 *log_delta_parameter_list_base, u64 v);
extern u8 fracterval_u128_log_mantissa_delta_u128(fru128 *a_base, u128 p, u128 q);
extern u8 fracterval_u128_log_mantissa_u128(fru128 *a_base, u128 p);
extern u8 fracterval_u128_log_u128(fru128 *a_base, u128 p);
extern u128 *fracterval_u128_log_u128_cache_init(ULONG log_idx_max, fru128 **log_list_base_base);
extern u8 fracterval_u128_log_u128_cached(fru128 *a_base, ULONG log_idx_max, fru128 *log_list_base, u128 *log_parameter_list_base, u128 p);
extern u8 fracterval_u128_log_u64(fru128 *a_base, u64 v);
extern u64 *fracterval_u128_log_u64_cache_init(ULONG log_idx_max, fru128 **log_list_base_base);
extern u8 fracterval_u128_log_u64_cached(fru128 *a_base, ULONG log_idx_max, fru128 *log_list_base, u64 *log_parameter_list_base, u64 v);
extern void fracterval_u128_multiply_fracterval_u128(fru128 *a_base, fru128 p, fru128 q);
extern void fracterval_u128_multiply_fractoid_u128(fru128 *a_base, fru128 p, u128 q);
extern void fracterval_u128_multiply_mantissa_u128(fru128 *a_base, fru128 p, u128 q);
extern void fracterval_u128_multiply_mantissa_u64(fru128 *a_base, fru128 p, u64 v);
extern u8 fracterval_u128_multiply_u64(fru128 *a_base, fru128 p, u64 v);
extern void fracterval_u128_nats_from_bits(fru128 *a_base, fru128 p);
extern u8 fracterval_u128_nats_to_bits(fru128 *a_base, fru128 p);
extern u8 fracterval_u128_rank_list_insert_ascending(fru128 p, ULONG *rank_count_base, ULONG *rank_idx_base, ULONG rank_idx_max_max, fru128 *rank_list_base, u128 *threshold_base);
extern u8 fracterval_u128_rank_list_insert_descending(fru128 p, ULONG *rank_count_base, ULONG *rank_idx_base, ULONG rank_idx_max_max, fru128 *rank_list_base, u128 *threshold_base);
extern fru128 *fracterval_u128_rank_list_malloc(ULONG rank_idx_max_max);
extern void fracterval_u128_root_fractoid_u128(fru128 *a_base, u128 p);
extern u8 fracterval_u128_shift_left(fru128 *a_base, u8 b, fru128 p);
extern u128 *fracterval_u128_u128_list_malloc(ULONG u128_idx_max);
extern void fracterval_u128_u128_list_zero(ULONG u128_idx_max, u128 *u128_list_base);
extern u64 *fracterval_u128_u64_list_malloc(ULONG u64_idx_max);
extern void fracterval_u128_u64_list_zero(ULONG u64_idx_max, u64 *u64_list_base);
extern u128 fractoid_u128_from_mantissa_u128_product(u128 p, u128 q);
extern u8 fractoid_u128_ratio_u128(u128 *a_base, u128 p, u128 q);
extern u8 fractoid_u128_ratio_u64(u128 *a_base, u64 v, u64 w);
extern u8 fractoid_u128_reciprocal_u128(u128 *a_base, u128 p);
extern u8 fractoid_u128_reciprocal_u64(u128 *a_base, u64 v);
extern u8 u128_divide_u128(u128 *a_base, u128 p, u128 q);
extern u8 u128_divide_u64_to_u128(u128 *a, u128 p, u64 v);
extern u8 u128_divide_u64_to_u64(u64 *a, u128 p, u64 v);
extern u128 u128_from_u128_pair_bit_idx(u8 b, u128 p, u128 q);
extern u8 u128_msb_get(u128 p);
extern u8 u128_multiply_u64(u128 *a, u128 p, u64 v);
extern u128 u128_from_u64_product(u64 v, u64 w);
