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
ASCII and UTF8 Functions
*/
#include "flag.h"
#include "flag_ascii.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "constant.h"
#include "debug.h"
#include "debug_xtrn.h"
#include "ascii_xtrn.h"

u8
ascii_decimal_to_u64_convert(char *digit_list_base, u64 *integer_base, u64 integer_max){
/*
Convert an allegedly ASCII decimal string to a u64.

In:

  *digit_list_base points to an untrustued string hopefully containing ASCII digits 0 through 9. Null termination must be guaranteed to occur within contiguously readable space.

  integer_base of the base of a u64 to receive the binary value of *digit_list_base.

  integer_max is the maximum acceptable value of Out:*integer_base.

Out:

  Returns zero on success, else one.

  *integer_base zero on failure, else the binary value of *digit_list_base on [0, integer_max].
*/
  u8 digit;
  u32 digit_idx;
  u64 integer;
  u8 status;

  integer=0;
  *integer_base=integer;
  status=1;
  if(digit_list_base[0]){
    digit_idx=0;
    do{
      digit=(u8)(digit_list_base[digit_idx]);
      digit_idx++;
      if(digit){
        digit=(u8)(digit-'0');
        if(digit<=9){
          if((U64_MAX/10)<=integer){
            if((U64_MAX/10)==integer){
              if(5<digit){
                break;
              }
            }else{
              break;
            }
          }
          integer=(integer*10)+digit;
        }else{
          break;
        }
      }else{
        if(integer<=integer_max){
          status=0;
          *integer_base=integer;
        }
        break;
      }
    }while(1);
  }
  return status;  
}

u8
ascii_decimal_to_u64_convert_negatable(char *digit_list_base, u64 *integer_base, u64 integer_max, u8 *sign_base){
/*
Convert an allegedly ASCII decimal string to a u64. If it's prefixed with "-", then indicate that it has negative sign.

In:

  *digit_list_base points to an untrustued string hopefully containing ASCII digits 0 through 9, potentially prefixed with a single "-" (but not "+") to indicate negativity. Null termination must be guaranteed to occur within contiguously readable space.

  integer_base of the base of a u64 to receive the binary value of *digit_list_base.

  integer_max is the maximum acceptable value of Out:*integer_base.

  *sign_base is undefined.

Out:

  Returns zero on success, else one.

  *integer_base zero on failure, else the binary value of *digit_list_base on [0, integer_max].

  *sign_base is one if the return value is zero and *digit_list_base was preceded by a single "-", even if it's "-0", "-00", etc.; else zero.
*/
  u8 digit;
  u32 digit_idx;
  u64 integer;
  u8 sign;
  u8 status;

  integer=0;
  sign=0;
  status=1;
  *integer_base=integer;
  if(digit_list_base[0]){
    digit_idx=0;
    do{
      digit=(u8)(digit_list_base[digit_idx]);
      digit_idx++;
      if(digit){
        digit=(u8)(digit-'0');
        if(digit<=9){
          if((U64_MAX/10)<=integer){
            if((U64_MAX/10)==integer){
              if(5<digit){
                break;
              }
            }else{
              break;
            }
          }
          integer=(integer*10)+digit;
        }else if(digit==(u8)('-'-'0')){
          if((digit_idx==1)&&(!sign)){
            sign=1;
          }else{
            break;
          }
        }else{
          break;
        }
      }else{
        if(integer<=integer_max){
/*
Check for "-" alone, which is invalid, so we must return (sign==0) according to Out.
*/
          if((sign==1)&&(digit_idx==2)){
            sign=0;
          }else{
            status=0;
            *integer_base=integer;
          }
        }
        break;
      }
    }while(1);
  }
  *sign_base=sign;
  return status;  
}

void *
ascii_free(void *base){
/*
To maximize portability and debuggability, this is the only function in which ASCII calls free().

In:

  base is the base of a memory region to free. May be NULL.

Out:

  Returns NULL so that the caller can easily maintain the good practice of NULLing out invalid pointers.

  *base is freed.
*/
  DEBUG_FREE_PARANOID(base);
  return NULL;
}

