/*
Filesys
Copyright 2017 Russell Leidich

This collection of files constitutes the Filesys Library. (This is a
library in the abstact sense; it's not intended to compile to a ".lib"
file.)

The Filesys Library is free software: you can redistribute it and/or
modify it under the terms of the GNU Limited General Public License as
published by the Free Software Foundation, version 3.

The Filesys Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
Limited General Public License version 3 for more details.

You should have received a copy of the GNU Limited General Public
License version 3 along with the Filesys Library (filename
"COPYING"). If not, see http://www.gnu.org/licenses/ .
*/
#define FILESYS_DIRECTORY_DEPTH_IDX_MAX 0xFEU
#define FILESYS_PATHNAME_CHAR_IDX_MAX 0xFFFFU
#define FILESYS_STATUS_OK 0U
#define FILESYS_STATUS_NOT_FOUND 1U
#define FILESYS_STATUS_TOO_BIG 2U
#define FILESYS_STATUS_READ_FAIL 3U
#define FILESYS_STATUS_SIZE_CHANGED 4U
#define FILESYS_STATUS_WRITE_FAIL 5U
#define FILESYS_STATUS_CALLER_CUSTOM 6U
#define FILESYS_STATUS_CALLER_CUSTOM2 7U

#ifdef WINDOWS
  #define FILESYS_FSEEKO fseeko64
  #define FILESYS_FTELLO ftello64
  #ifdef __MINGW32__
    #include <_mingw.h>
    #ifdef __MINGW64_VERSION_MAJOR
      #define FILESYS_PATH_SEPARATOR 0x2FU
    #else
      #define FILESYS_PATH_SEPARATOR 0x5CU
    #endif
  #endif
#else
  #define FILESYS_FSEEKO fseeko
  #define FILESYS_FTELLO ftello
  #define FILESYS_PATH_SEPARATOR 0x2FU
#endif
