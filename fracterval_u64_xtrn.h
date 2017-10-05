/*
Fracterval U64
Copyright 2017 Russell Leidich

This collection of files constitutes the Fracterval U64 Library. (This is a
library in the abstact sense; it's not intended to compile to a ".lib"
file.)

The Fracterval U64 Library is free software: you can redistribute it and/or
modify it under the terms of the GNU Limited General Public License as
published by the Free Software Foundation, version 3.

The Fracterval U64 Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
Limited General Public License version 3 for more details.

You should have received a copy of the GNU Limited General Public
License version 3 along with the Fracterval U64 Library (filename
"COPYING"). If not, see http://www.gnu.org/licenses/ .
*/
extern u8 fracterval_u64_divide_fracterval_u64(fru64 *a_base, fru64 p, fru64 q);
extern u8 fracterval_u64_divide_u64(fru64 *a_base, fru64 p, u64 v);
extern void *fracterval_u64_free(void *base);
extern u8 fracterval_u64_init(u32 build_break_count, u32 build_feature_count);
extern fru64 *fracterval_u64_list_malloc(ULONG fracterval_u64_idx_max);
extern u8 fracterval_u64_log_delta_u64(fru64 *a_base, u64 v);
extern u8 fracterval_u64_log_delta_u64_cached(fru64 *a_base, ULONG log_delta_idx_max, fru64 *log_delta_list_base, u64 *log_delta_parameter_list_base, u64 v);
extern u8 fracterval_u64_log_mantissa_delta_u64(fru64 *a_base, u64 p, u64 q);
extern u8 fracterval_u64_log_mantissa_u64(fru64 *a_base, u64 p);
extern u8 fracterval_u64_log_u64(fru64 *a_base, u64 v);
extern u64 *fracterval_u64_log_u64_cache_init(ULONG log_idx_max, fru64 **log_list_base_base);
extern u8 fracterval_u64_log_u64_cached(fru64 *a_base, ULONG log_idx_max, fru64 *log_list_base, u64 *log_parameter_list_base, u64 v);
extern void fracterval_u64_multiply_fracterval_u64(fru64 *a_base, fru64 p, fru64 q);
extern void fracterval_u64_multiply_fractoid_u64(fru64 *a_base, fru64 p, u64 q);
extern void fracterval_u64_multiply_mantissa_u64(fru64 *a_base, fru64 p, u64 q);
extern u8 fracterval_u64_multiply_u64(fru64 *a_base, fru64 p, u64 v);
extern void fracterval_u64_nats_from_bits(fru64 *a_base, fru64 p);
extern u8 fracterval_u64_nats_to_bits(fru64 *a_base, fru64 p);
extern u8 fracterval_u64_shift_left(fru64 *a_base, u8 b, fru64 p);
extern u64 *fracterval_u64_u64_list_malloc(ULONG u64_idx_max);
extern void fracterval_u64_u64_list_zero(ULONG u64_idx_max, u64 *u64_list_base);
extern u8 fractoid_u64_ratio_u64_saturate(u64 *a_base, u64 v, u64 w);
extern u8 fractoid_u64_reciprocal_u64_saturate(u64 *a_base, u64 v);
extern u64 fractoid_u64_scale_u64(u64 v, u64 w);
