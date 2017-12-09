/*
Zorb
Copyright 2017 Russell Leidich

This collection of files constitutes the Zorb Library. (This is a
library in the abstact sense; it's not intended to compile to a ".lib"
file.)

The Zorb Library is free software: you can redistribute it and/or
modify it under the terms of the GNU Limited General Public License as
published by the Free Software Foundation, version 3.

The Zorb Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
Limited General Public License version 3 for more details.

You should have received a copy of the GNU Limited General Public
License version 3 along with the Zorb Library (filename
"COPYING"). If not, see http://www.gnu.org/licenses/ .
*/
TYPEDEF_START
  u64 signature;
  u64 lmd2_following;
  u64 mask_idx_max;
  u32 mask_max;
  u32 zero;
  u64 freq_list[];
TYPEDEF_END(zorb_t)

#define ZORB_SIGNATURE 0x5BC4035C4EB46DB2ULL
