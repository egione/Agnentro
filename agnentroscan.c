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
Entropy Scanner and Weirdness Finder
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
#include "flag_agnentroscan.h"
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

#define AGNENTROSCAN_GEOMETRY_CHANNELIZE (1U<<AGNENTROSCAN_GEOMETRY_CHANNELIZE_BIT_IDX)
#define AGNENTROSCAN_GEOMETRY_CHANNELIZE_BIT_IDX 6U
#define AGNENTROSCAN_GEOMETRY_DELTAS (3U<<AGNENTROSCAN_GEOMETRY_DELTAS_BIT_IDX)
#define AGNENTROSCAN_GEOMETRY_DELTAS_BIT_IDX 4U
#define AGNENTROSCAN_GEOMETRY_GRANULARITY (3U<<AGNENTROSCAN_GEOMETRY_GRANULARITY_BIT_IDX)
#define AGNENTROSCAN_GEOMETRY_GRANULARITY_BIT_IDX 0U
#define AGNENTROSCAN_GEOMETRY_OVERLAP (1U<<AGNENTROSCAN_GEOMETRY_OVERLAP_BIT_IDX)
#define AGNENTROSCAN_GEOMETRY_OVERLAP_BIT_IDX 7U
#define AGNENTROSCAN_FORMAT_ASCENDING (1U<<AGNENTROSCAN_FORMAT_ASCENDING_BIT_IDX)
#define AGNENTROSCAN_FORMAT_ASCENDING_BIT_IDX 1U
#define AGNENTROSCAN_FORMAT_CAVALIER (1U<<AGNENTROSCAN_FORMAT_CAVALIER_BIT_IDX)
#define AGNENTROSCAN_FORMAT_CAVALIER_BIT_IDX 2U
#define AGNENTROSCAN_FORMAT_MERGE (1U<<AGNENTROSCAN_FORMAT_MERGE_BIT_IDX)
#define AGNENTROSCAN_FORMAT_MERGE_BIT_IDX 0U
#define AGNENTROSCAN_FORMAT_PRECISE (1U<<AGNENTROSCAN_FORMAT_PRECISE_BIT_IDX)
#define AGNENTROSCAN_FORMAT_PRECISE_BIT_IDX 4U
#define AGNENTROSCAN_FORMAT_PROGRESS (1U<<AGNENTROSCAN_FORMAT_PROGRESS_BIT_IDX)
#define AGNENTROSCAN_FORMAT_PROGRESS_BIT_IDX 3U
#define AGNENTROSCAN_SWEEP_STATUS_CUSTOM 0U
#define AGNENTROSCAN_SWEEP_STATUS_HAYSTACK 1U

void
agnentroscan_error_print(char *char_list_base){
  DEBUG_PRINT("ERROR: ");
  DEBUG_PRINT(char_list_base);
  DEBUG_PRINT(".\n");
  return;
}

void
agnentroscan_mode_text_print(u16 mode, u8 normalized_status){
  switch(mode){
  case AGNENTROPROX_MODE_AGNENTROPY:
    if(normalized_status){
      DEBUG_PRINT("compressivity");
    }else{
      DEBUG_PRINT("agnentropy");
    }
    break;
  case AGNENTROPROX_MODE_EXOELASTICITY:
    DEBUG_PRINT("exoelasticity");
    break;
  case AGNENTROPROX_MODE_EXOENTROPY:
    if(normalized_status){
      DEBUG_PRINT("exocompressivity");
    }else{
      DEBUG_PRINT("exoentropy");
    }
    break;
  case AGNENTROPROX_MODE_JSET:
    DEBUG_PRINT("(1-(Jensen-Shannon exodivergence))");
    break;
  case AGNENTROPROX_MODE_KURTOSIS:
    DEBUG_PRINT("obtuse kurtosis");
    break;
  case AGNENTROPROX_MODE_LET:
    DEBUG_PRINT("(1-(Leidich exodivergence))");
    break;
  case AGNENTROPROX_MODE_LOGFREEDOM:
    if(normalized_status){
      DEBUG_PRINT("dyspoissonism");
    }else{
      DEBUG_PRINT("logfreedom");
    }
    break;
  case AGNENTROPROX_MODE_SHANNON:
    if(normalized_status){
      DEBUG_PRINT("shannonism");
    }else{
      DEBUG_PRINT("Shannon entropy");
    }
    break;
  case AGNENTROPROX_MODE_VARIANCE:
    DEBUG_PRINT("obtuse variance");
    break;
  }
  return;
}

void
agnentroscan_out_of_memory_print(void){
  agnentroscan_error_print("Out of memory");
  return;
}

void
agnentroscan_parameter_error_print(char *char_list_base){
  DEBUG_PRINT("Invalid parameter: (");
  DEBUG_PRINT(char_list_base);
  DEBUG_PRINT("). For help, run without parameters.\n");
  return;
}

void
agnentroscan_warning_print(char *char_list_base){
  DEBUG_PRINT("WARNING: ");
  DEBUG_PRINT(char_list_base);
  DEBUG_PRINT(".\n");
  return;
}

