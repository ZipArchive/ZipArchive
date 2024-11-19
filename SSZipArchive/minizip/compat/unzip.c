/* zip.c -- Backwards compatible unzip interface
   part of the minizip-ng project

   Copyright (C) Nathan Moinvaziri
     https://github.com/zlib-ng/minizip-ng
   Copyright (C) 1998-2010 Gilles Vollant
     https://www.winimage.com/zLibDll/minizip.html

   This program is distributed under the terms of the same license as zlib.
   See the accompanying LICENSE file for the full text of the license.

 WARNING: Be very careful updating/overwriting this file.
 It has specific changes for ZipArchive support, notably removing tmu_date.
*/

#include "mz.h"
#include "mz_os.h"
#include "mz_strm.h"
#include "mz_strm_os.h"
#include "mz_zip.h"

#include <stdio.h> /* SEEK */

#include "unzip.h"

/***************************************************************************/

typedef struct mz_unzip_compat_s {
    void *stream;
    void *handle;
    uint64_t entry_index;
    int64_t entry_pos;
    int64_t total_out;
} mz_unzip_compat;

/***************************************************************************/

unzFile unzOpen(const char *path) {
    return unzOpen64(path);
}

unzFile unzOpen64(const void *path) {
    return unzOpen2(path, NULL);
}

unzFile unzOpen2(const char *path, zlib_filefunc_def *pzlib_filefunc_def) {
    unzFile unz = NULL;
    void *stream = NULL;

    if (pzlib_filefunc_def) {
        if (pzlib_filefunc_def->zopen_file) {
            stream = mz_stream_ioapi_create();
            if (!stream)
                return NULL;
            mz_stream_ioapi_set_filefunc(stream, pzlib_filefunc_def);
        } else if (pzlib_filefunc_def->opaque) {
            stream = mz_stream_create((mz_stream_vtbl *)pzlib_filefunc_def->opaque);
            if (!stream)
                return NULL;
        }
    }

    if (!stream) {
        stream = mz_stream_os_create();
        if (!stream)
            return NULL;
    }

    if (mz_stream_open(stream, path, MZ_OPEN_MODE_READ) != MZ_OK) {
        mz_stream_delete(&stream);
        return NULL;
    }

    unz = unzOpen_MZ(stream);
    if (!unz) {
        mz_stream_close(stream);
        mz_stream_delete(&stream);
        return NULL;
    }
    return unz;
}

unzFile unzOpen2_64(const void *path, zlib_filefunc64_def *pzlib_filefunc_def) {
    unzFile unz = NULL;
    void *stream = NULL;

    if (pzlib_filefunc_def) {
        if (pzlib_filefunc_def->zopen64_file) {
            stream = mz_stream_ioapi_create();
            if (!stream)
                return NULL;
            mz_stream_ioapi_set_filefunc64(stream, pzlib_filefunc_def);
        } else if (pzlib_filefunc_def->opaque) {
            stream = mz_stream_create((mz_stream_vtbl *)pzlib_filefunc_def->opaque);
            if (!stream)
                return NULL;
        }
    }

    if (!stream) {
        stream = mz_stream_os_create();
        if (!stream)
            return NULL;
    }

    if (mz_stream_open(stream, path, MZ_OPEN_MODE_READ) != MZ_OK) {
        mz_stream_delete(&stream);
        return NULL;
    }

    unz = unzOpen_MZ(stream);
    if (!unz) {
        mz_stream_close(stream);
        mz_stream_delete(&stream);
        return NULL;
    }
    return unz;
}

void *unzGetHandle_MZ(unzFile file) {
    mz_unzip_compat *compat = (mz_unzip_compat *)file;
    if (!compat)
        return NULL;
    return compat->handle;
}

void *unzGetStream_MZ(unzFile file) {
    mz_unzip_compat *compat = (mz_unzip_compat *)file;
    if (!compat)
        return NULL;
    return compat->stream;
}

unzFile unzOpen_MZ(void *stream) {
    mz_unzip_compat *compat = NULL;
    int32_t err = MZ_OK;
    void *handle = NULL;

    handle = mz_zip_create();
    if (!handle)
        return NULL;

    mz_zip_set_recover(handle, 1);

    err = mz_zip_open(handle, stream, MZ_OPEN_MODE_READ);
    if (err != MZ_OK) {
        mz_zip_delete(&handle);
        return NULL;
    }

    compat = (mz_unzip_compat *)calloc(1, sizeof(mz_unzip_compat));
    if (compat) {
        compat->handle = handle;
        compat->stream = stream;

        mz_zip_goto_first_entry(compat->handle);
    } else {
        mz_zip_delete(&handle);
    }

    return (unzFile)compat;
}

