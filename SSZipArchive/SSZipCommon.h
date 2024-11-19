#ifndef SSZipCommon
#define SSZipCommon

// typedefs moved from unzip.h to here for public access

/* unz_global_info structure contain global data about the ZIPfile
 These data comes from the end of central dir */

typedef struct unz_global_info_s {
    unsigned long number_entry; /* total number of entries in the central dir on this disk */
    unsigned long size_comment; /* size of the global comment of the zipfile */
    /* minizip-ng fields */
    uint32_t number_disk_with_CD; /* number the the disk with central dir, used for spanning ZIP */
} unz_global_info;

/* unz_file_info contain information about a file in the zipfile */
/* https://pkware.cachefly.net/webdocs/casestudies/APPNOTE.TXT */

typedef struct unz_file_info_s {
    unsigned long version;            /* version made by                 2 bytes */
    unsigned long version_needed;     /* version needed to extract       2 bytes */
    unsigned long flag;               /* general purpose bit flag        2 bytes */
    unsigned long compression_method; /* compression method              2 bytes */
    unsigned long dos_date;           /* last mod file date in Dos fmt   4 bytes */
    unsigned long crc;                /* crc-32                          4 bytes */
    uint64_t compressed_size;         /* compressed size                 4 bytes */
    uint64_t uncompressed_size;       /* uncompressed size               4 bytes */
    unsigned long size_filename;      /* filename length                 2 bytes */
    unsigned long size_file_extra;    /* extra field length              2 bytes */
    unsigned long size_file_comment;  /* file comment length             2 bytes */

    unsigned long disk_num_start; /* disk number start               2 bytes */
    unsigned long internal_fa;    /* internal file attributes        2 bytes */
    unsigned long external_fa;    /* external file attributes        4 bytes */

    // [ZipArchive] disabled: we don't need the tmu_date field because we already set the dos_date field
    // tm_unz tmu_date;

    /* minizip-ng fields */
    uint64_t disk_offset;
} unz_file_info;

#endif