int
main(int argc, char *argv[]){
  agnentroprox_t *agnentroprox_base;
  u8 append_mode;
  ULONG arg_idx;
  u8 cavalier_status;
  u8 channel_status;
  u8 clip_mode;
  u8 delete_status;
  u8 delta_count;
  u8 delta_idx;
  ULONG deltafy_mask_idx_max;
  u8 digit;
  u8 digit_shift;
  u8 direction_status;
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
  ULONG entropy_list_size;
  u128 entropy_mean;
  fru128 entropy_raw;
  u128 entropy_threshold;
  u8 file_status;
  u8 filesys_status;
  u8 granularity;
  u8 granularity_status;
  ULONG haystack_file_size;
  ULONG haystack_file_size_max;
  char *haystack_filename_base;
  ULONG haystack_filename_count;
  ULONG haystack_filename_idx;
  ULONG *haystack_filename_idx_list_base;
  char *haystack_filename_list_base;
  ULONG haystack_filename_list_char_idx;
  ULONG haystack_filename_list_char_idx_max;
  ULONG haystack_filename_list_char_idx_new;
  ULONG haystack_filename_list_size;
  ULONG haystack_filename_list_size_new;
  ULONG haystack_mask_idx_max;
  ULONG haystack_mask_idx_max_max;
  u8 *haystack_mask_list_base;
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
  u128 mean_f128;
  u8 merge_status;
  u16 mode;
  char *mode_text_base;
  u8 normalized_status;
  char *out_filename_base;
  char *out_filename_list_base;
  ULONG out_filename_list_char_idx;
  ULONG out_filename_list_char_idx_max;
  ULONG out_filename_list_char_idx_new;
  ULONG out_filename_list_size;
  ULONG out_filename_size;
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
  u64 score;
  ULONG score_idx;
  fru128 score_packed;
  u8 sign_status;
  u8 status;
  ULONG sweep_mask_idx_max;
  ULONG sweep_mask_idx_max_max;
  ULONG sweep_size;
  u8 sweep_status;
  char *sweep_text_base;
  ULONG utf8_idx_max;

  overflow_status=0;
  status=ascii_init(ASCII_BUILD_BREAK_COUNT_EXPECTED, 0);
  status=(u8)(status|filesys_init(FILESYS_BUILD_BREAK_COUNT_EXPECTED, 2));
  status=(u8)(status|fracterval_u128_init(FRU128_BUILD_BREAK_COUNT_EXPECTED, 0));
  loggamma_base=loggamma_init(LOGGAMMA_BUILD_BREAK_COUNT_EXPECTED, 0);
  status=(u8)(status|!loggamma_base);
  agnentroprox_base=NULL;
  dump_u8_list_base=NULL;
  dump_delta=0;
  dump_filename_base=NULL;
  dump_filename_size=0;
  dump_filename_char_idx=0;
  dump_filename_replacement_char_idx=ULONG_MAX;
  dump_filename_template_size=0;
  dump_size=0;
  dump_status=0;
  dump_template_filename_base=NULL;
  FRU128_SET_ZERO(entropy);
  entropy_list_base0=NULL;
  entropy_list_base1=NULL;
  entropy_list_size=0;
  FRU128_SET_ZERO(entropy_raw);
  haystack_filename_idx_list_base=NULL;
  haystack_filename_list_base=NULL;
  haystack_mask_list_base=NULL;
  match_u8_idx_list_base=NULL;
  U128_SET_ZERO(mean_f128);
  mode=0;
  normalized_status=0;
  out_filename_list_base=NULL;
  sign_status=0;
  do{
    if(status){
      agnentroscan_error_print("Outdated source code");
      break;
    }
    status=1;
    if((argc!=6)&&(argc!=7)&&(argc!=10)){
      DEBUG_PRINT("Agnentro Scan\nCopyright 2017 Russell Leidich\nhttp://agnentropy.blogspot.com\n");
      DEBUG_U32("build_id_in_hex", AGNENTROSCAN_BUILD_ID);
      DEBUG_PRINT("Scan for anomalies using various entropy metrics.\n\n");
      DEBUG_PRINT("Syntax:\n\n");
      DEBUG_PRINT("agnentroscan mode haystack geometry sweep ranks [format [dump_delta dump_size\n");
      DEBUG_PRINT("  dump_filename]]\n\n");
      DEBUG_PRINT("where all numerical parameters are decimal unless otherwise stated:\n\n");
      DEBUG_PRINT("(mode) is the type of entropy to compute in nats (bits times log(2)): \"A\" for\nagnentropy, \"E\" for exoentropy, \"L\" for logfreedom, or \"S\" for Shannon entropy.\nLowercase letters will compute the corresponding inverse normalized quantity,\ni.e. a 64-bit fraction on [0, 1] where greater values correspond to less\nentropy: \"a\" for compressivity, \"e\" for exocompressivity, \"l\" for\ndyspoissonism, or \"s\" for shannonism. Use \"j\" for Jensen-Shannon\nexodivergence, \"i\" for Leidich exodivergence, or \"x\" for exoelasticity, all of\nwhich being inherently normalized.\n\n");
      DEBUG_PRINT("(haystack) is the file or folder to analyze. In the latter case, all symlinks\nwill be ignored so that no subfolder will be processed more than once.\n\n");
      DEBUG_PRINT("(geometry) is a hex bitmap which controls mask processing:\n\n  bits 0-1: (granularity) Mask size minus 1. Note that 3 (32 bits per mask)\n  requires 64GiB of memory.\n\n  bit 4-5: (deltas) The number of times to compute the delta (discrete\n  derivative) of the mask list prior to considering (overlap). Each delta, if\n  any, will transform {A, B, C...} to  {A, (B-A), (C-B)...}. This is useful\n  for improving the entropy contrast of signals containing masks which\n  represent magnitudes, as opposed to merely symbols. Experiment to find the\n  optimum value for your data set. If the mask list size isn't a multiple of\n  ((granularity)+1) bytes, then the remainder bytes will remain unchanged.\n\n  bit 6: (channelize) Set if masks consist of parallel byte channels, for\n  example the red, green, and blue bytes of 24-bit pixels. This will cause\n  deltafication, if enabled, to occur on individual bytes, prior to considering\n  (overlap). For example, {A0:B0, A1:B1, A2:B2} (3 masks spanning 6 bytes)\n  would be transformed to {A0:B0, (A1-A0):(B1-B0), (A2-A1):(B2-B1)}.\n\n  bit 7: (overlap) Overlap masks on byte boundaries. For example, if\n  (granularity) is 2, then {A0:B0:C0, A1:B1:C1} (2 masks spanning 6 bytes, with\n  the low bytes being A0 and A1) would be processed as though it were\n  {A0:B0:C0, B0:C0:A1, C0:A1:B1, A1:B1:C1}. This can improve search quality in\n  cases where context matters, as opposed to merely the frequency distribution\n  of symbols.\n\n");
      DEBUG_PRINT("(sweep) is the nonzero number of masks in the sliding haystack window, except\nthat \"h\" means that the window size will equal the haystack size, in which\ncase no sliding can occur. For example, 5 means: a 5-byte window if\n(granularity)=0, a 10-byte window if (granularity)=1 and (overlap)=0, or a\n7-byte window if (granularity)=2 and (overlap)=1. Prefix with \"+\" to treat the\nfirst (sweep) bytes of the file as the entire haystack or \"-\" for the same with\n the last (sweep) masks.\n\n");
      DEBUG_PRINT("(ranks) is the nonzero number of slots in the list of (best) matches. Matches\nwill be reported as 0-based file offsets or filenames when haystack is a file\nor folder, respectively. In either case, results will be sorted in the order\nimplied by granularity, such that lesser offsets and files encountered earlier\nwill prevail in case of a tie. If (ranks) is prefixed with \"@\", then results\nwill be delivered to the folder or file following that symbol. In this case\nthey will be sorted by sweep window base offset, and not by entropy. Each\nresult will be an 8-byte fracterval mean (center) if (precise) is 0, else a\npair of 16-byte fracterval low and high values if (precise) is 1. In this way,\nit's possible to dump the results of an entire entropy transform. If (haystack)\nis a file in this case, then (ranks) will be treated as a file; otherwise it\nwill be treated as a folder.\n\n");
      DEBUG_PRINT("(format) is a hex bitmap which controls output formatting:\n\n  bit 0: (merge) Prevent the reporting of more than 1 match per sweep. This is\n  useful for filtering because usually many matches occur within the same\n  sweep. In either case, a sweep with global minimum or maximum score will be\n  reported at rank 0. Ignored when (haystack) is a folder.\n\n  bit 1: (ascending) Display results with the lowest scores (entropies) first.\n  Either way, ties will be resolved in favor of lower sweep offsets.\n\n  bit 2: (cavalier) Do not report errors encountered after commencing analysis.\n\n  bit 3: (progress) Make verbose comments about compute progress.\n\n  bit 4: (precise) Set to display entropy values as 64.64 fixed-point hex\n  fractervals. Fractervals are displayed as {(A.B), (C.D)} where (A.B) is the\n  lower bound and (C.D) is (1/(2^64)) less than the upper bound.\n\n");
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
        agnentroscan_error_print("One or more parameters is encoded using invalid UTF8");
        break;
      }
    }while((++arg_idx)<(ULONG)(argc));
    if(status){
      break;
    }
    sweep_text_base=argv[4];
    sweep_status=(u8)(sweep_text_base[0]);
    clip_mode=0;
    status=1;
    if(sweep_status=='+'){
      clip_mode=1;
    }else if(sweep_status=='-'){
      clip_mode=2;
    }
    if(clip_mode){
      sweep_text_base[0]='0';
    }
    status=ascii_decimal_to_u64_convert(sweep_text_base, &parameter, ULONG_MAX);
    sweep_mask_idx_max_max=0;
    sweep_status=AGNENTROSCAN_SWEEP_STATUS_CUSTOM;
    if(status){
      if(strlen(sweep_text_base)==1){
        sweep_status=(u8)(sweep_text_base[0]);
        status=0;
        if(sweep_status=='h'){
          sweep_status=AGNENTROSCAN_SWEEP_STATUS_HAYSTACK;
        }else{
          status=1;
        }
      }
    }else{
      status=!parameter;
      sweep_mask_idx_max_max=(ULONG)(parameter-1);
    }
    if(status){
      agnentroscan_parameter_error_print("sweep");
      break;
    }
    if(clip_mode){
      sweep_status=AGNENTROSCAN_SWEEP_STATUS_HAYSTACK;
    }
    out_filename_base=argv[5];
    out_filename_size=(ULONG)(strlen(out_filename_base));
    append_mode=0;
    if((1<out_filename_size)&&(out_filename_base[0]=='@')){
      if(out_filename_base[1]=='~'){
        agnentroscan_error_print("Path after \"@\" cannot begin with \"~\"; use /home/username/... instead");
        status=1;
        break;
      }
      append_mode=2;
    }else{
/*
Don't accept ranks larger than we can fit in the address space, considering that it will be used to allocate *entropy_list_base0, which consists of u128 fractervals.
*/
      status=ascii_decimal_to_u64_convert(argv[5], &parameter, ULONG_MAX>>(U128_SIZE_LOG2+1));
      status=(u8)(status|!parameter);
      if(status){
        agnentroscan_parameter_error_print("ranks");
        break;
      }
    }
    rank_idx_max_max=(ULONG)(parameter-1);
    status=ascii_hex_to_u64_convert(argv[3], &parameter, U8_MAX);
    status=(u8)(status&&(parameter&~(AGNENTROSCAN_GEOMETRY_CHANNELIZE|AGNENTROSCAN_GEOMETRY_DELTAS|AGNENTROSCAN_GEOMETRY_GRANULARITY|AGNENTROSCAN_GEOMETRY_OVERLAP)));
    if(status){
      agnentroscan_parameter_error_print("geometry");
      break;
    }
    channel_status=(u8)((parameter>>AGNENTROSCAN_GEOMETRY_CHANNELIZE_BIT_IDX)&1);
    delta_count=(u8)((parameter>>AGNENTROSCAN_GEOMETRY_DELTAS_BIT_IDX)&3);
    granularity=(u8)((parameter>>AGNENTROSCAN_GEOMETRY_GRANULARITY_BIT_IDX)&3);
    overlap_status=(u8)((parameter>>AGNENTROSCAN_GEOMETRY_OVERLAP_BIT_IDX)&1);
    mode_text_base=argv[1];
    if(strlen(mode_text_base)==1){
      mode=(u8)(mode_text_base[0]);
      normalized_status=!!(u8)(mode&('a'-'A'));
      mode=(u8)(mode|('a'-'A'));
      status=0;
      if(mode=='a'){
        mode=AGNENTROPROX_MODE_AGNENTROPY;
      }else if(mode=='e'){
        mode=AGNENTROPROX_MODE_EXOENTROPY;
      }else if(mode=='i'){
        mode=AGNENTROPROX_MODE_LET;
/*
Verify that the user understands that Leidich exodivergence is normalized, but then set normalized_status to zero because it already comes out of the transform normalized, so we don't need to do any further normalization.
*/
        status=!normalized_status;
        normalized_status=0;
      }else if(mode=='j'){
        mode=AGNENTROPROX_MODE_JSET;
/*
Verify that the user understands that Jensen-Shannon exodivergence is normalized, but then set normalized_status to zero because it already comes out of the transform normalized, so we don't need to do any further normalization.
*/
        status=!normalized_status;
        normalized_status=0;
      }else if(mode=='l'){
        mode=AGNENTROPROX_MODE_LOGFREEDOM;
      }else if(mode=='k'){
        mode=AGNENTROPROX_MODE_KURTOSIS;
        status=normalized_status;
      }else if(mode=='s'){
        mode=AGNENTROPROX_MODE_SHANNON;
      }else if(mode=='v'){
        mode=AGNENTROPROX_MODE_VARIANCE;
        status=normalized_status;
      }else if(mode=='x'){
        mode=AGNENTROPROX_MODE_EXOELASTICITY;
        status=!normalized_status;
        normalized_status=0;
      }else{
        status=1;
      }
    }
    if(status){
      agnentroscan_parameter_error_print("mode");
      break;
    }
    status=1;
    if(overlap_status&((mode==AGNENTROPROX_MODE_KURTOSIS)|(mode==AGNENTROPROX_MODE_VARIANCE))){
      agnentroscan_error_print("(overlap) is nonsensical with kurtosis and variance");
      break;
    }
    if(sweep_status==AGNENTROSCAN_SWEEP_STATUS_HAYSTACK){
      if(((mode==AGNENTROPROX_MODE_EXOENTROPY)&&normalized_status)||(mode==AGNENTROPROX_MODE_JSET)||(mode==AGNENTROPROX_MODE_LET)){
        status=1;
        DEBUG_PRINT("ERROR: By definition, ");
        agnentroscan_mode_text_print(mode, normalized_status);
        DEBUG_PRINT(" in haystack mode is ");
        if(mode==AGNENTROPROX_MODE_EXOENTROPY){
          DEBUG_PRINT("always (1/2).\n");
        }else if(mode==AGNENTROPROX_MODE_JSET){
          DEBUG_PRINT("undefined.\n");
        }else if(mode==AGNENTROPROX_MODE_LET){
          DEBUG_PRINT("always one.\n");
        }
        break;
      }
    }
    cavalier_status=0;
    merge_status=0;
    precise_status=0;
    progress_status=0;
    if(6<argc){
      status=ascii_hex_to_u64_convert(argv[6], &parameter, U8_MAX);
      status=(u8)(status||(parameter&~(AGNENTROSCAN_FORMAT_ASCENDING|AGNENTROSCAN_FORMAT_CAVALIER|AGNENTROSCAN_FORMAT_MERGE|AGNENTROSCAN_FORMAT_PRECISE|AGNENTROSCAN_FORMAT_PROGRESS)));
      if(status){
        agnentroscan_parameter_error_print("format");
        break;
      }
      if(!append_mode){
        append_mode=(u8)((parameter>>AGNENTROSCAN_FORMAT_ASCENDING_BIT_IDX)&1);
      }
      cavalier_status=(u8)((parameter>>AGNENTROSCAN_FORMAT_CAVALIER_BIT_IDX)&1);
      merge_status=(u8)((parameter>>AGNENTROSCAN_FORMAT_MERGE_BIT_IDX)&1);
      precise_status=(u8)((parameter>>AGNENTROSCAN_FORMAT_PRECISE_BIT_IDX)&1);
      progress_status=(u8)((parameter>>AGNENTROSCAN_FORMAT_PROGRESS_BIT_IDX)&1);
      if(9<argc){
        status=ascii_decimal_to_u64_convert_negatable(argv[7], &parameter, ULONG_MAX, &dump_delta_sign);
        if(status){
          agnentroscan_parameter_error_print("dump_delta");
          break;
        }
        dump_delta=(ULONG)(parameter);
        status=ascii_decimal_to_u64_convert(argv[8], &parameter, ULONG_MAX);
        status=(u8)(status|!parameter);
        if(status){
          agnentroscan_parameter_error_print("dump_size");
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
            agnentroscan_error_print("More than one \"@\" in (dump_filename)");
            break;
          }else if(dump_filename_replacement_char_idx==ULONG_MAX){
            status=1;
            agnentroscan_error_print("You need \"@\" in (dump_filename)");
            break;
          }
/*
We need to allocate space for the full dump filename, which will contain the existing filename template plus 15 more bytes because "@" will be replaced by a 16-digit hex value, plus another byte for the terminating null, which was not included in dump_filename_template_size.
*/
          dump_filename_size=dump_filename_template_size+15+1;
        }
      }
    }
    if((append_mode==2)&&(dump_status|merge_status)){
      status=1;
      agnentroscan_error_print("Dumping or merging is not allowed when (ranks) is prefixed with \"@\"");
      break;
    }
    haystack_filename_base=argv[2];
    haystack_file_size_max=0;
    haystack_filename_count=0;
    haystack_filename_list_size=U16_MAX;
    status=1;
    sweep_size=(sweep_mask_idx_max_max+1)*(u8)((u8)(granularity*(!overlap_status))+1);
    do{
      haystack_filename_list_char_idx_max=haystack_filename_list_size-1;
      haystack_filename_list_base=filesys_char_list_malloc(haystack_filename_list_char_idx_max);
      if(!haystack_filename_list_base){
        agnentroscan_out_of_memory_print();
        break;
      }
      haystack_filename_list_size_new=haystack_filename_list_size;
      haystack_filename_count=filesys_filename_list_get(&haystack_file_size_max, &file_status, haystack_filename_list_base, &haystack_filename_list_size_new, haystack_filename_base);
      if(!haystack_filename_count){
        agnentroscan_error_print("(haystack) not found or inaccessible");
        break;
      }
      status=0;
      if(clip_mode){
        haystack_file_size_max=sweep_size;
      }
      if(haystack_filename_list_size<haystack_filename_list_size_new){
        haystack_filename_list_base=filesys_free(haystack_filename_list_base);
        haystack_filename_list_size=haystack_filename_list_size_new;
        status=1;
      }
    }while(status);
    if(status){
      break;
    }
    mask_size=(u8)(granularity+1);
    status=1;
    if(haystack_file_size_max<mask_size){
      agnentroscan_error_print("All haystack files are smaller than a single (granularity+1)-sized mask");
      break;
    }
