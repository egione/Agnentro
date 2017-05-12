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
/*
File IO Kernel
*/
#include "flag.h"
#include "flag_filesys.h"
#include <dirent.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "constant.h"
#include "debug.h"
#include "debug_xtrn.h"
#include "filesys.h"
#include "filesys_xtrn.h"

char *
filesys_char_list_malloc(ULONG char_idx_max){
/*
Allocate a list of undefined (char)s.

To maximize portability and debuggability, this is the only function in which Filesys calls malloc().

In:

  char_idx_max is the number of (char)s to allocate, less one.

Out:

  Returns NULL on failure, else the base of (char_idx_max+1) undefined items, which should eventually be freed via filesys_free().
*/
  char *list_base;
  ULONG list_size;

  list_base=NULL;
  list_size=char_idx_max+1;
  if(list_size){
    list_base=DEBUG_MALLOC_PARANOID(list_size);
  }
  return list_base;
}

u8
filesys_file_read(ULONG *file_size_max_base, char *filename_base, void *void_list_base){
/*
Read a file into a buffer, subject to a size limit.

In:

  *file_size_max_base is the maximum number of bytes to read, such that if the file size exceeds this value, then FILESYS_STATUS_TOO_BIG will be returned.

  *filename_base is the NULL-terminated path and filename.

  *void_list_base is undefined and writable for *file_size_max_base bytes.

Out:

  Returns zero on success, else: (1) FILESYS_STATUS_SIZE_CHANGED if the file size was less when read than when its size had been read beforehand, (2) FILESYS_STATUS_NOT_FOUND if the file could not be opened, or (3) FILESYS_STATUS_TOO_BIG if the file size was greater than (In:*file_size_max_base).

  *file_size_max_base is the number of bytes actually read, which is (1) the size of the entire file if the return value is zero, (2) the number of bytes read if the return value is FILESYS_STATUS_SIZE_CHANGED, or (3) zero in all other cases. This is a noncanonical output, in the sense that it's valid regardless of whether or not the return value indicates an error condition.

  *void_list_base contains the first *file_size_max_base bytes of the file. All other bytes are undefined and may have changed. This is a noncanonical output, in the sense that it's valid regardless of whether or not the return value indicates an error condition.
*/
  ULONG filename_idx;
  u8 status;

  filename_idx=0;
  status=filesys_file_read_next(file_size_max_base, &filename_idx, filename_base, void_list_base);
  return status;
}

u8
filesys_file_read_exact(ULONG file_size, char *filename_base, void *void_list_base){
/*
Read a file into a buffer, so long as its size is exactly as expected.

In:

  file_size is the number of bytes to read, such that if the file size exceeds this value, then FILESYS_STATUS_TOO_BIG will be returned.

  *filename_base is the NULL-terminated path and filename.

  *void_list_base is undefined and writable for *file_size_max_base bytes.

Out:

  Returns zero on success, else: (1) FILESYS_STATUS_SIZE_CHANGED if the size of the file was less than file_size, (2) FILESYS_STATUS_NOT_FOUND if the file could not be opened, or (3) FILESYS_STATUS_TOO_BIG if the file size was greater than file_size.

  *void_list_base contains the first file_size bytes of the file. All other bytes are undefined and may have changed. This is a noncanonical output, in the sense that it's valid regardless of whether or not the return value indicates an error condition.
*/
  ULONG filename_idx;
  u8 status;
  ULONG file_size_read;

  filename_idx=0;
  file_size_read=file_size;
  status=filesys_file_read_next(&file_size_read, &filename_idx, filename_base, void_list_base);
  if(((!status)&&(file_size!=file_size_read))||(status==FILESYS_STATUS_TOO_BIG)){
    status=FILESYS_STATUS_SIZE_CHANGED;
  }
  return status;
}

