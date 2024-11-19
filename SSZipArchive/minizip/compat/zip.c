/* zip.c -- Backwards compatible zip interface
   part of the minizip-ng project

   Copyright (C) Nathan Moinvaziri
     https://github.com/zlib-ng/minizip-ng
   Copyright (C) 1998-2010 Gilles Vollant
     https://www.winimage.com/zLibDll/minizip.html

   This program is distributed under the terms of the same license as zlib.
   See the accompanying LICENSE file for the full text of the license.
*/

#include "mz.h"
#include "mz_os.h"
#include "mz_strm.h"
#include "mz_strm_os.h"
#include "mz_zip.h"

#include "zip.h"

/***************************************************************************/

typedef struct mz_zip_compat_s {
    void *stream;
    void *handle;
} mz_zip_compat;

/***************************************************************************/

static int32_t zipConvertAppendToStreamMode(int append) {
    int32_t mode = MZ_OPEN_MODE_WRITE;
    switch (append) {
    case APPEND_STATUS_CREATE:
        mode |= MZ_OPEN_MODE_CREATE;
        break;
    case APPEND_STATUS_CREATEAFTER:
        mode |= MZ_OPEN_MODE_CREATE | MZ_OPEN_MODE_APPEND;
        break;
    case APPEND_STATUS_ADDINZIP:
        mode |= MZ_OPEN_MODE_READ | MZ_OPEN_MODE_APPEND;
        break;
    }
    return mode;
}

zipFile zipOpen(const char *path, int append) {
    return zipOpen2(path, append, NULL, NULL);
}

zipFile zipOpen64(const void *path, int append) {
    return zipOpen2(path, append, NULL, NULL);
}

zipFile zipOpen2(const char *path, int append, zipcharpc *globalcomment, zlib_filefunc_def *pzlib_filefunc_def) {
    zipFile zip = NULL;
    int32_t mode = zipConvertAppendToStreamMode(append);
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

    if (mz_stream_open(stream, path, mode) != MZ_OK) {
        mz_stream_delete(&stream);
        return NULL;
    }

    zip = zipOpen_MZ(stream, append, globalcomment);

    if (!zip) {
        mz_stream_delete(&stream);
        return NULL;
    }

    return zip;
}

zipFile zipOpen2_64(const void *path, int append, zipcharpc *globalcomment, zlib_filefunc64_def *pzlib_filefunc_def) {
    zipFile zip = NULL;
    int32_t mode = zipConvertAppendToStreamMode(append);
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

    if (mz_stream_open(stream, path, mode) != MZ_OK) {
        mz_stream_delete(&stream);
        return NULL;
    }

    zip = zipOpen_MZ(stream, append, globalcomment);

    if (!zip) {
        mz_stream_delete(&stream);
        return NULL;
    }

    return zip;
}

zipFile zipOpen_MZ(void *stream, int append, zipcharpc *globalcomment) {
    mz_zip_compat *compat = NULL;
    int32_t err = MZ_OK;
    int32_t mode = zipConvertAppendToStreamMode(append);
    void *handle = NULL;

    handle = mz_zip_create();
    if (!handle)
        return NULL;

    err = mz_zip_open(handle, stream, mode);

    if (err != MZ_OK) {
        mz_zip_delete(&handle);
        return NULL;
    }

    if (globalcomment)
        mz_zip_get_comment(handle, globalcomment);

    compat = (mz_zip_compat *)calloc(1, sizeof(mz_zip_compat));
    if (compat) {
        compat->handle = handle;
        compat->stream = stream;
    } else {
        mz_zip_delete(&handle);
    }

    return (zipFile)compat;
}

void *zipGetHandle_MZ(zipFile file) {
    mz_zip_compat *compat = (mz_zip_compat *)file;
    if (!compat)
        return NULL;
    return compat->handle;
}

void *zipGetStream_MZ(zipFile file) {
    mz_zip_compat *compat = (mz_zip_compat *)file;
    if (!compat)
        return NULL;
    return (void *)compat->stream;
}

