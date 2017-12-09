/*
Leidich Message Digest
http://leidich-message-digest.blogspot.com
Copyright 2013-2014 Russell Leidich
March 10, 2014

This file is part of the Leidich Message Digest Library.

The Leidich Message Digest Library is free software: you can redistribute it
and/or modify it under the terms of the GNU Limited General Public License as
published by the Free Software Foundation, version 3.

The Leidich Message Digest Library is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Limited
General Public License version 3 for more details.

You should have received a copy of the GNU Limited General Public License
version 3 along with the Leidich Message Digest Library (filename "COPYING").
If not, see http://www.gnu.org/licenses/ .
*/
/*
Don't do this if you just want to generate pseudorandom numbers.
*/
#define LMD_ACCUMULATE(integer_u32,x_u32,lmd_u64) \
  lmd_u64+=(u64)(x_u32)*integer_u32;

/*
Don't do this if you just want to generate pseudorandom numbers.
*/
#define LMD_ACCUMULATOR_INIT(lmd_u64) \
  lmd_u64=0;

/*
Don't do this if you just want to generate pseudorandom numbers.
*/
#define LMD_FINALIZE(a_const_u32,c_u32,x_u32,iterand_u64,lmd_u64) \
  lmd_u64+=iterand_u64; \
  x_u32=(u32)(lmd_u64); \
  c_u32=(u32)(lmd_u64>>U32_BITS); \
  LMD_ITERATE_NO_ZERO_CHECK(a_const_u32,c_u32,x_u32,iterand_u64) \
  LMD_ITERATE_NO_ZERO_CHECK(a_const_u32,c_u32,x_u32,iterand_u64) \
  LMD_ITERATE_NO_ZERO_CHECK(a_const_u32,c_u32,x_u32,iterand_u64) \
  lmd_u64+=iterand_u64;
/*
Do this second. Don't do this if you just want to generate pseudorandom numbers, or taking the LMD of a string that can't be null. Otherwise, this is required because LMD_FINALIZE() expects iterand_u64 to have been initialized.
*/
#define LMD_ITERAND_INIT(c_u32,x_u32,iterand_u64) \
  iterand_u64=((u64)(c_u32)<<U32_BITS)|x_u32;

/*
Produces pseudorandom x_u32 on [0,U32_MAX].
*/
#define LMD_ITERATE_NO_ZERO_CHECK(a_const_u32,c_u32,x_u32,iterand_u64) \
  iterand_u64=((u64)(a_const_u32)*x_u32)+c_u32; \
  x_u32=(u32)(iterand_u64); \
  c_u32=(u32)(iterand_u64>>U32_BITS);

/*
Produces pseudorandom x_u32 on [1,U32_MAX].
*/
#define LMD_ITERATE_WITH_ZERO_CHECK(a_const_u32,c_u32,x_u32,iterand_u64) \
  do{ \
    LMD_ITERATE_NO_ZERO_CHECK(a_const_u32,c_u32,x_u32,iterand_u64) \
  }while(x_u32==0);

/*
Do this first, unless (c,x) has already been initialized.
*/
#define LMD_SEED_INIT(c0_const_u32,c_u32,x0_const_u32,x_u32) \
  c_u32=c0_const_u32; \
  x_u32=x0_const_u32;

/*
LMD2 is designed specifically for fast multiplication:

  LMD2_A*x==(x<<32)+(x<<12)-(x<<25)

The initial seeds provide 100% coverage against double-bit errors for up to 1,055,348 bytes -- just over 1MiB, and corresponding to the first 263,837 iteration outputs -- wherein each error occurs in the protected message (and not in the LMD2 itself, which is rare in the long message limit).

The number of nonzero DWORDs before (x == 0) appears is 11,460,787,448, excluding the seeds themselves, which implies a maximum string size of 45,843,149,792 bytes before ITERATE_WITH_ZERO_CHECK must be used.

If you're more concerned about generating long lists of nonzero X values, so you can then use LMD_ITERATE_NO_ZERO_CHECK for "small" files for speed, then use LMD3 instead.
*/
#define LMD_STRING_LMD2_GET(string_base,string_idx_min,string_size) (lmd_string_lmd2_custom_get(LMD2_C0,LMD2_X0,string_base,string_idx_min,string_size))
#define LMD_STRING_LMD3_GET(string_base,string_idx_min,string_size) (lmd_string_lmd2_custom_get(LMD3_C0,LMD3_X0,string_base,string_idx_min,string_size))
#define LMD2_A 0xFE001000UL
#define LMD2_C0 0xC97A34B3UL
#define LMD2_X0 0x129E5CFAUL

/*
LMD3 uses LMD2_A for fast multiplication, but is otherwise designed to produce a long initial cycle of nonzero values, specifically 49,327,206,862 of them (197,308,827,448 bytes' worth).

If you're more concerned about detecting accidental errors in blocks under 1MiB in size, then use LMD2 instead.
*/
#define LMD3_C0 0xDA6D32BAUL
#define LMD3_X0 0
