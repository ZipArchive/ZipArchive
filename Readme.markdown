# SSZipArchive

SSZipArchive is a simple utility class for unzipping files based on [ZipArchive](http://code.google.com/p/ziparchive).

Currently it only supports unzipping. In the future, creating zip files will be supported.

## Adding to your project

1. Add `SSZipArchive.h`, `SSZipArchive.m`, and `minizip` to your project.
2. Add the `libz` library to your target

## Usage

    NSString *path = @"path_to_your_zip_file";
    NSString *destination = @"path_to_the_folder_where_you_want_it_unzipped";
    [SSZipArchive unzipFileAtPath:path toDestination:destination];
