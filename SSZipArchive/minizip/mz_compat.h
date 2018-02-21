/* mz_compat.h -- Backwards compatible interface for older versions
   Version 2.2.7, January 30th, 2018
   part of the MiniZip project

   Copyright (C) 2010-2018 Nathan Moinvaziri
     https://github.com/nmoinvaz/minizip
   Copyright (C) 1998-2010 Gilles Vollant
     http://www.winimage.com/zLibDll/minizip.html

   This program is distributed under the terms of the same license as zlib.
   See the accompanying LICENSE file for the full text of the license.
*/

#ifndef _MZ_COMPAT_H
#define _MZ_COMPAT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "zlib.h"

/***************************************************************************/

#ifdef __GNUC__
#  define ZIP_UNUSED __attribute__((__unused__))
#else
#  define ZIP_UNUSED
#endif

#ifndef DEF_MEM_LEVEL
#  if MAX_MEM_LEVEL >= 8
#    define DEF_MEM_LEVEL 8
#  else
#    define DEF_MEM_LEVEL  MAX_MEM_LEVEL
#  endif
#endif

#if defined(USE_FILE32API)
#  define MZ_USE_FILE32API
#  define fopen64 fopen
#  define ftello64 ftell
#  define fseeko64 fseek
#else
#  if defined(__FreeBSD__) || defined(__NetBSD__) || defined(__DragonFly__) || \
      defined(__OpenBSD__) || defined(__APPLE__) || defined(__ANDROID__)
#    define fopen64 fopen
#    define ftello64 ftello
#    define fseeko64 fseeko
#  endif
#  ifdef _MSC_VER
#    define fopen64 fopen
#    if (_MSC_VER >= 1400) && (!(defined(NO_MSCVER_FILE64_FUNC)))
#      define ftello64 _ftelli64
#      define fseeko64 _fseeki64
#    else /* old MSC */
#      define ftello64 ftell
#      define fseeko64 fseek
#    endif
#  endif
#endif

/***************************************************************************/

#if defined(STRICTZIP) || defined(STRICTZIPUNZIP)
/* like the STRICT of WIN32, we define a pointer that cannot be converted
    from (void*) without cast */
typedef struct TagzipFile__ { int unused; } zip_file__;
typedef zip_file__ *zipFile;
#else
typedef voidp zipFile;
#endif

/***************************************************************************/

typedef void *zlib_filefunc_def;
typedef void *zlib_filefunc64_def;
typedef const char *zipcharpc;

/***************************************************************************/

typedef struct
{
    uint32_t    dos_date;
    uint16_t    internal_fa;        // internal file attributes        2 bytes
    uint32_t    external_fa;        // external file attributes        4 bytes
} zip_fileinfo;

/***************************************************************************/

#define ZIP_OK                          (0)
#define ZIP_EOF                         (0)
#define ZIP_ERRNO                       (Z_ERRNO)
#define ZIP_PARAMERROR                  (-102)
#define ZIP_BADZIPFILE                  (-103)
#define ZIP_INTERNALERROR               (-104)

#define Z_BZIP2ED                       (12)

#define APPEND_STATUS_CREATE            (1)
#define APPEND_STATUS_CREATEAFTER       (2)
#define APPEND_STATUS_ADDINZIP          (3)

/***************************************************************************/

extern zipFile ZEXPORT zipOpen(const char *path, int append);
extern zipFile ZEXPORT zipOpen64(const void *path, int append);
extern zipFile ZEXPORT zipOpen2(const char *path, int append, const char **globalcomment,
    zlib_filefunc_def *pzlib_filefunc_def);
extern zipFile ZEXPORT zipOpen2_64(const void *path, int append, const char **globalcomment, 
    zlib_filefunc64_def *pzlib_filefunc_def);

extern int ZEXPORT zipOpenNewFileInZip3(zipFile file, const char *filename, const zip_fileinfo *zipfi,
    const void *extrafield_local, uint16_t size_extrafield_local, const void *extrafield_global,
    uint16_t size_extrafield_global, const char *comment, uint16_t compression_method, int level,
    int raw, int windowBits, int memLevel, int strategy, const char *password,
    ZIP_UNUSED uint32_t crc_for_crypting);
extern int ZEXPORT zipOpenNewFileInZip3_64(zipFile file, const char *filename, const zip_fileinfo *zipfi,
    const void *extrafield_local, uint16_t size_extrafield_local, const void *extrafield_global,
    uint16_t size_extrafield_global, const char *comment, uint16_t compression_method, int level,
    int raw, int windowBits, int memLevel, int strategy, const char *password,
    ZIP_UNUSED uint32_t crc_for_crypting, int zip64);
