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
extern const char *loggamma_hex_base_list_base[(LOGGAMMA_COEFF_IDX_MAX+1)<<1];
extern void *loggamma_free(void *base);
extern loggamma_t *loggamma_free_all(loggamma_t *loggamma_base);
extern loggamma_t *loggamma_init(u32 build_break_count, u32 build_feature_count);
extern u8 loggamma_u64(fru128 *a_base, loggamma_t *loggamma_base, u64 v);
extern u64 *loggamma_u64_cache_init(ULONG loggamma_idx_max, fru128 **loggamma_list_base_base);
extern u8 loggamma_u64_cached(fru128 *a_base, loggamma_t *loggamma_base, ULONG loggamma_idx_max, fru128 *loggamma_list_base, u64 *loggamma_parameter_list_base, u64 v);
