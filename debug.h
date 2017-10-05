/*
Dyspoissometer
Copyright 2017 Russell Leidich
http://dyspoissonism.blogspot.com

This collection of files constitutes the Dyspoissometer Library. (This is a
library in the abstact sense; it's not intended to compile to a ".lib"
file.)

The Dyspoissometer Library is free software: you can redistribute it and/or
modify it under the terms of the GNU Limited General Public License as
published by the Free Software Foundation, version 3.

The Dyspoissometer Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
Limited General Public License version 3 for more details.

You should have received a copy of the GNU Limited General Public
License version 3 along with the Dyspoissometer Library (filename
"COPYING"). If not, see http://www.gnu.org/licenses/ .
*/
#define DEBUG_FREE_CORRUPTION_BYTE 0xD7U
#define DEBUG_MALLOC_CORRUPTION_BYTE 0xA9U
#ifdef DEBUG
  #define DEBUG_ALLOCATION_CHECK() debug_allocation_check()
  #define DEBUG_BIGUINT(name_base, chunk_idx_max, chunk_list_base) debug_biguint(name_base, chunk_idx_max, chunk_list_base)
  #define DEBUG_BITMAP(name_base, bit_count, bit_idx_min, chunk_list_base) debug_bitmap(name_base, bit_count, bit_idx_min, chunk_list_base)
  #define DEBUG_CALLOC_PARANOID(size) debug_calloc_paranoid(size)
  #define DEBUG_EXIT(status) debug_exit(status);
  #define DEBUG_F128(name_base, value) debug_f128(name_base, value);
  #define DEBUG_F128_PAIR(name_base, value0, value1) debug_f128_pair(name_base, value0, value1);
  #define DEBUG_F64(name_base, value) debug_f64(name_base, value);
  #define DEBUG_F64_PAIR(name_base, value0, value1) debug_f64_pair(name_base, value0, value1);
  #define DEBUG_FREE_PARANOID(base) debug_free_paranoid(base)
  #define DEBUG_LINE() debug_line(__LINE__)
  #define DEBUG_LIST(name_base, chunk_count, list_base, chunk_size_log2) debug_list(name_base, chunk_count, list_base, chunk_size_log2)
  #define DEBUG_LIST_CUSTOM(name_base, chunk_count, list_base, chunk_size_log2, compilable) debug_list_custom(name_base, chunk_count, list_base, chunk_size_log2, compilable)
  #define DEBUG_MALLOC_PARANOID(size) debug_malloc_paranoid(size)
  #define DEBUG_MEMORY_LEAK_REPORT(base) debug_memory_leak_report(base)
  #define DEBUG_NAME(name_base) debug_name(name_base)
  #define DEBUG_PRINT(string_base) debug_print(string_base)
  #define DEBUG_PRINT_FLUSH() debug_print_flush()
  #define DEBUG_PRINT_IF(status, string_base) debug_print_if(status, string_base)
  #define DEBUG_PRINT_NEWLINE_IF(string_base) debug_print_newline_if(char *name_base)
  #define DEBUG_PTR(name_base, base) debug_ptr(name_base, base)
  #define DEBUG_REALLOC_PARANOID(base, size) debug_realloc_paranoid(base, size)
  #define DEBUG_U128(name_base, value) debug_u128(name_base, value)
  #define DEBUG_U128_PAIR(name_base, value0, value1) debug_u128_pair(name_base, value0, value1)
  #define DEBUG_U16(name_base, value) debug_u16(name_base, value)
  #define DEBUG_U24(name_base, value) debug_u24(name_base, value)
  #define DEBUG_U32(name_base, value) debug_u32(name_base, value)
  #define DEBUG_U64(name_base, value) debug_u64(name_base, value)
  #define DEBUG_U64_PAIR(name_base, value0, value1) debug_u64_pair(name_base, value0, value1)
  #define DEBUG_U8(name_base, value) debug_u8(name_base, value)
#else
  #define DEBUG_ALLOCATION_CHECK()
  #define DEBUG_BIGUINT(name_base, chunk_idx_max, chunk_list_base)
  #define DEBUG_BITMAP(name_base, bit_count, bit_idx_min, chunk_list_base)
  #define DEBUG_CALLOC_PARANOID(size) calloc((size_t)(1), (size_t)(size))
  #define DEBUG_EXIT(status) exit(status);
  #define DEBUG_F64(name_base, value)
  #define DEBUG_F64_PAIR(name_base, value0, value1)
  #define DEBUG_F128(name_base, value)
  #define DEBUG_F128_PAIR(name_base, value0, value1)
  #define DEBUG_FREE_PARANOID(base) free(base)
  #define DEBUG_LINE()
  #define DEBUG_LIST(name_base, chunk_count, list_base, chunk_size_log2)
  #define DEBUG_LIST_CUSTOM(name_base, chunk_count, list_base, chunk_size_log2, compilable)
  #define DEBUG_MALLOC_PARANOID(size) malloc((size_t)(size))
  #define DEBUG_MEMORY_LEAK_REPORT(base)
  #define DEBUG_NAME(name_base) debug_name(name_base)
  #define DEBUG_PRINT(string_base)
  #define DEBUG_PRINT_FLUSH()
  #define DEBUG_PRINT_IF(status, string_base)
  #define DEBUG_PRINT_NEWLINE_IF(string_base)
  #define DEBUG_PTR(name_base, value)
  #define DEBUG_REALLOC_PARANOID(base, size) realloc(base, (size_t)(size))
  #define DEBUG_U128(name_base, value)
  #define DEBUG_U128_PAIR(name_base, value0, value1)
  #define DEBUG_U16(name_base, value)
  #define DEBUG_U24(name_base, value)
  #define DEBUG_U32(name_base, value)
  #define DEBUG_U64(name_base, value)
  #define DEBUG_U64_PAIR(name_base, value0, value1)
  #define DEBUG_U8(name_base, value)
#endif