u8
ascii_hex_to_u64_convert(char *digit_list_base, u64 *integer_base, u64 integer_max){
/*
Convert an allegedly ASCII hexadecimal string to a u64.

In:

  *digit_list_base points to an untrustued string hopefully containing ASCII digits 0 through 9, "A" through "F", or "a" through "f". Null termination must be guaranteed to occur within contiguously readable space.

  integer_base of the base of a u64 to receive the binary value of *digit_list_base.

  integer_max is the maximum acceptable value of Out:*integer_base.

Out:

  Returns zero on success, else one.

  *integer_base zero on failure, else the binary value of *digit_list_base on [0, integer_max].
*/
  u8 digit;
  u32 digit_idx;
  u64 integer;
  u8 status;

  integer=0;
  *integer_base=integer;
  status=1;
  if(digit_list_base[0]){
    digit_idx=0;
    do{
      digit=(u8)(digit_list_base[digit_idx]);
      digit_idx++;
      if(digit){
        digit=(u8)(digit-'0');
        if(9<digit){
          digit=(u8)(((u8)(digit+'0')|('a'-'A'))-'a');
          if(5<digit){
            break;
          }
          digit=(u8)(digit+0xA);
        }
        if(digit_idx<=(U64_BITS>>2)){
          integer=(integer<<4)|digit;
        }else{
          break;
        }
      }else{
        if(integer<=integer_max){
          status=0;
          *integer_base=integer;
        }
        break;
      }
    }while(1);
  }
  return status;  
}

u8
ascii_hex_to_u8_list_convert(ULONG digit_idx_max, ULONG digit_idx_min, char *digit_list_base, u8 *u8_list_base){
/*
Convert an allegedly ASCII hexadecimal string to a list of (u8)s.

In:

  digit_idx_max is the maximum index of *digit_list_base which does not contain null.

  digit_idx_min is the index of *digit_list_base at which to start reading, on [0, digit_idx_max].
  
  *digit_list_base points to an untrustued string hopefully containing ASCII digits 0 through 9, "A" through "F", or "a" through "f". Null termination must be guaranteed to occur within contiguously readable space.

  u8_list_base is the base at which to store (u8)s converted from the ASCII hex values. It must be writable for (((digit_idx_max-digit_idx_min)>>1)+1) items.

Out:

  Returns zero on success, else one on failure due to an odd number of nybbles, else 2.

  *u8_list_base is unchanged on failure, else populated with the byte equivalent of *digit_list_base, ordered such that if (*digit_list_base=="12aB") then (u8_list_base[0]==0x12) and (u8_list_base[1]==0xAB).
*/
  u8 byte;
  u8 digit;
  ULONG digit_idx;
  u8 status;
  ULONG u8_idx;

  status=1;
  status=(u8)(((digit_idx_max-digit_idx_min)&status)^status);
  if(!status){
    digit_idx=digit_idx_min;
    status=2;
    do{
      digit=(u8)(digit_list_base[digit_idx]);
      digit=(u8)(digit-'0');
      if(9<digit){
        digit=(u8)(((u8)(digit+'0')|('a'-'A'))-'a');
        if(5<digit){
          break;
        }
        digit=(u8)(digit+0xA);
      }
      digit_idx++;
    }while(digit_idx<=digit_idx_max);
    if(digit_idx_max<digit_idx){
      digit_idx=digit_idx_min;
      status=0;
      u8_idx=0;
      do{
        digit=(u8)(digit_list_base[digit_idx]);
        digit=(u8)(digit-'0');
        digit_idx++;
        if(9<digit){
          digit=(u8)(((u8)(digit+'0')|('a'-'A'))-('a'-0xA));
        }
        byte=(u8)(digit<<4);
        digit=(u8)(digit_list_base[digit_idx]);
        digit=(u8)(digit-'0');
        digit_idx++;
        if(9<digit){
          digit=(u8)(((u8)(digit+'0')|('a'-'A'))-('a'-0xA));
        }
        byte|=digit;
        u8_list_base[u8_idx]=byte;
        u8_idx++;
      }while(digit_idx<=digit_idx_max);        
    }
  }
  return status;  
}

