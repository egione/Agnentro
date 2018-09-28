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
Logarithmic Mask Logization Utility
*/
#include "flag.h"
#include "flag_ascii.h"
#include "flag_filesys.h"
#include "flag_agnentrolog.h"
#include <stdint.h>
#include <string.h>
#include "constant.h"
#include "debug.h"
#include "debug_xtrn.h"
#include "ascii_xtrn.h"
#include "bitscan.h"
#include "bitscan_xtrn.h"
#include "filesys.h"
#include "filesys_xtrn.h"

void
agnentrolog_error_print(char *char_list_base){
  DEBUG_PRINT("ERROR: ");
  DEBUG_PRINT(char_list_base);
  DEBUG_PRINT(".\n");
  return;
}

void
agnentrolog_out_of_memory_print(void){
  agnentrolog_error_print("Out of memory");
  return;
}

void
agnentrolog_parameter_error_print(char *char_list_base){
  DEBUG_PRINT("Invalid parameter: (");
  DEBUG_PRINT(char_list_base);
  DEBUG_PRINT("). For help, run without parameters.\n");
  return;
}

int
main(int argc, char *argv[]){
  ULONG arg_idx;
  u8 bit_count;
  u8 file_status;
  u8 filesys_status;
  ULONG in_file_idx;
  ULONG in_file_size;
  ULONG in_file_size_max;
  char *in_filename_base;
  ULONG in_filename_count;
  char *in_filename_list_base;
  ULONG in_filename_list_char_idx;
  ULONG in_filename_list_char_idx_max;
  ULONG in_filename_list_char_idx_new;
  ULONG in_filename_list_size;
  ULONG in_filename_list_size_new;
  ULONG in_u8_idx;
  ULONG in_u8_idx_max;
  u8 *in_u8_list_base;
  u8 mask_size_in;
  u64 mask_u64;
  char *out_filename_base;
  ULONG out_filename_char_idx;
  ULONG out_filename_char_idx_max;
  ULONG out_filename_char_idx_new;
  char *out_filename_list_base;
  ULONG out_filename_list_size;
  ULONG out_u8_idx;
  u64 parameter;
  u8 status;

  status=ascii_init(ASCII_BUILD_BREAK_COUNT_EXPECTED, 0);
  status=(u8)(status|filesys_init(FILESYS_BUILD_BREAK_COUNT_EXPECTED, 4));
  in_filename_list_base=NULL;
  in_u8_list_base=NULL;
  out_filename_list_base=NULL;
  do{
    if(status){
      agnentrolog_error_print("Outdated source code");
      break;
    }
    status=1;
    if(argc!=4){
      DEBUG_PRINT("Agnentro Log\nCopyright 2017 Russell Leidich\nhttp://agnentropy.blogspot.com\n");
      DEBUG_U32("build_id_in_hex", AGNENTROLOG_BUILD_ID);
      DEBUG_PRINT("Mask list logarithmic quantization utility.\n\n");
      DEBUG_PRINT("Function:\n\n");
      DEBUG_PRINT("Converts masks M up to 8 bytes long into bytes equal to (floor(log2(M))+1),\nsuch that (M=0) yields an output of 0:\n\n");
      DEBUG_PRINT("Syntax:\n\n");
      DEBUG_PRINT("  agnentrolog granularity input output\n\n");
      DEBUG_PRINT("(granularity) is one less than the number of bytes per input mask, on [0, 7].\n\n");
      DEBUG_PRINT("(input) is the file or folder from which to read multiples of (granularity+1)\nbytes.\n\n");
      DEBUG_PRINT("(output) is the file or folder, corresponding to but different from (input), to\nwhich to write the corresponding log values. Each output log is the integer\nfloor of the log2 of the input mask, plus 1. 0 maps to 0\n\n");
      break;
    }
    arg_idx=0;
    do{
      status=ascii_utf8_string_verify(argv[arg_idx]);
      if(status){
        agnentrolog_error_print("One or more parameters is encoded using invalid UTF8");
        break;
      }
    }while((++arg_idx)<(ULONG)(argc));
    if(status){
      break;
    }
    status=ascii_hex_to_u64_convert(argv[1], &parameter, U64_BYTE_MAX);
    if(status){
      agnentrolog_parameter_error_print("granularity");
      break;
    }
    in_filename_base=argv[2];
    out_filename_base=argv[3];
    in_file_size_max=0;
    in_filename_count=0;
    in_filename_list_size=U16_MAX;
    mask_size_in=(u8)(parameter+1);
    status=1;
    do{
      in_filename_list_char_idx_max=in_filename_list_size-1;
      in_filename_list_base=filesys_char_list_malloc(in_filename_list_char_idx_max);
      if(!in_filename_list_base){
        agnentrolog_out_of_memory_print();
        break;
      }
      in_filename_list_size_new=in_filename_list_size;
      in_filename_count=filesys_filename_list_get(&in_file_size_max, &file_status, in_filename_list_base, &in_filename_list_size_new, in_filename_base);
      if(!in_filename_count){
        agnentrolog_error_print("(textpath) not found or inaccessible");
        break;
      }
      status=filesys_filename_list_sort(in_filename_count, in_filename_list_base);
      if(status){
        agnentrolog_out_of_memory_print();
        break;
      }
      if(in_filename_list_size<in_filename_list_size_new){
        in_filename_list_base=filesys_free(in_filename_list_base);
        in_filename_list_size=in_filename_list_size_new;
        status=1;
      }
    }while(status);
    if(status){
      break;
    }
    in_u8_list_base=(u8 *)(DEBUG_MALLOC_PARANOID(in_file_size_max));
    status=!in_u8_list_base;
    out_filename_list_size=filesys_filename_list_morph_size_get(in_filename_count, in_filename_base, in_filename_list_base, out_filename_base);
    out_filename_char_idx_max=out_filename_list_size-1;
    out_filename_list_base=filesys_char_list_malloc(out_filename_char_idx_max);
    status=(u8)(status|!out_filename_list_base);
    if(status){
      agnentrolog_out_of_memory_print();
      break;
    }
    filesys_filename_list_morph(in_filename_count, in_filename_base, in_filename_list_base, out_filename_base, out_filename_list_base);
    in_filename_list_char_idx=0;
    in_file_idx=0;
    out_filename_char_idx=0;
    do{
      in_file_size=in_file_size_max;
      in_filename_list_char_idx_new=in_filename_list_char_idx;
      filesys_status=filesys_file_read_next(&in_file_size, &in_filename_list_char_idx_new, in_filename_list_base, in_u8_list_base);
      if((!in_file_size)||(in_file_size%mask_size_in)){
        filesys_status=FILESYS_STATUS_CALLER_CUSTOM;
      }
      status=!!filesys_status;
      if(!status){
        in_u8_idx=0;
        in_u8_idx_max=(ULONG)(in_file_size-1);
        out_u8_idx=0;
        do{
          mask_u64=0;
          memcpy(&mask_u64, &in_u8_list_base[in_u8_idx], (size_t)(mask_size_in));
          in_u8_idx+=mask_size_in;
          bit_count=0;
          if(mask_u64){
            BITSCAN_MSB64_SMALL_GET(bit_count, mask_u64);
            bit_count++;
          }
          in_u8_list_base[out_u8_idx]=bit_count;
          out_u8_idx++;
        }while(in_u8_idx<=in_u8_idx_max);
        out_filename_char_idx_new=out_filename_char_idx;
        filesys_status=filesys_file_write_next_obnoxious(out_u8_idx, &out_filename_char_idx_new, out_filename_list_base, in_u8_list_base);
        status=!!filesys_status;
      }
      if(status){
        if(filesys_status!=FILESYS_STATUS_WRITE_FAIL){
          DEBUG_PRINT(&in_filename_list_base[in_filename_list_char_idx]);
        }else{
          DEBUG_PRINT(&out_filename_list_base[out_filename_char_idx]);
        }
        DEBUG_PRINT("\n");
        switch(filesys_status){
        case FILESYS_STATUS_NOT_FOUND:
          agnentrolog_error_print("Not found");
          break;
        case FILESYS_STATUS_TOO_BIG:
          agnentrolog_error_print("Too big to fit in memory");
          break;
        case FILESYS_STATUS_READ_FAIL:
          agnentrolog_error_print("Read failed");
          break;
        case FILESYS_STATUS_WRITE_FAIL:
          agnentrolog_error_print("Write failed");
          break;
        case FILESYS_STATUS_SIZE_CHANGED:
          agnentrolog_error_print("File size changed during read");
          break;
        case FILESYS_STATUS_CALLER_CUSTOM:
          agnentrolog_error_print("File size not a nonzero multiple of (granularity+1)");
          break;
        default:
          agnentrolog_error_print("Internal error. Please report");
        }
      }
      in_filename_list_char_idx=in_filename_list_char_idx_new;
      in_file_idx++;
      out_filename_char_idx=out_filename_char_idx_new;
    }while(in_file_idx!=in_filename_count);
    status=0;
  }while(0);
  filesys_free(out_filename_list_base);
  DEBUG_FREE_PARANOID(in_u8_list_base);
  filesys_free(in_filename_list_base);
  DEBUG_ALLOCATION_CHECK();
  return status;
}
