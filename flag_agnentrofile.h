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
/*
Agnentro File Version Info
*/
/*
BUILD_ID must increase with every code release, even if only a comment changes. It allows the user to discern one build from another, but is invisible to other libraries. If this library depends on other libraries, then it should be expressed as the sum of the local build ID plus the build IDs of those libraries, and in any event must increase monotonically even if some of the latter are removed.
*/
#define AGNENTROFILE_BUILD_ID (5+AGNENTROCODEC_BUILD_ID+AGNENTROPROX_BUILD_ID+ASCII_BUILD_ID+BIGUINT_BUILD_ID+FILESYS_BUILD_ID+FRU128_BUILD_ID+FRU64_BUILD_ID+LOGGAMMA_BUILD_ID)