/*
Ignore the return status of agnentroprox_mask_idx_max_get() because it doesn't matter whether the maximum haystack size is compatible with granularity and overlap. Just check for a return of ULONG_MAX, which indicates that the maximum haystack size is smaller than (granularity+1).
*/
    haystack_mask_idx_max_max=agnentroprox_mask_idx_max_get(granularity, &granularity_status, haystack_file_size_max, overlap_status);
    if((haystack_mask_idx_max_max<sweep_mask_idx_max_max)||(haystack_mask_idx_max_max==ULONG_MAX)){
      agnentroscan_error_print("All (haystack) files contain fewer masks than sweep would require");
      break;
    }
    status=0;
    if(sweep_status==AGNENTROSCAN_SWEEP_STATUS_HAYSTACK){
      sweep_mask_idx_max_max=haystack_mask_idx_max_max;
    }
    if(append_mode==2){
      out_filename_list_size=filesys_filename_list_morph_size_get(haystack_filename_count, haystack_filename_base, haystack_filename_list_base, &out_filename_base[1]);
      out_filename_list_char_idx_max=out_filename_list_size-1;
      out_filename_list_base=filesys_char_list_malloc(out_filename_list_char_idx_max);
      status=!out_filename_list_base;
      if(status){
        agnentroscan_out_of_memory_print();
        break;
      }
      filesys_filename_list_morph(haystack_filename_count, haystack_filename_base, haystack_filename_list_base, &out_filename_base[1], out_filename_list_base);
      rank_idx_max_max=haystack_mask_idx_max_max-sweep_mask_idx_max_max;
    }
    match_idx_max_max=rank_idx_max_max;
    if(!file_status){
      if(dump_status|merge_status){
        agnentroscan_error_print("Dumping or merging is not allowed if (haystack) is a folder");
        status=1;
        break;
      }
      if(append_mode<=1){
        match_idx_max_max=0;
      }
    }else{
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
      agnentroscan_out_of_memory_print();
      break;
    }
    mask_max=(u32)((1U<<(granularity<<U8_BITS_LOG2)<<U8_BITS)-1);
    status=1;
    agnentroprox_base=agnentroprox_init(AGNENTROPROX_BUILD_BREAK_COUNT_EXPECTED, 4, granularity, loggamma_base, haystack_mask_idx_max_max, mask_max, mode, overlap_status, sweep_mask_idx_max_max);
    if(!agnentroprox_base){
      agnentroscan_out_of_memory_print();
      break;
    }
    U128_SET_ZERO(entropy_mean);
    U128_FROM_BOOL(entropy_threshold, append_mode);
    haystack_filename_list_char_idx=0;
    haystack_filename_idx=0;
    match_count=0;
    out_filename_list_char_idx=0;
    out_filename_list_char_idx_new=0;
    rank_count=0;
    rank_idx=0;
    do{
      deltafy_mask_idx_max=0;
      granularity_status=0;
      if(progress_status){
        DEBUG_PRINT("Analyzing ");
        DEBUG_PRINT(&haystack_filename_list_base[haystack_filename_list_char_idx]);
        DEBUG_PRINT("...\n");
      }
      haystack_file_size=haystack_file_size_max;
      haystack_filename_list_char_idx_new=haystack_filename_list_char_idx;
      if(clip_mode){
        direction_status=(u8)(clip_mode-1);
/*
Assume that the haystack is large enough to contain at least one sweep. If not, then filesys_status will come back as nonzero.
*/
        haystack_file_size=sweep_size;
        filesys_status=filesys_subfile_read_next(direction_status, &haystack_filename_list_char_idx_new, haystack_filename_list_base, haystack_file_size, 0, haystack_mask_list_base);
      }else{
        filesys_status=filesys_file_read_next(&haystack_file_size, &haystack_filename_list_char_idx_new, haystack_filename_list_base, haystack_mask_list_base);
      }
      status=1;
      if(!filesys_status){
        haystack_mask_idx_max=agnentroprox_mask_idx_max_get(granularity, &granularity_status, haystack_file_size, overlap_status);
        if(delta_count){
          deltafy_mask_idx_max=agnentroprox_mask_idx_max_get(granularity, &granularity_status, haystack_file_size, 0);
        }
        status=(haystack_mask_idx_max==ULONG_MAX);
        if(!status){
          sweep_mask_idx_max=sweep_mask_idx_max_max;
          if(sweep_status==AGNENTROSCAN_SWEEP_STATUS_HAYSTACK){
            sweep_mask_idx_max=haystack_mask_idx_max;
          }else if(haystack_mask_idx_max<sweep_mask_idx_max){
            filesys_status=FILESYS_STATUS_CALLER_CUSTOM2;
            status=1;
          }
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
                agnentroprox_mask_list_deltafy(channel_status, 1, granularity, deltafy_mask_idx_max, haystack_mask_list_base);
              }while((delta_idx++)!=delta_count);
            }
            if((mode==AGNENTROPROX_MODE_KURTOSIS)||(mode==AGNENTROPROX_MODE_VARIANCE)){
              mean_f128=agnentroprox_mask_list_mean_get(agnentroprox_base, haystack_mask_idx_max, haystack_mask_list_base, &sign_status);
              if(progress_status){
                if(delta_count){
                   DEBUG_PRINT("After deltafication, t");
                }else{
                  DEBUG_PRINT("T");
                }
                DEBUG_PRINT("his file seems to contain ");
                if(!sign_status){
                  DEBUG_PRINT("un");
                }
                DEBUG_PRINT("signed masks with a mean of\n");
                if(sign_status){
                  if(sign_status==1){
                    DEBUG_PRINT("+");
                  }else{
                    DEBUG_PRINT("-");
                  }
                }
                DEBUG_F128("", mean_f128);
                DEBUG_PRINT("\n");
              }
            }
            if(progress_status){
              DEBUG_PRINT("Doing ");
              agnentroscan_mode_text_print(mode, normalized_status);
              DEBUG_PRINT(" transform with");
              if(!overlap_status){
                DEBUG_PRINT("out");
              }
              DEBUG_PRINT(" mask overlap...\n");
            }
            rank_list_base=entropy_list_base0;
            if(append_mode<=1){
              append_mode^=normalized_status;
              if(!file_status){
                rank_list_base=entropy_list_base1;
              }
            }
            if(mode!=AGNENTROPROX_MODE_EXOELASTICITY){
              match_count=agnentroprox_entropy_transform(agnentroprox_base, append_mode, rank_list_base, haystack_mask_idx_max, haystack_mask_list_base, match_idx_max_max, match_u8_idx_list_base, mode, &overflow_status, sweep_mask_idx_max);
            }else{
              match_count=agnentroprox_exoelasticity_transform(agnentroprox_base, append_mode, rank_list_base, haystack_mask_idx_max, haystack_mask_list_base, match_idx_max_max, match_u8_idx_list_base, &overflow_status, sweep_mask_idx_max);
            }
            if(normalized_status){
              if(append_mode<=1){
                append_mode^=normalized_status;
              }
/*
Normalize all the matches.
*/
              entropy_raw=agnentroprox_entropy_raw_get(agnentroprox_base, sweep_mask_idx_max, &overflow_status);
              match_idx=match_count-1;
              do{
                entropy=rank_list_base[match_idx];
                if((mode==AGNENTROPROX_MODE_LOGFREEDOM)||(mode==AGNENTROPROX_MODE_SHANNON)){
                  entropy=agnentroprox_dyspoissonism_get(entropy, entropy_raw);
                }else{
                  entropy=agnentroprox_compressivity_get(entropy, entropy_raw);
                }
                rank_list_base[match_idx]=entropy;
              }while(match_idx--);
            }
            if(append_mode==2){
              entropy_list_size=match_count<<(U128_SIZE_LOG2+1);
              if(!precise_status){
                entropy_list_size>>=2;
                match_idx=0;
                score_idx=0;
                do{
                  entropy=rank_list_base[match_idx];
                  FRU128_SET_ZERO(score_packed);
                  FRU128_MEAN_TO_FTD128(entropy_mean, entropy);
                  U128_TO_U64_HI(score, entropy_mean);
                  U128_FROM_U64_LO(score_packed.a, score);
                  match_idx++;
                  if(match_idx==match_count){
                    break;
                  }
                  entropy=rank_list_base[match_idx];
                  FRU128_MEAN_TO_FTD128(entropy_mean, entropy);
                  U128_TO_U64_HI(score, entropy_mean);
                  U128_ADD_U64_HI_SELF(score_packed.a, score);
                  match_idx++;
                  if(match_idx==match_count){
                    break;
                  }
                  entropy=rank_list_base[match_idx];
                  FRU128_MEAN_TO_FTD128(entropy_mean, entropy);
                  U128_TO_U64_HI(score, entropy_mean);
                  U128_FROM_U64_LO(score_packed.b, score);
                  match_idx++;
                  if(match_idx==match_count){
                    break;
                  }
                  entropy=rank_list_base[match_idx];
                  FRU128_MEAN_TO_FTD128(entropy_mean, entropy);
                  U128_TO_U64_HI(score, entropy_mean);
                  U128_ADD_U64_HI_SELF(score_packed.b, score);
                  match_idx++;
                  if(match_idx==match_count){
                    break;
                  }
                  rank_list_base[score_idx]=score_packed;
                  score_idx++;
                }while(1);
                rank_list_base[score_idx]=score_packed;
              }
              out_filename_list_char_idx_new=out_filename_list_char_idx;
              filesys_status=filesys_file_write_next_obnoxious(entropy_list_size, &out_filename_list_char_idx_new, out_filename_list_base, rank_list_base);
              status=!!filesys_status;
            }else{
              entropy=rank_list_base[0];
              FRU128_MEAN_TO_FTD128(entropy_mean, entropy);
              if(!file_status){
                status=1;
                if((!append_mode)&&U128_IS_LESS_EQUAL(entropy_threshold, entropy_mean)){
                  status=fracterval_u128_rank_list_insert_descending(entropy, &rank_count, &rank_idx, rank_idx_max_max, entropy_list_base0, &entropy_threshold);
                }else if((append_mode==1)&&U128_IS_LESS_EQUAL(entropy_mean, entropy_threshold)){
                  status=fracterval_u128_rank_list_insert_ascending(entropy, &rank_count, &rank_idx, rank_idx_max_max, entropy_list_base0, &entropy_threshold);
                }
                if(!status){
                  rank_idx_min=rank_idx;
                  rank_idx=rank_count-1;
                  while(rank_idx!=rank_idx_min){
                    haystack_filename_idx_list_base[rank_idx]=haystack_filename_idx_list_base[rank_idx-1];
                    rank_idx--;
                  }
                  haystack_filename_idx_list_base[rank_idx]=haystack_filename_list_char_idx;
                }
                status=0;
              }
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
            if(filesys_status!=FILESYS_STATUS_WRITE_FAIL){
              DEBUG_PRINT(&haystack_filename_list_base[haystack_filename_list_char_idx]);
            }else{
              DEBUG_PRINT(&out_filename_list_base[out_filename_list_char_idx]);
            }
            DEBUG_PRINT("\n");
          }
/*
Don't warn about incompatible granularity if we have some other error which would moot the issue.
*/
          if(granularity_status&!status){
            agnentroscan_warning_print("Size wasn't a multiple of (granularity+1), so tail bytes were ignored");
          }
        }
        switch(filesys_status){
        case 0:
          if((append_mode<=1)&&match_count&&progress_status){
            if(sweep_status==AGNENTROSCAN_SWEEP_STATUS_HAYSTACK){
              DEBUG_PRINT("(haystack) ");
            }else{
              if(!append_mode){
                DEBUG_PRINT("Max ");
              }else{
                DEBUG_PRINT("Min ");
              }
            }
            if(precise_status){
              agnentroscan_mode_text_print(mode, normalized_status);
            }else{
              DEBUG_PRINT("score ");
            }
            DEBUG_PRINT(" is ");
            if(!precise_status){
              U128_TO_U64_HI(score, entropy_mean);
              DEBUG_U64("", score);
            }else{
              DEBUG_F128_PAIR("", entropy.a, entropy.b);
            }
            DEBUG_PRINT(".\n");
          }
          break;
        case FILESYS_STATUS_NOT_FOUND:
          agnentroscan_error_print("Not found");
          break;
        case FILESYS_STATUS_TOO_BIG:
          agnentroscan_error_print("Too big to fit in memory");
          break;
        case FILESYS_STATUS_READ_FAIL:
          agnentroscan_error_print("Read failed");
          break;
        case FILESYS_STATUS_SIZE_CHANGED:
          agnentroscan_error_print("File size changed during read");
          break;
        case FILESYS_STATUS_WRITE_FAIL:
          agnentroscan_error_print("Write failed");
          break;
        case FILESYS_STATUS_CALLER_CUSTOM:
          agnentroscan_error_print("Size is less than (granularity+1) bytes");
          break;
        case FILESYS_STATUS_CALLER_CUSTOM2:
          agnentroscan_error_print("Contains fewer masks than one sweep would require");
          break;
        default:
          agnentroscan_error_print("Internal error. Please report");
        }
      }
      haystack_filename_list_char_idx=haystack_filename_list_char_idx_new;
      haystack_filename_idx++;
      out_filename_list_char_idx=out_filename_list_char_idx_new;
      status=0;
    }while(haystack_filename_idx!=haystack_filename_count);
    if(sweep_status!=AGNENTROSCAN_SWEEP_STATUS_HAYSTACK){
/*
Don't attempt to merge ranking sweeps in haystack mode, as there's only one entropy value.
*/
       sweep_size=0;
    }
    if(append_mode<=1){
      if(progress_status){
        if(match_count|rank_count){
          DEBUG_PRINT("Here are the results ranked ");
          if(append_mode){
            DEBUG_PRINT("as");
          }else{
            DEBUG_PRINT("des");
          }
          DEBUG_PRINT("cending by ");
          agnentroscan_mode_text_print(mode, normalized_status);
          DEBUG_PRINT(":\n");
        }else{
          DEBUG_PRINT("No matches found!\n");
        }
      }
      if(!file_status){
        rank_idx=0;
        while(rank_idx<rank_count){
          entropy=entropy_list_base0[rank_idx];
          if(!precise_status){
            FRU128_MEAN_TO_FTD128(entropy_mean, entropy);
            U128_TO_U64_HI(score, entropy_mean);
            DEBUG_U64("", score);
          }else{
            DEBUG_F128_PAIR("", entropy.a, entropy.b);
          }
          DEBUG_PRINT(" ");
          DEBUG_PRINT(&haystack_filename_list_base[haystack_filename_idx_list_base[rank_idx]]);
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
                agnentroprox_mask_list_deltafy(channel_status, 0, granularity, deltafy_mask_idx_max, haystack_mask_list_base);
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
            entropy=entropy_list_base0[match_idx];
            if(!precise_status){
               FRU128_MEAN_TO_FTD128(entropy_mean, entropy);
              U128_TO_U64_HI(score, entropy_mean);
              DEBUG_U64("", score);
            }else{
              DEBUG_F128_PAIR("", entropy.a, entropy.b);
            }
            DEBUG_PRINT(" ");
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
    }
    status=1;
    if(overflow_status){
      if(!cavalier_status){
        agnentroscan_error_print("Fracterval precision was exhausted. Please report");
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
  filesys_free(out_filename_list_base);
  filesys_free(haystack_filename_list_base);
  loggamma_free_all(loggamma_base);
  DEBUG_ALLOCATION_CHECK();
  return status;
}
