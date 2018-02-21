/* mz_zip.h -- Zip manipulation
   Version 2.2.7, January 30th, 2018
   part of the MiniZip project

   Copyright (C) 2010-2018 Nathan Moinvaziri
     https://github.com/nmoinvaz/minizip
   Copyright (C) 2009-2010 Mathias Svensson
     Modifications for Zip64 support
     http://result42.com
   Copyright (C) 1998-2010 Gilles Vollant
     http://www.winimage.com/zLibDll/minizip.html

   This program is distributed under the terms of the same license as zlib.
   See the accompanying LICENSE file for the full text of the license.
*/

#ifndef _MZ_ZIP_H
#define _MZ_ZIP_H

#include <stdint.h>
#include <time.h>

#include "mz_strm.h"

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************/

typedef struct mz_zip_file_s
{
    uint16_t version_madeby;            // version made by 
    uint16_t version_needed;            // version needed to extract
    uint16_t flag;                      // general purpose bit flag 
    uint16_t compression_method;        // compression method
    time_t   modified_date;             // last modified date in unix time
    time_t   accessed_date;             // last accessed date in unix time
    time_t   creation_date;             // creation date in unix time
    uint32_t crc;                       // crc-32
    uint64_t compressed_size;           // compressed size
    uint64_t uncompressed_size;         // uncompressed size
    uint16_t filename_size;             // filename length
    uint16_t extrafield_size;           // extra field length
    uint16_t comment_size;              // file comment length
    uint32_t disk_number;               // disk number start
    uint64_t disk_offset;               // relative offset of local header
    uint16_t internal_fa;               // internal file attributes
    uint32_t external_fa;               // external file attributes

    char     *filename;                 // filename string
    uint8_t  *extrafield;               // extrafield data
    char     *comment;                  // comment string

#ifdef HAVE_AES
    uint16_t aes_version;               // winzip aes extension if not 0
    uint8_t  aes_encryption_mode;       // winzip aes encryption mode
#endif
} mz_zip_file;

/***************************************************************************/

extern void *  mz_zip_open(void *stream, int32_t mode);
// Create a zip file, no delete file in zip functionality

extern int32_t mz_zip_close(void *handle);
// Close the zip file

extern int32_t mz_zip_get_comment(void *handle, const char **comment);
// Get a pointer to the global comment

extern int32_t mz_zip_set_comment(void *handle, const char *comment);
// Set the global comment used for writing zip file

extern int32_t mz_zip_get_version_madeby(void *handle, uint16_t *version_madeby);
// Get the version made by

extern int32_t mz_zip_set_version_madeby(void *handle, uint16_t version_madeby);
// Set the version made by used for writing zip file

extern int32_t mz_zip_entry_write_open(void *handle, const mz_zip_file *file_info,
    int16_t compress_level, const char *password);
// Open for writing the current file in the zip file

extern int32_t mz_zip_entry_write(void *handle, const void *buf, uint32_t len);
// Write bytes from the current file in the zip file

extern int32_t mz_zip_entry_read_open(void *handle, int16_t raw, const char *password);
// Open for reading the current file in the zip file

extern int32_t mz_zip_entry_read(void *handle, void *buf, uint32_t len);
// Read bytes from the current file in the zip file

extern int32_t mz_zip_entry_get_info(void *handle, mz_zip_file **file_info);
// Get info about the current file, only valid while current entry is open

extern int32_t mz_zip_entry_get_local_info(void *handle, mz_zip_file **local_file_info);
// Get local info about the current file, only valid while current entry is being read

extern int32_t mz_zip_entry_close_raw(void *handle, uint64_t uncompressed_size, uint32_t crc32);
// Close the current file in the zip file where raw is compressed data

extern int32_t mz_zip_entry_close(void *handle);
// Close the current file in the zip file

/***************************************************************************/

extern int32_t mz_zip_get_number_entry(void *handle, int64_t *number_entry);
// Get the total number of entries

extern int32_t mz_zip_get_disk_number_with_cd(void *handle, int64_t *disk_number_with_cd);
// Get the the disk number containing the central directory record

extern int32_t mz_zip_goto_first_entry(void *handle);
// Go to the first entry in the zip file 

extern int32_t mz_zip_goto_next_entry(void *handle);
// Go to the next entry in the zip file or MZ_END_OF_LIST if reaching the end

typedef int32_t (*mz_filename_compare_cb)(void *handle, const char *filename1, const char *filename2);
extern int32_t mz_zip_locate_entry(void *handle, const char *filename, 
    mz_filename_compare_cb filename_compare_cb);
// Locate the file with the specified name in the zip file or MZ_END_LIST if not found

/***************************************************************************/

int32_t  mz_zip_dosdate_to_tm(uint64_t dos_date, struct tm *ptm);
// Convert dos date/time format to struct tm

time_t   mz_zip_dosdate_to_time_t(uint64_t dos_date);
// Convert dos date/time format to time_t

int32_t  mz_zip_time_t_to_tm(time_t unix_time, struct tm *ptm);
// Convert time_t to time struct 

uint32_t mz_zip_time_t_to_dos_date(time_t unix_time);
// Convert time_t to dos date/time format

uint32_t mz_zip_tm_to_dosdate(const struct tm *ptm);
// Convert struct tm to dos date/time format

int32_t  mz_zip_ntfs_to_unix_time(uint64_t ntfs_time, time_t *unix_time);
// Convert ntfs time to unix time

int32_t  mz_zip_unix_to_ntfs_time(time_t unix_time, uint64_t *ntfs_time);
// Convert unix time to ntfs time

/***************************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* _ZIP_H */
