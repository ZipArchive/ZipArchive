/* mz_compat.c -- Backwards compatible interface for older versions
   Version 2.2.7, January 30th, 2018
   part of the MiniZip project

   Copyright (C) 2010-2018 Nathan Moinvaziri
     https://github.com/nmoinvaz/minizip
   Copyright (C) 1998-2010 Gilles Vollant
     http://www.winimage.com/zLibDll/minizip.html

   This program is distributed under the terms of the same license as zlib.
   See the accompanying LICENSE file for the full text of the license.
*/

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "mz.h"
#include "mz_os.h"
#include "mz_strm.h"
#include "mz_strm_zlib.h"
#include "mz_zip.h"

#include "mz_compat.h"

/***************************************************************************/

typedef struct mz_compat_s {
    void *stream;
    void *handle;
} mz_compat;

/***************************************************************************/

extern zipFile ZEXPORT zipOpen(const char *path, int append)
{
    zlib_filefunc64_def pzlib = mz_stream_os_get_interface();
    return zipOpen2(path, append, NULL, &pzlib);
}

extern zipFile ZEXPORT zipOpen64(const void *path, int append)
{
    zlib_filefunc64_def pzlib = mz_stream_os_get_interface();
    return zipOpen2(path, append, NULL, &pzlib);
}

extern zipFile ZEXPORT zipOpen2(const char *path, int append, const char **globalcomment,
    zlib_filefunc_def *pzlib_filefunc_def)
{
    return zipOpen2_64(path, append, globalcomment, pzlib_filefunc_def);
}

extern zipFile ZEXPORT zipOpen2_64(const void *path, int append, const char **globalcomment, 
    zlib_filefunc64_def *pzlib_filefunc_def)
{
    mz_compat *compat = NULL;
    int32_t mode = MZ_OPEN_MODE_READWRITE;
    int32_t open_existing = 0;
    void *handle = NULL;
    void *stream = NULL;

    if (mz_stream_create(&stream, (mz_stream_vtbl *)*pzlib_filefunc_def) == NULL)
        return NULL;

    switch (append)
    {
    case APPEND_STATUS_CREATE:
        mode |= MZ_OPEN_MODE_CREATE;
        break;
    case APPEND_STATUS_CREATEAFTER:
        mode |= MZ_OPEN_MODE_CREATE | MZ_OPEN_MODE_APPEND;
        break;
    case APPEND_STATUS_ADDINZIP:
        open_existing = 1;
        break;
    }

    if (mz_stream_open(stream, path, mode) != MZ_OK)
    {
        mz_stream_delete(&stream);
        return NULL;
    }

    handle = mz_zip_open(stream, mode);

    if (handle == NULL)
    {
        mz_stream_delete(&stream);
        return NULL;
    }

    if (globalcomment != NULL)
        mz_zip_get_comment(handle, globalcomment);

    compat = (mz_compat *)malloc(sizeof(mz_compat));
    compat->handle = handle;
    compat->stream = stream;

    return (zipFile)compat;
}

extern int ZEXPORT zipOpenNewFileInZip5(zipFile file, const char *filename, const zip_fileinfo *zipfi,
    const void *extrafield_local, uint16_t size_extrafield_local, const void *extrafield_global,
    uint16_t size_extrafield_global, const char *comment, uint16_t compression_method, int level, 
    int raw, int windowBits, int memLevel, int strategy, const char *password, 
    ZIP_UNUSED uint32_t crc_for_crypting,  uint16_t version_madeby, uint16_t flag_base, int zip64)
{
    mz_compat *compat = (mz_compat *)file;
    mz_zip_file file_info = { 0 };

    if (compat == NULL)
        return MZ_PARAM_ERROR;
   
    memset(&file_info, 0, sizeof(file_info));

    if (zipfi != NULL)
    {
        file_info.modified_date = mz_zip_dosdate_to_time_t(zipfi->dos_date);
        file_info.external_fa = zipfi->external_fa;
        file_info.internal_fa = zipfi->internal_fa;
    }

    file_info.compression_method = compression_method;
    file_info.filename = (char *)filename;
    //file_info.extrafield_local = extrafield_local;
    //file_info.extrafield_local_size = size_extrafield_local;
    file_info.extrafield = (uint8_t *)extrafield_global;
    file_info.extrafield_size = size_extrafield_global;
    file_info.version_madeby = version_madeby;
    file_info.comment = (char *)comment;
    file_info.flag = flag_base;
#ifdef HAVE_AES
    if (password)
        file_info.aes_version = MZ_AES_VERSION;
#endif

    if (raw)
        level = 0;

    return mz_zip_entry_write_open(compat->handle, &file_info, (int16_t)level, password);
}