static time_t zipConvertZipDateToTime(tm_zip tmz_date) {
    struct tm tm_date;
    memset(&tm_date, 0, sizeof(struct tm));
    memcpy(&tm_date, &tmz_date, sizeof(tm_zip));
    tm_date.tm_year -= 1900;
    tm_date.tm_isdst = -1;
    return mz_zip_tm_to_time_t(&tm_date);
}

int zipOpenNewFileInZip5(zipFile file, const char *filename, const zip_fileinfo *zipfi, const void *extrafield_local,
                         uint16_t size_extrafield_local, const void *extrafield_global, uint16_t size_extrafield_global,
                         const char *comment, int compression_method, int level, int raw, int windowBits, int memLevel,
                         int strategy, const char *password, unsigned long crc_for_crypting,
                         unsigned long version_madeby, unsigned long flag_base, int zip64) {
    mz_zip_compat *compat = (mz_zip_compat *)file;
    mz_zip_file file_info;

    MZ_UNUSED(strategy);
    MZ_UNUSED(memLevel);
    MZ_UNUSED(windowBits);
    MZ_UNUSED(size_extrafield_local);
    MZ_UNUSED(extrafield_local);
    MZ_UNUSED(crc_for_crypting);

    if (!compat)
        return ZIP_PARAMERROR;

    /* The filename and comment length must fit in 16 bits. */
    if (filename && strlen(filename) > 0xffff)
        return ZIP_PARAMERROR;
    if (comment && strlen(comment) > 0xffff)
        return ZIP_PARAMERROR;

    memset(&file_info, 0, sizeof(file_info));

    if (zipfi) {
        if (zipfi->mz_dos_date != 0)
            file_info.modified_date = mz_zip_dosdate_to_time_t(zipfi->mz_dos_date);
        else
            file_info.modified_date = zipConvertZipDateToTime(zipfi->tmz_date);

        file_info.external_fa = (uint32_t)zipfi->external_fa;
        file_info.internal_fa = (uint16_t)zipfi->internal_fa;
    }

    if (!filename)
        filename = "-";

    file_info.compression_method = (uint16_t)compression_method;
    file_info.filename = filename;
    /* file_info.extrafield_local = extrafield_local; */
    /* file_info.extrafield_local_size = size_extrafield_local; */
    file_info.extrafield = extrafield_global;
    file_info.extrafield_size = size_extrafield_global;
    file_info.version_madeby = (uint16_t)version_madeby;
    file_info.comment = comment;
    if (file_info.comment)
        file_info.comment_size = (uint16_t)strlen(file_info.comment);
    file_info.flag = (uint16_t)flag_base;
    if (zip64)
        file_info.zip64 = MZ_ZIP64_FORCE;
    else
        file_info.zip64 = MZ_ZIP64_DISABLE;
#ifdef HAVE_WZAES
    if (password || (raw && (file_info.flag & MZ_ZIP_FLAG_ENCRYPTED)))
        file_info.aes_version = MZ_AES_VERSION;
#endif

    return mz_zip_entry_write_open(compat->handle, &file_info, (int16_t)level, (uint8_t)raw, password);
}

int zipOpenNewFileInZip4_64(zipFile file, const char *filename, const zip_fileinfo *zipfi, const void *extrafield_local,
                            uint16_t size_extrafield_local, const void *extrafield_global,
                            uint16_t size_extrafield_global, const char *comment, int compression_method, int level,
                            int raw, int windowBits, int memLevel, int strategy, const char *password,
                            unsigned long crc_for_crypting, unsigned long version_madeby, unsigned long flag_base,
                            int zip64) {
    return zipOpenNewFileInZip5(file, filename, zipfi, extrafield_local, size_extrafield_local, extrafield_global,
                                size_extrafield_global, comment, compression_method, level, raw, windowBits, memLevel,
                                strategy, password, crc_for_crypting, version_madeby, flag_base, zip64);
}