int unzClose(unzFile file) {
    mz_unzip_compat *compat = (mz_unzip_compat *)file;
    int32_t err = MZ_OK;

    if (!compat)
        return UNZ_PARAMERROR;

    if (compat->handle)
        err = unzClose_MZ(file);

    if (compat->stream) {
        mz_stream_close(compat->stream);
        mz_stream_delete(&compat->stream);
    }

    free(compat);

    return err;
}

/* Only closes the zip handle, does not close the stream */
int unzClose_MZ(unzFile file) {
    mz_unzip_compat *compat = (mz_unzip_compat *)file;
    int32_t err = MZ_OK;

    if (!compat)
        return UNZ_PARAMERROR;

    err = mz_zip_close(compat->handle);
    mz_zip_delete(&compat->handle);

    return err;
}

int unzGetGlobalInfo(unzFile file, unz_global_info *pglobal_info32) {
    mz_unzip_compat *compat = (mz_unzip_compat *)file;
    unz_global_info64 global_info64;
    int32_t err = MZ_OK;

    memset(pglobal_info32, 0, sizeof(unz_global_info));
    if (!compat)
        return UNZ_PARAMERROR;

    err = unzGetGlobalInfo64(file, &global_info64);
    if (err == MZ_OK) {
        pglobal_info32->number_entry = (uint32_t)global_info64.number_entry;
        pglobal_info32->size_comment = global_info64.size_comment;
        pglobal_info32->number_disk_with_CD = global_info64.number_disk_with_CD;
    }
    return err;
}

int unzGetGlobalInfo64(unzFile file, unz_global_info64 *pglobal_info) {
    mz_unzip_compat *compat = (mz_unzip_compat *)file;
    const char *comment_ptr = NULL;
    int32_t err = MZ_OK;

    memset(pglobal_info, 0, sizeof(unz_global_info64));
    if (!compat)
        return UNZ_PARAMERROR;
    err = mz_zip_get_comment(compat->handle, &comment_ptr);
    if (err == MZ_OK)
        pglobal_info->size_comment = (uint16_t)strlen(comment_ptr);
    if ((err == MZ_OK) || (err == MZ_EXIST_ERROR))
        err = mz_zip_get_number_entry(compat->handle, &pglobal_info->number_entry);
    if (err == MZ_OK)
        err = mz_zip_get_disk_number_with_cd(compat->handle, &pglobal_info->number_disk_with_CD);
    return err;
}

int unzGetGlobalComment(unzFile file, char *comment, unsigned long comment_size) {
    mz_unzip_compat *compat = (mz_unzip_compat *)file;
    const char *comment_ptr = NULL;
    int32_t err = MZ_OK;

    if (!comment || !comment_size)
        return UNZ_PARAMERROR;
    err = mz_zip_get_comment(compat->handle, &comment_ptr);
    if (err == MZ_OK) {
        strncpy(comment, comment_ptr, comment_size - 1);
        comment[comment_size - 1] = 0;
    }
    return err;
}

int unzOpenCurrentFile3(unzFile file, int *method, int *level, int raw, const char *password) {
    mz_unzip_compat *compat = (mz_unzip_compat *)file;
    mz_zip_file *file_info = NULL;
    int32_t err = MZ_OK;
    void *stream = NULL;

    if (!compat)
        return UNZ_PARAMERROR;
    if (method)
        *method = 0;
    if (level)
        *level = 0;

    if (mz_zip_entry_is_open(compat->handle) == MZ_OK) {
        /* zlib minizip does not error out here if close returns errors */
        unzCloseCurrentFile(file);
    }

    compat->total_out = 0;
    err = mz_zip_entry_read_open(compat->handle, (uint8_t)raw, password);
    if (err == MZ_OK)
        err = mz_zip_entry_get_info(compat->handle, &file_info);
    if (err == MZ_OK) {
        if (method) {
            *method = file_info->compression_method;
        }

        if (level) {
            *level = 6;
            switch (file_info->flag & 0x06) {
            case MZ_ZIP_FLAG_DEFLATE_SUPER_FAST:
                *level = 1;
                break;
            case MZ_ZIP_FLAG_DEFLATE_FAST:
                *level = 2;
                break;
            case MZ_ZIP_FLAG_DEFLATE_MAX:
                *level = 9;
                break;
            }
        }
    }
    if (err == MZ_OK)
        err = mz_zip_get_stream(compat->handle, &stream);
    if (err == MZ_OK)
        compat->entry_pos = mz_stream_tell(stream);
    return err;
}