extern int ZEXPORT zipOpenNewFileInZip4_64(zipFile file, const char *filename, const zip_fileinfo *zipfi,
    const void *extrafield_local, uint16_t size_extrafield_local, const void *extrafield_global,
    uint16_t size_extrafield_global, const char *comment, uint16_t compression_method, int level, 
    int raw, int windowBits, int memLevel,   int strategy, const char *password, 
    ZIP_UNUSED uint32_t crc_for_crypting, uint16_t version_madeby, uint16_t flag_base, int zip64)
{
    return zipOpenNewFileInZip5(file, filename, zipfi, extrafield_local, size_extrafield_local, 
        extrafield_global, size_extrafield_global, comment, compression_method, level, raw, windowBits, 
        memLevel, strategy, password, crc_for_crypting, version_madeby, flag_base, zip64);
}

extern int ZEXPORT zipOpenNewFileInZip4(zipFile file, const char *filename, const zip_fileinfo *zipfi,
    const void *extrafield_local, uint16_t size_extrafield_local, const void *extrafield_global,
    uint16_t size_extrafield_global, const char *comment, uint16_t compression_method, int level, 
    int raw, int windowBits, int memLevel, int strategy, const char *password, 
    ZIP_UNUSED uint32_t crc_for_crypting, uint16_t version_madeby, uint16_t flag_base)
{
    return zipOpenNewFileInZip4_64(file, filename, zipfi, extrafield_local, size_extrafield_local,
        extrafield_global, size_extrafield_global, comment, compression_method, level, raw, windowBits, 
        memLevel, strategy, password, crc_for_crypting, version_madeby, flag_base, 0);
}

extern int ZEXPORT zipOpenNewFileInZip3(zipFile file, const char *filename, const zip_fileinfo *zipfi,
    const void *extrafield_local, uint16_t size_extrafield_local, const void *extrafield_global,
    uint16_t size_extrafield_global, const char *comment, uint16_t compression_method, int level, 
    int raw, int windowBits, int memLevel, int strategy, const char *password, 
    ZIP_UNUSED uint32_t crc_for_crypting)
{
    return zipOpenNewFileInZip4_64(file, filename, zipfi, extrafield_local, size_extrafield_local,
        extrafield_global, size_extrafield_global, comment, compression_method, level, raw, windowBits, 
        memLevel, strategy, password, crc_for_crypting, MZ_VERSION_MADEBY, 0, 0);
}

extern int ZEXPORT zipOpenNewFileInZip3_64(zipFile file, const char *filename, const zip_fileinfo *zipfi,
    const void *extrafield_local, uint16_t size_extrafield_local, const void *extrafield_global,
    uint16_t size_extrafield_global, const char *comment, uint16_t compression_method, int level, 
    int raw, int windowBits, int memLevel, int strategy, const char *password, 
    ZIP_UNUSED uint32_t crc_for_crypting, int zip64)
{
    return zipOpenNewFileInZip4_64(file, filename, zipfi, extrafield_local, size_extrafield_local,
        extrafield_global, size_extrafield_global, comment, compression_method, level, raw, windowBits, 
        memLevel, strategy, password, crc_for_crypting, MZ_VERSION_MADEBY, 0, zip64);
}

extern int ZEXPORT zipWriteInFileInZip(zipFile file, const void *buf, uint32_t len)
{
    mz_compat *compat = (mz_compat *)file;
    if (compat == NULL)
        return MZ_PARAM_ERROR;
    return mz_zip_entry_write(compat->handle, buf, len);
}

extern int ZEXPORT zipCloseFileInZipRaw(zipFile file, uint32_t uncompressed_size, uint32_t crc32)
{
    mz_compat *compat = (mz_compat *)file;
    if (compat == NULL)
        return MZ_PARAM_ERROR;
    return mz_zip_entry_close_raw(compat->handle, uncompressed_size, crc32);
}

extern int ZEXPORT zipCloseFileInZip(zipFile file)
{
    return zipCloseFileInZipRaw(file, 0, 0);
}

extern int ZEXPORT zipCloseFileInZip64(zipFile file)
{
    return zipCloseFileInZipRaw(file, 0, 0);
}

extern int ZEXPORT zipClose(zipFile file, const char *global_comment)
{
    return zipClose_64(file, global_comment);
}

extern int ZEXPORT zipClose_64(zipFile file, const char *global_comment)
{
    return zipClose2_64(file, global_comment, MZ_VERSION_MADEBY);
}

