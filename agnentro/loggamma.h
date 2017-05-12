/*
Loggamma
Copyright 2017 Russell Leidich

This collection of files constitutes the Loggamma Library. (This is a
library in the abstact sense; it's not intended to compile to a ".lib"
file.)

The Loggamma Library is free software: you can redistribute it and/or
modify it under the terms of the GNU Limited General Public License as
published by the Free Software Foundation, version 3.

The Loggamma Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
Limited General Public License version 3 for more details.

You should have received a copy of the GNU Limited General Public
License version 3 along with the Loggamma Library (filename
"COPYING"). If not, see http://www.gnu.org/licenses/ .
*/
/*
Macro naming and variable naming conventions are as described in fracterval_u128.h.
*/
#define LOGGAMMA_03_HI 0x00U
#define LOGGAMMA_03_LO 0xB17217F7D1CF79ABULL
#define LOGGAMMA_04_HI 0x01U
#define LOGGAMMA_04_LO 0xCAB0BFA2A2002322ULL
#define LOGGAMMA_05_HI 0x03U
#define LOGGAMMA_05_LO 0x2D94EF92459F167AULL
#define LOGGAMMA_06_HI 0x04U
#define LOGGAMMA_06_LO 0xC9990F111E724D29ULL
#define LOGGAMMA_07_HI 0x06U
#define LOGGAMMA_07_LO 0x9449CEB3C072704CULL
#define LOGGAMMA_08_HI 0x08U
#define LOGGAMMA_08_LO 0x8670F996E617E593ULL
#define LOGGAMMA_09_HI 0x0AU
#define LOGGAMMA_09_LO 0x9AC7417E5B865296ULL
#define LOGGAMMA_0A_HI 0x0CU
#define LOGGAMMA_0A_LO 0xCD4490D3FBE7A583ULL
#define LOGGAMMA_0B_HI 0x0FU
#define LOGGAMMA_0B_LO 0x1ABAC84AA68A55DFULL
#define LOGGAMMA_0C_HI 0x11U
#define LOGGAMMA_0C_LO 0x80973F3A8D73CCD7ULL
#define LOGGAMMA_0D_HI 0x13U
#define LOGGAMMA_0D_LO 0xFCBA16D5014369A5ULL
#define LOGGAMMA_0E_HI 0x16U
#define LOGGAMMA_0E_LO 0x8D5A9C3B32CD8976ULL
#define LOGGAMMA_0F_HI 0x19U
#define LOGGAMMA_0F_LO 0x30F3DF162A427868ULL
#define LOGGAMMA_10_HI 0x1BU
#define LOGGAMMA_10_LO 0xE636A63FD346588FULL
#define LOGGAMMA_11_HI 0x1EU
#define LOGGAMMA_11_LO 0xABFF061F1A843F3EULL
#define LOGGAMMA_12_HI 0x21U
#define LOGGAMMA_12_LO 0x814C7E5E6A73791EULL
#define LOGGAMMA_13_HI 0x24U
#define LOGGAMMA_13_LO 0x653BE5ABDCA445B7ULL
#define LOGGAMMA_14_HI 0x27U
#define LOGGAMMA_14_LO 0x5702A66C7309D3F7ULL
#define LOGGAMMA_15_HI 0x2AU
#define LOGGAMMA_15_LO 0x55EAF5DAEF7BFDFFULL
#define LOGGAMMA_16_HI 0x2DU
#define LOGGAMMA_16_LO 0x6150C868E5521CBCULL
#define LOGGAMMA_17_HI 0x30U
#define LOGGAMMA_17_LO 0x789F57509E0B0D5FULL
#define LOGGAMMA_18_HI 0x33U
#define LOGGAMMA_18_LO 0x9B4F170AD4890E82ULL
#define LOGGAMMA_19_HI 0x36U
#define LOGGAMMA_19_LO 0xC8E4069D1A2824FCULL
#define LOGGAMMA_1A_HI 0x3AU
#define LOGGAMMA_1A_LO 0x00EC459ACBCE925CULL
#define LOGGAMMA_1B_HI 0x3DU
#define LOGGAMMA_1B_LO 0x42FEE2F8CF282BD9ULL
#define LOGGAMMA_1C_HI 0x40U
#define LOGGAMMA_1C_LO 0x8EBAD9F93FBA283CULL
#define LOGGAMMA_1D_HI 0x43U
#define LOGGAMMA_1D_LO 0xE3C634CC08FE90DBULL
#define LOGGAMMA_1E_HI 0x47U
#define LOGGAMMA_1E_LO 0x41CD4E45C8C26888ULL
#define LOGGAMMA_1F_HI 0x4AU
#define LOGGAMMA_1F_LO 0xA8822D674395C25AULL
#define LOGGAMMA_COEFF_IDX_MAX 26

#define LOGGAMMA_FROM_U64(_a, _m, _v, _z) \
  _z=(u8)(_z|loggamma_from_u64(&_a, _m, _v));

#define LOGGAMMA_FROM_U64_CACHED(_a, _m, _l0, _l1, _k, _v, _z) \
  _z=(u8)(_z|loggamma_from_u64_cached(&_a, _m, &_l0, _l1, _k, _v))

#define LOGGAMMA_FROM_U64_CACHED_UNSAFE(_a, _m, _l0, _l1, _k, _v) \
  do{ \
    u8 _s; \
    \
    _s=1; \
    if((_v)<=(_l0)){ \
      _a=(_k)[_v]; \
      _s=U128_IS_ONES(_a0); \
    } \
    if(_s){ \
      loggamma_from_u64_cached(&_a, _m, &_l0, _l1, _k, _v); \
    } \
  }while(0)

#define LOGGAMMA_LOG_2PI_HALF 0xEB3F8E4325F5A534ULL
#define LOGGAMMA_PARAMETER_MAX 0x64BF406864B4305ULL

TYPEDEF_START
  ULONG *chunk_idx_max_list_base;
  ULONG **coeff_base_list_base;
  ULONG *temp_chunk_list_base0;
  ULONG *temp_chunk_list_base1;
  ULONG *temp_chunk_list_base2;
  ULONG *temp_chunk_list_base3;
  ULONG *temp_chunk_list_base4;
TYPEDEF_END(loggamma_t)
