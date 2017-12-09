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
Agnentropy Proof of Concept
*/
#include "flag.h"
#include "flag_biguint.h"
#include "flag_fracterval_u128.h"
#include "flag_fracterval_u64.h"
#include "flag_loggamma.h"
#include "flag_agnentrocodec.h"
#include "flag_poissocache.h"
#include "flag_agnentroprox.h"
#include "flag_ascii.h"
#include "flag_filesys.h"
#include "flag_agnentrofile.h"
#include <stdint.h>
#include <string.h>
#include "constant.h"
#include "debug.h"
#include "debug_xtrn.h"
#include "fracterval_u128.h"
#include "fracterval_u128_xtrn.h"
#include "fracterval_u64.h"
#include "loggamma.h"
#include "loggamma_xtrn.h"
#include "agnentrocodec.h"
#include "agnentrocodec_xtrn.h"
#include "poissocache.h"
#include "agnentroprox.h"
#include "agnentroprox_xtrn.h"
#include "ascii_xtrn.h"
#include "biguint.h"
#include "biguint_xtrn.h"
#include "filesys.h"
#include "filesys_xtrn.h"

#define AGNENTROFILE_MODE_ESTIMATE 0U
#define AGNENTROFILE_MODE_EXACT 1U
#define AGNENTROFILE_MODE_COMPRESS 2U
#define AGNENTROFILE_MODE_DECOMPRESS 3U
#define AGNENTROFILE_MODE_SHUFFLE 4U
#define AGNENTROFILE_MODE_UNSHUFFLE 5U
#define AGNENTROFILE_MODE_INVALID U8_MAX

void
agnentrofile_error_print(char *char_list_base){
  DEBUG_PRINT("ERROR: ");
  DEBUG_PRINT(char_list_base);
  DEBUG_PRINT(".\n");
  return;
}

void
agnentrofile_mask_max_print(u8 granularity, u32 mask_max){
  switch(granularity){
  case U8_BYTE_MAX:
    DEBUG_U8("mask_max", (u8)(mask_max));
    break;
  case U16_BYTE_MAX:
    DEBUG_U16("mask_max", (u16)(mask_max));
    break;
  case U24_BYTE_MAX:
    DEBUG_U24("mask_max", mask_max);
    break;
  default:
    DEBUG_U32("mask_max", mask_max);
  }
  return;
}

void
agnentrofile_out_of_memory_print(void){
  agnentrofile_error_print("Out of memory");
  return;
}

ULONG
agnentrofile_random_get(u64 *random_seed_base, ULONG random_span){
/*
Use a 64-bit Marsaglia oscillator to produce pseudorandom numbers.
*/
  u32 marsaglia_c;
  u64 marsaglia_p;
  u32 marsaglia_x;
  ULONG random;
  u64 random_span_u64;
  u64 random_u64;

  marsaglia_p=*random_seed_base;
  MARSAGLIA_ITERATE(marsaglia_c, marsaglia_x, marsaglia_p);
  random_span_u64=(u64)(random_span);
  U64_PRODUCT_HI(marsaglia_p, random_span_u64, random_u64);
  random=(ULONG)(random_u64);
  *random_seed_base=marsaglia_p;
  return random;
}

ULONG
agnentrofile_random_get_reverse(u64 *random_seed_base, ULONG random_span){
/*
Run agnentrofile_random_get() backwards.
*/
  u32 marsaglia_c;
  u64 marsaglia_p;
  u32 marsaglia_x;
  ULONG random;
  u64 random_span_u64;
  u64 random_u64;

  marsaglia_p=*random_seed_base;
  MARSAGLIA_ITERATE_REVERSE(marsaglia_c, marsaglia_x, marsaglia_p);
  random_span_u64=(u64)(random_span);
  U64_PRODUCT_HI(marsaglia_p, random_span_u64, random_u64);
  random=(ULONG)(random_u64);
  *random_seed_base=marsaglia_p;
  return random;
}

