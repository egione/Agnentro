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
TYPEDEF_START
  fru128 entropy;
  fru128 ld_coeff;
  fru128 mask_span_log;
  fru128 sum_quartics;
  fru128 sum_squares;
  u128 haystack_mask_count_recip_half;
  u128 log2_recip_half;
  u128 mean_f128;
  u128 mean_unsigned;
  u128 needle_mask_count_recip_half;
  ULONG *freq_list_base0;
  ULONG *freq_list_base1;
  fru64 *log_delta_list_base;
  u64 *log_delta_parameter_list_base;
  fru64 *log_list_base;
  u64 *log_parameter_list_base;
  fru128 *log_u128_list_base;
  u128 *log_u128_parameter_list_base;
  loggamma_t *loggamma_base;
  fru128 *loggamma_list_base;
  u64 *loggamma_parameter_list_base;
  poissocache_t *poissocache_base;
  ULONG log_delta_idx_max;
  ULONG log_idx_max;
  ULONG log_u128_idx_max;
  ULONG loggamma_idx_max;
  ULONG mask_count0;
  ULONG mask_count1;
  u32 mask_max;
  u32 mask_sign_mask;
  u8 delta_status;
  u8 ignored_status;
  u8 mask_max_msb;
  u8 granularity;
  u8 mean_shift;
  u8 overlap_status;
  u8 sign_status;
  u8 sweep_mask_idx_max_bit_count;
  u8 variance_shift;
TYPEDEF_END(agnentroprox_t)

#define AGNENTROPROX_LOG2_RECIP_HALF_HI 0xB8AA3B295C17F0BBULL
#define AGNENTROPROX_LOG2_RECIP_HALF_LO 0xBE87FED0691D3E88ULL
#define AGNENTROPROX_MODE_AGNENTROPY (1U<<AGNENTROPROX_MODE_AGNENTROPY_BIT_IDX)
#define AGNENTROPROX_MODE_AGNENTROPY_BIT_IDX 0U
#define AGNENTROPROX_MODE_DIVENTROPY (1U<<AGNENTROPROX_MODE_DIVENTROPY_BIT_IDX)
#define AGNENTROPROX_MODE_DIVENTROPY_BIT_IDX 1U
#define AGNENTROPROX_MODE_EXOELASTICITY (1U<<AGNENTROPROX_MODE_EXOELASTICITY_BIT_IDX)
#define AGNENTROPROX_MODE_EXOELASTICITY_BIT_IDX 2U
#define AGNENTROPROX_MODE_EXOENTROPY (1U<<AGNENTROPROX_MODE_EXOENTROPY_BIT_IDX)
#define AGNENTROPROX_MODE_EXOENTROPY_BIT_IDX 3U
#define AGNENTROPROX_MODE_JSDT (1U<<AGNENTROPROX_MODE_JSDT_BIT_IDX)
#define AGNENTROPROX_MODE_JSDT_BIT_IDX 8U
#define AGNENTROPROX_MODE_JSET (1U<<AGNENTROPROX_MODE_JSET_BIT_IDX)
#define AGNENTROPROX_MODE_JSET_BIT_IDX 9U
#define AGNENTROPROX_MODE_KURTOSIS (1U<<AGNENTROPROX_MODE_KURTOSIS_BIT_IDX)
#define AGNENTROPROX_MODE_KURTOSIS_BIT_IDX 4U
#define AGNENTROPROX_MODE_LDT (1U<<AGNENTROPROX_MODE_LDT_BIT_IDX)
#define AGNENTROPROX_MODE_LDT_BIT_IDX 10U
#define AGNENTROPROX_MODE_LET (1U<<AGNENTROPROX_MODE_LET_BIT_IDX)
#define AGNENTROPROX_MODE_LET_BIT_IDX 11U
#define AGNENTROPROX_MODE_LOGFREEDOM (1U<<AGNENTROPROX_MODE_LOGFREEDOM_BIT_IDX)
#define AGNENTROPROX_MODE_LOGFREEDOM_BIT_IDX 5U
#define AGNENTROPROX_MODE_SHANNON (1U<<AGNENTROPROX_MODE_SHANNON_BIT_IDX)
#define AGNENTROPROX_MODE_SHANNON_BIT_IDX 6U
#define AGNENTROPROX_MODE_VARIANCE (1U<<AGNENTROPROX_MODE_VARIANCE_BIT_IDX)
#define AGNENTROPROX_MODE_VARIANCE_BIT_IDX 7U
