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
Mask Quantization Utility
*/
#include "flag.h"
#include "flag_ascii.h"
#include "flag_filesys.h"
#include "flag_fracterval_u128.h"
#include "flag_fracterval_u64.h"
#include "flag_agnentroquant.h"
#include <stdint.h>
#include <string.h>
#include "constant.h"
#include "debug.h"
#include "debug_xtrn.h"
#include "ascii_xtrn.h"
#include "filesys.h"
#include "filesys_xtrn.h"
#include "fracterval_u128.h"
#include "fracterval_u128_xtrn.h"
#include "fracterval_u64.h"
#include "fracterval_u64_xtrn.h"

#define AGNENTROQUANT_MODE_BIAS (1U<<AGNENTROQUANT_MODE_BIAS_BIT_IDX)
#define AGNENTROQUANT_MODE_BIAS_BIT_IDX 7U
#define AGNENTROQUANT_MODE_CHANNELIZE (1U<<AGNENTROQUANT_MODE_CHANNELIZE_BIT_IDX)
#define AGNENTROQUANT_MODE_CHANNELIZE_BIT_IDX 6U
#define AGNENTROQUANT_MODE_DELTAS (3U<<AGNENTROQUANT_MODE_DELTAS_BIT_IDX)
#define AGNENTROQUANT_MODE_DELTAS_BIT_IDX 4U
#define AGNENTROQUANT_MODE_LOGLINEAR (1U<<AGNENTROQUANT_MODE_LOGLINEAR_BIT_IDX)
#define AGNENTROQUANT_MODE_LOGLINEAR_BIT_IDX 1U
#define AGNENTROQUANT_MODE_NORMALIZE (1U<<AGNENTROQUANT_MODE_LOGLINEAR_BIT_IDX)
#define AGNENTROQUANT_MODE_NORMALIZE_BIT_IDX 0U

void
agnentroquant_error_print(char *char_list_base){
  DEBUG_PRINT("ERROR: ");
  DEBUG_PRINT(char_list_base);
  DEBUG_PRINT(".\n");
  return;
}

void
agnentroquant_out_of_memory_print(void){
  agnentroquant_error_print("Out of memory");
  return;
}

void
agnentroquant_parameter_error_print(char *char_list_base){
  DEBUG_PRINT("Invalid parameter: (");
  DEBUG_PRINT(char_list_base);
  DEBUG_PRINT("). For help, run without parameters.\n");
  return;
}

