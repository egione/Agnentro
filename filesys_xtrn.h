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
extern char *filesys_char_list_malloc(ULONG char_idx_max);
extern u8 filesys_file_read(ULONG *file_size_max_base, char *filename_base, void *void_list_base);
extern u8 filesys_file_read_exact(ULONG file_size, char *filename_base, void *void_list_base);
extern u8 filesys_file_read_next(ULONG *file_size_max_base, ULONG *filename_idx_base, char *filename_list_base, void *void_list_base);
extern u8 filesys_file_size_get(u64 *file_size_base, char *filename_base);
#ifdef _64_
  #define filesys_file_size_ulong_get filesys_file_size_get
#else
  extern u8 filesys_file_size_ulong_get(ULONG *file_size_base, char *filename_base);
#endif
extern u8 filesys_file_write(ULONG file_size, char *filename_base, void *void_list_base);
extern u8 filesys_file_write_next_obnoxious(ULONG file_size, ULONG *filename_idx_base, char *filename_list_base, void *void_list_base);
extern u8 filesys_file_write_obnoxious(ULONG file_size, char *filename_base, void *void_list_base);
extern ULONG filesys_filename_isolate(ULONG *filename_idx_min_base, char *filename_list_base);
extern ULONG filesys_filename_list_get(ULONG *file_size_max_base, u8 *file_status_base, char *filename_list_base, ULONG *filename_list_size_max_base, char *target_base);
extern void filesys_filename_list_morph(ULONG filename_count, char *source_base, char *source_filename_list_base, char *target_base, char *target_filename_list_base);
extern ULONG filesys_filename_list_morph_size_get(ULONG filename_count, char *filename_list_base, char *source_base, char *target_base);
extern u8 filesys_filename_list_sort(ULONG filename_count, char *filename_list_base);
extern void *filesys_free(void *base);
extern ULONG filesys_hull_size_get(ULONG size_projected);
extern ULONG filesys_hull_size_get(ULONG size_projected);
extern u8 filesys_init(u32 build_break_count, u32 build_feature_count);
extern u8 filesys_subfile_read_next(u8 direction_status, ULONG *filename_idx_base, char *filename_list_base, ULONG read_size, u64 read_u8_idx_min, void *void_list_base);
