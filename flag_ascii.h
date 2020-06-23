/*
ASCII
Copyright 2017 Russell Leidich

This collection of files constitutes the ASCII Library. (This is a
library in the abstact sense; it's not intended to compile to a ".lib"
file.)

The ASCII Library is free software: you can redistribute it and/or
modify it under the terms of the GNU Limited General Public License as
published by the Free Software Foundation, version 3.

The ASCII Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
Limited General Public License version 3 for more details.

You should have received a copy of the GNU Limited General Public
License version 3 along with the ASCII Library (filename
"COPYING"). If not, see http://www.gnu.org/licenses/ .
*/
/*
ASCII Version Info
*/
/*
BUILD_BREAK_COUNT must increase each time either: (1) a change is made which is not backward-compatible, (2) a bug is fixed which corrects a potential security vulnerability or behavioral error other than user interface aesthetics or slow performance, or (3) critical documentation or comment updates have been made without which the foregoing would not be evident to the user.

This value is set to the sum of breakage events in the source code of this library, which is hardcoded as a constant, plus the same sum pertaining to the libraries upon which its correct execution depends. If such a library is removed, then said constant must increase by enough to make the new sum either equal to or greater than its prior value, depending upon whether the removal was coincident with a breakage event, or not, respectively.

When calling the initialization code for this library, always use the corresponding constant ending in "_EXPECTED". This will force a runtime error, in addition to a build error below, if the programmer fails to notice subordinate build breakages which may affect the correctness of transactions with this library.
*/
#define ASCII_BUILD_BREAK_COUNT 0
#define ASCII_BUILD_BREAK_COUNT_EXPECTED 0
#if ASCII_BUILD_BREAK_COUNT!=ASCII_BUILD_BREAK_COUNT_EXPECTED
  #error ASCII is unaware of the latest non-backward-compatible changes to the libraries that it uses.
#endif
/*
BUILD_FEATURE_COUNT must increase each time a feature is added for which callers should be able to query via initialization code. This could even include performance enhancements.

This value is set to the number of features added to the source code of this library, which is hardcoded as a constant, plus the same sum pertaining to the libraries upon which it depends. If such a library is removed, then said constant must increase by enough to make the new sum either equal to or greater than its prior value, depending upon whether the removal was coincident with a feature addition, or not, respectively.

This value should NOT be used by any library other than this one; doing so could require this library to support more features than the caller actually needs. Instead, source code should hardcode a constant parameter to the call to the initialization code (for this library) which conveys the minimum required value of this count.
*/
#define ASCII_BUILD_FEATURE_COUNT 1
/*
BUILD_ID must increase with every code release, even if only a comment changes. It allows the user to discern one build from another, but is invisible to other libraries. If this library depends on other libraries, then it should be expressed as the sum of the local build ID plus the build IDs of those libraries, and in any event must increase monotonically even if some of the latter are removed.
*/
#define ASCII_BUILD_ID 5
