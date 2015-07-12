#ZipArchive
[![Carthage compatible](https://img.shields.io/badge/Carthage-compatible-4BC51D.svg?style=flat)](https://github.com/Carthage/Carthage)
[![License](http://img.shields.io/badge/license-MIT-lightgrey.svg?style=flat
)](http://mit-license.org)

ZipArchive is a simple utility class for zipping and unzipping files on iOS and Mac.

##Installation and Setup
####Carthage
Carthage builds the dependencies and provides a binary framework for ZipArchive. To add ZipArchive to your project using Carthage, create a Cartfile and add: 

```ogdl
github "ZipArchive/ZipArchive" "master"
```

####CocoaPods
*currently not working.

####Manual
1. Add `Main.h` and `Main.m` to your project.
2. Add the `minizip` folder to your project.
3. Add the `libz` library to your target

##Usage
```objective-c
// Unzip Operation
NSString *zipPath = @"path_to_your_zip_file";
NSString *destinationPath = @"path_to_the_folder_where_you_want_it_unzipped";
    
[Main unzipFileAtPath:zipPath 
        toDestination:destinationPath];
    
// Zip Operation
NSString *zippedPath = @"path_where_you_want_the_file_created";
NSArray *inputPaths = @[[[NSBundle mainBundle] pathForResource:@"photo1" ofType:@"jpg"],
                        [[NSBundle mainBundle] pathForResource:@"photo1" ofType:@"jpg"]];
    
[Main createZipFileAtPath:zippedPath
         withFilesAtPaths:inputPaths];
    
// Zip Directory
[Main createZipFileAtPath:zippedPath
  withContentsOfDirectory:inputPaths];
```

###Licensing
ZipArchive is released under the [MIT license](https://github.com/ZipArchive/ZipArchive/raw/master/LICENSE) and our slightly modified version of [Minizip](http://www.winimage.com/zLibDll/minizip.html) 1.1 is licensed under the [Zlib license](http://www.zlib.net/zlib_license.html).
