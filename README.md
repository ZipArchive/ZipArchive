
# SSZipArchive

ZipArchive is a simple utility class for zipping and unzipping files on iOS and Mac.

- Unzip zip files;
- Unzip password protected zip files;
- Create new zip files;
- Append to existing zip files;
- Zip files;
- Zip-up NSData instances. (with a filename)

## Installation and Setup

### CocoaPods

`pod install SSZipArchive`

### Carthage
`github "ZipArchive/ZipArchive"`

### Manual

1. Add the `SSZipArchive` and `minizip` folders to your project.
2. Add the `libz` library to your target

SSZipArchive requires ARC.

## Usage

### Objective-C

```objective-c
// Create
[SSZipArchive createZipFileAtPath: zipPath withContentsOfDirectory: sampleDataPath];

// Unzip
[SSZipArchive unzipFileAtPath:zipPath toDestination: unzipPath];
```

### Swift

```swift
// Create
SSZipArchive.createZipFileAtPath(zipPath, withContentsOfDirectory: sampleDataPath)

// Unzip
SSZipArchive.unzipFileAtPath(zipPath, toDestination: unzipPath)
```

## License

SSZipArchive is protected under the [MIT license](https://github.com/samsoffes/ssziparchive/raw/master/LICENSE) and our slightly modified version of [Minizip](http://www.winimage.com/zLibDll/minizip.html) 1.1 is licensed under the [Zlib license](http://www.zlib.net/zlib_license.html).

## Acknowledgments

Big thanks to [aish](http://code.google.com/p/ziparchive) for creating [ZipArchive](http://code.google.com/p/ziparchive). The project that inspired SSZipArchive. Thank you [@randomsequence](https://github.com/randomsequence) for implementing the creation support tech and to [@johnezang](https://github.com/johnezang) for all his amazing help along the way.