u8
filesys_file_read_next(ULONG *file_size_max_base, ULONG *filename_idx_base, char *filename_list_base, void *void_list_base){
/*
Read a file specified in a filename list, into a buffer, subject to a size limit.

In:

  *file_size_max_base is the maximum number of bytes to read, such that if the file size exceeds this value, then FILESYS_STATUS_TOO_BIG will be returned.

  *filename_idx_base is the base index of the filename at filename_list_base.

  *filename_list_base is a concatenation of NULL-terminated filenames.

  *void_list_base is undefined and writable for *file_size_max_base bytes.

Out:

  Returns zero on success, else: (1) FILESYS_STATUS_SIZE_CHANGED if the file size was less when read than when its size had been read beforehand, (2) FILESYS_STATUS_NOT_FOUND if the file could not be opened, or (3) FILESYS_STATUS_TOO_BIG if the file size was greater than (In:*file_size_max_base).

  *filename_idx_base is the base index of the next filename at filename_list_base, or else the postterminal index of filename_list_base. This is a noncanonical output, in the sense that it's valid regardless of whether or not the return value indicates an error condition.

  *file_size_max_base is the number of bytes actually read, which is (1) the size of the entire file if the return value is zero, (2) the number of bytes read if the return value is FILESYS_STATUS_SIZE_CHANGED, or (3) zero in all other cases. This is a noncanonical output, in the sense that it's valid regardless of whether or not the return value indicates an error condition.

  *void_list_base contains the first *file_size_max_base bytes of the file. All other bytes are undefined and may have changed. This is a noncanonical output, in the sense that it's valid regardless of whether or not the return value indicates an error condition.
*/
  ULONG file_size_read;
  u64 file_size_u64;
  ULONG filename_idx;
  ULONG filename_size;
  FILE *handle;
  u8 status;

  filename_idx=*filename_idx_base;
  filename_size=(ULONG)(strlen(&filename_list_base[filename_idx]));
  file_size_read=0;
  *filename_idx_base=filename_idx+filename_size+1;
  status=filesys_file_size_get(&file_size_u64, &filename_list_base[filename_idx]);
  if(!status){
    status=FILESYS_STATUS_TOO_BIG;
    if(file_size_u64<=*file_size_max_base){
      handle=fopen(&filename_list_base[filename_idx], "rb");
      status=FILESYS_STATUS_NOT_FOUND;
      if(handle){
        file_size_read=(ULONG)(fread(void_list_base, (size_t)(U8_SIZE), (size_t)(file_size_u64), handle));
        status=FILESYS_STATUS_SIZE_CHANGED;
        if(file_size_read==file_size_u64){
          status=0;
        }
        fclose(handle);
      }
    }
  }
  *file_size_max_base=file_size_read;
  return status;
}

u8
filesys_file_size_get(u64 *file_size_base, char *filename_base){
/*
Get the size of a file via (1) fseeko() on MinGW for Windows because lstat() doesn't work or (2) lstat() otherwise.

In:

  *file_size_base is undefined.

  *filename_base is the NULL-terminated path and filename.

Out:

  Returns zero on success, else FILESYS_STATUS_NOT_FOUND.

  *file_size base is the number of bytes in the file.
*/
  u64 file_size;
  #ifndef WINDOWS
    struct stat file_stat;
  #else
    FILE *handle;
  #endif
  u8 status;

  status=FILESYS_STATUS_NOT_FOUND;
  #ifndef WINDOWS
    if(!lstat(filename_base, &file_stat)){
      file_size=(u64)(file_stat.st_size);
      status=0;
    }
  #else
    handle=fopen(filename_base, "rb");
    if(handle){
      if(!FILESYS_FSEEKO(handle, 0, SEEK_END)){
        file_size=(u64)(FILESYS_FTELLO(handle));
        if(~file_size){
          status=0;
        }
      }
      fclose(handle);
    }
  #endif
  if(!status){
    *file_size_base=file_size;
  }
  return status;
}

