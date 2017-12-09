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
Mask List Comparison and Absorption Utility
*/
#include "flag.h"
#include "flag_ascii.h"
#include "flag_biguint.h"
#include "flag_filesys.h"
#include "flag_fracterval_u128.h"
#include "flag_fracterval_u64.h"
#include "flag_loggamma.h"
#include "flag_maskops.h"
#include "flag_poissocache.h"
#include "flag_agnentroprox.h"
#include "flag_agnentrozorb.h"
#include "flag_zorb.h"
#include <stdint.h>
#include <string.h>
#include "constant.h"
#include "debug.h"
#include "debug_xtrn.h"
#include "biguint.h"
#include "biguint_xtrn.h"
#include "fracterval_u128.h"
#include "fracterval_u128_xtrn.h"
#include "fracterval_u64.h"
#include "fracterval_u64_xtrn.h"
#include "loggamma.h"
#include "loggamma_xtrn.h"
#include "maskops.h"
#include "maskops_xtrn.h"
#include "poissocache.h"
#include "agnentroprox.h"
#include "agnentroprox_xtrn.h"
#include "ascii_xtrn.h"
#include "filesys.h"
#include "filesys_xtrn.h"
#include "zorb.h"
#include "zorb_xtrn.h"

#define AGNENTROZORB_GEOMETRY_ABSORB 3U
#define AGNENTROZORB_GEOMETRY_ABSORB_ADD 1U
#define AGNENTROZORB_GEOMETRY_ABSORB_ADD_MUNDANE 3U
#define AGNENTROZORB_GEOMETRY_ABSORB_BIT_IDX 9U
#define AGNENTROZORB_GEOMETRY_ABSORB_REPORT 0U
#define AGNENTROZORB_GEOMETRY_ABSORB_SUBTRACT 2U
#define AGNENTROZORB_GEOMETRY_CHANNELIZE 1U
#define AGNENTROZORB_GEOMETRY_CHANNELIZE_BIT_IDX 6U
#define AGNENTROZORB_GEOMETRY_DELTAS 3U
#define AGNENTROZORB_GEOMETRY_DELTAS_BIT_IDX 4U
#define AGNENTROZORB_GEOMETRY_DENSIFY 1U
#define AGNENTROZORB_GEOMETRY_DENSIFY_BIT_IDX 2U
#define AGNENTROZORB_GEOMETRY_GRANULARITY 3U
#define AGNENTROZORB_GEOMETRY_GRANULARITY_BIT_IDX 0U
#define AGNENTROZORB_GEOMETRY_OVERLAP 1U
#define AGNENTROZORB_GEOMETRY_OVERLAP_BIT_IDX 7U
#define AGNENTROZORB_GEOMETRY_POLARITY 1U
#define AGNENTROZORB_GEOMETRY_POLARITY_BIT_IDX 8U
#define AGNENTROZORB_GEOMETRY_SURROUNDIFY 1U
#define AGNENTROZORB_GEOMETRY_SURROUNDIFY_BIT_IDX 3U
#define AGNENTROZORB_STATUS_ALERT_BIT_IDX 2U
#define AGNENTROZORB_STATUS_ERROR_BIT_IDX 0U
#define AGNENTROZORB_STATUS_WARNING_BIT_IDX 1U

void
agnentrozorb_error_print(char *char_list_base){
  DEBUG_PRINT("ERROR: ");
  DEBUG_PRINT(char_list_base);
  DEBUG_PRINT(".\n");
  return;
}

void
agnentrozorb_out_of_memory_print(void){
  agnentrozorb_error_print("Out of memory");
  return;
}

void
agnentrozorb_parameter_error_print(char *char_list_base){
  DEBUG_PRINT("Invalid parameter: (");
  DEBUG_PRINT(char_list_base);
  DEBUG_PRINT("). For help, run without parameters.\n");
  return;
}

void
agnentrozorb_warning_print(char *char_list_base){
  DEBUG_PRINT("WARNING: ");
  DEBUG_PRINT(char_list_base);
  DEBUG_PRINT(".\n");
  return;
}

