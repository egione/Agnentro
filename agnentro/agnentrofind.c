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
Approximate Binary Search
*/
#include "flag.h"
#include "flag_ascii.h"
#include "flag_biguint.h"
#include "flag_filesys.h"
#include "flag_fracterval_u64.h"
#include "flag_fracterval_u128.h"
#include "flag_loggamma.h"
#include "flag_poissocache.h"
#include "flag_agnentroprox.h"
#include "flag_agnentrofind.h"
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
#include "poissocache.h"
#include "poissocache_xtrn.h"
#include "agnentroprox.h"
#include "agnentroprox_xtrn.h"
#include "ascii_xtrn.h"
#include "filesys.h"
#include "filesys_xtrn.h"

#define AGNENTROFIND_GEOMETRY_CHANNELIZE (1U<<AGNENTROFIND_GEOMETRY_CHANNELIZE_BIT_IDX)
#define AGNENTROFIND_GEOMETRY_CHANNELIZE_BIT_IDX 6U
#define AGNENTROFIND_GEOMETRY_DELTAS (3U<<AGNENTROFIND_GEOMETRY_DELTAS_BIT_IDX)
#define AGNENTROFIND_GEOMETRY_DELTAS_BIT_IDX 4U
#define AGNENTROFIND_GEOMETRY_GRANULARITY (3U<<AGNENTROFIND_GEOMETRY_GRANULARITY_BIT_IDX)
#define AGNENTROFIND_GEOMETRY_GRANULARITY_BIT_IDX 0U
#define AGNENTROFIND_GEOMETRY_MODE (1U<<AGNENTROFIND_GEOMETRY_GRANULARITY_BIT_IDX)
#define AGNENTROFIND_GEOMETRY_MODE_BIT_IDX 3U
#define AGNENTROFIND_GEOMETRY_OVERLAP (1U<<AGNENTROFIND_GEOMETRY_OVERLAP_BIT_IDX)
#define AGNENTROFIND_GEOMETRY_OVERLAP_BIT_IDX 7U
#define AGNENTROFIND_FORMAT_ASCENDING (1U<<AGNENTROFIND_FORMAT_ASCENDING_BIT_IDX)
#define AGNENTROFIND_FORMAT_ASCENDING_BIT_IDX 1U
#define AGNENTROFIND_FORMAT_CAVALIER (1U<<AGNENTROFIND_FORMAT_CAVALIER_BIT_IDX)
#define AGNENTROFIND_FORMAT_CAVALIER_BIT_IDX 2U
#define AGNENTROFIND_FORMAT_MERGE (1U<<AGNENTROFIND_FORMAT_MERGE_BIT_IDX)
#define AGNENTROFIND_FORMAT_MERGE_BIT_IDX 0U
#define AGNENTROFIND_FORMAT_PRECISE (1U<<AGNENTROFIND_FORMAT_PRECISE_BIT_IDX)
#define AGNENTROFIND_FORMAT_PRECISE_BIT_IDX 4U
#define AGNENTROFIND_FORMAT_PROGRESS (1U<<AGNENTROFIND_FORMAT_PROGRESS_BIT_IDX)
#define AGNENTROFIND_FORMAT_PROGRESS_BIT_IDX 3U
#define AGNENTROFIND_SWEEP_STATUS_EXACT 0U
#define AGNENTROFIND_SWEEP_STATUS_CUSTOM 1U
#define AGNENTROFIND_SWEEP_STATUS_NEEDLE 2U
#define AGNENTROFIND_SWEEP_STATUS_HAYSTACK 3U

void
agnentrofind_error_print(char *char_list_base){
  DEBUG_PRINT("ERROR: ");
  DEBUG_PRINT(char_list_base);
  DEBUG_PRINT(".\n");
  return;
}

void
agnentrofind_out_of_memory_print(void){
  agnentrofind_error_print("Out of memory");
  return;
}

void
agnentrofind_parameter_error_print(char *char_list_base){
  DEBUG_PRINT("Invalid parameter: (");
  DEBUG_PRINT(char_list_base);
  DEBUG_PRINT("). For help, run without parameters.\n");
  return;
}

void
agnentrofind_warning_print(char *char_list_base){
  DEBUG_PRINT("WARNING: ");
  DEBUG_PRINT(char_list_base);
  DEBUG_PRINT(".\n");
  return;
}

