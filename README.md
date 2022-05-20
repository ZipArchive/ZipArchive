[![CI](https://github.com/ZipArchive/ZipArchive/workflows/CI/badge.svg)](https://github.com/ZipArchive/ZipArchive/actions?query=workflow%3ACI)

# SSZipArchive

ZipArchive is a simple utility class for zipping and unzipping files on iOS, macOS and tvOS.

- Unzip zip files;
- Unzip password protected zip files;
- Unzip AES encrypted zip files;
- Create zip files;
- Create large (> 4.3Gb) files;
- Create password protected zip files;
- Create AES encrypted zip files;
- Choose compression level;
- Zip-up NSData instances. (with a filename)

## Installation and Setup

*The main release branch is configured to support Objective-C and Swift 3+.*

SSZipArchive works on Xcode 7 and above, iOS 9 and above, tvOS 9 and above, macOS 10.9 and above, watchOS 2 and above.

### CocoaPods
In your Podfile:  
`pod 'SSZipArchive'`

You should define your minimum deployment target explicitly, like:
`platform :ios, '9.0'`

Recommended CocoaPods version should be at least CocoaPods 1.7.5.

### SPM
Add a Swift Package reference to https://github.com/ZipArchive/ZipArchive.git (SSZipArchive 2.4.2 and higher or master)

### Carthage
In your Cartfile:  
`github "ZipArchive/ZipArchive"`

Note: We are no longer releasing a Carthage pre-built package as of 2.2.3. Developers are encouraged to build one themselves.

### Manual

1. Add the `SSZipArchive` and `minizip` folders to your project.
2. Add the `libz` and `libiconv` libraries to your target.
3. Add the `Security` framework to your target.
4. Add the following GCC_PREPROCESSOR_DEFINITIONS: `HAVE_INTTYPES_H HAVE_PKCRYPT HAVE_STDINT_H HAVE_WZAES HAVE_ZLIB $(inherited)`.

SSZipArchive requires ARC.

## Usage

### Objective-C

```objective-c

//Import "#import <ZipArchive.h>" for SPM/Carthage, and "#import <SSZipArchive.h>" for CocoaPods.

// Create
[SSZipArchive createZipFileAtPath:zipPath withContentsOfDirectory:sampleDataPath];

// Unzip
[SSZipArchive unzipFileAtPath:zipPath toDestination:unzipPath];
```

### Swift

```swift
//Import "import ZipArchive" for SPM/Carthage, and "import SSZipArchive" for CocoaPods.

// Create
SSZipArchive.createZipFileAtPath(zipPath, withContentsOfDirectory: sampleDataPath)

// Unzip
SSZipArchive.unzipFileAtPath(zipPath, toDestination: unzipPath)
```

## License

SSZipArchive is protected under the [MIT license](https://github.com/samsoffes/ssziparchive/raw/master/LICENSE) and our slightly modified version of [minizip-ng (formally minizip)](https://github.com/zlib-ng/minizip-ng) 3.0.2 is licensed under the [Zlib license](https://www.zlib.net/zlib_license.html).

## Acknowledgments

* Big thanks to *aish* for creating [ZipArchive](https://code.google.com/archive/p/ziparchive/). The project that inspired SSZipArchive.
* Thank you [@soffes](https://github.com/soffes) for the actual name of SSZipArchive.
* Thank you [@randomsequence](https://github.com/randomsequence) for implementing the creation support tech.
* Thank you [@johnezang](https://github.com/johnezang) for all his amazing help along the way.
* Thank you [@nmoinvaz](https://github.com/nmoinvaz) for minizip-ng (formally minizip), the core of ZipArchive.
* Thank you to [all the contributors](https://github.com/ZipArchive/ZipArchive/graphs/contributors).