int
main(int argc, char *argv[]){
  u8 absorb_status;
  agnentroprox_t *agnentroprox_base;
  u8 alert_status;
  ULONG arg_idx;
  u8 channel_status;
  u8 delta_count;
  u8 delta_idx;
  u8 densify_status;
  u8 error_status;
  u8 filesys_status;
  u8 granularity;
  u8 granularity_channelized;
  u8 granularity_status;
  fru128 jsd;
  u128 jsd_mean;
  loggamma_t *loggamma_base;
  ULONG mask_idx_max;
  ULONG mask_idx_max_max;
  ULONG mask_idx_max_parallel;
  ULONG mask_idx_max_parallel_channelized;
  u8 *mask_list_base;
  ULONG mask_list_file_size;
  char *mask_list_filename_base;
  u32 mask_max;
  u32 mask_max_densify;
  u32 mask_max_finalize;
  u32 mask_max_surroundify;
  u32 mask_min;
  u32 mask_min_densify;
  u32 mask_min_surroundify;
  u8 mask_size;
  ULONG *maskops_bitmap_base;
  u32 *maskops_u32_list_base;
  u8 overlap_status;
  u64 parameter;
  u8 polarity_status;
  u8 reset_status;
  u64 score;
  u8 status;
  u8 surroundify_status;
  u64 threshold;
  ULONG threshold_digit_count;
  u8 warning_status;
  zorb_t *zorb_base;
  ULONG zorb_file_size;
  char *zorb_filename_base;
  ULONG zorb_filename_char_idx_max;
  ULONG zorb_mask_idx_max;

  agnentroprox_base=NULL;
  alert_status=0;
  error_status=1;
  mask_list_base=NULL;
  mask_max_densify=0;
  mask_min_densify=0;
  mask_max_finalize=0;
  mask_max_surroundify=0;
  mask_min_surroundify=0;
  maskops_bitmap_base=NULL;
  maskops_u32_list_base=NULL;
  warning_status=0;
  zorb_base=NULL;
  status=ascii_init(ASCII_BUILD_BREAK_COUNT_EXPECTED, 0);
  status=(u8)(status|filesys_init(FILESYS_BUILD_BREAK_COUNT_EXPECTED, 0));
  loggamma_base=loggamma_init(LOGGAMMA_BUILD_BREAK_COUNT_EXPECTED, 0);
  status=(u8)(status|!loggamma_base);
  do{
    if(status){
      agnentrozorb_error_print("Outdated source code");
      break;
    }
    if((argc!=4)&&(argc!=5)){
      DEBUG_PRINT("Agnentrozorb\nCopyright 2017 Russell Leidich\nhttp://agnentropy.blogspot.com\n");
      DEBUG_U32("build_id_in_hex", AGNENTROZORB_BUILD_ID);
      DEBUG_PRINT("Mask list comparison and absorption utility.\n\n");
      DEBUG_PRINT("Function:\n\n");
      DEBUG_PRINT("  Reports the (normalized) negated Jensen-Shannon divergence (NJSD, which is\n  one minus the Jensen-Shannon divergence) between a frequency list and a mask\n  list, then optionally adds or subtracts the frequency list implied by the\n  latter to or from the former.\n\n");
      DEBUG_PRINT("Returns:\n\n  bit 0: Set if and only if an error occurred.\n\n  bit 1: Set if and only if a warning occurred.\n\n  bit 2: Set if and only if the result was out-of-bounds (as defined by\n  (threshold)).\n\n");
      DEBUG_PRINT("Syntax:\n\n");
      DEBUG_PRINT("  agnentrozorb geometry zorbfile masklist [threshold]\n\n");
      DEBUG_PRINT("  (geometry) is a hex bitmap which controls mask processing. Do NOT use the\n  same (zorbfile) with different (geometry) values:\n\n  bits 0-1: (granularity) Mask size minus 1. Note that 3 (32 bits per mask)\n  requires 64GiB of memory.\n\n  bit 2: (densify) Set to enable densification (mask utilization footprint\n  minimization) after deltafication.\n\n  bit 3: (surroundify) After densification, subtract the minimum mask from all\n  masks, so as to make the new minimum 0. Then convert all masks to their\n  surround codes relative to their new maximum.\n\n  bit 4-5: (deltas) The number of times to compute the delta (discrete\n  derivative) of the mask list prior to considering (overlap). Each delta, if\n  any, will transform {A, B, C...} to  {A, (B-A), (C-B)...}. This is useful\n  for improving the entropy contrast of signals containing masks which\n  represent magnitudes, as opposed to merely symbols. Experiment to find the\n  optimum value for your data set.\n\n  bit 6: (channelize) Set if masks consist of parallel byte channels, for\n  example the red, green, and blue bytes of 24-bit pixels. This will cause\n  deltafication, if enabled, to occur on individual bytes, prior to considering\n  (overlap). For example, {A0:B0, A1:B1, A2:B2} (3 masks spanning 6 bytes)\n  would be transformed to {A0:B0, (A1-A0):(B1-B0), (A2-A1):(B2-B1)}.\n\n  bit 7: (overlap) Overlap masks on byte boundaries. For example, if\n  (granularity) is 2, then {A0:B0:C0, A1:B1:C1} (2 masks spanning 6 bytes, with\n  the low bytes being A0 and A1) would be processed as though it were\n  {A0:B0:C0, B0:C0:A1, C0:A1:B1, A1:B1:C1}. This can improve search quality in\n  cases where context matters, as opposed to merely the frequency distribution\n  of symbols. It only affects search -- not preprocessing.\n\n  bit 8: (polarity) is zero if (threshold) is the maximum NJSD which is to be\n  considered mundane, else one if (threshold) is the minimum such value.\n\n  bits 9-10: (absorb) tells how to integrate (masklist) into (zorbfile), if at\n  all. For the purposes of said integration, (masklist) will be presumed to\n  contain up to 256 masks if (channelize) is one, else (256^(granularity+1)).\n  If (zorbfile) doesn't exist, then no alert will be raised, and 01, 10, and\n  11 will all be treated as 01.\n\n    00 to simply report the NJSD.\n\n    01 to report the NJSD, compute the frequency list implied by (masklist),\n    then add it to the frequency list contained in (zorbfile). If (zorbfile)\n    doesn't exist, then the NJSD will be reported as (polarity) and (zorbfile)\n    will be initialized in a manner consistent with the frequency list\n    corresponding to (masklist).\n\n    10 to report the NJSD, compute the frequency list implied by (masklist),\n    then subtract it from the frequency list contained in (zorbfile). If\n    (zorbfile) doesn't exist, then behavior will be the same as with (01).\n\n    11 is like 01, but only mundane signals will be added to the frequency\n    list. This prevents anomalies from being gradually subsumed into\n    expectation.\n\n");
      DEBUG_PRINT("  (zorbfile) will be created if it doesn't exist. Otherwise, it's the output of\n  previous Agnentrozorb sessions created with the same (geometry) settings.\n\n");
      DEBUG_PRINT("  (masklist) is an input file which will be preprocessed according to\n  (geometry) prior to comparison with and integration into with (zorbfile), as\n  specified by (absorb).\n\n");
      DEBUG_PRINT("  (threshold) is an optional hex value up to 64-bits which specifies the\n  maximum (if (polarity) is zero) or minimum (if (polarity) is one) mundane\n  NJSD. If fewer than 16 hex digits are provided, then the provided digits will\n  be interpreted as the most significant, with the rest being zeroes. Values\n  which are not mundane will contain a \"*\" after their reported NJSD. The NJSD\n  itself is of course transcendental irrational, so for these purposes the NJSD\n  is deemed to equal the mean of the interval on which it is known to exist. If\n  not specified, this value will be presumed to be zero.\n\n");
      break;
    }
    arg_idx=0;
    do{
      status=ascii_utf8_string_verify(argv[arg_idx]);
      if(status){
        agnentrozorb_error_print("One or more parameters is encoded using invalid UTF8");
        break;
      }
    }while((++arg_idx)<(ULONG)(argc));
    if(status){
      break;
    }
    threshold=0;
    if(4<argc){
      threshold_digit_count=(ULONG)(strlen(argv[4]));
      status=((U64_BITS>>2)<threshold_digit_count);
      if(!status){
        status=ascii_hex_to_u64_convert(argv[4], &threshold, U64_MAX);
      }
      if(status){
        agnentrozorb_parameter_error_print("threshold");
        break;
      }
/*
By definition, (threshold) digits are aligned high, so shift left accordingly.
*/
      threshold<<=U64_BITS-(threshold_digit_count<<2);
    }
    status=ascii_hex_to_u64_convert(argv[1], &parameter, 0x7FF);
    if(status){
      agnentrozorb_parameter_error_print("geometry");
      break;
    }
    absorb_status=(u8)((parameter>>AGNENTROZORB_GEOMETRY_ABSORB_BIT_IDX)&AGNENTROZORB_GEOMETRY_ABSORB);
    channel_status=(u8)((parameter>>AGNENTROZORB_GEOMETRY_CHANNELIZE_BIT_IDX)&AGNENTROZORB_GEOMETRY_CHANNELIZE);
    delta_count=(u8)((parameter>>AGNENTROZORB_GEOMETRY_DELTAS_BIT_IDX)&AGNENTROZORB_GEOMETRY_DELTAS);
    densify_status=(u8)((parameter>>AGNENTROZORB_GEOMETRY_DENSIFY_BIT_IDX)&AGNENTROZORB_GEOMETRY_DENSIFY);
    granularity=(u8)((parameter>>AGNENTROZORB_GEOMETRY_GRANULARITY_BIT_IDX)&AGNENTROZORB_GEOMETRY_GRANULARITY);
    overlap_status=(u8)((parameter>>AGNENTROZORB_GEOMETRY_OVERLAP_BIT_IDX)&AGNENTROZORB_GEOMETRY_OVERLAP);
    polarity_status=(u8)((parameter>>AGNENTROZORB_GEOMETRY_POLARITY_BIT_IDX)&AGNENTROZORB_GEOMETRY_POLARITY);
    surroundify_status=(u8)((parameter>>AGNENTROZORB_GEOMETRY_SURROUNDIFY_BIT_IDX)&AGNENTROZORB_GEOMETRY_SURROUNDIFY);
    zorb_filename_base=argv[2];
    status=1;
    zorb_filename_char_idx_max=(ULONG)(strlen(zorb_filename_base));
    if(4<zorb_filename_char_idx_max){
      if(zorb_filename_base[zorb_filename_char_idx_max-4]=='.'){
        if(zorb_filename_base[zorb_filename_char_idx_max-3]=='z'){
          if(zorb_filename_base[zorb_filename_char_idx_max-2]=='r'){
            if(zorb_filename_base[zorb_filename_char_idx_max-1]=='b'){
              status=0;
            }
          }
        }
      }
    }
    if(status){
      agnentrozorb_error_print("For the sake of consistency, (zorbfile) must end in \".zrb\"");
      break;
    }
    mask_max=(u32)((1U<<(granularity<<U8_BITS_LOG2)<<U8_BITS)-1);
    status=zorb_size_ulong_get(mask_max, &zorb_file_size);
    if(status){
      agnentrozorb_error_print("Insufficient memory to support (zorbfile) consistent with (granularity)");
      break;
    }
    mask_list_filename_base=argv[3];
    filesys_status=filesys_file_size_ulong_get(&mask_list_file_size, mask_list_filename_base);
    if(filesys_status){
      agnentrozorb_error_print("(masklist) not found");
      break;
    }
    mask_idx_max=agnentroprox_mask_idx_max_get(granularity, &granularity_status, mask_list_file_size, overlap_status);
    if(granularity_status){
      if(mask_idx_max!=ULONG_MAX){
        warning_status=1;
        agnentrozorb_warning_print("(masklist) size is not a multiple of (granularity+1). Remainder bytes\nwill be ignored");
      }else{
        agnentrozorb_error_print("(masklist) is too small to contain even one mask");
        break;
      }
    }
    mask_list_base=agnentroprox_mask_list_malloc(granularity, mask_idx_max, overlap_status);
    if(!mask_list_base){
      agnentrozorb_out_of_memory_print();
      break;
    }
    filesys_status=filesys_file_read_exact(mask_list_file_size, mask_list_filename_base, mask_list_base);
    if(filesys_status){
      if(filesys_status==FILESYS_STATUS_SIZE_CHANGED){
        agnentrozorb_error_print("(masklist) changed while being read");
        break;
      }else{
        agnentrozorb_error_print("(masklist) read failed");
        break;
      }
    }
    zorb_base=zorb_init(ZORB_BUILD_BREAK_COUNT_EXPECTED, 0, mask_max);
    if(!zorb_base){
      agnentrozorb_error_print("Zorb init failed, probably due to huge (granularity)");
      break;
    }
    filesys_status=filesys_file_read_exact(zorb_file_size, zorb_filename_base, zorb_base);
    if(filesys_status==FILESYS_STATUS_SIZE_CHANGED){
      agnentrozorb_error_print("(zorbfile) size is inconsistent with (granularity)");
      break;
    }
    if(densify_status){
      maskops_bitmap_base=maskops_bitmap_malloc((u64)(mask_max));
      status=(u8)(status|!maskops_bitmap_base);
      maskops_u32_list_base=maskops_u32_list_malloc((ULONG)(mask_max));
      status=(u8)(status|!maskops_u32_list_base);
    }
/*
Set mask_idx_max_max to the largest value it could be without wrapping so that Agnentroprox will just continue to allow us to accumulate mask frequencies for a long time over multiple instances of this app.
*/
    mask_idx_max_max=ULONG_MAX-1;
    if(mask_max<mask_idx_max_max){
      mask_idx_max_max-=mask_max;
      mask_idx_max_max--;
      agnentroprox_base=agnentroprox_init(AGNENTROPROX_BUILD_BREAK_COUNT_EXPECTED, 12, granularity, loggamma_base, mask_idx_max_max, mask_max, AGNENTROPROX_MODE_LDT, overlap_status, mask_idx_max);
    }
    if(!agnentroprox_base){
      agnentrozorb_error_print("Agnentroprox init failed, probably due to huge (granularity)");
      break;
    }
    reset_status=0;
    score=0;
    if(filesys_status){
      if((absorb_status!=AGNENTROZORB_GEOMETRY_ABSORB_REPORT)&(filesys_status==FILESYS_STATUS_NOT_FOUND)){
        zorb_reset(agnentroprox_base, zorb_base);
        absorb_status=AGNENTROZORB_GEOMETRY_ABSORB_ADD;
        reset_status=1;
        score=0;
        score-=polarity_status;
      }else{ 
        agnentrozorb_error_print("Can't find or read (zorbfile)");
        break;
      }
    }else{
      status=zorb_check(mask_max, zorb_base, zorb_file_size);
      if(status){
        agnentrozorb_error_print("(zorbfile) is corrupt or has wrong size");
        break;
      }
    }
    mask_idx_max_parallel=0;
    granularity_channelized=granularity;
    mask_size=(u8)(granularity+1);
    if(channel_status){
      granularity_channelized=U8_BYTE_MAX;
    }
    if(delta_count|densify_status|surroundify_status){
      mask_idx_max_parallel=agnentroprox_mask_idx_max_get(granularity, &granularity_status, mask_list_file_size, 0);
      if(granularity_status){
        agnentrozorb_error_print("(masklist) size must be a multiply of (granularity+1) bytes when\n(densify), (surroundify), or (deltas) is nonzero, regardless of (overlap).");
        break;
      }
    }
    mask_idx_max_parallel_channelized=mask_idx_max_parallel;
    if(channel_status){
      mask_idx_max_parallel_channelized=(mask_idx_max_parallel_channelized*mask_size)+granularity;
    }
    if(delta_count){
      delta_idx=0;
      do{
        maskops_deltafy(channel_status, 1, granularity, mask_idx_max_parallel, mask_list_base);
      }while((delta_idx++)!=delta_count);
    }
    if(densify_status|surroundify_status){
      maskops_unsign(granularity_channelized, mask_idx_max_parallel_channelized, mask_list_base, &mask_max, &mask_min);
      mask_max_densify=mask_max;
      mask_max_finalize=mask_max;
      mask_max_surroundify=mask_max;
      mask_min_densify=mask_min;
      mask_min_surroundify=mask_min;
      if(densify_status){
        maskops_densify_bitmap_prepare(maskops_bitmap_base, granularity_channelized, mask_idx_max_parallel_channelized, mask_list_base, mask_max_densify, mask_min_densify, 1);
        mask_max_finalize=maskops_densify_remask_prepare(maskops_bitmap_base, 1, mask_max_densify, mask_min_densify, maskops_u32_list_base);
        mask_max_surroundify=mask_max_finalize;
        mask_min_surroundify=0;
        maskops_densify(1, granularity_channelized, mask_idx_max_parallel_channelized, mask_list_base, mask_min_densify, maskops_u32_list_base);
      }
      if(surroundify_status){
        mask_max_finalize=maskops_surroundify(channel_status, 1, granularity, mask_idx_max_parallel, mask_list_base, mask_max_surroundify, mask_min_surroundify);
      }
      if(channel_status){
        mask_max_finalize=(mask_max_finalize+(mask_max_finalize<<U8_BITS)+(mask_max_finalize<<U16_BITS)+(mask_max_finalize<<U24_BITS))&mask_max;
      }
      agnentroprox_mask_max_set(agnentroprox_base, mask_max_finalize);
    }
    status=zorb_mask_list_load(agnentroprox_base, mask_list_base, mask_list_file_size);
    if(status){
      agnentrozorb_error_print("(masklist) is too big or has mistmatched granularity");
      break;
    }
    if(!reset_status){
      status=zorb_mask_idx_max_get(&zorb_mask_idx_max, zorb_base);
      if(!status){
        status=zorb_freq_list_export(agnentroprox_base, zorb_base);
      }
      if(status){
        agnentrozorb_error_print("(zorbfile) frequencies are too big to handle");
        break;
      }
      jsd=agnentroprox_jsd_get(agnentroprox_base, 0, zorb_mask_idx_max, NULL);
      FRU128_MEAN_TO_FTD128(jsd_mean, jsd);
      U128_TO_U64_HI(score, jsd_mean);
    }
    if(!(((absorb_status==AGNENTROZORB_GEOMETRY_ABSORB_ADD_MUNDANE)&alert_status)|(absorb_status==AGNENTROZORB_GEOMETRY_ABSORB_REPORT))){
      if(absorb_status!=AGNENTROZORB_GEOMETRY_ABSORB_SUBTRACT){
        status=zorb_freq_list_add(agnentroprox_base);
        if(status){
          agnentrozorb_error_print("(zorbfile) can't absorb any more data. Please report");
          break;
        }
      }else{
        status=zorb_freq_list_subtract(agnentroprox_base);
        if(status){
          agnentrozorb_error_print("(zorbfile) can't subtract (masklist), perhaps because you did duplicate\nsubtractions previously");
          break;
        }
      }
      status=zorb_freq_list_import(agnentroprox_base, zorb_base);
      if(status){
        agnentrozorb_error_print("Internal error. Please report");
        break;
      }
      zorb_finalize(agnentroprox_base, zorb_base);
      filesys_status=filesys_file_write_obnoxious(zorb_file_size, zorb_filename_base, zorb_base);
      if(filesys_status){
        agnentrozorb_error_print("Can't write to (zorbfile)");
        break;
      }
    }
    if(!polarity_status){
      alert_status=(threshold<score);
    }else{
      alert_status=(score<threshold);
    }
    DEBUG_U64("", score);
    if(alert_status){
      DEBUG_PRINT("*");
    }
    DEBUG_PRINT(" ");
    DEBUG_PRINT(mask_list_filename_base);
    DEBUG_PRINT("\n");
    error_status=0;
  }while(0);
  status=0;
  if(alert_status){
    status=(u8)(status|(1U<<AGNENTROZORB_STATUS_ALERT_BIT_IDX));
  }
  if(error_status){
    status=(u8)(status|(1U<<AGNENTROZORB_STATUS_ERROR_BIT_IDX));
  }
  if(warning_status){
    status=(u8)(status|(1U<<AGNENTROZORB_STATUS_WARNING_BIT_IDX));
  }
  agnentroprox_free_all(agnentroprox_base);
  maskops_free(maskops_u32_list_base);
  maskops_free(maskops_bitmap_base);
  zorb_free(zorb_base);
  agnentroprox_free(mask_list_base);
  loggamma_free(loggamma_base);
  DEBUG_ALLOCATION_CHECK();
  return status;
}