int
main(int argc, char *argv[]){
  agnentroprox_t *agnentroprox_base;
  ULONG arg_idx;
  u8 ascending_status;
  u8 case_insensitive_status;
  u8 cavalier_status;
  u8 channel_status;
  u8 delete_status;
  u8 delta_count;
  u8 delta_idx;
  ULONG deltafy_mask_idx_max;
  u8 digit;
  u8 digit_shift;
  char *dump_u8_list_base;
  ULONG dump_delta;
  u8 dump_delta_sign;
  char *dump_filename_base;
  ULONG dump_filename_replacement_char_idx;
  ULONG dump_filename_size;
  ULONG dump_filename_tail_size;
  ULONG dump_filename_char_idx;
  char *dump_template_filename_base;
  ULONG dump_filename_template_size;
  ULONG dump_size;
  ULONG dump_size_clipped;
  ULONG dump_size_max;
  u8 dump_status;
  fru128 entropy;
  fru128 *entropy_list_base0;
  fru128 *entropy_list_base1;
  fru128 entropy_raw;
  u8 file_status;
  u8 filesys_status;
  u8 granularity;
  u8 granularity_status;
  ULONG haystack_file_size;
  ULONG haystack_file_size_max;
  char *haystack_filename_base;
  ULONG haystack_filename_char_idx;
  ULONG haystack_filename_char_idx_max;
  ULONG haystack_filename_char_idx_new;
  ULONG haystack_filename_count;
  ULONG haystack_filename_idx;
  ULONG *haystack_filename_idx_list_base;
  ULONG haystack_filename_size;
  ULONG haystack_filename_size_new;
  ULONG haystack_mask_idx_max;
  ULONG haystack_mask_idx_max_max;
  u8 *haystack_mask_list_base;
  char *haystack_name_base;
  loggamma_t *loggamma_base;
  u32 mask_max;
  u8 mask_size;
  ULONG match_count;
  ULONG match_idx;
  ULONG match_idx_max_max;
  ULONG match_idx_nested;
  ULONG match_idx_old;
  ULONG match_u8_idx;
  ULONG *match_u8_idx_list_base;
  ULONG match_u8_idx_max;
  ULONG match_u8_idx_nested;
  ULONG match_u8_idx_old;
  ULONG match_u8_idx_post;
  u64 match_u8_idx_u64;
  u8 merge_status;
  u16 mode;
  ULONG needle_file_size;
  ULONG needle_mask_idx_max;
  u8 *needle_mask_list_base;
  char *needle_name_base;
  ULONG needle_name_size;
  ULONG needle_name_size_minus_1;
  char needle_prefix;
  u8 overflow_status;
  u8 overlap_status;
  u64 parameter;
  u8 precise_status;
  u8 progress_status;
  ULONG rank_count;
  ULONG rank_idx;
  ULONG rank_idx_max_max;
  ULONG rank_idx_min;
  fru128 *rank_list_base;
  u128 score;
  u128 score_threshold;
  u64 score_u64;
  u8 status;
  ULONG sweep_mask_idx_max;
  ULONG sweep_mask_idx_max_max;
  ULONG sweep_size;
  u8 sweep_status;
  char *sweep_text_base;
  ULONG utf8_idx_max;

  overflow_status=0;
  status=ascii_init(0, 0);
  status=(u8)(status|filesys_init(0, 0));
  loggamma_base=loggamma_init(0, 0);
  status=(u8)(status|!loggamma_base);
  agnentroprox_base=NULL;
  case_insensitive_status=0;
  dump_u8_list_base=NULL;
  dump_delta=0;
  dump_filename_base=NULL;
  dump_filename_size=0;
  dump_filename_char_idx=0;
  dump_filename_replacement_char_idx=ULONG_MAX;
  dump_template_filename_base=NULL;
  dump_filename_template_size=0;
  dump_size=0;
  dump_status=0;
  entropy_list_base0=NULL;
  entropy_list_base1=NULL;
  FRU128_SET_ZERO(entropy_raw);
  granularity_status=0;
  haystack_filename_idx_list_base=NULL;
  haystack_filename_base=NULL;
  haystack_mask_list_base=NULL;
  match_u8_idx_list_base=NULL;
  needle_mask_list_base=NULL;
  sweep_size=0;
  do{
    if(status){
      agnentrofind_error_print("Outdated source code");
      break;
    }
    status=1;
    if((argc!=6)&&(argc!=7)&&(argc!=10)){
      DEBUG_PRINT("Agnentro Find\nCopyright 2017 Russell Leidich\nhttp://agnentropy.blogspot.com\n");
      DEBUG_U32("build_id_in_hex", AGNENTROFIND_BUILD_ID);
      DEBUG_PRINT("Fuzzy file finder based on information distance.\n\n");
      DEBUG_PRINT("Syntax:\n\n");
      DEBUG_PRINT("agnentrofind needle haystack geometry sweep ranks [format [dump_delta dump_size\n");
      DEBUG_PRINT("  dump_filename]]\n\n");
      DEBUG_PRINT("where all numerical parameters are decimal unless otherwise stated:\n\n");
      DEBUG_PRINT("(needle) is one of the following: (0) a UTF8 text string prefixed with \"@\", and\nsurrounded by single or double quotes if necessary; (1) a series of hex bytes\nstarting with \"+\", for example \"+5cE2\" which means 5C followed by E2; or (2)\nthe name of a file containing the binary data to find. If you don't know some\nof the bytes in the middle, then fake them as plausibly as possible.\n\n");
      DEBUG_PRINT("(haystack) is the file or folder to search for matches (no wildcard characters).\nIn the latter case, all symlinks will be ignored so that no subfolder will be\nprocessed more than once.\n\n");
      DEBUG_PRINT("(geometry) is a hex bitmap which controls mask processing:\n\n  bits 0-1: (granularity) Mask size minus 1. Note that 3 (32 bits per mask)\n  requires 64GiB of memory.\n\n  bit 3: (mode) 0 to use divcompressivity or 1 to use coshannonism.\n\n  bit 4-5: (deltas) The number of times to compute the delta (discrete\n  derivative) of the mask list prior to considering (overlap). Each delta, if\n  any, will transform {A, B, C...} to  {A, (B-A), (C-B)...}. This is useful\n  for improving the entropy contrast of signals containing masks which\n  represent magnitudes, as opposed to merely symbols. Experiment to find the\n  optimum value for your data set. If the mask list size isn't a multiple of\n  ((granularity)+1) bytes, then the remainder bytes will remain unchanged.\n\n  bit 6: (channelize) Set if masks consist of parallel byte channels, for\n  example the red, green, and blue bytes of 24-bit pixels. This will cause\n  deltafication, if enabled, to occur on individual bytes, prior to considering\n  (overlap). For example, {A0:B0, A1:B1, A2:B2} (3 masks spanning 6 bytes)\n  would be transformed to {A0:B0, (A1-A0):(B1-B0), (A2-A1):(B2-B1)}.\n\n  bit 7: (overlap) Overlap masks on byte boundaries. For example, if\n  (granularity) is 2, then {A0:B0:C0, A1:B1:C1} (2 masks spanning 6 bytes, with\n  the low bytes being A0 and A1) would be processed as though it were\n  {A0:B0:C0, B0:C0:A1, C0:A1:B1, A1:B1:C1}. This can improve search quality in\n  cases where context matters, as opposed to merely the frequency distribution\n  of symbols.\n\n");
      DEBUG_PRINT("(sweep) is the nonzero number of masks in the sliding haystack window, except\nfor the following special cases: \"e\" or \"i\" to find only exact matches or\ncase-insensitive exact matches, respectively; or \"n\" or \"h\" to find approximate\nmatches the same size as (needle) or (haystack), respectively. For example, 5\nmeans: a 5-byte window if (granularity)=0, a 10-byte window if (granularity)=1\nand (overlap)=0, or a 7-byte window if (granularity)=2 and (overlap)=1.\n\n");
      DEBUG_PRINT("(ranks) is the nonzero number of slots in the list of (best) matches. Matches\nwill be reported as 0-based file offsets or filenames when haystack is a file\nor folder, respectively. In either case, results will be sorted in the order\nimplied by granularity, such that lesser offsets and files encountered earlier\nwill prevail in case of a tie.\n\n");
      DEBUG_PRINT("(format) is a hex bitmap which controls output formatting:\n\n  bit 0: (merge) Prevent the reporting of more than 1 match per sweep. This is\n  useful for filtering because usually many matches occur within the same\n  sweep. In either case, a sweep with global minimum or maximum score will be\n  reported at rank 0. Ignored when (haystack) is a folder.\n\n  bit 1: (ascending) Display worst matches first. Either way, ties will be\n  resolved in favor of lower sweep offsets.\n\n  bit 2: (cavalier) Do not report errors encountered after commencing analysis.\n\n  bit 3: (progress) Make verbose comments about compute progress.\n\n  bit 4: (precise) Set to display entropy values as 64.64 fixed-point hex\n  fractervals. Fractervals are displayed as {(A.B), (C.D)} where (A.B) is the\n  lower bound and (C.D) is (1/(2^64)) less than the upper bound.\n\n");
      DEBUG_PRINT("The following options are only valid when haystack is a file:\n\n");
      DEBUG_PRINT("(dump_delta) is the number of bytes after the base of a match at which to start\ndumping, such that 0 means to start at the match itself. Prefix with \"-\" to\nindicate a negative value. Reported match offsets will be adjusted accordingly,\nsaturating to [0, (haystack size)-1].\n\n");
      DEBUG_PRINT("(dump_size) must be provided along with dump_delta. It\'s the nonzero number of\nbytes to dump per match.\n\n");
      DEBUG_PRINT("(dump_filename) is \"+\" or \"@\" for hex or UTF8 to stdout, respectively, or\nanything containing \"@\" to be used as a filename template for output, where\n\"@\" will be replaced by the adjusted match offset in hex. Each such file\nwill overwrite any existing version.\n\n");
      break;
    }
    arg_idx=0;
    do{
      status=ascii_utf8_string_verify(argv[arg_idx]);
      if(status){
        agnentrofind_error_print("One or more parameters is encoded using invalid UTF8");
        break;
      }
    }while((++arg_idx)<(ULONG)(argc));
    if(status){
      break;
    }
    sweep_text_base=argv[4];
    status=ascii_decimal_to_u64_convert(sweep_text_base, &parameter, ULONG_MAX);
    sweep_mask_idx_max_max=0;
    sweep_status=AGNENTROFIND_SWEEP_STATUS_CUSTOM;
    if(status){
      if(strlen(sweep_text_base)==1){
        sweep_status=(u8)(sweep_text_base[0]);
        status=0;
        if(sweep_status=='e'){
          sweep_status=AGNENTROFIND_SWEEP_STATUS_EXACT;
        }else if(sweep_status=='h'){
          sweep_status=AGNENTROFIND_SWEEP_STATUS_HAYSTACK;
        }else if(sweep_status=='i'){
          sweep_status=AGNENTROFIND_SWEEP_STATUS_EXACT;
          case_insensitive_status=1;
        }else if(sweep_status=='n'){
          sweep_status=AGNENTROFIND_SWEEP_STATUS_NEEDLE;
        }else{
          status=1;
        }
      }
    }else{
      status=!parameter;
      sweep_mask_idx_max_max=(ULONG)(parameter-1);
    }
    if(status){
      agnentrofind_parameter_error_print("sweep");
      break;
    }
/*
Don't accept ranks larger than we can fit in the address space, considering that it will be used to allocate *entropy_list_base0, which consists of u128 fractervals.
*/
    status=ascii_decimal_to_u64_convert(argv[5], &parameter, ULONG_MAX>>(U128_SIZE_LOG2+1));
    status=(u8)(status|!parameter);
    if(status){
      agnentrofind_parameter_error_print("ranks");
      break;
    }
    rank_idx_max_max=(ULONG)(parameter-1);
    status=ascii_hex_to_u64_convert(argv[3], &parameter, U8_MAX);
    status=(u8)(status&&(parameter&~(AGNENTROFIND_GEOMETRY_CHANNELIZE|AGNENTROFIND_GEOMETRY_DELTAS|AGNENTROFIND_GEOMETRY_GRANULARITY|AGNENTROFIND_GEOMETRY_MODE|AGNENTROFIND_GEOMETRY_OVERLAP)));
    if(status){
      agnentrofind_parameter_error_print("geometry");
      break;
    }
    ascending_status=0;
    cavalier_status=0;
    channel_status=(u8)((parameter>>AGNENTROFIND_GEOMETRY_CHANNELIZE_BIT_IDX)&1);
    delta_count=(u8)((parameter>>AGNENTROFIND_GEOMETRY_DELTAS_BIT_IDX)&3);
    granularity=(u8)((parameter>>AGNENTROFIND_GEOMETRY_GRANULARITY_BIT_IDX)&3);
    merge_status=0;
    mode=AGNENTROPROX_MODE_DIVENTROPY;
    if((parameter>>AGNENTROFIND_GEOMETRY_MODE_BIT_IDX)&1){
      mode=AGNENTROPROX_MODE_COSHANNONISM;
    }    
    overlap_status=(u8)((parameter>>AGNENTROFIND_GEOMETRY_OVERLAP_BIT_IDX)&1);
    precise_status=0;
    progress_status=0;
    if(6<argc){
      status=ascii_hex_to_u64_convert(argv[6], &parameter, U8_MAX);
      status=(u8)(status||(parameter&~(AGNENTROFIND_FORMAT_ASCENDING|AGNENTROFIND_FORMAT_CAVALIER|AGNENTROFIND_FORMAT_MERGE|AGNENTROFIND_FORMAT_PRECISE|AGNENTROFIND_FORMAT_PROGRESS)));
      if(status){
        agnentrofind_parameter_error_print("format");
        break;
      }
      ascending_status=(u8)((parameter>>AGNENTROFIND_FORMAT_ASCENDING_BIT_IDX)&1);
      cavalier_status=(u8)((parameter>>AGNENTROFIND_FORMAT_CAVALIER_BIT_IDX)&1);
      merge_status=(u8)((parameter>>AGNENTROFIND_FORMAT_MERGE_BIT_IDX)&1);
      precise_status=(u8)((parameter>>AGNENTROFIND_FORMAT_PRECISE_BIT_IDX)&1);
      progress_status=(u8)((parameter>>AGNENTROFIND_FORMAT_PROGRESS_BIT_IDX)&1);
      if((sweep_status==AGNENTROFIND_SWEEP_STATUS_EXACT)&&delta_count&&!cavalier_status){
        agnentrofind_warning_print("You asked for exact matches with deltafication enabled. That's weird,\nbut OK");
      }
      if(9<argc){
        status=ascii_decimal_to_u64_convert_negatable(argv[7], &parameter, ULONG_MAX, &dump_delta_sign);
        if(status){
          agnentrofind_parameter_error_print("dump_delta");
          break;
        }
        dump_delta=(ULONG)(parameter);
        status=ascii_decimal_to_u64_convert(argv[8], &parameter, ULONG_MAX);
        status=(u8)(status|!parameter);
        if(status){
          agnentrofind_parameter_error_print("dump_size");
          break;
        }
        dump_template_filename_base=argv[9];
        dump_size=(ULONG)(parameter);
        if(!strcmp(dump_template_filename_base, "+")){
          dump_status=1;
        }else if(!strcmp(dump_template_filename_base, "@")){
          dump_status=2;
        }else{
          dump_status=3;
          dump_filename_template_size=(ULONG)(strlen(dump_template_filename_base));
          status=0;
          do{
            if(dump_template_filename_base[dump_filename_char_idx]=='@'){
              status=(dump_filename_replacement_char_idx!=ULONG_MAX);
              dump_filename_replacement_char_idx=dump_filename_char_idx;
            }
            dump_filename_char_idx++;
          }while(dump_filename_char_idx!=dump_filename_template_size);
          if(status){
            agnentrofind_error_print("More than one \"@\" in (dump_filename)");
            break;
          }else if(dump_filename_replacement_char_idx==ULONG_MAX){
            status=1;
            agnentrofind_error_print("You need \"@\" in (dump_filename)");
            break;
          }
/*
We need to allocate space for the full dump filename, which will contain the existing filename template plus 15 more bytes because "@" will be replaced by a 16-digit hex value, plus another byte for the terminating null, which was not included in dump_filename_template_size.
*/
          dump_filename_size=dump_filename_template_size+15+1;
        }
      }
    }
    needle_name_base=argv[1];
    needle_prefix=needle_name_base[0];
    deltafy_mask_idx_max=0;
    mask_size=(u8)(granularity+1);
    needle_file_size=0;
    needle_mask_idx_max=0;
    needle_name_size=(ULONG)(strlen(needle_name_base));
    needle_name_size_minus_1=needle_name_size-1;
    status=1;
    if(needle_prefix=='+'){
      if(needle_name_size_minus_1&1){
        agnentrofind_error_print("needle must contain an even number of nybbles");
        break;
      }
      needle_file_size=needle_name_size_minus_1>>1;
    }else if(needle_prefix=='@'){
      needle_file_size=needle_name_size_minus_1;
    }else{
      status=filesys_file_size_ulong_get(&needle_file_size, needle_name_base);
      if(status==FILESYS_STATUS_TOO_BIG){
        agnentrofind_error_print("(needle) size exceeds memory size");
        break;
      }else if(status){
        agnentrofind_error_print("Cannot determine (needle) size");
        break;
      }
    }
    needle_mask_idx_max=agnentroprox_mask_idx_max_get(granularity, &granularity_status, needle_file_size, overlap_status);
    if(delta_count){
      deltafy_mask_idx_max=agnentroprox_mask_idx_max_get(granularity, &granularity_status, needle_file_size, 0);
    }
    if(granularity_status){
      if(needle_mask_idx_max==ULONG_MAX){
        status=1;
        agnentrofind_error_print("(needle) size is less than (granularity+1) bytes");
        break;
      }else if(!cavalier_status){
        agnentrofind_warning_print("(needle) size isn't a multiple of (granularity+1), so tail bytes will\nbe ignored");
      }
    }
    status=1;
    if((sweep_status==AGNENTROFIND_SWEEP_STATUS_EXACT)||(sweep_status==AGNENTROFIND_SWEEP_STATUS_NEEDLE)){
      sweep_mask_idx_max_max=needle_mask_idx_max;
    }
    needle_mask_list_base=agnentroprox_mask_list_malloc(granularity, needle_mask_idx_max, overlap_status);
    if(!needle_mask_list_base){
      agnentrofind_out_of_memory_print();
      break;
    }
    if(progress_status){
      DEBUG_PRINT("Processing (needle)...\n");
    }
    if(needle_prefix=='+'){
      status=ascii_hex_to_u8_list_convert(needle_name_size_minus_1, 1, needle_name_base, needle_mask_list_base);
      status=!!status;
      if(status){
        agnentrofind_error_print("(needle) is not a valid hexadecimal value");
        break;
      }
      status=1;
    }else if(needle_prefix=='@'){
      memcpy(&needle_mask_list_base[0], &needle_name_base[1], (size_t)(needle_name_size_minus_1));
    }else{
      status=filesys_file_read_exact(needle_file_size, needle_name_base, needle_mask_list_base);
      if(status){
        if(status==FILESYS_STATUS_SIZE_CHANGED){
          agnentrofind_error_print("(needle) size changed during execution");
          break;
        }else{
          agnentrofind_error_print("Could not read (needle)");
          break;
        }
      }
    }
    haystack_name_base=argv[2];
    haystack_file_size_max=0;
    haystack_filename_count=0;
    haystack_filename_size=U16_MAX;
    status=1;
    do{
      haystack_filename_char_idx_max=haystack_filename_size-1;
      haystack_filename_base=filesys_char_list_malloc(haystack_filename_char_idx_max);
      if(!haystack_filename_base){
        agnentrofind_out_of_memory_print();
        break;
      }
      haystack_filename_size_new=haystack_filename_size;
      haystack_filename_count=filesys_filename_list_get(&haystack_file_size_max, &file_status, haystack_filename_base, &haystack_filename_size_new, haystack_name_base);
      if(!haystack_filename_count){
        agnentrofind_error_print("(haystack) not found or inaccessible");
        break;
      }
      status=0;
      if(haystack_filename_size<haystack_filename_size_new){
        haystack_filename_base=filesys_free(haystack_filename_base);
        haystack_filename_size=haystack_filename_size_new;
        status=1;
      }
    }while(status);
    if(status){
      break;
    }
    status=1;
    if(haystack_file_size_max<mask_size){
      agnentrofind_error_print("All haystack files are smaller than a single (granularity+1)-sized mask");
      break;
    }
/*
Ignore the return status of agnentroprox_mask_idx_max_get() because it doesn't matter whether the maximum haystack size is compatible with granularity and overlap. Just check for a return of ULONG_MAX, which indicates that the maximum haystack size is smaller than (granularity+1).
*/
    haystack_mask_idx_max_max=agnentroprox_mask_idx_max_get(granularity, &granularity_status, haystack_file_size_max, overlap_status);
    if((haystack_mask_idx_max_max<sweep_mask_idx_max_max)||(haystack_mask_idx_max_max==ULONG_MAX)){
      agnentrofind_error_print("All (haystack) files contain fewer masks than sweep would require");
      break;
    }
    status=0;
    if(sweep_status==AGNENTROFIND_SWEEP_STATUS_HAYSTACK){
      sweep_mask_idx_max_max=haystack_mask_idx_max_max;
    }
    if(!file_status){
/*
If we're in exact match mode, then set match_idx_max_max to ULONG_MAX because we want to count all matches in the file. Otherwise set it to zero, indicating that we want only the entropy extremum.
*/
      match_idx_max_max=0;
      if(sweep_status==AGNENTROFIND_SWEEP_STATUS_EXACT){
        match_idx_max_max--;
      }
      if(dump_status|merge_status){
        agnentrofind_error_print("Dumping or merging is not available if (haystack) is a folder");
        status=1;
        break;
      }
    }else{
      match_idx_max_max=rank_idx_max_max;
      if(dump_status==2){
        dump_size_max=dump_size;
        if(!dump_size){
          dump_size_max=haystack_file_size_max;
        }
        dump_u8_list_base=ascii_utf8_safe_list_malloc(dump_size_max);
        status=(u8)(status|!dump_u8_list_base);
      }else if(dump_status==3){
        dump_filename_base=filesys_char_list_malloc(dump_filename_size);
        status=(u8)(status|!dump_filename_base);
        if(!status){
          memcpy(dump_filename_base, dump_template_filename_base, (size_t)(dump_filename_replacement_char_idx));
          memset(&dump_filename_base[dump_filename_replacement_char_idx], '0', (size_t)(16));
          dump_filename_tail_size=dump_filename_template_size-dump_filename_replacement_char_idx-1;
          memcpy(&dump_filename_base[dump_filename_replacement_char_idx+16], &dump_template_filename_base[dump_filename_replacement_char_idx+1], (size_t)(dump_filename_tail_size));
          dump_filename_base[dump_filename_size-1]=0;
        }
      }
    }
    entropy_list_base0=fracterval_u128_rank_list_malloc(rank_idx_max_max);
    status=(u8)(status|!entropy_list_base0);
    entropy_list_base1=fracterval_u128_rank_list_malloc(0);
    status=(u8)(status|!entropy_list_base1);
    haystack_filename_idx_list_base=agnentroprox_ulong_list_malloc(rank_idx_max_max);
    status=(u8)(status|!haystack_filename_idx_list_base);
    haystack_mask_list_base=agnentroprox_mask_list_malloc(granularity, haystack_mask_idx_max_max, overlap_status);
    status=(u8)(status|!haystack_mask_list_base);
    if(file_status){
      match_u8_idx_list_base=agnentroprox_ulong_list_malloc(match_idx_max_max);
      status=(u8)(status|!match_u8_idx_list_base);
    }
    if(status){
      agnentrofind_out_of_memory_print();
      break;
    }
    mask_max=(u32)((1U<<(granularity<<U8_BITS_LOG2)<<U8_BITS)-1);
    status=1;
    agnentroprox_base=agnentroprox_init(0, 0, granularity, loggamma_base, haystack_mask_idx_max_max, mask_max, mode, needle_mask_idx_max, overlap_status, sweep_mask_idx_max_max);
    if(!agnentroprox_base){
      agnentrofind_out_of_memory_print();
      break;
    }
    if(delta_count){
      if(progress_status){
        DEBUG_PRINT("Computing ");
        switch(delta_count){
        case 1:
          DEBUG_PRINT("1st");
          break;
        case 2:
          DEBUG_PRINT("2nd");
          break;
        case 3:
          DEBUG_PRINT("3rd");
          break;
        }
        DEBUG_PRINT(" delta with");
        if(!channel_status){
          DEBUG_PRINT("out");
        }
        DEBUG_PRINT(" channelization...\n");
      }
/*
As specified in the help text, any remainder bytes will be unchanged by deltafication. We are guaranteed at least one full mask, due to agnentroprox_mask_idx_max_get() returning good status.
*/
      delta_idx=0;
      do{
        status=agnentroprox_mask_list_deltafy(channel_status, 1, granularity, deltafy_mask_idx_max, needle_mask_list_base);
      }while((delta_idx++)!=delta_count);
    }
/*
If (sweep_status==AGNENTROFIND_SWEEP_STATUS_EXACT), then we need to look for exact matches, otherwise approximate ones, using entirely different Agnentroprox functions. In the former case, *rank_list_base will end up containing sorted maximum match counts; in the latter, it will end up containing sorted divcompressivities.
*/
    if(sweep_status!=AGNENTROFIND_SWEEP_STATUS_EXACT){
      agnentroprox_needle_mask_list_load(agnentroprox_base, needle_mask_idx_max, needle_mask_list_base);
    }
    entropy_raw=agnentroprox_entropy_raw_get(agnentroprox_base, needle_mask_idx_max, &overflow_status);
    U128_SET_ZERO(score);
    U128_FROM_BOOL(score_threshold, ascending_status);
    haystack_filename_char_idx=0;
    haystack_filename_idx=0;
    match_count=0;
    rank_count=0;
    rank_idx=0;
    do{
      deltafy_mask_idx_max=0;
      granularity_status=0;
      if(progress_status){
        DEBUG_PRINT("Analyzing ");
        DEBUG_PRINT(&haystack_filename_base[haystack_filename_char_idx]);
        DEBUG_PRINT("...\n");
      }
      haystack_file_size=haystack_file_size_max;
      haystack_filename_char_idx_new=haystack_filename_char_idx;
      filesys_status=filesys_file_read_next(&haystack_file_size, &haystack_filename_char_idx_new, haystack_filename_base, haystack_mask_list_base);
      status=1;
      if(!filesys_status){
        haystack_mask_idx_max=agnentroprox_mask_idx_max_get(granularity, &granularity_status, haystack_file_size, overlap_status);
        if(delta_count){
          deltafy_mask_idx_max=agnentroprox_mask_idx_max_get(granularity, &granularity_status, haystack_file_size, 0);
        }
        status=(haystack_mask_idx_max==ULONG_MAX);
        if(!status){
          sweep_mask_idx_max=sweep_mask_idx_max_max;
          if(sweep_status==AGNENTROFIND_SWEEP_STATUS_HAYSTACK){
            sweep_mask_idx_max=haystack_mask_idx_max;
          }else if(haystack_mask_idx_max<sweep_mask_idx_max){
            filesys_status=FILESYS_STATUS_CALLER_CUSTOM2;
            status=1;
          }
          sweep_size=(sweep_mask_idx_max+1)*(u8)((u8)(granularity*(!overlap_status))+1);
          if(!status){
            if(delta_count){
              if(progress_status){
                DEBUG_PRINT("Computing ");
                switch(delta_count){
                case 1:
                  DEBUG_PRINT("1st");
                  break;
                case 2:
                  DEBUG_PRINT("2nd");
                  break;
                case 3:
                  DEBUG_PRINT("3rd");
                  break;
                }
                DEBUG_PRINT(" delta with");
                if(!channel_status){
                  DEBUG_PRINT("out");
                }
                DEBUG_PRINT(" channelization...\n");
              }
              delta_idx=0;
              do{
                status=agnentroprox_mask_list_deltafy(channel_status, 1, granularity, deltafy_mask_idx_max, haystack_mask_list_base);
              }while((delta_idx++)!=delta_count);
            }
            if(progress_status){
              DEBUG_PRINT("Doing ");
              if(sweep_status!=AGNENTROFIND_SWEEP_STATUS_EXACT){
                if(mode==AGNENTROPROX_MODE_COSHANNONISM){
                  DEBUG_PRINT("coshannonism");
                }else{
                  DEBUG_PRINT("divcompressivity");
                }
                DEBUG_PRINT(" transform with");
                if(!overlap_status){
                  DEBUG_PRINT("out");
                }
                DEBUG_PRINT(" mask overlap...\n");
              }else{
                DEBUG_PRINT("string search...\n");
              }
            }
            rank_list_base=entropy_list_base0;
            if(!file_status){
              rank_list_base=entropy_list_base1;
            }
            if(sweep_status!=AGNENTROFIND_SWEEP_STATUS_EXACT){
              if(mode==AGNENTROPROX_MODE_COSHANNONISM){
                match_count=agnentroprox_coshannonism_transform(agnentroprox_base, ascending_status, rank_list_base, haystack_mask_idx_max, haystack_mask_list_base, match_idx_max_max, match_u8_idx_list_base, &overflow_status, sweep_mask_idx_max);
                entropy=rank_list_base[0];
              }else{
                ascending_status=!ascending_status;
                match_count=agnentroprox_diventropy_transform(agnentroprox_base, ascending_status, rank_list_base, haystack_mask_idx_max, haystack_mask_list_base, match_idx_max_max, match_u8_idx_list_base, &overflow_status, sweep_mask_idx_max);
                ascending_status=!ascending_status;
/*
We're guaranteed at least one match, so go ahead and convert it to divcompressivity.
*/
                entropy=rank_list_base[0];
                if(mode!=AGNENTROPROX_MODE_COSHANNONISM){
                  entropy=agnentroprox_compressivity_get(entropy, entropy_raw);
                }
              }
              FRU128_MEAN_TO_FTD128(score, entropy);
            }else{
              match_count=agnentroprox_match_find(ascending_status, case_insensitive_status, granularity, haystack_mask_idx_max, haystack_mask_list_base, match_idx_max_max, match_u8_idx_list_base, needle_mask_idx_max, needle_mask_list_base, overlap_status);
              U128_FROM_U64_HI(score, (u64)(match_count));
              FRU128_FROM_FTD128(entropy, score);
            }
            if(!file_status){
              if(U128_IS_NOT_ZERO(score)||(sweep_status!=AGNENTROFIND_SWEEP_STATUS_EXACT)){
                status=1;
                if(ascending_status){
                  if(U128_IS_LESS_EQUAL(score, score_threshold)){
                    status=fracterval_u128_rank_list_insert_ascending(entropy, &rank_count, &rank_idx, rank_idx_max_max, entropy_list_base0, &score_threshold);
                  }
                }else{
                  if(U128_IS_LESS_EQUAL(score_threshold, score)){
                    status=fracterval_u128_rank_list_insert_descending(entropy, &rank_count, &rank_idx, rank_idx_max_max, entropy_list_base0, &score_threshold);
                  }
                }
                if(!status){
                  rank_idx_min=rank_idx;
                  rank_idx=rank_count-1;
                  while(rank_idx!=rank_idx_min){
                    haystack_filename_idx_list_base[rank_idx]=haystack_filename_idx_list_base[rank_idx-1];
                    rank_idx--;
                  }
                  haystack_filename_idx_list_base[rank_idx]=haystack_filename_char_idx;
                }
                status=0;
              }
            }else if((mode!=AGNENTROPROX_MODE_COSHANNONISM)&&(sweep_status!=AGNENTROFIND_SWEEP_STATUS_EXACT)){
/*
We're guaranteed at least one match, so go ahead and convert it, and any others if they exist, to divcompressivity.
*/
              match_idx=0;
              do{
                entropy=entropy_list_base0[match_idx];
                entropy=agnentroprox_compressivity_get(entropy, entropy_raw);
                entropy_list_base0[match_idx]=entropy;
                match_idx++;
              }while(match_idx!=match_count);
            }
          }
        }else{
          filesys_status=FILESYS_STATUS_CALLER_CUSTOM;
          status=1;
        }
      }
      if(((!cavalier_status)&(granularity_status|status))|(progress_status&!status)){
        if(!cavalier_status){
          if((!progress_status)&(granularity_status|status)){
            DEBUG_PRINT(&haystack_filename_base[haystack_filename_char_idx]);
            DEBUG_PRINT("\n");
          }
/*
Don't warn about incompatible granularity if we have some other error which would moot the issue.
*/
          if(granularity_status&!status){
            agnentrofind_warning_print("Size wasn't a multiple of (granularity+1), so tail bytes were ignored");
          }
        }
        switch(filesys_status){
        case 0:
          if(match_count&&progress_status){
            if(sweep_status!=AGNENTROFIND_SWEEP_STATUS_EXACT){
              if(sweep_status==AGNENTROFIND_SWEEP_STATUS_HAYSTACK){
                DEBUG_PRINT("(haystack) ");
              }else{
                if(!ascending_status){
                  DEBUG_PRINT("Max ");
                }else{
                  DEBUG_PRINT("Min ");
                }
              }
              DEBUG_PRINT("score is ");
            }else{
              DEBUG_PRINT("Match count is ");
            }
            if((!precise_status)|(sweep_status==AGNENTROFIND_SWEEP_STATUS_EXACT)){
              U128_TO_U64_HI(score_u64, score);
              DEBUG_U64("", score_u64);
            }else{
              DEBUG_F128_PAIR("", entropy.a, entropy.b);
            }
            DEBUG_PRINT(".\n");
          }
          break;
        case FILESYS_STATUS_NOT_FOUND:
          agnentrofind_error_print("Not found");
          break;
        case FILESYS_STATUS_TOO_BIG:
          agnentrofind_error_print("Too big to fit in memory");
          break;
        case FILESYS_STATUS_READ_FAIL:
          agnentrofind_error_print("Read failed");
          break;
        case FILESYS_STATUS_SIZE_CHANGED:
          agnentrofind_error_print("File size changed during read");
          break;
        case FILESYS_STATUS_CALLER_CUSTOM:
          agnentrofind_error_print("Size is less than (granularity+1) bytes");
          break;
        case FILESYS_STATUS_CALLER_CUSTOM2:
          agnentrofind_error_print("Contains fewer masks than one sweep would require");
          break;
        default:
          agnentrofind_error_print("Internal error. Please report");
        }
      }
      haystack_filename_char_idx=haystack_filename_char_idx_new;
      haystack_filename_idx++;
      status=0;
    }while(haystack_filename_idx!=haystack_filename_count);
    if(progress_status){
      if(match_count|rank_count){
        DEBUG_PRINT("Here are the results ranked ");
        if(ascending_status){
          DEBUG_PRINT("as");
        }else{
          DEBUG_PRINT("des");
        }
        DEBUG_PRINT("ending by ");
        if(sweep_status!=AGNENTROFIND_SWEEP_STATUS_EXACT){
          if(mode==AGNENTROPROX_MODE_COSHANNONISM){
            DEBUG_PRINT("coshannonism:\n");
          }else{
            DEBUG_PRINT("divcompressivity:\n");
          }
        }else{
          if(!file_status){
            DEBUG_PRINT("match count:\n");
          }else{
            DEBUG_PRINT("base offset:\n");
          }
        }
      }else{
        DEBUG_PRINT("No matches found!\n");
      }
    }
    if(!file_status){
      rank_idx=0;
      while(rank_idx<rank_count){
        entropy=entropy_list_base0[rank_idx];
        if((!precise_status)|(sweep_status==AGNENTROFIND_SWEEP_STATUS_EXACT)){
          FRU128_MEAN_TO_FTD128(score, entropy);
          U128_TO_U64_HI(score_u64, score);
          DEBUG_U64("", score_u64);
        }else{
          DEBUG_F128_PAIR("", entropy.a, entropy.b);
        }
        DEBUG_PRINT(" ");
        DEBUG_PRINT(&haystack_filename_base[haystack_filename_idx_list_base[rank_idx]]);
        DEBUG_PRINT("\n");
        rank_idx++;
      }
    }else{
      if(match_count){
        if(merge_status){
/*
Get rid of overlapping sweep regions, prioritizing those of inferior rank for removal.
*/
          match_idx=match_count-1;
          while(match_idx){
            match_u8_idx=match_u8_idx_list_base[match_idx];
            match_idx_nested=0;
            do{
              match_u8_idx_nested=match_u8_idx_list_base[match_idx_nested];
              delete_status=0;
              if(match_u8_idx<match_u8_idx_nested){
                if((match_u8_idx_nested-match_u8_idx)<sweep_size){
                  delete_status=1;
                  break;
                }
              }else if(match_u8_idx_nested<match_u8_idx){
                if((match_u8_idx-match_u8_idx_nested)<sweep_size){
                  delete_status=1;
                  break;
                }
              }
              match_idx_nested++;
            }while(match_idx_nested!=match_count);
            if(delete_status){
              match_idx_nested=match_idx;
              while((++match_idx_nested)!=match_count){
                match_u8_idx_list_base[match_idx_nested-1]=match_u8_idx_list_base[match_idx_nested];
                entropy_list_base0[match_idx_nested-1]=entropy_list_base0[match_idx_nested];
              }
              match_count--;
            }
            match_idx--;
          }
        }
        if(dump_status){
          if(delta_count){
            if(progress_status){
              DEBUG_PRINT("Undoing deltafication...\n");
            }
            delta_idx=0;
            do{
              status=agnentroprox_mask_list_deltafy(channel_status, 0, granularity, deltafy_mask_idx_max, haystack_mask_list_base);
            }while((delta_idx++)!=delta_count);
          }
          if(progress_status){
            DEBUG_PRINT("Dumping requested data...\n");
          }
          match_idx=0;
          match_idx_old=0;
          match_u8_idx=ULONG_MAX;
          do{
            match_u8_idx_old=match_u8_idx;
            match_u8_idx=match_u8_idx_list_base[match_idx_old];
            match_idx_old++;
            if(dump_delta_sign){
              if(match_u8_idx<=dump_delta){
                match_u8_idx=0;
                if(!match_u8_idx_old){
                  match_idx--;
                }
              }else{
                match_u8_idx-=dump_delta;
              }
            }else{
              match_u8_idx+=dump_delta;
              if((match_u8_idx<dump_delta)||(haystack_file_size<=match_u8_idx)){
                match_u8_idx=haystack_file_size-1;
                if(match_u8_idx==match_u8_idx_old){
                  break;
                }
              }
            }
            match_u8_idx_list_base[match_idx]=match_u8_idx;
            match_idx++;
          }while(match_idx_old!=match_count);
          match_count=match_idx;
        }
        match_idx=0;
        do{
          if(sweep_status!=AGNENTROFIND_SWEEP_STATUS_EXACT){
            entropy=entropy_list_base0[match_idx];
            if(!precise_status){
              FRU128_MEAN_TO_FTD128(score, entropy);
              U128_TO_U64_HI(score_u64, score);
              DEBUG_U64("", score_u64);
            }else{
              DEBUG_F128_PAIR("", entropy.a, entropy.b);
            }
            DEBUG_PRINT(" ");
          }
          match_u8_idx=match_u8_idx_list_base[match_idx];
          DEBUG_U64("", match_u8_idx);
          DEBUG_PRINT("\n");
          if(dump_status){
            match_u8_idx_post=match_u8_idx+dump_size;
            if((match_u8_idx_post<dump_size)||(haystack_file_size<match_u8_idx_post)){
              match_u8_idx_post=haystack_file_size;
            }
            dump_size_clipped=match_u8_idx_post-match_u8_idx;
            switch(dump_status){
            case 1:
              DEBUG_LIST("", dump_size_clipped, (u8 *)(&haystack_mask_list_base[match_u8_idx]), 0);
              break;
            case 2:
              match_u8_idx_max=match_u8_idx_post-1;
              dump_size_clipped=ascii_utf8_sanitize(1, dump_u8_list_base, match_u8_idx_max, match_u8_idx, (char *)(haystack_mask_list_base), &utf8_idx_max);
              DEBUG_PRINT("  {");
              DEBUG_PRINT(dump_u8_list_base);
              DEBUG_PRINT("}\n");
              break;
            case 3:
              digit_shift=(u8)(U64_BITS-4);
              dump_filename_char_idx=dump_filename_replacement_char_idx;
              match_u8_idx_u64=match_u8_idx;
              do{
                digit=(u8)((match_u8_idx_u64>>digit_shift)&0xF);
                digit_shift=(u8)(digit_shift-4);
                if(digit<=9){
                  digit=(u8)(digit+'0');
                }else{
                  digit=(u8)(digit+'A'-0xA);
                }
                dump_filename_base[dump_filename_char_idx]=(char)(digit);
                dump_filename_char_idx++;
              }while(digit_shift<=U64_BIT_MAX);
              status=filesys_file_write(dump_size_clipped, dump_filename_base, &haystack_mask_list_base[match_u8_idx]);
              if(((!cavalier_status)&status)|progress_status){
                DEBUG_PRINT("  Saving to ");
                DEBUG_PRINT(dump_filename_base);
                DEBUG_PRINT("... ");
                if((!cavalier_status)&status){
                  DEBUG_PRINT("failed.\n");
                }else{
/*
If (cavalier_status==status==1), we shouldn't report the error because we've been told not to, which is probably because the user expects a predictable output format for subsequent parsing. So don't report it.
*/
                  DEBUG_PRINT("done.\n");
                }
              }
              break;
            }
          }
          match_idx++;
        }while(match_idx!=match_count);
      }
    }
    status=1;
    if(overflow_status){
      if(!cavalier_status){
        agnentrofind_error_print("Fracterval precision was exhausted. Please report");
      }
      break;
    }
    status=0;
  }while(0);
  agnentroprox_free_all(agnentroprox_base);
  agnentroprox_free(match_u8_idx_list_base);
  agnentroprox_free(haystack_mask_list_base);
  agnentroprox_free(haystack_filename_idx_list_base);
  fracterval_u128_free(entropy_list_base1);
  fracterval_u128_free(entropy_list_base0);
  filesys_free(dump_filename_base);
  ascii_free(dump_u8_list_base);
  filesys_free(haystack_filename_base);
  agnentroprox_free(needle_mask_list_base);
  loggamma_free_all(loggamma_base);
  DEBUG_ALLOCATION_CHECK();
  return status;
}
