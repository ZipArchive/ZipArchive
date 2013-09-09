# SSZipArchive

SSZipArchive is a simple utility class for zipping and unzipping files. Features:

* Unzipping zip files
* Unzipping password protected zip files
* Creating zip files
* Appending to zip files
* Zipping files
* Zipping NSData with a filename

## Adding to your project

1. Add the `SSZipArchive` and `minizip` folders to your project.
2. Add the `libz` library to your target

SSZipArchive requires ARC.

## Usage

``` objective-c
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
```

## Tests

Simply, open the Xcode 5 or higher project in the Tests directory and press Command-U to run the tests.

## License

SSZipArchive is licensed under the [MIT license](https://github.com/samsoffes/ssziparchive/raw/master/LICENSE).  A slightly modified version of [Minizip](http://www.winimage.com/zLibDll/minizip.html) 1.1 is also included and is licensed under the [Zlib license](http://www.zlib.net/zlib_license.html).

## Thanks

Thanks [aish](http://code.google.com/p/ziparchive) for creating [ZipArchive](http://code.google.com/p/ziparchive) which SSZipArchive is based on, Johnnie Walker ([@randomsequence](https://github.com/randomsequence)) for implementing creation support, and John Engelhart ([@johnezang](https://github.com/johnezang)) for all his amazing help along the way.
