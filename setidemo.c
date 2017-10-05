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
SETI Signal Detection Demo
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

#define SETIDEMO_MASK_COUNT_LOG2 24U
#define SETIDEMO_MASK_IDX_MAX ((ULONG)((1ULL<<SETIDEMO_MASK_COUNT_LOG2)-1))
#define SETIDEMO_SWEEP_IDX_MAX_MAX (SETIDEMO_MASK_IDX_MAX>>4)

void
setidemo_error_print(char *char_list_base){
  DEBUG_PRINT("ERROR: ");
  DEBUG_PRINT(char_list_base);
  DEBUG_PRINT(".\n");
  return;
}

void
setidemo_out_of_memory_print(void){
  setidemo_error_print("Out of memory");
  return;
}

int
main(int argc, char *argv[]){
  agnentroprox_t *agnentroprox_base;
  u128 agnentropy_score;
  ULONG arg_idx;
  ULONG channel_size;
  u8 delta_status;
  u64 dispersion_optimizer;
  fru128 entropy_list_base[1];
  u128 exoentropy_score;
  u128 exoelasticity_score;
  ULONG file_mask_idx_max;
  ULONG file_mask_idx_min;
  u64 file_mask_idx_min_u64;
  ULONG file_mask_idx_post;
  u8 *file_mask_list_base;
  u8 *file_mask_list_base_new;
  ULONG file_size;
  u64 file_size_u64;
  char *filename_base;
  u128 fourier_score;
  u8 fourier_status;
  u64 fourier_sum;
  u64 fourier_sum_absolute;
  u64 fourier_sum_max;
  u8 ignored_status;
  u64 iteration;
  u128 kurtosis_score;
  u128 logfreedom_score;
  loggamma_t *loggamma_base;
  u8 mask;
  ULONG mask_idx;
  ULONG mask_idx_min;
  ULONG mask_idx_old;
  u8 mask_old;
  u8 *mask_list_base;
  ULONG match_u8_idx;
  u128 mean_f128;
  u64 mean_u64;
  u8 mean_u8;
  ULONG miss_count;
  u8 mode;
  u8 mode_bitmap;
  u8 mode_bitmap_copy;
  u8 overflow_status;
  u64 parameter;
  u8 parity;
  u64 random;
  u128 random_u128;
  u128 score;
  u64 score_delta;
  u128 score_max;
  u64 seed;
  u128 shannon_score;
  u8 sign_status;
  u8 status;
  ULONG sweep_mask_count;
  ULONG sweep_mask_idx_max;
  u8 toggle;
  u128 variance_score;

  delta_status=1;
  ignored_status=0;
  overflow_status=0;
  status=ascii_init(ASCII_BUILD_BREAK_COUNT_EXPECTED, 0);
  status=(u8)(status|filesys_init(FILESYS_BUILD_BREAK_COUNT_EXPECTED, 0));
  status=(u8)(status|fracterval_u128_init(FRU128_BUILD_BREAK_COUNT_EXPECTED, 0));
  loggamma_base=loggamma_init(LOGGAMMA_BUILD_BREAK_COUNT_EXPECTED, 0);
  status=(u8)(status|!loggamma_base);
  agnentroprox_base=NULL;
  file_mask_list_base=NULL;
  mask_list_base=NULL;
  do{
    if(status){
      setidemo_error_print("Outdated source code");
      break;
    }
    status=1;
    if(argc!=5){
      DEBUG_PRINT("Search for Extraterrestrial Intelligence Signal Detection Demo\nCopyright 2017 Russell Leidich\n\n");
      DEBUG_PRINT("This demo will superimpose a very weak signal of a specified size on top of a\nrandom contiguous (2^24)-byte subset of a SETI data file, thereby simulating an\nalien carrier wave intercept. It will then display, on each iteration, a table\nof tested detection modes and their associated hex fractions. The fraction is\nthe particular mode's mean fraction of overlap between its highest ranking\nsweep and the actual sweep where the signal was injected. (A sweep is just a\ncontiguous subset of the aforementioned subset. Think of it as a byte-by-byte\nsliding window in which a particular entropy metric is evalutated, with the\nresults being sorted so that the first rank corresponds to greatest\nentropy.)\n\n");
      DEBUG_PRINT("Syntax:\n\n");
      DEBUG_PRINT("setidemo file mode_bitmap seed sweep\n\n");
      DEBUG_PRINT("where:\n\n");
      DEBUG_PRINT("(file) is a 2-channel signed byte data file from:\n\n  http://setiquest.org/wiki/index.php?title=SetiQuest_Data_Links&oldid=3581\n\n");
      DEBUG_PRINT("(mode_bitmap) is a 16-bit hex value, the bits of which giving the desired test modes:\n\n");
      DEBUG_PRINT("  bit 0: agnentropy\n  bit 1: exoelasticity\n  bit 2: exoentropy\n  bit 3: logfreedom\n  bit 4: shannon entropy\n  bit 5: obtuse kurtosis\n  bit 6: obtuse variance\n  bit 7: ideal absolute Fourier integral\n  bit 8: preprocess with deltafication\n\n");
      DEBUG_PRINT("(\"Obtuse\" refers to the fact that the mean of the entire (2^24)-sample subset\nwill be assumed equal to the mean of the sweep; otherwise, compute complexity\nwould grow out of hand due to the nonlinear relationship among mean, variance,\nand kurtosis.) Test various modes to evaluate sensitivity and speed. Agnentropy\nis fastest; kurtosis is slowest. Usually, exoentropy is the most sensitive;\nkurtosis is the least. Set bit 6 to enable Fourier analysis at the optimum\nfrequency required to detect the signal; this provides a sense of best case\nperformance, given unlimited compute power and advance knowledge of the signal\ntype. Set bit 7 to take the first delta before testing. For example, the mask\nlist {A, B, C...} would be transformed to {A, (B-A), (C-B)...}, wrapped to 8\nbits. This will usually enhance sensitivity, but will degrade it for Fourier.\n\n");
      DEBUG_PRINT("(seed) is a decimal random number less than (2^64).\n\n");
      DEBUG_PRINT("(sweep) is the nonzero decimal number of samples (bytes, in the case of SETI\nfiles) in the sliding window of entropy analysis for each mode. For the sake of\nsimplicity, only channel 0 will be analyzed. (2^24) contiguous samples will be\nread from the file at a random base offset on each iteration. Then, sweep bytes\nof signal will be injected into its end zone, where it's hardest to find by\nluck due to result ranking rules. The signal will alternately increment and\ndecrement successive bytes, saturating the results to the signed 8-bit range.\nThis represents the lowest-amplitude signal which could theoretically be\ndetected, and which affects all sweep bytes. Start with 100000, then adjust\nas necessary until the fractions are all around (1/2), i.e. (80...) in hex.\n\n");
      break;
    }
    arg_idx=0;
    do{
      status=ascii_utf8_string_verify(argv[arg_idx]);
      if(status){
        setidemo_error_print("One or more parameters is encoded using invalid UTF8");
        break;
      }
    }while((++arg_idx)<(ULONG)(argc));
    if(status){
      break;
    }
    status=ascii_hex_to_u64_convert(argv[2], &parameter, 0x1FF);
    status=(u8)(status|!parameter);
    if(status){
      setidemo_error_print("Invalid mode_bitmap");
      break;
    }
    delta_status=(u8)((parameter>>8)&1);
    fourier_status=(u8)((parameter>>7)&1);
    mode_bitmap=0;
    if((parameter>>0)&1){
      mode_bitmap=(u8)(mode_bitmap|AGNENTROPROX_MODE_AGNENTROPY);
    }
    if((parameter>>1)&1){
      mode_bitmap=(u8)(mode_bitmap|AGNENTROPROX_MODE_EXOELASTICITY);
    }
    if((parameter>>2)&1){
      mode_bitmap=(u8)(mode_bitmap|AGNENTROPROX_MODE_EXOENTROPY);
    }
    if((parameter>>3)&1){
      mode_bitmap=(u8)(mode_bitmap|AGNENTROPROX_MODE_LOGFREEDOM);
    }
    if((parameter>>4)&1){
      mode_bitmap=(u8)(mode_bitmap|AGNENTROPROX_MODE_SHANNON);
    }
    if((parameter>>5)&1){
      mode_bitmap=(u8)(mode_bitmap|AGNENTROPROX_MODE_KURTOSIS);
    }
    if((parameter>>6)&1){
      mode_bitmap=(u8)(mode_bitmap|AGNENTROPROX_MODE_VARIANCE);
    }
    status=ascii_decimal_to_u64_convert(argv[3], &seed, U64_MAX);
    if(status){
      setidemo_error_print("Invalid seed");
      break;
    }
    status=ascii_decimal_to_u64_convert(argv[4], &parameter, SETIDEMO_SWEEP_IDX_MAX_MAX+1);
    status=(u8)(status|!parameter);
    if(status){
      setidemo_error_print("sweep must be on [1, (2^20)]");
      break;
    }
    sweep_mask_count=(ULONG)(parameter);
    sweep_mask_idx_max=sweep_mask_count-1;
    filename_base=argv[1];
    status=filesys_file_size_get(&file_size_u64, filename_base);
    if(status){
      setidemo_error_print("File not found");
      break;
    }
    file_size=(ULONG)(file_size_u64);
    status=(file_size!=file_size_u64);
    if(status){
      setidemo_error_print("File too big");
      break;
    }
    status=(file_size<((SETIDEMO_MASK_IDX_MAX+1)<<1));
    if(status){
      setidemo_error_print("File too small");
      break;
    }
/*
It's safe to ignore the status return from agnentroprox_mask_idx_max_get() when (granularity==U8_BYTE_MAX).
*/
    file_mask_idx_max=agnentroprox_mask_idx_max_get(U8_BYTE_MAX, &status, file_size, 0);
    file_mask_list_base=agnentroprox_mask_list_malloc(U8_BYTE_MAX, file_mask_idx_max, 0);
    status=!file_mask_list_base;
    if(status){
      setidemo_out_of_memory_print();
      break;
    }
    status=(u8)(file_size&1);
    if(status){
      setidemo_error_print("SETI data files must have an even number of bytes");
      break;
    }
    DEBUG_PRINT("Reading ");
    DEBUG_PRINT(filename_base);
    DEBUG_PRINT("...\n");
    status=filesys_file_read_exact(file_size, filename_base, file_mask_list_base);
    status=(u8)(status|(file_size!=file_size_u64));
    if(status){
      setidemo_error_print("File read failed");
      break;
    }
    DEBUG_PRINT("Extracting channel 0 and discarding channel 1...\n");
    mask_idx=0;
    do{
      file_mask_list_base[mask_idx>>1]=file_mask_list_base[mask_idx];
      mask_idx+=2;
    }while(mask_idx<=file_mask_idx_max);
    file_mask_idx_max>>=1;
    file_mask_list_base_new=agnentroprox_mask_list_realloc(U8_BYTE_MAX, file_mask_idx_max, file_mask_list_base, 0);
    if(file_mask_list_base_new){
      file_mask_list_base=file_mask_list_base_new;
    }
    mask_list_base=agnentroprox_mask_list_malloc(U8_BYTE_MAX, SETIDEMO_MASK_IDX_MAX, 0);
    status=!mask_list_base;
    if(status){
      setidemo_out_of_memory_print();
      break;
    }
    DEBUG_PRINT("Initializing Agnentro...\n");
    agnentroprox_base=agnentroprox_init(AGNENTROPROX_BUILD_BREAK_COUNT_EXPECTED, 2, U8_BYTE_MAX, loggamma_base, SETIDEMO_MASK_IDX_MAX, U8_MAX, mode_bitmap, 0, sweep_mask_idx_max);
    status=!agnentroprox_base;
    if(status){
      setidemo_error_print("Agnentro initialization failed");
      break;
    }
    if(delta_status){
      DEBUG_PRINT("Computing first delta of samples...\n");
      agnentroprox_mask_list_deltafy(0, 1, U8_BYTE_MAX, file_mask_idx_max, file_mask_list_base);
    }
    DEBUG_PRINT("Computing mean of ");
    if(delta_status){
      DEBUG_PRINT("first delta of ");
    }
    DEBUG_PRINT("samples...\n");
    mean_f128=agnentroprox_mask_list_mean_get(agnentroprox_base, file_mask_idx_max, file_mask_list_base, &sign_status);
    U128_TO_U64_HI(mean_u64, mean_f128);
    mean_u8=(u8)(mean_u64>>(U64_BITS-U8_BITS));
    status=(!sign_status)|(1<mean_u8);
    if(status){
      setidemo_error_print("SETI files consist of alternating signed byte channels with a mean close to 0.\nThis file does not fit that description.");
      break;
    }
    U128_SET_ZERO(agnentropy_score);
    channel_size=file_mask_idx_max+1;
    dispersion_optimizer=seed;
    U128_SET_ZERO(exoelasticity_score);
    U128_SET_ZERO(exoentropy_score);
    U128_SET_ZERO(fourier_score);
    iteration=0;
    U128_SET_ZERO(kurtosis_score);
    U128_SET_ZERO(logfreedom_score);
    U128_SET_ZERO(score);
    U128_SET_ZERO(score_max);
    U128_SET_ZERO(shannon_score);
    U128_SET_ZERO(variance_score);
    DEBUG_PRINT("Dumping metadata...\n\n");
    DEBUG_U64("channel_0_size", channel_size);
    DEBUG_U8("delta_status", delta_status);
    DEBUG_PRINT("file=");
    DEBUG_PRINT(filename_base);
    DEBUG_PRINT("\n");
    DEBUG_U64("file_size", file_size);
    DEBUG_PRINT("mean_signed=");
    if(sign_status==1){
      DEBUG_PRINT("+");
    }else{
      DEBUG_PRINT("-");
    }
    DEBUG_F128("",mean_f128);
    DEBUG_PRINT("\n");
    DEBUG_U64("seed", seed);
    DEBUG_U64("subset_size", SETIDEMO_MASK_IDX_MAX+1);
    DEBUG_U64("sweep_size", sweep_mask_count);
    DEBUG_PRINT("\nStarting signal injection simulation...\n\n");
    do{
      DEBUG_U64("iteration",iteration);
/*
Use reverse incrementation to compute base offsets of successive analyses of (SETIDEMO_MASK_IDX_MAX+1) masks because doing so will result in the widest coverage of the mask list as quickly as possible.
*/
      do{
        random=dispersion_optimizer;
        dispersion_optimizer++;
        REVERSE_U64(random);
        U128_FROM_U64_PRODUCT(random_u128, random, (u64)(file_mask_idx_max));
        U128_TO_U64_HI(file_mask_idx_min_u64, random_u128);
        file_mask_idx_min=(ULONG)(file_mask_idx_min_u64);
        file_mask_idx_post=file_mask_idx_min+SETIDEMO_MASK_IDX_MAX+1;
      }while((file_mask_idx_post<file_mask_idx_min)||((file_mask_idx_max+1)<file_mask_idx_post));
      DEBUG_U64("subset_base_offset", file_mask_idx_min);
      memcpy(mask_list_base, &file_mask_list_base[file_mask_idx_min], (size_t)(SETIDEMO_MASK_IDX_MAX+1));
      mask_idx_min=SETIDEMO_MASK_IDX_MAX-sweep_mask_idx_max;
      mode=0;
      mode_bitmap_copy=mode_bitmap;
/*
Add the lowest possible amplitude, highest possible frequency signal to the last (SETIDEMO_MASK_IDX_MAX+1) bytes of the mask list. The signal is simply an amplitude-one square wave, where we increment the first pixel, decrement the second, increment the third, etc., being careful not to cause signed integer wrap. Doing so won't affect the mean, upon which kurtosis and variance depend. By the way, we inject it in the mask list "end zone" instead of somewhere in the middle because by default sweeps which return equal entropies will favor lesser offsets, so the sweep containing part of the signal needs to be the top ranked sweep. At least half of the sweep must overlap the signal in order to consider the signal found.
*/
      mask_idx=mask_idx_min;
      toggle=0;
      if(!delta_status){
        do{
          mask=mask_list_base[mask_idx];
          if(toggle&&(mask!=I8_MAX)){
            mask++;
          }else if(mask!=U8_SPAN_HALF){
            mask--;
          }
          toggle=!toggle;
          mask_list_base[mask_idx]=mask;
        }while((mask_idx++)!=SETIDEMO_MASK_IDX_MAX);
      }else{
/*
Injecting a signal of amplitude one means that the first deltas will alternately increase and decrease by 2 (except for the first one, which is negligible).
*/
        do{
          mask=mask_list_base[mask_idx];
          if(toggle){
            if(mask<(I8_MAX-1)){
              mask=(u8)(mask+2);
            }else if(mask==(I8_MAX-1)){
              mask++;
            }
          }else{
            if((U8_SPAN_HALF+1)<mask){
              mask=(u8)(mask-2);
            }else if(mask==(U8_SPAN_HALF+1)){
              mask--;
            }
          }
          toggle=!toggle;
          mask_list_base[mask_idx]=mask;
        }while((mask_idx++)!=SETIDEMO_MASK_IDX_MAX);
      }
      if(fourier_status){
        fourier_sum=0;
        mask_idx=0;
        toggle=0;
        do{
          mask=mask_list_base[mask_idx];
          if(!toggle){
            fourier_sum+=mask;
            if(I8_MAX<mask){
              fourier_sum-=U8_SPAN;
            }
          }else{
            fourier_sum-=mask;
            if(I8_MAX<mask){
              fourier_sum+=U8_SPAN;
            }
          }
          toggle=!toggle;
        }while((mask_idx++)!=sweep_mask_idx_max);
        fourier_sum_absolute=fourier_sum;
        if(fourier_sum_absolute>>U64_BIT_MAX){
          fourier_sum_absolute=(0U-fourier_sum_absolute);
        }
        fourier_sum_max=fourier_sum_absolute;
        mask_idx=sweep_mask_count;
        mask_idx_old=0;
        match_u8_idx=0;
        parity=(u8)(sweep_mask_idx_max&1);
        do{
          mask=mask_list_base[mask_idx];
          mask_old=mask_list_base[mask_idx_old];
          if(!toggle){
            fourier_sum+=mask;
            if(I8_MAX<mask){
              fourier_sum-=U8_SPAN;
            }
            if(parity){
              fourier_sum-=mask_old;
              if(I8_MAX<mask_old){
                fourier_sum+=U8_SPAN;
              }
            }else{
              fourier_sum+=mask_old;
              if(I8_MAX<mask_old){
                fourier_sum-=U8_SPAN;
              }
            }
          }else{
            fourier_sum-=mask;
            if(I8_MAX<mask){
              fourier_sum+=U8_SPAN;
            }
            if(parity){
              fourier_sum+=mask_old;
              if(I8_MAX<mask_old){
                fourier_sum-=U8_SPAN;
              }
            }else{
              fourier_sum-=mask_old;
              if(I8_MAX<mask_old){
                fourier_sum+=U8_SPAN;
              }
            }
          }
          mask_idx_old++;
          fourier_sum_absolute=fourier_sum;
          if(fourier_sum_absolute>>U64_BIT_MAX){
            fourier_sum_absolute=(0U-fourier_sum_absolute);
          }
          if(fourier_sum_max<fourier_sum_absolute){
            fourier_sum_max=fourier_sum_absolute;
            match_u8_idx=mask_idx_old;
          }
          toggle=!toggle;
        }while((mask_idx++)!=SETIDEMO_MASK_IDX_MAX);
/*
Set score_delta to the number of masks in the signal which were not included in the top ranking sweep.
*/
        miss_count=mask_idx_min-match_u8_idx;
        miss_count=MIN(miss_count, sweep_mask_count);
        score_delta=sweep_mask_count-miss_count;
        U128_ADD_U64_LO_SELF(fourier_score, score_delta);
      }
      do{
        if(mode_bitmap_copy&AGNENTROPROX_MODE_AGNENTROPY){
          mode=AGNENTROPROX_MODE_AGNENTROPY;
        }else if(mode_bitmap_copy&AGNENTROPROX_MODE_EXOENTROPY){
          mode=AGNENTROPROX_MODE_EXOENTROPY;
        }else if(mode_bitmap_copy&AGNENTROPROX_MODE_EXOELASTICITY){
          mode=AGNENTROPROX_MODE_EXOELASTICITY;
        }else if(mode_bitmap_copy&AGNENTROPROX_MODE_KURTOSIS){
          mode=AGNENTROPROX_MODE_KURTOSIS;
        }else if(mode_bitmap_copy&AGNENTROPROX_MODE_LOGFREEDOM){
          mode=AGNENTROPROX_MODE_LOGFREEDOM;
        }else if(mode_bitmap_copy&AGNENTROPROX_MODE_SHANNON){
          mode=AGNENTROPROX_MODE_SHANNON;
        }else if(mode_bitmap_copy&AGNENTROPROX_MODE_VARIANCE){
          mode=AGNENTROPROX_MODE_VARIANCE;
        }
        mode_bitmap_copy=(u8)(mode^mode_bitmap_copy);
/*
Find the highest entropy sweep, with ties resolving in favor of earlier sweeps (i.e. away from the end zone where the signal actually resides).
*/
        if(mode!=AGNENTROPROX_MODE_EXOELASTICITY){
          agnentroprox_entropy_transform(agnentroprox_base, 0, entropy_list_base, SETIDEMO_MASK_IDX_MAX, mask_list_base, 0, &match_u8_idx, mode, &overflow_status, sweep_mask_idx_max);
        }else{
          agnentroprox_exoelasticity_transform(agnentroprox_base, 1, entropy_list_base, SETIDEMO_MASK_IDX_MAX, mask_list_base, 0, &match_u8_idx, &overflow_status, sweep_mask_idx_max);
        }
        switch(mode){
        case AGNENTROPROX_MODE_AGNENTROPY:
          score=agnentropy_score;
          break;
        case AGNENTROPROX_MODE_EXOENTROPY:
          score=exoentropy_score;
          break;
        case AGNENTROPROX_MODE_EXOELASTICITY:
          score=exoelasticity_score;
          break;
        case AGNENTROPROX_MODE_KURTOSIS:
          score=kurtosis_score;
          break;
        case AGNENTROPROX_MODE_LOGFREEDOM:
          score=logfreedom_score;
          break;
        case AGNENTROPROX_MODE_SHANNON:
          score=shannon_score;
          break;
        case AGNENTROPROX_MODE_VARIANCE:
          score=variance_score;
          break;
        }
/*
Set score_delta to the number of masks in the signal which were not included in the top ranking sweep.
*/
        miss_count=mask_idx_min-match_u8_idx;
        miss_count=MIN(miss_count, sweep_mask_count);
        score_delta=sweep_mask_count-miss_count;
        U128_ADD_U64_LO_SELF(score, score_delta);
        switch(mode){
        case AGNENTROPROX_MODE_AGNENTROPY:
          agnentropy_score=score;
          break;
        case AGNENTROPROX_MODE_EXOENTROPY:
          exoentropy_score=score;
          break;
        case AGNENTROPROX_MODE_EXOELASTICITY:
          exoelasticity_score=score;
          break;
        case AGNENTROPROX_MODE_KURTOSIS:
          kurtosis_score=score;
          break;
        case AGNENTROPROX_MODE_LOGFREEDOM:
          logfreedom_score=score;
          break;
        case AGNENTROPROX_MODE_SHANNON:
          shannon_score=score;
          break;
        case AGNENTROPROX_MODE_VARIANCE:
          variance_score=score;
          break;
        }
        status=overflow_status;
        if(status){
          setidemo_error_print("Fracterval precision was exhausted. Please report");
          break;
        }
      }while(mode_bitmap_copy);
      if(status){
        break;
      }
      U128_ADD_U64_LO_SELF(score_max, sweep_mask_count);
      if(mode_bitmap&AGNENTROPROX_MODE_AGNENTROPY){
        FTD128_RATIO_U128_SATURATE(score, agnentropy_score, score_max, ignored_status);
        DEBUG_U128("agnentropy_score     ", score);
      }
      if(mode_bitmap&AGNENTROPROX_MODE_EXOENTROPY){
        FTD128_RATIO_U128_SATURATE(score, exoentropy_score, score_max, ignored_status);
        DEBUG_U128("exoentropy_score     ", score);
      }
      if(mode_bitmap&AGNENTROPROX_MODE_EXOELASTICITY){
        FTD128_RATIO_U128_SATURATE(score, exoelasticity_score, score_max, ignored_status);
        DEBUG_U128("exoelasticity_score  ", score);
      }
      if(fourier_status){
        FTD128_RATIO_U128_SATURATE(score, fourier_score, score_max, ignored_status);
        DEBUG_U128("fourier_score        ", score);
      }
      if(mode_bitmap&AGNENTROPROX_MODE_LOGFREEDOM){
        FTD128_RATIO_U128_SATURATE(score, logfreedom_score, score_max, ignored_status);
        DEBUG_U128("logfreedom_score     ", score);
      }
      if(mode_bitmap&AGNENTROPROX_MODE_SHANNON){
        FTD128_RATIO_U128_SATURATE(score, shannon_score, score_max, ignored_status);
        DEBUG_U128("shannon_score        ", score);
      }
      if(mode_bitmap&AGNENTROPROX_MODE_KURTOSIS){
        FTD128_RATIO_U128_SATURATE(score, kurtosis_score, score_max, ignored_status);
        DEBUG_U128("obtuse_kurtosis_score", score);
      }
      if(mode_bitmap&AGNENTROPROX_MODE_VARIANCE){
        FTD128_RATIO_U128_SATURATE(score, variance_score, score_max, ignored_status);
        DEBUG_U128("obtuse_variance_score", score);
      }
    }while(++iteration);
    if(status){
      break;
    }
  }while(0);
  agnentroprox_free_all(agnentroprox_base);
  agnentroprox_free(mask_list_base);
  agnentroprox_free(file_mask_list_base);
  return status;
}