u8
ascii_init(u32 build_break_count, u32 build_feature_count){
/*
Verify that the source code is sufficiently updated.

In:

  build_break_count is the caller's most recent knowledge of ASCII_BUILD_BREAK_COUNT, which will fail if the caller is unaware of all critical updates.

  build_feature_count is the caller's most recent knowledge of ASCII_BUILD_FEATURE_COUNT, which will fail if this library is not up to date with the caller's expectations.

Out:

  Returns one if (build_break_count!=ASCII_BUILD_BREAK_COUNT) or (build_feature_count>ASCII_BUILD_FEATURE_COUNT). Otherwise, returns zero.
*/
  u8 status;

  status=(u8)(build_break_count!=ASCII_BUILD_BREAK_COUNT);
  status=(u8)(status|(ASCII_BUILD_FEATURE_COUNT<build_feature_count));
  return status;
}

char *
ascii_utf8_safe_list_malloc(ULONG unsafe_idx_max){
/*
Allocate sufficient space for the maximum possible translated size of an invalid UTF8 string.

In:

  unsafe_idx_max is one less than (the size of the unsafe string including its terminating zero, if any). (This is a byte index, not a UTF8 index.)

Out:

  Returns NULL on failure, else the base of ((((unsafe_idx_max-unsafe_idx_min)+1)*3)+1) undefined bytes, which will suffice to store the entire translation of the unsafe string, including its terminating null, if any, and regardless of whether or not it contains any valid UTF8.
*/
  ULONG list_size;
  char *safe_list_base;
/*
The maximum expansion factor is 3 on account of the size of the replacement character. But we need to add one for the terminating zero in case the unsafe string doesn't have one.
*/
  list_size=((unsafe_idx_max+1)*3)+1;
  safe_list_base=NULL;
  if((((list_size-1)/3)-1)==unsafe_idx_max){
    safe_list_base=DEBUG_MALLOC_PARANOID(list_size);
  }
  return safe_list_base;
}

