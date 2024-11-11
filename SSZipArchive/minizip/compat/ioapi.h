
#ifndef ZLIBIOAPI64_H
#define ZLIBIOAPI64_H

#include <stdint.h>

typedef uint64_t ZPOS64_T;

#ifndef ZEXPORT
#  define ZEXPORT
#endif

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************/

#define ZLIB_FILEFUNC_SEEK_SET             (0)
#define ZLIB_FILEFUNC_SEEK_CUR             (1)
#define ZLIB_FILEFUNC_SEEK_END             (2)

#define ZLIB_FILEFUNC_MODE_READ            (1)
#define ZLIB_FILEFUNC_MODE_WRITE           (2)
#define ZLIB_FILEFUNC_MODE_READWRITEFILTER (3)

#define ZLIB_FILEFUNC_MODE_EXISTING        (4)
#define ZLIB_FILEFUNC_MODE_CREATE          (8)

#ifndef ZCALLBACK
#  define ZCALLBACK
#endif

/***************************************************************************/

typedef void *(ZCALLBACK *open_file_func)(void *opaque, const char *filename, int mode);
typedef void *(ZCALLBACK *open64_file_func)(void *opaque, const void *filename, int mode);
typedef unsigned long(ZCALLBACK *read_file_func)(void *opaque, void *stream, void *buf, unsigned long size);
typedef unsigned long(ZCALLBACK *write_file_func)(void *opaque, void *stream, const void *buf, unsigned long size);
typedef int(ZCALLBACK *close_file_func)(void *opaque, void *stream);
typedef int(ZCALLBACK *testerror_file_func)(void *opaque, void *stream);
typedef long(ZCALLBACK *tell_file_func)(void *opaque, void *stream);
typedef ZPOS64_T(ZCALLBACK *tell64_file_func)(void *opaque, void *stream);
typedef long(ZCALLBACK *seek_file_func)(void *opaque, void *stream, unsigned long offset, int origin);
typedef long(ZCALLBACK *seek64_file_func)(void *opaque, void *stream, ZPOS64_T offset, int origin);

/***************************************************************************/

typedef struct zlib_filefunc_def_s {
    open_file_func zopen_file;
    read_file_func zread_file;
    write_file_func zwrite_file;
    tell_file_func ztell_file;
    seek_file_func zseek_file;
    close_file_func zclose_file;
    testerror_file_func zerror_file;
    void *opaque;
} zlib_filefunc_def;

typedef struct zlib_filefunc64_def_s {
    open64_file_func zopen64_file;
    read_file_func zread_file;
    write_file_func zwrite_file;
    tell64_file_func ztell64_file;
    seek64_file_func zseek64_file;
    close_file_func zclose_file;
    testerror_file_func zerror_file;
    void *opaque;
} zlib_filefunc64_def;

/***************************************************************************/

/* Compatibility layer with the original minizip library (ioapi.h and iowin32.h). */
ZEXPORT void fill_fopen_filefunc(zlib_filefunc_def *pzlib_filefunc_def);
ZEXPORT void fill_fopen64_filefunc(zlib_filefunc64_def *pzlib_filefunc_def);
ZEXPORT void fill_win32_filefunc(zlib_filefunc_def *pzlib_filefunc_def);
ZEXPORT void fill_win32_filefunc64(zlib_filefunc64_def *pzlib_filefunc_def);
ZEXPORT void fill_win32_filefunc64A(zlib_filefunc64_def *pzlib_filefunc_def);

/* Compatibility layer with older minizip-ng (ioapi_mem.h). */
ZEXPORT void fill_memory_filefunc(zlib_filefunc_def *pzlib_filefunc_def);

/***************************************************************************/

int32_t mz_stream_ioapi_set_filefunc(void *stream, zlib_filefunc_def *filefunc);
int32_t mz_stream_ioapi_set_filefunc64(void *stream, zlib_filefunc64_def *filefunc);

void *mz_stream_ioapi_create(void);
void mz_stream_ioapi_delete(void **stream);

/***************************************************************************/

#ifdef __cplusplus
}
#endif

#endif