extern int ZEXPORT zipClose2_64(zipFile file, const char *global_comment, uint16_t version_madeby)
{
    mz_compat *compat = (mz_compat *)file;
    int32_t err = MZ_OK;

    if (compat == NULL)
        return MZ_PARAM_ERROR;

    if (global_comment != NULL)
        mz_zip_set_comment(compat->handle, global_comment);

    mz_zip_set_version_madeby(compat->handle, version_madeby);
    err = mz_zip_close(compat->handle);

    if (compat->stream != NULL)
    {
        mz_stream_close(compat->stream);
        mz_stream_delete(&compat->stream);
    }

    free(compat);

    return err;
}

/***************************************************************************/

extern unzFile ZEXPORT unzOpen(const char *path)
{
    return unzOpen64(path);
}

extern unzFile ZEXPORT unzOpen64(const void *path)
{
    zlib_filefunc64_def pzlib = mz_stream_os_get_interface();
    return unzOpen2(path, &pzlib);
}

extern unzFile ZEXPORT unzOpen2(const char *path, zlib_filefunc_def *pzlib_filefunc_def)
{
    return unzOpen2_64(path, pzlib_filefunc_def);
}

extern unzFile ZEXPORT unzOpen2_64(const void *path, zlib_filefunc64_def *pzlib_filefunc_def)
{
    mz_compat *compat = NULL;
    int32_t mode = MZ_OPEN_MODE_READ;
    void *handle = NULL;
    void *stream = NULL;

    if (mz_stream_create(&stream, (mz_stream_vtbl *)*pzlib_filefunc_def) == NULL)
        return NULL;
    
    if (mz_stream_open(stream, path, mode) != MZ_OK)
    {
        mz_stream_delete(&stream);
        return NULL;
    }

    handle = mz_zip_open(stream, mode);

    if (handle == NULL)
    {
        mz_stream_delete(&stream);
        return NULL;
    }

    compat = (mz_compat *)malloc(sizeof(mz_compat));
    compat->handle = handle;
    compat->stream = stream;

    mz_zip_goto_first_entry(compat->handle);
    return (unzFile)compat;
}

extern int ZEXPORT unzClose(unzFile file)
{
    mz_compat *compat = (mz_compat *)file;
    int32_t err = MZ_OK;

    if (compat == NULL)
        return MZ_PARAM_ERROR;

    err = mz_zip_close(compat->handle);

    if (compat->stream != NULL)
    {
        mz_stream_close(compat->stream);
        mz_stream_delete(&compat->stream);
    }

    free(compat);

    return err;
}

extern int ZEXPORT unzGetGlobalInfo(unzFile file, unz_global_info* pglobal_info32)
{
    mz_compat *compat = (mz_compat *)file;
    unz_global_info64 global_info64;
    int32_t err = MZ_OK;

    memset(pglobal_info32, 0, sizeof(unz_global_info));
    if (compat == NULL)
        return MZ_PARAM_ERROR;

    err = unzGetGlobalInfo64(file, &global_info64);
    if (err == MZ_OK)
    {
        pglobal_info32->number_entry = (uint32_t)global_info64.number_entry;
        pglobal_info32->size_comment = global_info64.size_comment;
        pglobal_info32->number_disk_with_CD = global_info64.number_disk_with_CD;
    }
    return err;
}

extern int ZEXPORT unzGetGlobalInfo64(unzFile file, unz_global_info64 *pglobal_info)
{
    mz_compat *compat = (mz_compat *)file;
    const char *comment_ptr = NULL;
    int32_t err = MZ_OK;

    memset(pglobal_info, 0, sizeof(unz_global_info64));
    if (compat == NULL)
        return MZ_PARAM_ERROR;
    err = mz_zip_get_comment(compat->handle, &comment_ptr);
    if (err == MZ_OK)
        pglobal_info->size_comment = (uint16_t)strlen(comment_ptr);       
    if ((err == MZ_OK) || (err == MZ_EXIST_ERROR))
        err = mz_zip_get_number_entry(compat->handle, (int64_t *)&pglobal_info->number_entry);
    if (err == MZ_OK)
        err = mz_zip_get_disk_number_with_cd(compat->handle, (int64_t *)&pglobal_info->number_disk_with_CD);
    return err;
}