int
main(int argc, char *argv[]){
  agnentrocodec_t *agnentrocodec_base;
  agnentroprox_t *agnentroprox_base;
  fru128 agnentropy;
  u64 agnentropy_bit_count;
  u64 agnentropy_bit_idx_max;
  fru128 agnentropy_bits;
  u8 automask_status;
  u64 bit_idx;
  u8 granularity;
  ULONG input_file_size;
  char *input_filename_base;
  loggamma_t *loggamma_base;
  u32 mask;
  ULONG mask_count;
  ULONG mask_idx_max;
  u8 mask_idx_max_logplex_bit_count;
  ULONG mask_idx0;
  ULONG mask_idx1;
  u8 *mask_list_base;
  u32 mask_max;
  u8 mask_max_logplex_bit_count;
  ULONG mask_remainder;
  u8 mask_size;
  u8 mode;
  u64 output_bit_count;
  fru128 output_bits;
  ULONG output_file_size;
  u64 output_file_size_u64;
  char *output_filename_base;
  u8 overflow_status;
  u64 parameter;
  u64 random_seed;
  u8 status;
  u64 transfer_bit_idx_max;
  ULONG transfer_size;
  ULONG u8_idx0;
  ULONG u8_idx1;
  u64 untrusted_bit_count_minus_1;
  ULONG zip_chunk_idx_max;
  ULONG zip_chunk_idx_max_max;
  ULONG *zip_chunk_list_base;

  overflow_status=0;
  status=ascii_init(ASCII_BUILD_BREAK_COUNT_EXPECTED, 0);
  status=(u8)(status|biguint_init(BIGUINT_BUILD_BREAK_COUNT_EXPECTED, 0));
  status=(u8)(status|filesys_init(FILESYS_BUILD_BREAK_COUNT_EXPECTED, 0));
  status=(u8)(status|fracterval_u128_init(FRU128_BUILD_BREAK_COUNT_EXPECTED, 0));
  agnentrocodec_base=NULL;
  agnentroprox_base=NULL;
  loggamma_base=NULL;
  mask_list_base=NULL;
  mask_max=0;
  mode=AGNENTROFILE_MODE_INVALID;
  zip_chunk_list_base=NULL;
  do{
    if(status){
      agnentrofile_error_print("Outdated source code");
      break;
    }
    status=1;
    if((argc!=5)&&(argc!=6)){
      DEBUG_PRINT("Agnentro File\nCopyright 2017 Russell Leidich\nhttp://agnentropy.blogspot.com\n");
      DEBUG_U32("build_id_in_hex", AGNENTROFILE_BUILD_ID);
      DEBUG_PRINT("Analyze or compress a file using Agnentro.\n\n");
      DEBUG_PRINT("Syntax:\n\n");
      DEBUG_PRINT("  agnentrofile mode input_file automask granularity [output_file]\n\n");
      DEBUG_PRINT("where:\n\n");
      DEBUG_PRINT("(mode) is one of the following:\n\n  0. Analyze the input file and display its agnentropy and its implied\n     compressed size in bits (error +/-2). Fast! \n  1. Analyze the input file and display the exact compressed size in bits based\n     on agnentropic encoding. Slow!\n  2. Compress the input file. The output file shall consist of a bitwise\n     concatenation of: (a) a logplex giving the maximum input mask index, (b)\n     a logplex giving the maximum input mask value actually observed, and\n     finally (c) the agnentropic code of the input padded with high zero bits\n     to maintain byte granularity.\n  3. Decompress the input file which was compressed with the same granularity.\n  4. Pseudorandomly shuffle the (granularity+1)-byte chunks of the input file\n     to form the output file. This is done to eliminate all contextual\n     compressiblity, so that direct comparison can be made against other\n     compression apps.\n  5. Invert the shuffling performed in #4 just to prove that it's reversible.\n\n");
      DEBUG_PRINT("(input_file) is the name of the file to analyze or compress.\n\n");
      DEBUG_PRINT("(automask) is 0 to assume that the maximum mask is ((256^(granularity+1))-1),\nelse 1 to discover it by examining the file at the given granularity.\n\n");
      DEBUG_PRINT("(granularity) is the number of bytes per mask, less one. Note that 3 (32 bits per\nmask) requires 64GiB of memory.\n\n");
      DEBUG_PRINT("(output_file) is the name of the file to overwrite, for (mode>1).\n\n");
      break;
    }
    loggamma_base=loggamma_init(LOGGAMMA_BUILD_BREAK_COUNT_EXPECTED, 0);
    if(!loggamma_base){
      agnentrofile_error_print("Loggamma initialization failed");
      break;
    }
    status=ascii_decimal_to_u64_convert(argv[3], &parameter, 1);
    if(status){
      agnentrofile_error_print("Invalid automask");
      break;
    }
    automask_status=(u8)(parameter);
    status=ascii_decimal_to_u64_convert(argv[4], &parameter, U32_BYTE_MAX);
    if(status){
      agnentrofile_error_print("Invalid granularity");
      break;
    }
    granularity=(u8)(parameter);
    status=ascii_decimal_to_u64_convert(argv[1], &parameter, AGNENTROFILE_MODE_UNSHUFFLE);
    if(status){
      agnentrofile_error_print("Invalid mode");
      break;
    }
    mode=(u8)(parameter);
    if((mode!=AGNENTROFILE_MODE_ESTIMATE)&&(mode!=AGNENTROFILE_MODE_EXACT)){
      if(argc!=6){
        agnentrofile_error_print("output_file not specified");
        break;
      }
    }else if(argc!=5){
      agnentrofile_error_print("output_file not allowed in analysis mode");
      break;
    }
    input_filename_base=argv[2];
    status=filesys_file_size_ulong_get(&input_file_size, input_filename_base);
    if(status==FILESYS_STATUS_TOO_BIG){
      agnentrofile_error_print("input_file exceeds memory size");
      break;
    }else if(status||(!status&&(!input_file_size))){
      agnentrofile_error_print("Cannot determine size of input_file");
      break;
    }
    DEBUG_U64("input_file_size", input_file_size);
    mask_count=0;
    mask_idx_max=0;
    mask_size=(u8)(granularity+1);
    status=1;
    transfer_size=input_file_size;
    zip_chunk_idx_max=0;
    zip_chunk_idx_max_max=0;
    if(mode!=AGNENTROFILE_MODE_DECOMPRESS){
      mask_count=input_file_size/mask_size;
      mask_remainder=input_file_size%mask_size;
      if(mask_remainder){
        agnentrofile_error_print("input_file size is not a multiple of (granularity+1) bytes");
        break;
      }
      if(input_file_size!=(mask_count*mask_size)){
        agnentrofile_error_print("input_file size exeeds machine address space");
        break;
      }
      mask_idx_max=mask_count-1;
      DEBUG_U64("mask_idx_max", mask_idx_max);
      if(mode==AGNENTROFILE_MODE_ESTIMATE){
        mask_list_base=agnentroprox_mask_list_malloc(granularity, mask_idx_max, 0);
      }else{
        mask_list_base=agnentrocodec_mask_list_malloc(granularity, mask_idx_max);
      }
      if(!mask_list_base){
        agnentrofile_out_of_memory_print();
        break;
      }
      status=filesys_file_read(&transfer_size, input_filename_base, mask_list_base);
    }else{
      zip_chunk_idx_max_max=input_file_size>>ULONG_SIZE_LOG2;
      if((input_file_size>>ULONG_SIZE_LOG2)!=zip_chunk_idx_max_max){
        agnentrofile_error_print("input_file size exeeds machine address space");
        break;
      }
      zip_chunk_list_base=biguint_malloc(zip_chunk_idx_max_max);
      if(!zip_chunk_list_base){
        agnentrofile_out_of_memory_print();
        break;
      }
      status=filesys_file_read(&transfer_size, input_filename_base, zip_chunk_list_base);
      if(!status){
/*
Convert the compressed ("zip") file we just imported into a canonical biguint.
*/
        transfer_bit_idx_max=((u64)(transfer_size)<<U8_BITS_LOG2)-1;
        status=biguint_bitmap_import(transfer_bit_idx_max, &zip_chunk_idx_max, zip_chunk_idx_max_max, zip_chunk_list_base);
      }
    }
    if(status){
      agnentrofile_error_print("input_file read or import failed");
      break;
    }
    if((mode==AGNENTROFILE_MODE_ESTIMATE)||(mode==AGNENTROFILE_MODE_EXACT)||(mode==AGNENTROFILE_MODE_COMPRESS)){
      mask_max=(u32)(((u32)(1U<<(granularity<<U8_BITS_LOG2))<<U8_BITS)-1);
      if(automask_status){
        mask_max=agnentrocodec_mask_max_get(granularity, mask_idx_max, mask_list_base);
      }
      agnentrofile_mask_max_print(granularity, mask_max);
      if(!mask_max){
        agnentrofile_error_print("Unsupported configuration: input_file is all zeroes");
        break;
      }
/*
Allocate space for the compressed ("zip") file if in fact we need to perform (de)compression.
*/
      zip_chunk_idx_max_max=0;
      if(mode!=AGNENTROFILE_MODE_ESTIMATE){
        zip_chunk_idx_max_max=agnentrocodec_code_chunk_idx_max_max_get(loggamma_base, mask_idx_max, mask_max);
      }
/*
An additional 128 bits will more than suffice for the logplex encodings of mask_idx_max and mask_max. Adjust zip_chunk_idx_max_max accordingly.
*/
      zip_chunk_idx_max_max=zip_chunk_idx_max_max+((U64_SIZE<<1)>>ULONG_SIZE_LOG2);
      if(zip_chunk_idx_max_max<((U64_SIZE<<1)>>ULONG_SIZE_LOG2)){
        agnentrofile_out_of_memory_print();
        break;
      }
      zip_chunk_list_base=biguint_malloc(zip_chunk_idx_max_max);
    }
    output_file_size=0;
    if((mode==AGNENTROFILE_MODE_ESTIMATE)||(mode==AGNENTROFILE_MODE_EXACT)||(mode==AGNENTROFILE_MODE_COMPRESS)||(mode==AGNENTROFILE_MODE_DECOMPRESS)){
      if(mode!=AGNENTROFILE_MODE_DECOMPRESS){
        if(mode==AGNENTROFILE_MODE_ESTIMATE){
          agnentroprox_base=agnentroprox_init(AGNENTROPROX_BUILD_BREAK_COUNT_EXPECTED, 2, granularity, loggamma_base, mask_idx_max, mask_max, AGNENTROPROX_MODE_AGNENTROPY, 0, mask_idx_max);
          if(!agnentroprox_base){
            agnentrofile_error_print("Agnentroprox initialization failed.\nTry reducing granularity or file size");
            break;
          }
        }else{
          agnentrocodec_base=agnentrocodec_init(AGNENTROCODEC_BUILD_BREAK_COUNT_EXPECTED, 0, granularity, loggamma_base, mask_idx_max, mask_max);
          if(!agnentrocodec_base){
            agnentrofile_error_print("Agnentro initialization failed.\nTry reducing granularity or file size");
            break;
          }
        }
        bit_idx=0;
        parameter=mask_idx_max;
        status=biguint_logplex_encode_u64(&bit_idx, &zip_chunk_idx_max, zip_chunk_idx_max_max, zip_chunk_list_base, parameter);
        if(status){
          agnentrofile_out_of_memory_print();
          break;
        }
        mask_idx_max_logplex_bit_count=(u8)(bit_idx);
        DEBUG_U8("mask_idx_max_logplex_bit_count", mask_idx_max_logplex_bit_count);
        parameter=mask_max;
        status=biguint_logplex_encode_u64(&bit_idx, &zip_chunk_idx_max, zip_chunk_idx_max_max, zip_chunk_list_base, parameter);
        if(status){
          agnentrofile_out_of_memory_print();
          break;
        }
        mask_max_logplex_bit_count=(u8)(bit_idx-mask_idx_max_logplex_bit_count);
        status=1;
        DEBUG_U8("mask_max_logplex_bit_count", mask_max_logplex_bit_count);
        if(mode==AGNENTROFILE_MODE_ESTIMATE){
          agnentropy=agnentroprox_entropy_delta_get(agnentroprox_base, mask_idx_max, mask_list_base, AGNENTROPROX_MODE_AGNENTROPY, 1, &overflow_status, 0);
          DEBUG_F128_PAIR("agnentropy_in_nats", agnentropy.a, agnentropy.b);
          FRU128_NATS_TO_BITS(agnentropy_bits, agnentropy, overflow_status);
          DEBUG_F128_PAIR("agnentropy_in_bits", agnentropy_bits.a, agnentropy_bits.b);
          FRU128_ADD_U64_HI(output_bits, agnentropy_bits, bit_idx, overflow_status);
          DEBUG_F128_PAIR("overhead_plus_agnentropy_in_bits", output_bits.a, output_bits.b);
          U128_TO_U64_HI(output_bit_count, output_bits.b);
          output_bit_count++;
        }else{
          agnentropy_bit_idx_max=agnentrocodec_encode(agnentrocodec_base, mask_idx_max, mask_list_base);
          agnentropy_bit_count=agnentropy_bit_idx_max+1;
          DEBUG_U64("agnentropy_bit_count", agnentropy_bit_count);
          status=agnentrocodec_code_export(agnentrocodec_base, bit_idx, &zip_chunk_idx_max, zip_chunk_idx_max_max, zip_chunk_list_base);
          if(status){
            agnentrofile_error_print("agnentrocodec_code_export() failed");
            break;
          }
          output_bit_count=bit_idx+agnentropy_bit_count;
          DEBUG_U64("output_bit_count", output_bit_count);
        }
        output_file_size=ULONG_MAX;
        if(output_bit_count){
          output_file_size_u64=(output_bit_count>>U8_BITS_LOG2)+!!(output_bit_count&U8_BIT_MAX);
          output_file_size=(ULONG)(output_file_size_u64);
          #ifdef _32_
            if(output_file_size!=output_file_size_u64){
              output_file_size=ULONG_MAX;
            }
          #endif
        }
      }else{
        bit_idx=0;
        status=biguint_logplex_decode_u64(&bit_idx, zip_chunk_idx_max_max, zip_chunk_list_base, &parameter);
        if(status){
          agnentrofile_error_print("mask_idx_max has a corrupt logplex code or needs more than 64 bits");
          break;
        }
        status=1;
        mask_idx_max_logplex_bit_count=(u8)(bit_idx);
        mask_idx_max=(ULONG)(parameter);
        if(mask_idx_max!=parameter){
          agnentrofile_error_print("mask_idx_max exceeds ULONG_BITS bits");
          break;
        }
        DEBUG_U64("mask_idx_max", mask_idx_max);
        status=biguint_logplex_decode_u64(&bit_idx, zip_chunk_idx_max_max, zip_chunk_list_base, &parameter);
        if(status){
          agnentrofile_error_print("mask_max has a corrupt logplex code or needs more than 64 bits");
          break;
        }
        status=1;
        mask_max_logplex_bit_count=(u8)(bit_idx-mask_idx_max_logplex_bit_count);
        mask_max=(u32)(parameter);
        if(mask_max!=parameter){
          agnentrofile_error_print("Cannot handle (mask_max>((2^32)-1))");
          break;
        }
        agnentrofile_mask_max_print(granularity, mask_max);
        if(!mask_max){
          agnentrofile_error_print("Unsupported configuration: mask_max is zero");
          break;
        }
        if((u32)((1U<<(granularity<<U8_BITS_LOG2)<<U8_BITS)-1)<mask_max){
          agnentrofile_error_print("mask_max exceeds (((granularity+1)<<8)-1)");
        }
        DEBUG_U8("mask_idx_max_logplex_bit_count", mask_idx_max_logplex_bit_count);
        DEBUG_U8("mask_max_logplex_bit_count", mask_max_logplex_bit_count);
        agnentrocodec_base=agnentrocodec_init(AGNENTROCODEC_BUILD_BREAK_COUNT_EXPECTED, 0, granularity, loggamma_base, mask_idx_max, mask_max);
        if(!agnentrocodec_base){
          agnentrofile_error_print("Agnentro initialization failed");
          break;
        }
        mask_list_base=agnentrocodec_mask_list_malloc(granularity, mask_idx_max);
        if(!mask_list_base){
          agnentrofile_out_of_memory_print();
          break;
        }
        untrusted_bit_count_minus_1=((u64)(input_file_size)<<U8_BITS_LOG2)-bit_idx-1;
        status=agnentrocodec_code_import(agnentrocodec_base, untrusted_bit_count_minus_1, bit_idx, zip_chunk_idx_max, zip_chunk_list_base);
        if(status){
          agnentrofile_error_print("agnentrocodec_code_import() failed");
          break;
        }
        agnentrocodec_decode(agnentrocodec_base, mask_idx_max, mask_list_base);
        output_file_size=(ULONG)(((u64)(mask_idx_max)+1)*mask_size);
      }
    }else{
      mask_idx0=0;
      mask=0;
      output_file_size=(ULONG)(((u64)(mask_idx_max)+1)*mask_size);
      random_seed=1;
      u8_idx0=0;
      do{
        mask_idx1=agnentrofile_random_get(&random_seed, mask_count);
        u8_idx1=mask_idx1*mask_size;
        if(mode==AGNENTROFILE_MODE_SHUFFLE){
          memcpy(&mask, &mask_list_base[u8_idx0], (size_t)(mask_size));
          memcpy(&mask_list_base[u8_idx0], &mask_list_base[u8_idx1], (size_t)(mask_size));
          memcpy(&mask_list_base[u8_idx1], &mask, (size_t)(mask_size));
        }
        u8_idx0+=mask_size;
      }while((mask_idx0++)!=mask_idx_max);
      if(mode==AGNENTROFILE_MODE_UNSHUFFLE){
        mask_idx0=mask_idx_max;
        do{
          u8_idx0-=mask_size;
          memcpy(&mask, &mask_list_base[u8_idx0], (size_t)(mask_size));
          memcpy(&mask_list_base[u8_idx0], &mask_list_base[u8_idx1], (size_t)(mask_size));
          memcpy(&mask_list_base[u8_idx1], &mask, (size_t)(mask_size));
          mask_idx1=agnentrofile_random_get_reverse(&random_seed, mask_count);
          u8_idx1=mask_idx1*mask_size;
        }while(mask_idx0--);
      }
    }
    DEBUG_U64("output_file_size", output_file_size);
    if((mode!=AGNENTROFILE_MODE_ESTIMATE)&&(mode!=AGNENTROFILE_MODE_EXACT)){
      output_filename_base=argv[5];
      if(mode==AGNENTROFILE_MODE_COMPRESS){
/*
Convert the canonical biguint at zip_chunk_list_base into a bitmap ready for export.
*/
        transfer_bit_idx_max=((u64)(output_file_size)<<U8_BITS_LOG2)-1;
        status=biguint_bitmap_export(transfer_bit_idx_max, zip_chunk_idx_max, zip_chunk_idx_max_max, zip_chunk_list_base);
        if(!status){
          status=filesys_file_write(output_file_size, output_filename_base, zip_chunk_list_base);
        }
      }else{
        status=filesys_file_write(output_file_size, output_filename_base, mask_list_base);
      }
      if(status){
        agnentrofile_error_print("output_file export or write failed");
        break;
      }
    }
    status=0;
  }while(0);
  if(mode==AGNENTROFILE_MODE_ESTIMATE){
    agnentroprox_free_all(agnentroprox_base);
    biguint_free(zip_chunk_list_base);
    agnentroprox_free(mask_list_base);
  }else{
    agnentrocodec_free_all(agnentrocodec_base);
    biguint_free(zip_chunk_list_base);
    agnentrocodec_free(mask_list_base);
  }
  loggamma_free_all(loggamma_base);
  if(overflow_status){
    agnentrofile_error_print("Fracterval precision was exhausted. Please report");
  }
  DEBUG_ALLOCATION_CHECK();
  return status;
}