int zipOpenNewFileInZip4(zipFile file, const char *filename, const zip_fileinfo *zipfi, const void *extrafield_local,
                         uint16_t size_extrafield_local, const void *extrafield_global, uint16_t size_extrafield_global,
                         const char *comment, int compression_method, int level, int raw, int windowBits, int memLevel,
                         int strategy, const char *password, unsigned long crc_for_crypting,
                         unsigned long version_madeby, unsigned long flag_base) {
    return zipOpenNewFileInZip4_64(file, filename, zipfi, extrafield_local, size_extrafield_local, extrafield_global,
                                   size_extrafield_global, comment, compression_method, level, raw, windowBits,
                                   memLevel, strategy, password, crc_for_crypting, version_madeby, flag_base, 0);
}

int zipOpenNewFileInZip3(zipFile file, const char *filename, const zip_fileinfo *zipfi, const void *extrafield_local,
                         uint16_t size_extrafield_local, const void *extrafield_global, uint16_t size_extrafield_global,
                         const char *comment, int compression_method, int level, int raw, int windowBits, int memLevel,
                         int strategy, const char *password, unsigned long crc_for_crypting) {
    return zipOpenNewFileInZip3_64(file, filename, zipfi, extrafield_local, size_extrafield_local, extrafield_global,
                                   size_extrafield_global, comment, compression_method, level, raw, windowBits,
                                   memLevel, strategy, password, crc_for_crypting, 0);
}

int zipOpenNewFileInZip3_64(zipFile file, const char *filename, const zip_fileinfo *zipfi, const void *extrafield_local,
                            uint16_t size_extrafield_local, const void *extrafield_global,
                            uint16_t size_extrafield_global, const char *comment, int compression_method, int level,
                            int raw, int windowBits, int memLevel, int strategy, const char *password,
                            unsigned long crc_for_crypting, int zip64) {
    return zipOpenNewFileInZip4_64(file, filename, zipfi, extrafield_local, size_extrafield_local, extrafield_global,
                                   size_extrafield_global, comment, compression_method, level, raw, windowBits,
                                   memLevel, strategy, password, crc_for_crypting, MZ_VERSION_MADEBY, 0, zip64);
}

int zipOpenNewFileInZip2(zipFile file, const char *filename, const zip_fileinfo *zipfi, const void *extrafield_local,
                         uint16_t size_extrafield_local, const void *extrafield_global, uint16_t size_extrafield_global,
                         const char *comment, int compression_method, int level, int raw) {
    return zipOpenNewFileInZip3_64(file, filename, zipfi, extrafield_local, size_extrafield_local, extrafield_global,
                                   size_extrafield_global, comment, compression_method, level, raw, 0, 0, 0, NULL, 0,
                                   0);
}

int zipOpenNewFileInZip2_64(zipFile file, const char *filename, const zip_fileinfo *zipfi, const void *extrafield_local,
                            uint16_t size_extrafield_local, const void *extrafield_global,
                            uint16_t size_extrafield_global, const char *comment, int compression_method, int level,
                            int raw, int zip64) {
    return zipOpenNewFileInZip3_64(file, filename, zipfi, extrafield_local, size_extrafield_local, extrafield_global,
                                   size_extrafield_global, comment, compression_method, level, raw, 0, 0, 0, NULL, 0,
                                   zip64);
}

int zipOpenNewFileInZip(zipFile file, const char *filename, const zip_fileinfo *zipfi, const void *extrafield_local,
                        uint16_t size_extrafield_local, const void *extrafield_global, uint16_t size_extrafield_global,
                        const char *comment, int compression_method, int level) {
    return zipOpenNewFileInZip_64(file, filename, zipfi, extrafield_local, size_extrafield_local, extrafield_global,
                                  size_extrafield_global, comment, compression_method, level, 0);
}