extern int ZEXPORT unzGetGlobalComment(unzFile file, char *comment, uint16_t comment_size)
{
    mz_compat *compat = (mz_compat *)file;
    const char *comment_ptr = NULL;
    int32_t err = MZ_OK;

    if (comment == NULL || comment_size == 0)
        return MZ_PARAM_ERROR;
    err = mz_zip_get_comment(compat->handle, &comment_ptr);
    if (err == MZ_OK)
        strncpy(comment, comment_ptr, comment_size);
    return err;
}

extern int ZEXPORT unzOpenCurrentFile3(unzFile file, int *method, int *level, int raw, const char *password)
{
    mz_compat *compat = (mz_compat *)file;
    if (compat == NULL)
        return MZ_PARAM_ERROR;
    if (method != NULL)
        *method = 0;
    if (level != NULL)
        *level = 0;
    return mz_zip_entry_read_open(compat->handle, (int16_t)raw, password);
}

extern int ZEXPORT unzOpenCurrentFile(unzFile file)
{
    return unzOpenCurrentFile3(file, NULL, NULL, 0, NULL);
}

extern int ZEXPORT unzOpenCurrentFilePassword(unzFile file, const char *password)
{
    return unzOpenCurrentFile3(file, NULL, NULL, 0, password);
}

extern int ZEXPORT unzOpenCurrentFile2(unzFile file, int *method, int *level, int raw)
{
    return unzOpenCurrentFile3(file, method, level, raw, NULL);
}

extern int ZEXPORT unzReadCurrentFile(unzFile file, voidp buf, uint32_t len)
{
    mz_compat *compat = (mz_compat *)file;
    if (compat == NULL)
        return MZ_PARAM_ERROR;
    return mz_zip_entry_read(compat->handle, buf, len);
}

extern int ZEXPORT unzCloseCurrentFile(unzFile file)
{
    mz_compat *compat = (mz_compat *)file;
    if (compat == NULL)
        return MZ_PARAM_ERROR;
    return mz_zip_entry_close(compat->handle);
}

extern int ZEXPORT unzGetCurrentFileInfo(unzFile file, unz_file_info *pfile_info, char *filename,
    uint16_t filename_size, void *extrafield, uint16_t extrafield_size, char *comment, uint16_t comment_size)
{
    mz_compat *compat = (mz_compat *)file;
    mz_zip_file *file_info = NULL;
    int16_t bytes_to_copy = 0;
    int32_t err = MZ_OK;

    if (compat == NULL)
        return MZ_PARAM_ERROR;
    err = mz_zip_entry_get_info(compat->handle, &file_info);

    if ((err == MZ_OK) && (pfile_info != NULL))
    {
        pfile_info->version = file_info->version_madeby;
        pfile_info->version_needed = file_info->version_needed;
        pfile_info->flag = file_info->flag;
        pfile_info->compression_method = file_info->compression_method;
        pfile_info->dos_date = mz_zip_time_t_to_dos_date(file_info->modified_date);
        pfile_info->crc = file_info->crc;

        pfile_info->size_filename = file_info->filename_size;
        pfile_info->size_file_extra = file_info->extrafield_size;
        pfile_info->size_file_comment = file_info->comment_size;

        pfile_info->disk_num_start = (uint16_t)file_info->disk_number;
        pfile_info->internal_fa = file_info->internal_fa;
        pfile_info->external_fa = file_info->external_fa;

        pfile_info->compressed_size = (uint32_t)file_info->compressed_size;
        pfile_info->uncompressed_size = (uint32_t)file_info->uncompressed_size;

        if (filename_size > 0 && filename != NULL)
        {
            bytes_to_copy = filename_size;
            if (bytes_to_copy > file_info->filename_size)
                bytes_to_copy = file_info->filename_size;
            memcpy(filename, file_info->filename, bytes_to_copy);
        }
        if (extrafield_size > 0 && extrafield != NULL)
        {
            bytes_to_copy = extrafield_size;
            if (bytes_to_copy > file_info->extrafield_size)
                bytes_to_copy = file_info->extrafield_size;
            memcpy(extrafield, file_info->extrafield, bytes_to_copy);
        }
        if (comment_size > 0 && comment != NULL)
        {
            bytes_to_copy = comment_size;
            if (bytes_to_copy > file_info->comment_size)
                bytes_to_copy = file_info->comment_size;
            memcpy(comment, file_info->comment, bytes_to_copy);
        }
    }
    return err;
}

