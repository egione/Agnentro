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
#define BIGUINT_CANONIZE(chunk_idx_max, chunk_list_base) \
  while((!chunk_list_base[chunk_idx_max])&&chunk_idx_max){ \
    chunk_idx_max--; \
  }
#define BIGUINT_COMPARE_EQUAL 0
#define BIGUINT_COMPARE_LESS 1
#define BIGUINT_COMPARE_GREATER 2
#define BIGUINT_IS_NOT_ZERO(chunk_idx_max, chunk_list_base) ((chunk_list_base)[chunk_idx_max]||chunk_idx_max)
#define BIGUINT_IS_ZERO(chunk_idx_max, chunk_list_base) (!BIGUINT_IS_NOT_ZERO(chunk_idx_max, chunk_list_base))
#define BIGUINT_SET_ULONG(chunk_idx_max, chunk_list_base, uint) \
  chunk_idx_max=0; \
  (chunk_list_base)[0]=uint
#define BIGUINT_SET_ZERO(chunk_idx_max, chunk_list_base) \
  chunk_idx_max=0; \
  (chunk_list_base)[0]=0