ULONG
ascii_utf8_sanitize(u8 eager_status, char *safe_list_base, ULONG unsafe_idx_max, ULONG unsafe_idx_min, char *unsafe_list_base, ULONG *utf8_idx_max_base){
/*
Translate an alleged UTF8 string into a similar string which is safe for display. This entails the replacement of undefined UTF8, null zero, alternative null (UTF8(0xC0, 0x80)), and Controls C1 (UTF8(0xC2, [0x80, 0x9F])) with the replacement character (UTF8(0xEF, 0xBF, 0xBD)).

In:

  eager_status is one to allow storage the replacement character to *safe_list_base if the first UTF8 code point at unsafe_list_base is invalid, else zero to wait until after the first valid code point is copied to *safe_list_base, if ever. In the former case, the number of code points at *safe_list_base may vary as unsafe_idx_min is decremented sequentially; in the latter, the number of code points is guaranteed not to decrease, all other inputs being equal.

  *safe_list_base is undefined and writable for ((((unsafe_idx_max-unsafe_idx_min)+1)*3)+1) bytes; use ascii_utf8_safe_list_malloc() to allocate it properly.

  unsafe_idx_max is the maximum index of *unsafe_list_base at which to translate. (This is a byte index, not a UTF8 index.) For null-terminated strings, it should equal the index of the null character, i.e. the output of strlen(); otherwise the null will be replaced by a replacement character at *safe_list_base.

  unsafe_idx_min is the index of *unsafe_list_base at which to begin translation, which may fall in the middle of a code point. (This is a byte index, not a UTF8 index.)

  *unsafe_list_base is allegedly valid UTF8.

  *utf_count_base is undefined.

Out:

  Returns the number of bytes at *safe_list_base excluding the terminating null, which should equal strlen(safe_list_base).

  *safe_list_base is a UTF8 string as consistent as possible with the input region of unsafe_list_base. Characters below the space (0x20), including null zero, have been translated to the replacement character, as have all invalid code points at unsafe_list_base. Invalid code points are terminated as soon as they are determined to be so, such that translation restarts with the next byte. *safe_list_base is valid according to the chart at https://en.wikipedia.org/wiki/UTF-8 . safe_list_base[return value] is zero for the purpose of null termination.

  *utf8_idx_max_base is the number of code points at *safe_list_base, excluding any terminating null, on [0, ((unsafe_idx_max-unsafe_idx_min)+1)].
*/
  u8 byte0;
  u8 byte1;
  u8 byte2;
  u8 byte3;
  ULONG safe_idx;
  ULONG safe_size;
  ULONG unsafe_idx;
  ULONG unsafe_size_minus_1;
  ULONG utf8_idx_max;
  u8 valid_status;

  byte0=0;
  byte1=0;
  byte2=0;
  byte3=0;
  safe_idx=0;
  unsafe_idx=unsafe_idx_min;
  utf8_idx_max=0;
  do{
    byte0=(u8)(unsafe_list_base[unsafe_idx]);
    unsafe_size_minus_1=unsafe_idx_max-unsafe_idx;
    unsafe_idx_min=unsafe_idx;
    unsafe_idx++;
    valid_status=0;
    if(byte0<=0x7F){
      valid_status=(0x1F<byte0);
    }else if(((byte0&0xC0)!=0x80)&&unsafe_size_minus_1){
      byte1=(u8)(unsafe_list_base[unsafe_idx]);
      if((byte1&0xC0)==0x80){
        unsafe_idx++;
        if((byte0&0xE0)==0xC0){
          if(0xC2<=(byte0&0xDE)){
            valid_status=!((byte0==0xC2)&&(byte1<=0x9F));
          }
        }else if(1<unsafe_size_minus_1){
          byte2=(u8)(unsafe_list_base[unsafe_idx]);
          if((byte2&0xC0)==0x80){
            unsafe_idx++;
            if((byte0&0xF0)==0xE0){
              if(byte0==0xE0){
                valid_status=((byte1&0xA0)==0xA0);
              }else if(byte0==0xED){
                valid_status=((byte1&0xBE)!=0xB2);
              }else{
                valid_status=1;
              }
            }else if(2<unsafe_size_minus_1){
              byte3=(u8)(unsafe_list_base[unsafe_idx]);
              if((byte3&0xC0)==0x80){
                unsafe_idx++;
                if((byte0&0xF0)==0xF0){
                  if(byte0==0xF0){
                    valid_status=(0x90<=(byte1&0xB0));
                  }else if(byte0==0xF4){
                    valid_status=((byte1&0xB0)==0x80);
                  }else if(byte0<=0xF3){
                    valid_status=1;
                  }
                }
              }
            }
          }
        }
      }
    }
    safe_size=(u8)(unsafe_idx-unsafe_idx_min);
    if(!valid_status){
      safe_size=0;
      if(safe_idx|eager_status){
        byte0=0xEF;
        byte1=0xBF;
        byte2=0xBD;
        safe_size=3;
      }
    }
    if(safe_size){
      safe_list_base[safe_idx]=(char)(byte0);
      safe_idx++;
      utf8_idx_max++;
      if(1<safe_size){
        safe_list_base[safe_idx]=(char)(byte1);
        safe_idx++;
        if(2<safe_size){
          safe_list_base[safe_idx]=(char)(byte2);
          safe_idx++;
          if(3<safe_size){
            safe_list_base[safe_idx]=(char)(byte3);
            safe_idx++;
          }
        }
      }
    }
  }while(unsafe_idx<=unsafe_idx_max);
  safe_list_base[safe_idx]=(char)(0);
  *utf8_idx_max_base=utf8_idx_max;
  return safe_idx;
}

