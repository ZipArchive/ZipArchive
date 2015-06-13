#ZipArchive

ZipArchive is a simple utility class for zipping and unzipping files. 

You can do the following:

- Unzip zip files
- Unzip password protected zip files
- Create new zip files
- Append to existing zip files
- Zip files
- Zip-up `NSData` instances (with a filename)

##How to add ZipArchive to your project

1. Add the `SSZipArchive` and `minizip` folders to your project.
2. Add the `libz` library to your target

ZipArchive requires ARC.

###Usage

```objective-c
// Unzipping
NSString *zipPath = @"path_to_your_zip_file";
NSString *destinationPath = @"path_to_the_folder_where_you_want_it_unzipped";
[SSZipArchive unzipFileAtPath:zipPath toDestination:destinationPath];

// Zipping
NSString *zippedPath = @"path_where_you_want_the_file_created";
NSArray *inputPaths = [NSArray arrayWithObjects:
                       [[NSBundle mainBundle] pathForResource:@"photo1" ofType:@"jpg"],
                       [[NSBundle mainBundle] pathForResource:@"photo2" ofType:@"jpg"]
                       nil];
[SSZipArchive createZipFileAtPath:zippedPath withFilesAtPaths:inputPaths];

// Zipping directory
[SSZipArchive createZipFileAtPath:zippedPath withContentsOfDirectory:inputPath];
```

###Licensing
ZipArchive is protected under the [MIT license](https://github.com/ZipArchive/ZipArchive/raw/master/LICENSE) and our slightly modified version of [Minizip](http://www.winimage.com/zLibDll/minizip.html) 1.1 is licensed under the [Zlib license](http://www.zlib.net/zlib_license.html).

## Acknowledgments
Big thanks to [Aish](http://code.google.com/p/ziparchive) for creating [ZipArchive](http://code.google.com/p/ziparchive). The project that inspired ZipArchive. Thank you [@randomsequence](https://github.com/randomsequence) for implementing the creation support tech and to [@johnezang](https://github.com/johnezang) for all his amazing help along the way.