/*
ASCII
Copyright 2017 Russell Leidich

This collection of files constitutes the ASCII Library. (This is a
library in the abstact sense; it's not intended to compile to a ".lib"
file.)

The ASCII Library is free software: you can redistribute it and/or
modify it under the terms of the GNU Limited General Public License as
published by the Free Software Foundation, version 3.

The ASCII Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
Limited General Public License version 3 for more details.

You should have received a copy of the GNU Limited General Public
License version 3 along with the ASCII Library (filename
"COPYING"). If not, see http://www.gnu.org/licenses/ .
*/
extern u8 ascii_decimal_to_u64_convert(char *decimal_base, u64 *integer_base, u64 integer_max);
extern u8 ascii_hex_to_u64_convert(char *digit_list_base, u64 *integer_base, u64 integer_max);
extern u8 ascii_decimal_to_u64_convert_negatable(char *digit_list_base, u64 *integer_base, u64 integer_max, u8 *sign_base);
extern u8 ascii_hex_to_u8_list_convert(ULONG digit_idx_max, ULONG digit_idx_min, char *digit_list_base, u8 *u8_list_base);
extern void *ascii_free(void *base);
extern u8 ascii_init(u32 build_break_count, u32 build_feature_count);
extern char *ascii_utf8_safe_list_malloc(ULONG unsafe_idx_max);
extern ULONG ascii_utf8_sanitize(u8 eager_status, char *safe_list_base, ULONG unsafe_idx_max, ULONG unsafe_idx_min, char *unsafe_list_base, ULONG *utf8_idx_max_base);
extern u8 ascii_utf8_string_verify(char *unsafe_list_base);
extern u8 ascii_utf8_verify(ULONG unsafe_idx_max, char *unsafe_list_base);