int zipOpenNewFileInZip64(zipFile file, const char *filename, const zip_fileinfo *zipfi, const void *extrafield_local,
                          uint16_t size_extrafield_local, const void *extrafield_global,
                          uint16_t size_extrafield_global, const char *comment, int compression_method, int level,
                          int zip64) {
    return zipOpenNewFileInZip2_64(file, filename, zipfi, extrafield_local, size_extrafield_local, extrafield_global,
                                   size_extrafield_global, comment, compression_method, level, 0, zip64);
}

int zipOpenNewFileInZip_64(zipFile file, const char *filename, const zip_fileinfo *zipfi, const void *extrafield_local,
                           uint16_t size_extrafield_local, const void *extrafield_global,
                           uint16_t size_extrafield_global, const char *comment, int compression_method, int level,
                           int zip64) {
    return zipOpenNewFileInZip2_64(file, filename, zipfi, extrafield_local, size_extrafield_local, extrafield_global,
                                   size_extrafield_global, comment, compression_method, level, 0, zip64);
}

int zipWriteInFileInZip(zipFile file, const void *buf, uint32_t len) {
    mz_zip_compat *compat = (mz_zip_compat *)file;
    int32_t written = 0;
    if (!compat || len >= INT32_MAX)
        return ZIP_PARAMERROR;
    written = mz_zip_entry_write(compat->handle, buf, (int32_t)len);
    if ((written < 0) || ((uint32_t)written != len))
        return ZIP_ERRNO;
    return ZIP_OK;
}

int zipCloseFileInZipRaw(zipFile file, unsigned long uncompressed_size, unsigned long crc32) {
    return zipCloseFileInZipRaw64(file, uncompressed_size, crc32);
}

int zipCloseFileInZipRaw64(zipFile file, uint64_t uncompressed_size, unsigned long crc32) {
    mz_zip_compat *compat = (mz_zip_compat *)file;
    if (!compat)
        return ZIP_PARAMERROR;
    return mz_zip_entry_close_raw(compat->handle, (int64_t)uncompressed_size, (uint32_t)crc32);
}

int zipCloseFileInZip(zipFile file) {
    return zipCloseFileInZip64(file);
}

int zipCloseFileInZip64(zipFile file) {
    mz_zip_compat *compat = (mz_zip_compat *)file;
    if (!compat)
        return ZIP_PARAMERROR;
    return mz_zip_entry_close(compat->handle);
}

int zipClose(zipFile file, const char *global_comment) {
    return zipClose_64(file, global_comment);
}

int zipClose_64(zipFile file, const char *global_comment) {
    return zipClose2_64(file, global_comment, MZ_VERSION_MADEBY);
}

int zipClose2_64(zipFile file, const char *global_comment, uint16_t version_madeby) {
    mz_zip_compat *compat = (mz_zip_compat *)file;
    int32_t err = MZ_OK;

    if (compat->handle)
        err = zipClose2_MZ(file, global_comment, version_madeby);

    if (compat->stream) {
        mz_stream_close(compat->stream);
        mz_stream_delete(&compat->stream);
    }

    free(compat);

    return err;
}

/* Only closes the zip handle, does not close the stream */
int zipClose_MZ(zipFile file, const char *global_comment) {
    return zipClose2_MZ(file, global_comment, MZ_VERSION_MADEBY);
}

/* Only closes the zip handle, does not close the stream */
int zipClose2_MZ(zipFile file, const char *global_comment, uint16_t version_madeby) {
    mz_zip_compat *compat = (mz_zip_compat *)file;
    int32_t err = MZ_OK;

    if (!compat)
        return ZIP_PARAMERROR;
    if (!compat->handle)
        return err;

    if (global_comment)
        mz_zip_set_comment(compat->handle, global_comment);

    mz_zip_set_version_madeby(compat->handle, version_madeby);
    err = mz_zip_close(compat->handle);
    mz_zip_delete(&compat->handle);

    return err;
}