#ifdef _32_
  u8
  filesys_file_size_ulong_get(ULONG *file_size_base, char *filename_base){
/*
Filter filesys_file_size_get() output to ensure address space compliance for 32-bit systems.

In:

  *file_size_base is undefined.

  *filename_base is the NULL-terminated path and filename.

Out:

  Returns zero on success, else FILESYS_STATUS_NOT_FOUND.

  *file_size base is the number of bytes in the file.
*/
    ULONG file_size;
    u64 file_size_u64;
    u8 status;

    status=filesys_file_size_get(&file_size_u64, filename_base);
    if(!status){
      file_size=(ULONG)(file_size_u64);
      status=FILESYS_STATUS_TOO_BIG;
      if(file_size==file_size_u64){
        status=0;
        *file_size_base=file_size;
      }
    }
    return status;
  }
#endif

u8
filesys_file_write(ULONG file_size, char *filename_base, void *void_list_base){
/*
Write a buffer to a file.

In:

  file_size is the number of bytes to write.

  *filename_base is the NULL-terminated path and filename.

  *void_list_base contains the file_size bytes to write.

Out:

  Returns zero on success else FILESYS_STATUS_WRITE_FAIL. In the latter case, the file has been closed but its contents are undefined.
*/
  ULONG file_size_written;
  FILE *handle;
  u8 status;

  handle=fopen(filename_base, "wb");
  status=FILESYS_STATUS_WRITE_FAIL;
  if(handle){
    status=0;
    if(file_size){
      file_size_written=(ULONG)(fwrite(void_list_base, (size_t)(U8_SIZE), (size_t)(file_size), handle));
      if(file_size!=file_size_written){
        status=FILESYS_STATUS_WRITE_FAIL;
      }
    }
    if(fclose(handle)){
      status=FILESYS_STATUS_WRITE_FAIL;
    }
  }
  return status;  
}

