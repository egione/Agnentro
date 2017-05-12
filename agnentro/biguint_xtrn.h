/*
Biguint
Copyright 2017 Russell Leidich

This collection of files constitutes the Biguint Library. (This is a
library in the abstact sense; it's not intended to compile to a ".lib"
file.)

The Biguint Library is free software: you can redistribute it and/or
modify it under the terms of the GNU Limited General Public License as
published by the Free Software Foundation, version 3.

The Biguint Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
Limited General Public License version 3 for more details.

You should have received a copy of the GNU Limited General Public
License version 3 along with the Biguint Library (filename
"COPYING"). If not, see http://www.gnu.org/licenses/ .
*/
extern ULONG biguint_add_biguint(ULONG chunk_idx_max0, ULONG chunk_idx_max1, ULONG *chunk_list_base0, ULONG *chunk_list_base1);
extern ULONG biguint_add_u64(ULONG chunk_idx_max, ULONG *chunk_list_base, u64 uint);
extern ULONG biguint_bit_clear(u64 bit_idx, ULONG chunk_idx_max, ULONG *chunk_list_base);
extern ULONG biguint_bit_flip(u64 bit_idx, ULONG chunk_idx_max, ULONG *chunk_list_base);
extern u8 biguint_bit_get(u64 bit_idx, ULONG chunk_idx_max, ULONG *chunk_list_base);
extern ULONG biguint_bit_set(u64 bit_idx, ULONG chunk_idx_max, ULONG *chunk_list_base);
extern ULONG biguint_bitmap_clear(u64 bit_count_minus_1, u64 bit_idx_min, ULONG chunk_idx_max, ULONG *chunk_list_base);
extern ULONG biguint_bitmap_copy(u64 bit_count_minus_1, u64 bit_idx_min0, u64 bit_idx_min1, ULONG chunk_idx_max0, ULONG chunk_idx_max1, ULONG *chunk_list_base0, ULONG *chunk_list_base1);
extern ULONG biguint_bitmap_copy_reverse(u64 bit_count_minus_1, u64 bit_idx_min0, u64 bit_idx_min1, ULONG chunk_idx_max0, ULONG chunk_idx_max1, ULONG *chunk_list_base0, ULONG *chunk_list_base1);
extern void biguint_bitmap_copy_reverse_unsafe(u64 bit_count_minus_1, u64 bit_idx_min0, u64 bit_idx_min1, ULONG *chunk_list_base0, ULONG *chunk_list_base1);
extern void biguint_bitmap_copy_unsafe(u64 bit_count_minus_1, u64 bit_idx_min0, u64 bit_idx_min1, ULONG *chunk_list_base0, ULONG *chunk_list_base1);
extern u8 biguint_bitmap_export(u64 bit_idx_max, ULONG chunk_idx_max, ULONG chunk_idx_max_max, ULONG *chunk_list_base);
extern u8 biguint_bitmap_import(u64 bit_idx_max, ULONG *chunk_idx_max_base, ULONG chunk_idx_max_max, ULONG *chunk_list_base);
extern ULONG biguint_bitmap_set(u64 bit_count_minus_1, u64 bit_idx_min, ULONG chunk_idx_max, ULONG *chunk_list_base);
extern u8 biguint_chunk_idx_max_get(ULONG *biguint_chunk_idx_max_base, u64 *logplex_bit_idx_min_base, ULONG logplex_chunk_idx_max, ULONG *logplex_chunk_list_base);
extern u8 biguint_compare_biguint(ULONG chunk_idx_max0, ULONG chunk_idx_max1, ULONG *chunk_list_base0, ULONG *chunk_list_base1);
extern u8 biguint_compare_u128(ULONG chunk_idx_max, ULONG *chunk_list_base, u128 uint);
extern u8 biguint_compare_u64(ULONG chunk_idx_max, ULONG *chunk_list_base, u64 uint);
extern ULONG biguint_copy(ULONG chunk_idx_max, ULONG *chunk_list_base0, ULONG *chunk_list_base1);
extern ULONG biguint_decrement(ULONG chunk_idx_max, ULONG *chunk_list_base);
extern ULONG biguint_divide_biguint(ULONG *chunk_idx_max_base0, ULONG chunk_idx_max1, ULONG *chunk_list_base0, ULONG *chunk_list_base1, ULONG *chunk_list_base2);
extern ULONG biguint_divide_u64(ULONG chunk_idx_max, ULONG *chunk_list_base, u64 *remainder_base, u64 uint);
extern void *biguint_free(void *base);
extern u8 biguint_from_ascii_decimal(ULONG *chunk_idx_max_base, ULONG chunk_idx_max_max, ULONG *chunk_list_base, const char *digit_list_base);
extern u8 biguint_from_ascii_hex(ULONG *chunk_idx_max_base, ULONG chunk_idx_max_max, ULONG *chunk_list_base, const char *digit_list_base);
extern ULONG biguint_from_u128(ULONG *chunk_list_base, u128 uint);
extern ULONG biguint_from_u64(ULONG *chunk_list_base, u64 uint);
extern ULONG biguint_increment(ULONG chunk_idx_max, ULONG *chunk_list_base);
extern u8 biguint_init(u32 build_break_count, u32 build_feature_count);
extern u8 biguint_is_power_of_2(ULONG chunk_idx_max, ULONG *chunk_list_base);
extern u8 biguint_is_power_of_2_minus_1(ULONG chunk_idx_max, ULONG *chunk_list_base);
extern ULONG biguint_logplex_chunk_idx_max_get(ULONG biguint_chunk_idx_max, ULONG *biguint_chunk_list_base, u64 logplex_bit_idx_min);
extern u8 biguint_logplex_decode(ULONG *biguint_chunk_idx_max_base, ULONG *biguint_chunk_list_base, u64 *logplex_bit_idx_min_base, ULONG logplex_chunk_idx_max, ULONG *logplex_chunk_list_base);
extern u8 biguint_logplex_decode_u64(u64 *logplex_bit_idx_min_base, ULONG logplex_chunk_idx_max, ULONG *logplex_chunk_list_base, u64 *uint_base);
extern u8 biguint_logplex_encode(ULONG biguint_chunk_idx_max, ULONG *biguint_chunk_list_base, u64 *logplex_bit_idx_min_base, ULONG *logplex_chunk_idx_max_base, ULONG logplex_chunk_idx_max_max, ULONG *logplex_chunk_list_base);
extern u8 biguint_logplex_encode_u64(u64 *logplex_bit_idx_min_base, ULONG *logplex_chunk_idx_max_base, ULONG logplex_chunk_idx_max_max, ULONG *logplex_chunk_list_base, u64 uint);
extern u8 biguint_logplex_mantissa_get(u64 *logplex_bit_idx_min_base, ULONG logplex_chunk_idx_max, ULONG *logplex_chunk_list_base, u64 *mantissa_bit_idx_min_base, u64 *mantissa_msb_base);
extern u64 biguint_logplex_msb_get(ULONG chunk_idx_max, ULONG *chunk_list_base);
extern u64 biguint_lsb_get(ULONG chunk_idx_max, ULONG *chunk_list_base);
extern ULONG *biguint_malloc(ULONG ulong_idx_max);
extern u64 biguint_msb_get(ULONG chunk_idx_max, ULONG *chunk_list_base);
extern ULONG biguint_multiply_add_biguint(ULONG chunk_idx_max0, ULONG chunk_idx_max1, ULONG chunk_idx_max2, ULONG *chunk_list_base0, ULONG *chunk_list_base1, ULONG *chunk_list_base2);
extern ULONG biguint_multiply_biguint(ULONG chunk_idx_max0, ULONG chunk_idx_max1, ULONG *chunk_list_base0, ULONG *chunk_list_base1);
extern ULONG biguint_multiply_u64(ULONG chunk_idx_max, ULONG *chunk_list_base, u64 uint);
extern ULONG biguint_pochhammer_multiply(ULONG chunk_idx_max, ULONG *chunk_list_base, ULONG factor_count, ULONG factor_min);
extern ULONG biguint_power_of_2_get(u64 bit_idx_max, ULONG *chunk_list_base);
extern ULONG biguint_power_of_2_minus_1_get(u64 bit_count, ULONG *chunk_list_base);
extern ULONG biguint_reverse(ULONG chunk_idx_max, ULONG *chunk_list_base);
extern void biguint_reverse_unsafe(ULONG chunk_idx_max, ULONG *chunk_list_base);
extern ULONG biguint_shift_left(u64 bit_count, ULONG chunk_idx_max, ULONG *chunk_list_base);
extern ULONG biguint_shift_right(u64 bit_count, ULONG chunk_idx_max, ULONG *chunk_list_base);
extern ULONG biguint_subtract_biguint(ULONG chunk_idx_max0, ULONG chunk_idx_max1, ULONG *chunk_list_base0, ULONG *chunk_list_base1);
extern ULONG biguint_subtract_u64_shifted(u64 bit_count, ULONG chunk_idx_max, ULONG *chunk_list_base, u64 uint);
extern ULONG biguint_subtract_from_biguint(ULONG chunk_idx_max0, ULONG chunk_idx_max1, ULONG *chunk_list_base0, ULONG *chunk_list_base1);
extern ULONG biguint_subtract_u64(ULONG chunk_idx_max, ULONG *chunk_list_base, u64 uint);
extern ULONG biguint_swap_biguint(ULONG *chunk_idx_max0_base, ULONG chunk_idx_max1, ULONG *chunk_list_base0, ULONG *chunk_list_base1);
extern u8 biguint_to_ascii_decimal(ULONG chunk_idx_max, ULONG *chunk_list_base, ULONG digit_idx_max_max, char *digit_list_base);
extern u8 biguint_to_ascii_hex(ULONG chunk_idx_max, ULONG *chunk_list_base, ULONG digit_idx_max_max, char *digit_list_base);
extern u128 biguint_to_u128_saturate(ULONG chunk_idx_max, ULONG *chunk_list_base, u8 *overflow_status_base);
extern u128 biguint_to_u128_wrap(ULONG chunk_idx_max, ULONG *chunk_list_base);
extern u64 biguint_to_u64_saturate(ULONG chunk_idx_max, ULONG *chunk_list_base, u8 *overflow_status_base);
extern u64 biguint_to_u64_wrap(ULONG chunk_idx_max, ULONG *chunk_list_base);
extern ULONG biguint_truncate(u64 bit_idx_max, ULONG chunk_idx_max, ULONG *chunk_list_base);