extern int ZEXPORT zipOpenNewFileInZip4(zipFile file, const char *filename, const zip_fileinfo *zipfi,
    const void *extrafield_local, uint16_t size_extrafield_local, const void *extrafield_global,
    uint16_t size_extrafield_global, const char *comment, uint16_t compression_method, int level,
    int raw, int windowBits, int memLevel, int strategy, const char *password,
    ZIP_UNUSED uint32_t crc_for_crypting, uint16_t version_madeby, uint16_t flag_base);
extern int ZEXPORT zipOpenNewFileInZip4_64(zipFile file, const char *filename, const zip_fileinfo *zipfi,
    const void *extrafield_local, uint16_t size_extrafield_local, const void *extrafield_global,
    uint16_t size_extrafield_global, const char *comment, uint16_t compression_method, int level,
    int raw, int windowBits, int memLevel, int strategy, const char *password,
    ZIP_UNUSED uint32_t crc_for_crypting, uint16_t version_madeby, uint16_t flag_base, int zip64);
extern int ZEXPORT zipOpenNewFileInZip5(zipFile file, const char *filename, const zip_fileinfo *zipfi,
    const void *extrafield_local, uint16_t size_extrafield_local, const void *extrafield_global,
    uint16_t size_extrafield_global, const char *comment, uint16_t compression_method, int level,
    int raw, int windowBits, int memLevel, int strategy, const char *password,
    ZIP_UNUSED uint32_t crc_for_crypting, uint16_t version_madeby, uint16_t flag_base, int zip64);

extern int ZEXPORT zipWriteInFileInZip(zipFile file, const void *buf, uint32_t len);

extern int ZEXPORT zipCloseFileInZipRaw(zipFile file, uint32_t uncompressed_size, uint32_t crc32);
extern int ZEXPORT zipCloseFileInZip(zipFile file);
extern int ZEXPORT zipCloseFileInZip64(zipFile file);

extern int ZEXPORT zipClose(zipFile file, const char *global_comment);
extern int ZEXPORT zipClose_64(zipFile file, const char *global_comment);
extern int ZEXPORT zipClose2_64(zipFile file, const char *global_comment, uint16_t version_madeby);

/***************************************************************************/

#if defined(STRICTUNZIP) || defined(STRICTZIPUNZIP)
/* like the STRICT of WIN32, we define a pointer that cannot be converted
    from (void*) without cast */
typedef struct TagunzFile__ { int unused; } unz_file__;
typedef unz_file__ *unzFile;
#else
typedef voidp unzFile;
#endif

/***************************************************************************/

#define UNZ_OK                          (0)
#define UNZ_END_OF_LIST_OF_FILE         (-100)
#define UNZ_ERRNO                       (Z_ERRNO)
#define UNZ_EOF                         (0)
#define UNZ_PARAMERROR                  (-102)
#define UNZ_BADZIPFILE                  (-103)
#define UNZ_INTERNALERROR               (-104)
#define UNZ_CRCERROR                    (-105)
#define UNZ_BADPASSWORD                 (-106)

/***************************************************************************/

typedef struct unz_global_info64_s
{
    uint64_t number_entry;          // total number of entries in the central dir on this disk 
    uint32_t number_disk_with_CD;   // number the the disk with central dir, used for spanning ZIP
    uint16_t size_comment;          // size of the global comment of the zipfile 
} unz_global_info64;

typedef struct unz_global_info_s
{
    uint32_t number_entry;          // total number of entries in the central dir on this disk 
    uint32_t number_disk_with_CD;   // number the the disk with central dir, used for spanning ZIP
    uint16_t size_comment;          // size of the global comment of the zipfile 
} unz_global_info;

typedef struct unz_file_info64_s
{
    uint16_t version;               // version made by                 2 bytes 
    uint16_t version_needed;        // version needed to extract       2 bytes 
    uint16_t flag;                  // general purpose bit flag        2 bytes 
    uint16_t compression_method;    // compression method              2 bytes 
    uint32_t dos_date;              // last mod file date in Dos fmt   4 bytes 
    uint32_t crc;                   // crc-32                          4 bytes 
    uint64_t compressed_size;       // compressed size                 8 bytes 
    uint64_t uncompressed_size;     // uncompressed size               8 bytes 
    uint16_t size_filename;         // filename length                 2 bytes 
    uint16_t size_file_extra;       // extra field length              2 bytes 
    uint16_t size_file_comment;     // file comment length             2 bytes 

    uint32_t disk_num_start;        // disk number start               4 bytes 
    uint16_t internal_fa;           // internal file attributes        2 bytes 
    uint32_t external_fa;           // external file attributes        4 bytes 

    uint64_t disk_offset;

    uint16_t size_file_extra_internal;
} unz_file_info64;