ULONG
filesys_filename_list_get(ULONG *file_size_max_base, u8 *file_status_base, char *filename_list_base, ULONG *filename_list_size_max_base, char *target_base){
/*
Given a file, return the name of that file if it exists. Given a folder, return the relative path and name of every file which it contains, ignoring simlinks, block devices, character devices, and sockets.

In:

  *file_size_max_base undefined.

  *file_status_base undefined.

  *filename_list_base is undefined and writable for (*filename_list_size_max_base+1) bytes.

  *filename_list_size_max_base is the maximum number of bytes which may be written to *filename_list_base.

  *target_base is the file or folder to find, and in the latter case, spider. Both local and global paths are allowed.

Out:

  Returns the number of items at *filename_list_base, all of which being NULL-terminated. Zero implied that *target_base was not found.

  *filename_list_base is just a copy of *target_base if it's a file that exists, else a concatenation of (return value) NULL-terminated relative paths and filenames subordinate to *target_base.

  *filename_list_size_max_base is the number of bytes at filename_list_base.
*/
  u8 continue_status;
  DIR *dir_handle;
  DIR *dir_handle_new;
  DIR **dir_handle_list_base;
  struct dirent *dirent_base;
  ULONG dirent_idx;
  char *dirname_base;
  ULONG dirname_size;
  ULONG file_count;
  ULONG file_size;
  ULONG file_size_max;
  u64 file_size_u64;
  char *filename_base;
  ULONG filename_list_size;
  ULONG filename_list_size_max;
  ULONG filename_list_size_new;
  #ifndef WINDOWS
    mode_t filename_mode;
  #endif
  ULONG filename_size;
  ULONG filename_size_old;
  ULONG *filename_size_list_base;
  #ifndef WINDOWS
    struct stat filename_stat;
  #endif
  u8 rollback_status;

  filename_list_size_max=*filename_list_size_max_base;
  file_count=0;
  file_size=0;
  file_size_max=0;
  file_size_u64=0;
  filename_list_size=0;
  *file_status_base=1;
  filename_size=(ULONG)(strlen(target_base));
  if((1<filename_size)&&(target_base[filename_size-1]==FILESYS_PATH_SEPARATOR)){
    filename_size--;
    target_base[filename_size]=0;
  }
  if(filename_size&&(filename_size<=(FILESYS_PATHNAME_CHAR_IDX_MAX+1))){
    filename_size++;
    dir_handle_list_base=DEBUG_MALLOC_PARANOID((FILESYS_DIRECTORY_DEPTH_IDX_MAX+1)*(ULONG)(sizeof(DIR *)));
    filename_base=filesys_char_list_malloc(FILESYS_PATHNAME_CHAR_IDX_MAX);
    filename_size_list_base=DEBUG_MALLOC_PARANOID((FILESYS_DIRECTORY_DEPTH_IDX_MAX+1)<<ULONG_SIZE_LOG2);
    if(dir_handle_list_base&&filename_base&&filename_size_list_base){
      strcpy(filename_base, target_base);
      continue_status=1;
      dir_handle=NULL;
      dirent_idx=0;
      dirname_size=0;
      filename_size_old=filename_size;
      rollback_status=0;
      do{
        if(!rollback_status){
/*
Ignore everything but files and folders -- even links to them -- because we don't want duplicate analysis and definitely don't want infinite recursion. We have to use lstat() instead of just inspecting dirent_base->d_type because DT_LNK is not consistently reported by Ubuntu. But lstat() doesn't work in Windows under MinGW; fortunately, in that case, links (shortcuts) appear to operate like files rather than folders, and therefore carry no risk of self-referential loops, even though they could result in unavoidable duplicate analysis.
*/
          #ifndef WINDOWS
            if(!lstat(filename_base, &filename_stat)){
              filename_mode=filename_stat.st_mode;
              if(!(S_ISDIR(filename_mode)||S_ISREG(filename_mode))){
                rollback_status=1;
              }
              if(S_ISBLK(filename_mode)||S_ISCHR(filename_mode)||S_ISFIFO(filename_mode)||S_ISLNK(filename_mode)||S_ISSOCK(filename_mode)){
                rollback_status=1;
              }
            }else{
              rollback_status=1;
            }
          #endif
        }
        if(!rollback_status){
          dir_handle_new=opendir(filename_base);
          if(!dir_handle_new){
            #ifndef WINDOWS
              file_size_u64=(u64)(filename_stat.st_size);
            #else
              rollback_status=!!filesys_file_size_get(&file_size_u64, filename_base);
            #endif
            file_size=(ULONG)(file_size_u64);
            #ifdef _32_
              if(file_size!=file_size_u64){
                rollback_status=1;
              }
            #endif
            if(!rollback_status){
              file_size_max=MAX(file_size, file_size_max);
              filename_list_size_new=filename_list_size+filename_size;
              if((filename_list_size<filename_list_size_new)&&(filename_list_size_new<=filename_list_size_max)){
                file_count++;
                memcpy(&filename_list_base[filename_list_size], filename_base, (size_t)(filename_size));
                filename_list_size=filename_list_size_new;
              }
              rollback_status=1;
            }
          }else{
            if(dirent_idx<=FILESYS_DIRECTORY_DEPTH_IDX_MAX){
              dir_handle_list_base[dirent_idx]=dir_handle;
              dir_handle=dir_handle_new;
              filename_size_list_base[dirent_idx]=filename_size_old;
              dirent_idx++;
              filename_size_old=filename_size;
              rollback_status=0;
            }else{
              closedir(dir_handle_new);
              rollback_status=1;
            }
          }
        }
        if(rollback_status){
          continue_status=!!dirent_idx;
          filename_size=filename_size_old;
          filename_base[filename_size_old-1]=0;
        }
        if(continue_status){
          do{
            continue_status=0;
            dirent_base=readdir(dir_handle);
            if(!dirent_base){
              break;
            }
            continue_status=1;
            dirname_base=&dirent_base->d_name[0];
            dirname_size=(ULONG)(strlen(dirname_base)+1);
            rollback_status=0;
            filename_size=filename_size_old+dirname_size;
            if((FILESYS_PATHNAME_CHAR_IDX_MAX+1)<filename_size){
              rollback_status=1;
            }else if(dirname_size<=3){
              if(1<dirname_size){
                if(dirname_base[0]=='.'){
                  if((dirname_size==2)||(dirname_base[1]=='.')){
                    rollback_status=1;
                  }
                }
              }else{
                rollback_status=1;
              }
            }
          }while(rollback_status);
        }
        if(continue_status){
          filename_size=filename_size_old;
          if(filename_base[filename_size_old-2]==FILESYS_PATH_SEPARATOR){
            filename_size--;
          }
          filename_base[filename_size-1]=FILESYS_PATH_SEPARATOR;
          memcpy(&filename_base[filename_size], dirname_base, (size_t)(dirname_size));
          filename_size+=dirname_size;
        }else{
          if(dir_handle){
            closedir(dir_handle);
          }
          if(dirent_idx){
            dirent_idx--;
            dir_handle=dir_handle_list_base[dirent_idx];
            filename_size_old=filename_size_list_base[dirent_idx];
            rollback_status=1;
            if(dirent_idx){
              continue_status=1;
            }else{
              *file_status_base=0;
            }
          }
        }
      }while(continue_status);
    }
    DEBUG_FREE_PARANOID(filename_size_list_base);
    filesys_free(filename_base);
    DEBUG_FREE_PARANOID(dir_handle_list_base);
  }
  *file_size_max_base=file_size_max;
  *filename_list_size_max_base=filesys_hull_size_get(filename_list_size);
  return file_count;
}

