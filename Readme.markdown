# SSZipArchive

SSZipArchive is a simple utility class for unzipping files based on [ZipArchive](http://code.google.com/p/ziparchive) by aish.

Currently it only supports unzipping. In the future, creating zip files will be supported.

## Adding to your project

1. Add `SSZipArchive.h`, `SSZipArchive.m`, and `minizip` to your project.
2. Add the `libz` library to your target

## Usage

```objective-c
NSString *path = @"path_to_your_zip_file";
NSString *destination = @"path_to_the_folder_where_you_want_it_unzipped";
[SSZipArchive unzipFileAtPath:path toDestination:destination];
```

## License

SSZipArchive is licensed under the [MIT license](https://github.com/samsoffes/ssziparchive/raw/master/LICENSE).  A slightly modified version of [Minizip](http://www.winimage.com/zLibDll/minizip.html) 1.01h is also included and is licensed under the [Zlib license](http://www.zlib.net/zlib_license.html).