typedef struct unz_file_info_s
{
    uint16_t version;               // version made by                 2 bytes 
    uint16_t version_needed;        // version needed to extract       2 bytes 
    uint16_t flag;                  // general purpose bit flag        2 bytes 
    uint16_t compression_method;    // compression method              2 bytes 
    uint32_t dos_date;              // last mod file date in Dos fmt   4 bytes 
    uint32_t crc;                   // crc-32                          4 bytes 
    uint32_t compressed_size;       // compressed size                 4 bytes 
    uint32_t uncompressed_size;     // uncompressed size               4 bytes 
    uint16_t size_filename;         // filename length                 2 bytes 
    uint16_t size_file_extra;       // extra field length              2 bytes 
    uint16_t size_file_comment;     // file comment length             2 bytes 

    uint16_t disk_num_start;        // disk number start               2 bytes 
    uint16_t internal_fa;           // internal file attributes        2 bytes 
    uint32_t external_fa;           // external file attributes        4 bytes

    uint64_t disk_offset;
} unz_file_info;

/***************************************************************************/

typedef int (*unzFileNameComparer)(unzFile file, const char *filename1, const char *filename2);
typedef int (*unzIteratorFunction)(unzFile file);
typedef int (*unzIteratorFunction2)(unzFile file, unz_file_info64 *pfile_info, char *filename,
    uint16_t filename_size, void *extrafield, uint16_t extrafield_size, char *comment, uint16_t comment_size);

/***************************************************************************/

extern unzFile ZEXPORT unzOpen(const char *path);
extern unzFile ZEXPORT unzOpen64(const void *path);
extern unzFile ZEXPORT unzOpen2(const char *path, zlib_filefunc_def *pzlib_filefunc_def);
extern unzFile ZEXPORT unzOpen2_64(const void *path, zlib_filefunc64_def *pzlib_filefunc_def);

extern int ZEXPORT unzClose(unzFile file);
extern int ZEXPORT unzGetGlobalInfo(unzFile file, unz_global_info* pglobal_info32);
extern int ZEXPORT unzGetGlobalInfo64(unzFile file, unz_global_info64 *pglobal_info);
extern int ZEXPORT unzGetGlobalComment(unzFile file, char *comment, uint16_t comment_size);
extern int ZEXPORT unzOpenCurrentFile(unzFile file);
extern int ZEXPORT unzOpenCurrentFilePassword(unzFile file, const char *password);
extern int ZEXPORT unzOpenCurrentFile2(unzFile file, int *method, int *level, int raw);
extern int ZEXPORT unzOpenCurrentFile3(unzFile file, int *method, int *level, int raw, const char *password);
extern int ZEXPORT unzReadCurrentFile(unzFile file, voidp buf, uint32_t len);
extern int ZEXPORT unzCloseCurrentFile(unzFile file);

extern int ZEXPORT unzGetCurrentFileInfo(unzFile file, unz_file_info *pfile_info, char *filename,
    uint16_t filename_size, void *extrafield, uint16_t extrafield_size, char *comment, uint16_t comment_size);
extern int ZEXPORT unzGetCurrentFileInfo64(unzFile file, unz_file_info64 * pfile_info, char *filename,
    uint16_t filename_size, void *extrafield, uint16_t extrafield_size, char *comment, uint16_t comment_size);
extern int ZEXPORT unzGoToFirstFile(unzFile file);
extern int ZEXPORT unzGoToNextFile(unzFile file);
extern int ZEXPORT unzLocateFile(unzFile file, const char *filename, unzFileNameComparer filename_compare_func);

/***************************************************************************/

void fill_fopen_filefunc(zlib_filefunc_def *pzlib_filefunc_def);
void fill_fopen64_filefunc(zlib_filefunc64_def *pzlib_filefunc_def);
void fill_win32_filefunc(zlib_filefunc_def *pzlib_filefunc_def);
void fill_win32_filefunc64(zlib_filefunc64_def *pzlib_filefunc_def);
void fill_win32_filefunc64A(zlib_filefunc64_def *pzlib_filefunc_def);
void fill_win32_filefunc64W(zlib_filefunc64_def *pzlib_filefunc_def);

/***************************************************************************/

#define check_file_exists       mz_os_file_exists
#define dosdate_to_tm           mz_zip_dosdate_to_tm
#define change_file_date        mz_os_set_file_date
#define get_file_date           mz_os_get_file_date
#define is_large_file(x)        (mz_os_get_file_size(x) >= UINT32_MAX)
#define makedir                 mz_make_dir
#define get_file_crc(p,b,bs,rc) mz_get_file_crc(p,rc)

#define MKDIR                   mz_os_make_dir
#define CHDIR                   chdir

/***************************************************************************/

#ifdef __cplusplus
}
#endif

#endif
