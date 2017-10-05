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
Agnentrocodec Version Info
*/
/*
BUILD_BREAK_COUNT increases each time either: (1) a change is made which is not backward compatible, (2) a bug is fixed which corrects a potential security vulnerability or behavioral error other than user interface aesthetics or slow performance, or (3) critical documentation or comment updates have been made without which the foregoing would not be evident to the user. If this library depends on other libraries, then it should be expressed as the sum of the local break count plus the break counts of the dependencies, and in any event must increase monotonically even if some of the latter are removed. Initialization code receiving a _different_ expected value from the caller must fail.
*/
#define AGNENTROCODEC_BUILD_BREAK_COUNT (0+BIGUINT_BUILD_BREAK_COUNT+FRU128_BUILD_BREAK_COUNT+LOGGAMMA_BUILD_BREAK_COUNT)
/*
BUILD_FEATURE_COUNT increases each time a feature is added for which callers should be able to query via initialization code. This could even include performance enhancements without which a caller might opt to require the user to upgrade. If this library depends on other libraries, then it should be expressed as the sum of the local feature count plus the feature counts of the dependencies, and in any event must increase monotonically even if some of the latter are removed. Initialization code receiving a _greater_ expected value from the caller must fail.
*/
#define AGNENTROCODEC_BUILD_FEATURE_COUNT (0+BIGUINT_BUILD_FEATURE_COUNT+FRU128_BUILD_FEATURE_COUNT+LOGGAMMA_BUILD_FEATURE_COUNT)
/*
BUILD_ID must increase with every code release, even if only a comment changes. It allows the user to discern one build from another, but is invisible to other libraries. If this library depends on other libraries, then it should be expressed as the sum of the local build ID plus the build IDs of the dependencies, and in any event must increase monotonically even if some of the latter are removed.
*/
#define AGNENTROCODEC_BUILD_ID (4+BIGUINT_BUILD_ID+FRU128_BUILD_ID+LOGGAMMA_BUILD_ID)