int unzOpenCurrentFile(unzFile file) {
    return unzOpenCurrentFile3(file, NULL, NULL, 0, NULL);
}

int unzOpenCurrentFilePassword(unzFile file, const char *password) {
    return unzOpenCurrentFile3(file, NULL, NULL, 0, password);
}

int unzOpenCurrentFile2(unzFile file, int *method, int *level, int raw) {
    return unzOpenCurrentFile3(file, method, level, raw, NULL);
}

int unzReadCurrentFile(unzFile file, void *buf, uint32_t len) {
    mz_unzip_compat *compat = (mz_unzip_compat *)file;
    int32_t err = MZ_OK;
    if (!compat || len >= INT32_MAX)
        return UNZ_PARAMERROR;
    err = mz_zip_entry_read(compat->handle, buf, (int32_t)len);
    if (err > 0)
        compat->total_out += (uint32_t)err;
    return err;
}

int unzCloseCurrentFile(unzFile file) {
    mz_unzip_compat *compat = (mz_unzip_compat *)file;
    int32_t err = MZ_OK;
    if (!compat)
        return UNZ_PARAMERROR;
    err = mz_zip_entry_close(compat->handle);
    return err;
}

// [ZipArchive] disabled for performances: we don't need the tmu_date field because we already set the mz_dos_date field:
// https://github.com/madler/zlib/blob/643e17b7498d12ab8d15565662880579692f769d/contrib/minizip/zip.h#L102
//static void unzConvertTimeToUnzTime(time_t time, tm_unz *tmu_date) {
//    struct tm tm_date;
//    memset(&tm_date, 0, sizeof(struct tm));
//    mz_zip_time_t_to_tm(time, &tm_date);
//    memcpy(tmu_date, &tm_date, sizeof(tm_unz));
//    tmu_date->tm_year += 1900;
//}

int unzGetCurrentFileInfo(unzFile file, unz_file_info *pfile_info, char *filename, unsigned long filename_size,
                          void *extrafield, unsigned long extrafield_size, char *comment, unsigned long comment_size) {
    mz_unzip_compat *compat = (mz_unzip_compat *)file;
    mz_zip_file *file_info = NULL;
    uint16_t bytes_to_copy = 0;
    int32_t err = MZ_OK;

    if (!compat)
        return UNZ_PARAMERROR;

    err = mz_zip_entry_get_info(compat->handle, &file_info);
    if (err != MZ_OK)
        return err;

    if (pfile_info) {
        pfile_info->version = file_info->version_madeby;
        pfile_info->version_needed = file_info->version_needed;
        pfile_info->flag = file_info->flag;
        pfile_info->compression_method = file_info->compression_method;
        pfile_info->mz_dos_date = mz_zip_time_t_to_dos_date(file_info->modified_date);
        // [ZipArchive] disabled for performances: we don't need the tmu_date field because we already set the mz_dos_date field
        // unzConvertTimeToUnzTime(file_info->modified_date, &pfile_info->tmu_date);
        pfile_info->crc = file_info->crc;

        pfile_info->size_filename = file_info->filename_size;
        pfile_info->size_file_extra = file_info->extrafield_size;
        pfile_info->size_file_comment = file_info->comment_size;

        pfile_info->disk_num_start = (uint16_t)file_info->disk_number;
        pfile_info->internal_fa = file_info->internal_fa;
        pfile_info->external_fa = file_info->external_fa;

        pfile_info->compressed_size = (uint32_t)file_info->compressed_size;
        pfile_info->uncompressed_size = (uint32_t)file_info->uncompressed_size;
    }
    if (filename_size > 0 && filename && file_info->filename) {
        bytes_to_copy = (uint16_t)filename_size;
        if (bytes_to_copy > file_info->filename_size)
            bytes_to_copy = file_info->filename_size;
        memcpy(filename, file_info->filename, bytes_to_copy);
        if (bytes_to_copy < filename_size)
            filename[bytes_to_copy] = 0;
    }
    if (extrafield_size > 0 && extrafield) {
        bytes_to_copy = (uint16_t)extrafield_size;
        if (bytes_to_copy > file_info->extrafield_size)
            bytes_to_copy = file_info->extrafield_size;
        memcpy(extrafield, file_info->extrafield, bytes_to_copy);
    }
    if (comment_size > 0 && comment && file_info->comment) {
        bytes_to_copy = (uint16_t)comment_size;
        if (bytes_to_copy > file_info->comment_size)
            bytes_to_copy = file_info->comment_size;
        memcpy(comment, file_info->comment, bytes_to_copy);
        if (bytes_to_copy < comment_size)
            comment[bytes_to_copy] = 0;
    }
    return err;
}