u8
ascii_utf8_string_verify(char *unsafe_list_base){
/*
Verify that a null-terminated alleged UTF8 string is in fact valid (or null) and free of ASCII below 0x20 (apart from the terminating null), alternative null UTF8(0xC0, 0x80), and Controls C1 (UTF8(0xC2, [0x80, 0x9F])).

In:

  *unsafe_list_base is a null-terminated string consisting of allegedly valid UTF8.

Out:

  Returns zero if *unsafe_list_base consists entirely of valid UTF8 up to its null terminator, or is simply null; else one. The alternative null character (UTF8(0xC0, 0x80)) is considered invalid.
*/
  u8 status;
  ULONG unsafe_idx_max;

  status=0;
  unsafe_idx_max=(ULONG)(strlen(unsafe_list_base));
  if(unsafe_idx_max){
    unsafe_idx_max--;
    status=ascii_utf8_verify(unsafe_idx_max, unsafe_list_base);
  }
  return status;
}

u8
ascii_utf8_verify(ULONG unsafe_idx_max, char *unsafe_list_base){
/*
Verify that an alleged UTF8 string is in fact valid and free of ASCII below 0x20 including nulls (zero as well as UTF8(0xC0, 0x80)) and Controls C1 (UTF8(0xC2, [0x80, 0x9F])).

In:

  unsafe_idx_max is the maximum index of *unsafe_list_base at which to verify. (This is a byte index, not a UTF8 index.)

  *unsafe_list_base is allegedly valid UTF8.

Out:

  Returns zero if *unsafe_list_base consists entirely of valid UTF8, as defined in the summary above, and spanning exactly (unsafe_idx_max+1) bytes, else one.
*/
  u8 byte0;
  u8 byte1;
  u8 byte2;
  u8 byte3;
  u8 status;
  ULONG unsafe_idx;
  ULONG unsafe_size_minus_1;
  u8 valid_status;

  byte0=0;
  byte1=0;
  byte2=0;
  byte3=0;
  unsafe_idx=0;
  do{
    byte0=(u8)(unsafe_list_base[unsafe_idx]);
    unsafe_size_minus_1=unsafe_idx_max-unsafe_idx;
    unsafe_idx++;
    valid_status=0;
    if(byte0<=0x7F){
      valid_status=(0x1F<byte0);
    }else if(((byte0&0xC0)!=0x80)&&unsafe_size_minus_1){
      byte1=(u8)(unsafe_list_base[unsafe_idx]);
      if((byte1&0xC0)==0x80){
        unsafe_idx++;
        if((byte0&0xE0)==0xC0){
          if(0xC2<=(byte0&0xDE)){
            valid_status=!((byte0==0xC2)&&(byte1<=0x9F));
          }
        }else if(1<unsafe_size_minus_1){
          byte2=(u8)(unsafe_list_base[unsafe_idx]);
          if((byte2&0xC0)==0x80){
            unsafe_idx++;
            if((byte0&0xF0)==0xE0){
              if(byte0==0xE0){
                valid_status=((byte1&0xA0)==0xA0);
              }else if(byte0==0xED){
                valid_status=((byte1&0xBE)!=0xB2);
              }else{
                valid_status=1;
              }
            }else if(2<unsafe_size_minus_1){
              byte3=(u8)(unsafe_list_base[unsafe_idx]);
              if((byte3&0xC0)==0x80){
                unsafe_idx++;
                if((byte0&0xF0)==0xF0){
                  if(byte0==0xF0){
                    valid_status=(0x90<=(byte1&0xB0));
                  }else if(byte0==0xF4){
                    valid_status=((byte1&0xB0)==0x80);
                  }else if(byte0<=0xF3){
                    valid_status=1;
                  }
                }
              }
            }
          }
        }
      }
    }
  }while((unsafe_idx<=unsafe_idx_max)&&valid_status);
  status=!valid_status;
  return status;
}
