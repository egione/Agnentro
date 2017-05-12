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
Compiler Control
*/
#if !(defined(_32_)||defined(_64_))
  #error "Use 'gcc -D_32_' or '-D_64_' for 32/64-bit memory indexes (ULONG)s."
#elif (defined(_32_)&&defined(_64_))
  #error "You have defined both _32_ and _64_. Choose one."
#endif
#if !(defined(DEBUG)||defined(DEBUG_OFF))
  #error "Use 'gcc -DDEBUG' for debugging or 'gcc -DDEBUG_OFF' for release."
#elif defined(DEBUG)&&defined(DEBUG_OFF)
  #error "You have defined both DEBUG and DEBUG_OFF. Choose one only."
#endif
#if defined(WINDOWS)
  #undef WINDOWS
#endif
#if defined(_WIN32)||defined(__WIN32__)||defined(_WIN64)||defined(__WIN64__)
  #define WINDOWS
#endif