int unzGetCurrentFileInfo64(unzFile file, unz_file_info64 *pfile_info, char *filename, unsigned long filename_size,
                            void *extrafield, unsigned long extrafield_size, char *comment,
                            unsigned long comment_size) {
    mz_unzip_compat *compat = (mz_unzip_compat *)file;
    mz_zip_file *file_info = NULL;
    uint16_t bytes_to_copy = 0;
    int32_t err = MZ_OK;

    if (!compat)
        return UNZ_PARAMERROR;

    err = mz_zip_entry_get_info(compat->handle, &file_info);
    if (err != MZ_OK)
        return err;

    if (pfile_info) {
        pfile_info->version = file_info->version_madeby;
        pfile_info->version_needed = file_info->version_needed;
        pfile_info->flag = file_info->flag;
        pfile_info->compression_method = file_info->compression_method;
        pfile_info->mz_dos_date = mz_zip_time_t_to_dos_date(file_info->modified_date);
        // [ZipArchive] disabled for performances: we don't need the tmu_date field because we already set the mz_dos_date field
        // unzConvertTimeToUnzTime(file_info->modified_date, &pfile_info->tmu_date);
        pfile_info->crc = file_info->crc;

        pfile_info->size_filename = file_info->filename_size;
        pfile_info->size_file_extra = file_info->extrafield_size;
        pfile_info->size_file_comment = file_info->comment_size;

        pfile_info->disk_num_start = file_info->disk_number;
        pfile_info->internal_fa = file_info->internal_fa;
        pfile_info->external_fa = file_info->external_fa;

        pfile_info->compressed_size = (uint64_t)file_info->compressed_size;
        pfile_info->uncompressed_size = (uint64_t)file_info->uncompressed_size;
    }
    if (filename_size > 0 && filename && file_info->filename) {
        bytes_to_copy = (uint16_t)filename_size;
        if (bytes_to_copy > file_info->filename_size)
            bytes_to_copy = file_info->filename_size;
        memcpy(filename, file_info->filename, bytes_to_copy);
        if (bytes_to_copy < filename_size)
            filename[bytes_to_copy] = 0;
    }
    if (extrafield_size > 0 && extrafield) {
        bytes_to_copy = (uint16_t)extrafield_size;
        if (bytes_to_copy > file_info->extrafield_size)
            bytes_to_copy = file_info->extrafield_size;
        memcpy(extrafield, file_info->extrafield, bytes_to_copy);
    }
    if (comment_size > 0 && comment && file_info->comment) {
        bytes_to_copy = (uint16_t)comment_size;
        if (bytes_to_copy > file_info->comment_size)
            bytes_to_copy = file_info->comment_size;
        memcpy(comment, file_info->comment, bytes_to_copy);
        if (bytes_to_copy < comment_size)
            comment[bytes_to_copy] = 0;
    }
    return err;
}

int unzGoToFirstFile(unzFile file) {
    mz_unzip_compat *compat = (mz_unzip_compat *)file;
    if (!compat)
        return UNZ_PARAMERROR;
    compat->entry_index = 0;
    return mz_zip_goto_first_entry(compat->handle);
}

int unzGoToNextFile(unzFile file) {
    mz_unzip_compat *compat = (mz_unzip_compat *)file;
    int32_t err = MZ_OK;
    if (!compat)
        return UNZ_PARAMERROR;
    err = mz_zip_goto_next_entry(compat->handle);
    if (err != MZ_END_OF_LIST)
        compat->entry_index += 1;
    return err;
}

