/* mz_os_posix.c -- System functions for posix
   Version 2.2.7, January 30th, 2018
   part of the MiniZip project

   Copyright (C) 2010-2018 Nathan Moinvaziri
     https://github.com/nmoinvaz/minizip

   This program is distributed under the terms of the same license as zlib.
   See the accompanying LICENSE file for the full text of the license.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/stat.h>

#if defined unix || defined __APPLE__
#  include <unistd.h>
#  include <utime.h>
#endif
#if defined __linux__
#  include <bsd/stdlib.h>
#else
#  include <stdlib.h>
#endif

#include "mz.h"
#include "mz_strm.h"
#include "mz_os.h"
#include "mz_os_posix.h"

/***************************************************************************/

int32_t mz_posix_rand(uint8_t *buf, int32_t size)
{
    arc4random_buf(buf, size);
    return size;
}

int32_t mz_posix_file_exists(const char *path)
{
    struct stat stat_info;

    memset(&stat_info, 0, sizeof(stat_info));
    if (stat(path, &stat_info) == 0)
        return MZ_OK;

    return MZ_EXIST_ERROR;
}

int64_t mz_posix_get_file_size(const char *path)
{
    struct stat stat_info;

    memset(&stat_info, 0, sizeof(stat_info));
    if (stat(path, &stat_info) == 0)
        return stat_info.st_size;

    return 0;
}

int32_t mz_posix_get_file_date(const char *path, time_t *modified_date, time_t *accessed_date, time_t *creation_date)
{
    struct stat stat_info;
    char *name = NULL;
    size_t len = 0;
    int32_t err = MZ_INTERNAL_ERROR;


    memset(&stat_info, 0, sizeof(stat_info));

    if (strcmp(path, "-") != 0)
    {
        // Not all systems allow stat'ing a file with / appended
        len = strlen(path);
        name = (char *)malloc(len + 1);
        strncpy(name, path, len + 1);
        name[len] = 0;
        if (name[len - 1] == '/')
            name[len - 1] = 0;

        if (stat(name, &stat_info) == 0)
        {
            if (modified_date != NULL)
                *modified_date = stat_info.st_mtime;
            if (accessed_date != NULL)
                *accessed_date = stat_info.st_atime;
            // Creation date not supported
            if (creation_date != NULL)
                *creation_date = 0;

            err = MZ_OK;
        }

        free(name);
    }

    return err;
}

int32_t mz_posix_set_file_date(const char *path, time_t modified_date, time_t accessed_date, time_t creation_date)
{
    struct utimbuf ut;

    ut.actime = accessed_date;
    ut.modtime = modified_date;
    // Creation date not supported

    if (utime(path, &ut) != 0)
        return MZ_INTERNAL_ERROR;

    return MZ_OK;
}

int32_t mz_posix_make_dir(const char *path)
{
    int32_t err = 0;

    err = mkdir(path, 0755);

    if (err != 0 && errno != EEXIST)
        return MZ_INTERNAL_ERROR;

    return MZ_OK;
}

DIR* mz_posix_open_dir(const char *path)
{
    return opendir(path);
}

struct dirent* mz_posix_read_dir(DIR *dir)
{
    if (dir == NULL)
        return NULL;
    return readdir(dir);
}

int32_t mz_posix_close_dir(DIR *dir)
{
    if (dir == NULL)
        return MZ_PARAM_ERROR;
    if (closedir(dir) == -1)
        return MZ_INTERNAL_ERROR;
    return MZ_OK;
}

int32_t mz_posix_is_dir(const char *path)
{
    struct stat path_stat;
    stat(path, &path_stat);
    if (S_ISDIR(path_stat.st_mode))
        return MZ_OK;
    return MZ_EXIST_ERROR;
}