void *
filesys_free(void *base){
/*
To maximize portability and debuggability, this is the only function in which Filesys calls free().

In:

  base is the base of a memory region to free. May be NULL.

Out:

  Returns NULL so that the caller can easily maintain the good practice of NULLing out invalid pointers.

  *base is freed.
*/
  DEBUG_FREE_PARANOID(base);
  return NULL;
}

ULONG
filesys_hull_size_get(ULONG size_projected){
/*
Compute MAX(MIN(4, 2^(ceil(log2(size_projected))+1)), ULONG_MAX). This enables exponential backoff of projected allocation needs, which will terminate in tractable time because the extra incrementation prevents hysteresis jitter near powers of 2. The lower bound of 4 isn't magical; it's just convenient and not worth optimizing down.

In:

  size_projected is the number of bytes anticipated to be needed in a future allocation, with no added margin.

Out:

  Returns the number of bytes described in the summary, in order to minimize the incidence of insufficient allocation, yet without wasting too much memory.
*/
  ULONG hull_size;

  hull_size=2;
  do{
    size_projected>>=1;
    hull_size<<=1;
  }while(size_projected);
  hull_size-=!hull_size;
  return hull_size;
}

u8
filesys_init(u32 build_break_count, u32 build_feature_count){
/*
Verify that the source code is sufficiently updated.

In:

  build_break_count is the caller's most recent knowledge of FILESYS_BUILD_BREAK_COUNT, which will fail if the caller is unaware of all critical updates.

  build_feature_count is the caller's most recent knowledge of FILESYS_BUILD_FEATURE_COUNT, which will fail if this library is not up to date with the caller's expectations.

Out:

  Returns one if (build_break_count!=FILESYS_BUILD_BREAK_COUNT) or (build_feature_count>FILESYS_BUILD_FEATURE_COUNT). Otherwise, returns zero.
*/
  u8 status;

  status=(u8)(build_break_count!=FILESYS_BUILD_BREAK_COUNT);
  status=(u8)(status|(FILESYS_BUILD_FEATURE_COUNT<build_feature_count));
  return status;
}