#if !defined(MZ_COMPAT_VERSION) || MZ_COMPAT_VERSION < 110
#  ifdef WIN32
#    define UNZ_DEFAULT_IGNORE_CASE 1
#  else
#    define UNZ_DEFAULT_IGNORE_CASE 0
#  endif

int unzLocateFile(unzFile file, const char *filename, unzFileNameCase filename_case) {
    mz_unzip_compat *compat = (mz_unzip_compat *)file;
    mz_zip_file *file_info = NULL;
    uint64_t preserve_index = 0;
    int32_t err = MZ_OK;
    int32_t result = 0;
    uint8_t ignore_case = UNZ_DEFAULT_IGNORE_CASE;

    if (!compat)
        return UNZ_PARAMERROR;

    if (filename_case == 1) {
        ignore_case = 0;
    } else if (filename_case > 1) {
        ignore_case = 1;
    }

    preserve_index = compat->entry_index;

    err = mz_zip_goto_first_entry(compat->handle);
    while (err == MZ_OK) {
        err = mz_zip_entry_get_info(compat->handle, &file_info);
        if (err != MZ_OK)
            break;

        result = mz_path_compare_wc(filename, file_info->filename, !ignore_case);

        if (result == 0)
            return MZ_OK;

        err = mz_zip_goto_next_entry(compat->handle);
    }

    compat->entry_index = preserve_index;
    return err;
}
#else
int unzLocateFile(unzFile file, const char *filename, unzFileNameComparer filename_compare_func) {
    mz_unzip_compat *compat = (mz_unzip_compat *)file;
    mz_zip_file *file_info = NULL;
    uint64_t preserve_index = 0;
    int32_t err = MZ_OK;
    int32_t result = 0;

    if (!compat)
        return UNZ_PARAMERROR;

    preserve_index = compat->entry_index;

    err = mz_zip_goto_first_entry(compat->handle);
    while (err == MZ_OK) {
        err = mz_zip_entry_get_info(compat->handle, &file_info);
        if (err != MZ_OK)
            break;

        if ((intptr_t)filename_compare_func > 2) {
            result = filename_compare_func(file, filename, file_info->filename);
        } else {
            int32_t case_sensitive = (int32_t)(intptr_t)filename_compare_func;
            result = mz_path_compare_wc(filename, file_info->filename, !case_sensitive);
        }

        if (result == 0)
            return MZ_OK;

        err = mz_zip_goto_next_entry(compat->handle);
    }

    compat->entry_index = preserve_index;
    return err;
}
#endif

/***************************************************************************/

int unzGetFilePos(unzFile file, unz_file_pos *file_pos) {
    unz64_file_pos file_pos64;
    int32_t err = 0;

    err = unzGetFilePos64(file, &file_pos64);
    if (err < 0)
        return err;

    file_pos->pos_in_zip_directory = (uint32_t)file_pos64.pos_in_zip_directory;
    file_pos->num_of_file = (uint32_t)file_pos64.num_of_file;
    return err;
}

int unzGoToFilePos(unzFile file, unz_file_pos *file_pos) {
    mz_unzip_compat *compat = (mz_unzip_compat *)file;
    unz64_file_pos file_pos64;

    if (!compat || !file_pos)
        return UNZ_PARAMERROR;

    file_pos64.pos_in_zip_directory = file_pos->pos_in_zip_directory;
    file_pos64.num_of_file = file_pos->num_of_file;

    return unzGoToFilePos64(file, &file_pos64);
}

int unzGetFilePos64(unzFile file, unz64_file_pos *file_pos) {
    mz_unzip_compat *compat = (mz_unzip_compat *)file;
    int64_t offset = 0;

    if (!compat || !file_pos)
        return UNZ_PARAMERROR;

    offset = unzGetOffset64(file);
    if (offset < 0)
        return (int)offset;

    file_pos->pos_in_zip_directory = offset;
    file_pos->num_of_file = compat->entry_index;
    return UNZ_OK;
}