int
main(int argc, char *argv[]){
  ULONG arg_idx;
  u8 ascii;
  u8 bias_status;
  u8 channel_status;
  u8 delta_count;
  u8 delta_idx;
  u8 file_status;
  u8 filesys_status;
  ULONG in_file_count;
  ULONG in_file_idx;
  ULONG in_file_size;
  ULONG in_file_size_max;
  char *in_filename_base;
  ULONG in_filename_char_idx;
  ULONG in_filename_char_idx_max;
  ULONG in_filename_char_idx_new;
  ULONG in_filename_idx;
  ULONG in_filename_idx_max;
  ULONG in_filename_size;
  ULONG in_filename_size_new;
  char *in_name_base;
  ULONG in_u8_idx;
  ULONG in_u8_idx_max;
  u8 *in_u8_list_base;
  u8 mask_bit_count_delta;
  u8 mask_bit_count_in;
  u8 mask_bit_count_out;
  u8 mask_bit_idx;
  u8 mask_size_in;
  u8 mask_size_out;
  u64 mask_span_in;
  u64 mask_span_out;
  u8 mask_span_power_of_2_status;
  u64 mask_u64;
  u64 mask_u64_max;
  u64 mask_u64_min;
  u64 mask_u64_new;
  u64 mask_u64_old;
  u8 mask_u8;
  u8 mask_u8_idx;
  u128 mask_u128;
  u8 normalize_status;
  char *out_filename_base;
  ULONG out_filename_size;
  ULONG out_u8_idx;
  u8 overflow_status;
  u64 parameter;
  u8 status;

  out_filename_base=NULL;
  overflow_status=0;
  status=ascii_init(0, 0);
  status=(u8)(status|filesys_init(0, 0));
  in_filename_base=NULL;
  in_u8_list_base=NULL;
  do{
    if(status){
      agnentroquant_error_print("Outdated source code");
      break;
    }
    status=1;
    if(argc!=6){
      DEBUG_PRINT("Agnentro Quant\nCopyright 2017 Russell Leidich\nhttp://agnentropy.blogspot.com\n");
      DEBUG_U32("build_id_in_hex", AGNENTROQUANT_BUILD_ID);
      DEBUG_PRINT("Mask list quantization utility.\n\n");
      DEBUG_PRINT("Syntax:\n\n");
      DEBUG_PRINT("agnentroquant mode granularity mask_max input output\n\n");
      DEBUG_PRINT("(mode) controls behavior:\n\n  bit 0: (normalize) converts the minimum and maximum input masks to zero and\n  mask_max, respectively. This is as opposed to assuming that all input masks\n  allowed by (granularity) may actually occur. Normalization occurs after all\n  other transformations.\n\n  bit 4-5: (deltas) The number of times to compute the delta (discrete\n  derivative) of the mask list prior to considering (overlap). Each delta, if\n  any, will transform {A, B, C...} to  {A, (B-A), (C-B)...}. This is useful\n  for improving the entropy contrast of signals containing masks which\n  represent magnitudes, as opposed to merely symbols. Experiment to find the\n  optimum value for your data set. If the mask list size isn't a multiple of\n  ((granularity)+1) bytes, then the remainder bytes will remain unchanged.\n\n  bit 6: (channelize) Set if masks consist of parallel byte channels, for\n  example the red, green, and blue bytes of 24-bit pixels. This will cause\n  deltafication, if enabled, to occur on individual bytes. In any event, it\n  force the granularity to be subsequently treated as 0.\n\n  bit 7:(bias) Add (2^(granularity*8)+7) to each sample after deltafication\n  but before normalization. This helps to reduce the mask span, and thus aids\n  normalization, because (-1) no longer translates to the maximum mask value.\n\n");
      DEBUG_PRINT("(granularity) is one less than the number of bytes per input mask, on [0, 7].\n\n");
      DEBUG_PRINT("(mask_max) is a nonzero hex value up to 32 bits wide which is the maximum\nallowed mask.\n\n");
      DEBUG_PRINT("(input) is the file or folder from which to read multiples of (granularity+1)\nbytes.\n\n");
      DEBUG_PRINT("(output) is the folder, different from (input), to which to write the\ncorresponding quantized masks in files of identical names. Each mask will\nconsist of the minimum possible size sufficient to store (mask_max).\n\n");
      break;
    }
    arg_idx=0;
    do{
      status=ascii_utf8_string_verify(argv[arg_idx]);
      if(status){
        agnentroquant_error_print("One or more parameters is encoded using invalid UTF8");
        break;
      }
    }while((++arg_idx)<(ULONG)(argc));
    if(status){
      break;
    }
    status=ascii_hex_to_u64_convert(argv[1], &parameter, U8_MAX);
    if(status|(parameter&(~(AGNENTROQUANT_MODE_BIAS|AGNENTROQUANT_MODE_CHANNELIZE|AGNENTROQUANT_MODE_DELTAS|AGNENTROQUANT_MODE_LOGLINEAR|AGNENTROQUANT_MODE_NORMALIZE)))){
      agnentroquant_parameter_error_print("mode");
      break;
    }
    bias_status=(u8)((parameter>>AGNENTROQUANT_MODE_BIAS_BIT_IDX)&1);
    channel_status=(u8)((parameter>>AGNENTROQUANT_MODE_CHANNELIZE_BIT_IDX)&1);
    delta_count=(u8)((parameter>>AGNENTROQUANT_MODE_DELTAS_BIT_IDX)&3);
    normalize_status=(u8)((parameter>>AGNENTROQUANT_MODE_NORMALIZE_BIT_IDX)&1);
    status=ascii_hex_to_u64_convert(argv[2], &parameter, U64_BYTE_MAX);
    if(status){
      agnentroquant_parameter_error_print("granularity");
      break;
    }
    mask_size_in=(u8)(parameter+1);
    status=ascii_hex_to_u64_convert(argv[3], &parameter, U32_MAX);
    status=(u8)(status|!parameter);
    if(status){
      agnentroquant_parameter_error_print("mask_max");
      break;
    }
    mask_span_out=parameter+1;
    mask_size_out=U8_SIZE;
    if(parameter>>U8_BITS){
      mask_size_out++;
      if(parameter>>U16_BITS){
        mask_size_out++;
        if(parameter>>U24_BITS){
          mask_size_out++;
        }
      }
    }
    if(mask_size_in<mask_size_out){
      agnentroquant_error_print("mask_max exceeds what granularity allows");
      status=1;
      break;
    }
    mask_bit_count_in=(u8)(mask_size_in<<U8_BITS_LOG2);
    mask_bit_count_out=0;
    mask_span_power_of_2_status=!(mask_span_out&parameter);
    do{
      parameter>>=1;
      mask_bit_count_out++;
    }while(parameter);
    mask_bit_count_delta=mask_bit_count_in;
    if(normalize_status){
      mask_bit_count_delta=U64_BITS;
    }
    mask_bit_count_delta=(u8)(mask_bit_count_delta-mask_bit_count_out);
    out_filename_size=(ULONG)(strlen(argv[5]));
/*
Allocate enough space for the maximum path size that filesys_filename_list_get may append, plus one for a path separator, plus one for a terminating null.
*/
    out_filename_size+=U16_MAX+2;
    out_filename_base=filesys_char_list_malloc(out_filename_size);
    out_filename_size-=U16_MAX+2;
    status=!out_filename_base;
    if(status){
      agnentroquant_out_of_memory_print();
      break;
    }
    memcpy(out_filename_base, argv[5], (size_t)(out_filename_size));
    if(out_filename_size&&!file_status){
      if(out_filename_base[out_filename_size-1]!=FILESYS_PATH_SEPARATOR){
        out_filename_base[out_filename_size]=FILESYS_PATH_SEPARATOR;
        out_filename_size++;
      }
    }
    in_name_base=argv[4];
    in_file_size_max=0;
    in_file_count=0;
    in_filename_size=U16_MAX;
    mask_span_in=0;
    status=1;
    do{
      in_filename_char_idx_max=in_filename_size-1;
      in_filename_base=filesys_char_list_malloc(in_filename_char_idx_max);
      if(!in_filename_base){
        agnentroquant_out_of_memory_print();
        break;
      }
      in_filename_size_new=in_filename_size;
      in_file_count=filesys_filename_list_get(&in_file_size_max, &file_status, in_filename_base, &in_filename_size_new, in_name_base);
      if(!in_file_count){
        agnentroquant_error_print("(textpath) not found or inaccessible");
        break;
      }
      status=0;
      if(in_filename_size<in_filename_size_new){
        in_filename_base=filesys_free(in_filename_base);
        in_filename_size=in_filename_size_new;
        status=1;
      }
    }while(status);
    if(status){
      break;
    }
    in_u8_list_base=(u8 *)(DEBUG_MALLOC_PARANOID(in_file_size_max));
    status=!in_u8_list_base;
    if(status){
      agnentroquant_out_of_memory_print();
      break;
    }
    in_filename_char_idx=0;
    in_file_idx=0;
    mask_u64_max=0;
    mask_u64_min=0;
    do{
      in_file_size=in_file_size_max;
      in_filename_char_idx_new=in_filename_char_idx;
      filesys_status=filesys_file_read_next(&in_file_size, &in_filename_char_idx_new, in_filename_base, in_u8_list_base);
      if((!in_file_size)||(in_file_size%mask_size_in)){
        filesys_status=FILESYS_STATUS_CALLER_CUSTOM;
      }
      status=!!filesys_status;
      if(!status){
        in_u8_idx_max=(ULONG)(in_file_size-1);
        if(delta_count){
          delta_idx=0;
          do{
            in_u8_idx=0;
            mask_u64_old=0;
            if(!channel_status){
              do{
                mask_u64=0;
                memcpy(&mask_u64, &in_u8_list_base[in_u8_idx], (size_t)(mask_size_in));
                mask_u64_new=mask_u64-mask_u64_old;
                mask_u64_old=mask_u64;
                memcpy(&in_u8_list_base[in_u8_idx], &mask_u64_new, (size_t)(mask_size_in));
                in_u8_idx+=mask_size_in;
              }while(in_u8_idx<=in_u8_idx_max);
            }else{
              do{
                mask_u64=0;
                memcpy(&mask_u64, &in_u8_list_base[in_u8_idx], (size_t)(mask_size_in));
                mask_bit_idx=0;
                mask_u64_new=0;
                mask_u8_idx=0;
                do{
                  mask_u8=(u8)(mask_u64>>mask_bit_idx);
                  mask_u8=(u8)(mask_u8-(mask_u64_old>>mask_bit_idx));
                  mask_u64_new|=(u64)(mask_u8)<<mask_bit_idx;
                  mask_u8_idx++;
                }while(mask_u8_idx!=mask_size_in);
                mask_u64_old=mask_u64;
                memcpy(&in_u8_list_base[in_u8_idx], &mask_u64_new, (size_t)(mask_size_in));
                in_u8_idx+=mask_size_in;
              }while(in_u8_idx<=in_u8_idx_max);
            }
            delta_idx++;
          }while(delta_idx!=delta_count);
        }
        if(channel_status){
          mask_size_in=1;
        }
        if(bias_status){
          in_u8_idx=0;
          do{
            in_u8_list_base[in_u8_idx+mask_size_in-1]^=U8_SPAN_HALF;
            in_u8_idx+=mask_size_in;
          }while(in_u8_idx<=in_u8_idx_max);
          mask_span_in=mask_u64_max-mask_u64_min+1;
        }
        if(normalize_status){
          in_u8_idx=0;
          mask_u64_max=0;
          mask_u64_min=~mask_u64_max;
          do{
            mask_u64=0;
            memcpy(&mask_u64, &in_u8_list_base[in_u8_idx], (size_t)(mask_size_in));
            in_u8_idx+=mask_size_in;
            mask_u64_max=MAX(mask_u64, mask_u64_max);
            mask_u64_min=MIN(mask_u64, mask_u64_min);
          }while(in_u8_idx<=in_u8_idx_max);
          mask_span_in=mask_u64_max-mask_u64_min+1;
        }
        in_u8_idx=0;
        out_u8_idx=0;
        do{
          mask_u64=0;
          memcpy(&mask_u64, &in_u8_list_base[in_u8_idx], (size_t)(mask_size_in));
          in_u8_idx+=mask_size_in;
          if(normalize_status){
            if(mask_span_in){
              mask_u64-=mask_u64_min;
              FTD64_RATIO_U64_SATURATE_SELF(mask_u64, mask_span_in, overflow_status);
            }
            if(mask_span_power_of_2_status){
              mask_u64>>=mask_bit_count_delta;
            }else{
              U128_FROM_U64_PRODUCT(mask_u128, mask_u64, mask_span_out);
              U128_TO_U64_HI(mask_u64, mask_u128);
            }
          }else{
            if(mask_span_power_of_2_status){
              mask_u64>>=mask_bit_count_delta;
            }else{
              U128_FROM_U64_PRODUCT(mask_u128, mask_u64, mask_span_out);
              U128_SHIFT_RIGHT_SELF(mask_u128, mask_bit_count_in);
              U128_TO_U64_LO(mask_u64, mask_u128);
            }
          }
          memcpy(&in_u8_list_base[out_u8_idx], &mask_u64, (size_t)(mask_size_out));
          out_u8_idx+=mask_size_out;
        }while(in_u8_idx<=in_u8_idx_max);
        in_filename_idx_max=(ULONG)(strlen(&in_filename_base[in_filename_char_idx]));
        in_filename_idx_max+=in_filename_char_idx;
        in_filename_idx=in_filename_idx_max;
        while(in_filename_idx){
          ascii=(u8)(in_filename_base[in_filename_idx-1]);
          if(ascii==FILESYS_PATH_SEPARATOR){
            break;
          }
          in_filename_idx--;
        }
        in_filename_size=in_filename_idx_max-in_filename_idx+1;
        memcpy(&out_filename_base[out_filename_size], &in_filename_base[in_filename_idx], (size_t)(in_filename_size));
        filesys_status=filesys_file_write(out_u8_idx, out_filename_base, in_u8_list_base);
        status=!!filesys_status;
      }
      if(status){
        if(filesys_status!=FILESYS_STATUS_WRITE_FAIL){
          DEBUG_PRINT(&in_filename_base[in_filename_char_idx]);
        }else{
          DEBUG_PRINT(out_filename_base);
        }
        DEBUG_PRINT("\n");
        switch(filesys_status){
        case FILESYS_STATUS_NOT_FOUND:
          agnentroquant_error_print("Not found");
          break;
        case FILESYS_STATUS_TOO_BIG:
          agnentroquant_error_print("Too big to fit in memory");
          break;
        case FILESYS_STATUS_READ_FAIL:
          agnentroquant_error_print("Read failed");
          break;
        case FILESYS_STATUS_WRITE_FAIL:
          agnentroquant_error_print("Write failed");
          break;
        case FILESYS_STATUS_SIZE_CHANGED:
          agnentroquant_error_print("File size changed during read");
          break;
        case FILESYS_STATUS_CALLER_CUSTOM:
          agnentroquant_error_print("File size not a nonzero multiple of (granularity+1)");
          break;
        default:
          agnentroquant_error_print("Internal error. Please report");
        }
      }
      in_filename_char_idx=in_filename_char_idx_new;
      in_file_idx++;
      status=0;
    }while(in_file_idx!=in_file_count);
  }while(0);
  DEBUG_FREE_PARANOID(in_u8_list_base);
  filesys_free(in_filename_base);
  filesys_free(out_filename_base);
  DEBUG_ALLOCATION_CHECK();
  return status;
}