extern int ZEXPORT unzGetCurrentFileInfo64(unzFile file, unz_file_info64 * pfile_info, char *filename,
    uint16_t filename_size, void *extrafield, uint16_t extrafield_size, char *comment, uint16_t comment_size)
{
    mz_compat *compat = (mz_compat *)file;
    mz_zip_file *file_info = NULL;
    int16_t bytes_to_copy = 0;
    int32_t err = MZ_OK;

    if (compat == NULL)
        return MZ_PARAM_ERROR;

    err = mz_zip_entry_get_info(compat->handle, &file_info);

    if ((err == MZ_OK) && (pfile_info != NULL))
    {
        pfile_info->version = file_info->version_madeby;
        pfile_info->version_needed = file_info->version_needed;
        pfile_info->flag = file_info->flag;
        pfile_info->compression_method = file_info->compression_method;
        pfile_info->dos_date = mz_zip_time_t_to_dos_date(file_info->modified_date);
        pfile_info->crc = file_info->crc;

        pfile_info->size_filename = file_info->filename_size;
        pfile_info->size_file_extra = file_info->extrafield_size;
        pfile_info->size_file_comment = file_info->comment_size;

        pfile_info->disk_num_start = file_info->disk_number;
        pfile_info->internal_fa = file_info->internal_fa;
        pfile_info->external_fa = file_info->external_fa;

        pfile_info->compressed_size = file_info->compressed_size;
        pfile_info->uncompressed_size = file_info->uncompressed_size;

        if (filename_size > 0 && filename != NULL)
        {
            bytes_to_copy = filename_size;
            if (bytes_to_copy > file_info->filename_size)
                bytes_to_copy = file_info->filename_size;
            memcpy(filename, file_info->filename, bytes_to_copy);
            if (file_info->filename_size < filename_size)
                filename[file_info->filename_size] = 0;
        }

        if (extrafield_size > 0 && extrafield != NULL)
        {
            bytes_to_copy = extrafield_size;
            if (bytes_to_copy > file_info->extrafield_size)
                bytes_to_copy = file_info->extrafield_size;
            memcpy(extrafield, file_info->extrafield, bytes_to_copy);
        }

        if (comment_size > 0 && comment != NULL)
        {
            bytes_to_copy = comment_size;
            if (bytes_to_copy > file_info->comment_size)
                bytes_to_copy = file_info->comment_size;
            memcpy(comment, file_info->comment, bytes_to_copy);
            if (file_info->comment_size < comment_size)
                comment[file_info->comment_size] = 0;
        }
    }
    return err;
}

extern int ZEXPORT unzGoToFirstFile(unzFile file)
{
    mz_compat *compat = (mz_compat *)file;
    if (compat == NULL)
        return MZ_PARAM_ERROR;
    return mz_zip_goto_first_entry(compat->handle);
}

extern int ZEXPORT unzGoToNextFile(unzFile file)
{
    mz_compat *compat = (mz_compat *)file;
    if (compat == NULL)
        return MZ_PARAM_ERROR;
    return mz_zip_goto_next_entry(compat->handle);
}

extern int ZEXPORT unzLocateFile(unzFile file, const char *filename, unzFileNameComparer filename_compare_func)
{
    mz_compat *compat = (mz_compat *)file;
    if (compat == NULL)
        return MZ_PARAM_ERROR;
    return mz_zip_locate_entry(compat->handle, filename, filename_compare_func);
}

/***************************************************************************/

void fill_fopen_filefunc(zlib_filefunc_def *pzlib_filefunc_def)
{
    if (pzlib_filefunc_def != NULL)
        *pzlib_filefunc_def = mz_stream_os_get_interface();
}

void fill_fopen64_filefunc(zlib_filefunc64_def *pzlib_filefunc_def)
{
    if (pzlib_filefunc_def != NULL)
        *pzlib_filefunc_def = mz_stream_os_get_interface();
}

void fill_win32_filefunc(zlib_filefunc_def *pzlib_filefunc_def)
{
    if (pzlib_filefunc_def != NULL)
        *pzlib_filefunc_def = mz_stream_os_get_interface();
}

void fill_win32_filefunc64(zlib_filefunc64_def *pzlib_filefunc_def)
{
    if (pzlib_filefunc_def != NULL)
        *pzlib_filefunc_def = mz_stream_os_get_interface();
}

void fill_win32_filefunc64A(zlib_filefunc64_def *pzlib_filefunc_def)
{
    if (pzlib_filefunc_def != NULL)
        *pzlib_filefunc_def = mz_stream_os_get_interface();
}

void fill_win32_filefunc64W(zlib_filefunc64_def *pzlib_filefunc_def)
{
    // NOTE: You should no longer pass in widechar string to open function
    if (pzlib_filefunc_def != NULL)
        *pzlib_filefunc_def = mz_stream_os_get_interface();
}

