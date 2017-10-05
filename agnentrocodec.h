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
License version 3 along with the Agnentro Library (haystack_filename
"COPYING"). If not, see http://www.gnu.org/licenses/ .
*/
TYPEDEF_START
  ULONG *code_chunk_list_base;
  ULONG freq_idx_min_list_base[U32_BITS];
  ULONG *freq_tree_base;
  ULONG *pochhammer_chunk_list_base;
  ULONG *remainder_chunk_list_base;
  ULONG *span_chunk_list_base;
  ULONG *term_chunk_list_base;
  u64 code_bit_idx_max;
  u64 code_bit_idx_max_max;
  ULONG code_chunk_idx_max;
  ULONG span_chunk_idx_max;
  u32 mask_max;
  u8 granularity;
TYPEDEF_END(agnentrocodec_t)
