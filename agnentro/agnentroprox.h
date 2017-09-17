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
  fru128 haystack_implied_entropy;
  fru128 haystack_shannon_entropy;
  fru128 jsd_coeff;
  fru128 *loggamma_list_base;
  fru128 mask_span_log;
  fru128 needle_implied_entropy;
  fru128 needle_shannon_entropy;
  fru128 normalizer;
  fru128 sum_quartics;
  fru128 sum_squares;
  u128 mean_f128;
  u128 mean_unsigned;
  fru64 *log_delta_list_base;
  fru64 *log_list_base;
  ULONG *freq_list_base0;
  ULONG *freq_list_base1;
  ULONG *freq_list_base2;
  loggamma_t *loggamma_base;
  poissocache_t *poissocache_base;
  ULONG log_delta_idx_max;
  ULONG log_delta_idx_max_max;
  ULONG log_idx_max;
  ULONG log_idx_max_max;
  ULONG loggamma_idx_max;
  ULONG loggamma_idx_max_max;
  ULONG mask_count0;
  ULONG mask_count1;
  ULONG mask_count2;
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
#define AGNENTROPROX_MODE_AGNENTROPY (1U<<0)
#define AGNENTROPROX_MODE_DIVENTROPY (1U<<1)
#define AGNENTROPROX_MODE_EXOELASTICITY (1U<<2)
#define AGNENTROPROX_MODE_EXOENTROPY (1U<<3)
#define AGNENTROPROX_MODE_JSD (1U<<8)
#define AGNENTROPROX_MODE_KURTOSIS (1U<<4)
#define AGNENTROPROX_MODE_LOGFREEDOM (1U<<5)
#define AGNENTROPROX_MODE_SHANNON (1U<<6)
#define AGNENTROPROX_MODE_VARIANCE (1U<<7)