int unzGoToFilePos64(unzFile file, const unz64_file_pos *file_pos) {
    mz_unzip_compat *compat = (mz_unzip_compat *)file;
    int32_t err = MZ_OK;

    if (!compat || !file_pos)
        return UNZ_PARAMERROR;

    err = mz_zip_goto_entry(compat->handle, file_pos->pos_in_zip_directory);
    if (err == MZ_OK)
        compat->entry_index = file_pos->num_of_file;
    return err;
}

unsigned long unzGetOffset(unzFile file) {
    return (uint32_t)unzGetOffset64(file);
}

int64_t unzGetOffset64(unzFile file) {
    mz_unzip_compat *compat = (mz_unzip_compat *)file;
    if (!compat)
        return UNZ_PARAMERROR;
    return mz_zip_get_entry(compat->handle);
}

int unzSetOffset(unzFile file, unsigned long pos) {
    return unzSetOffset64(file, pos);
}

int unzSetOffset64(unzFile file, int64_t pos) {
    mz_unzip_compat *compat = (mz_unzip_compat *)file;
    if (!compat)
        return UNZ_PARAMERROR;
    return (int)mz_zip_goto_entry(compat->handle, pos);
}

int unzGetLocalExtrafield(unzFile file, void *buf, unsigned int len) {
    mz_unzip_compat *compat = (mz_unzip_compat *)file;
    mz_zip_file *file_info = NULL;
    int32_t err = MZ_OK;
    int32_t bytes_to_copy = 0;

    if (!compat || !buf || len >= INT32_MAX)
        return UNZ_PARAMERROR;

    err = mz_zip_entry_get_local_info(compat->handle, &file_info);
    if (err != MZ_OK)
        return err;

    bytes_to_copy = (int32_t)len;
    if (bytes_to_copy > file_info->extrafield_size)
        bytes_to_copy = file_info->extrafield_size;

    memcpy(buf, file_info->extrafield, bytes_to_copy);
    return MZ_OK;
}

int32_t unzTell(unzFile file) {
    return unztell(file);
}

int32_t unztell(unzFile file) {
    return (int32_t)unztell64(file);
}

uint64_t unzTell64(unzFile file) {
    return unztell64(file);
}

uint64_t unztell64(unzFile file) {
    mz_unzip_compat *compat = (mz_unzip_compat *)file;
    if (!compat)
        return UNZ_PARAMERROR;
    return compat->total_out;
}

int unzSeek(unzFile file, int32_t offset, int origin) {
    return unzSeek64(file, offset, origin);
}

int unzSeek64(unzFile file, int64_t offset, int origin) {
    mz_unzip_compat *compat = (mz_unzip_compat *)file;
    mz_zip_file *file_info = NULL;
    int64_t position = 0;
    int32_t err = MZ_OK;
    void *stream = NULL;

    if (!compat)
        return UNZ_PARAMERROR;
    err = mz_zip_entry_get_info(compat->handle, &file_info);
    if (err != MZ_OK)
        return err;
    if (file_info->compression_method != MZ_COMPRESS_METHOD_STORE)
        return UNZ_ERRNO;

    if (origin == SEEK_SET)
        position = offset;
    else if (origin == SEEK_CUR)
        position = compat->total_out + offset;
    else if (origin == SEEK_END)
        position = (int64_t)file_info->compressed_size + offset;
    else
        return UNZ_PARAMERROR;

    if (position > (int64_t)file_info->compressed_size)
        return UNZ_PARAMERROR;

    err = mz_zip_get_stream(compat->handle, &stream);
    if (err == MZ_OK)
        err = mz_stream_seek(stream, compat->entry_pos + position, MZ_SEEK_SET);
    if (err == MZ_OK)
        compat->total_out = position;
    return err;
}

int unzEndOfFile(unzFile file) {
    return unzeof(file);
}

int unzeof(unzFile file) {
    mz_unzip_compat *compat = (mz_unzip_compat *)file;
    mz_zip_file *file_info = NULL;
    int32_t err = MZ_OK;

    if (!compat)
        return UNZ_PARAMERROR;
    err = mz_zip_entry_get_info(compat->handle, &file_info);
    if (err != MZ_OK)
        return err;
    if (compat->total_out == (int64_t)file_info->uncompressed_size)
        return 1;
    return 0;
}

void *unzGetStream(unzFile file) {
    mz_unzip_compat *compat = (mz_unzip_compat *)file;
    if (!compat)
        return NULL;
    return (void *)compat->stream;
}

/***************************************************************************/
