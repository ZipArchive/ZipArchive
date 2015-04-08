# SSZipArchive [2.0]
[![Build](https://img.shields.io/travis/joyent/node.svg)](#)
[![Language](http://img.shields.io/badge/language-objectivec-brightgreen.svg?style=flat
)](https://developer.apple.com/library/mac/documentation/Cocoa/Conceptual/ProgrammingWithObjectiveC/Introduction/Introduction.html)
[![Language](http://img.shields.io/badge/language-swift-brightgreen.svg?style=flat
)](https://developer.apple.com/swift)
[![License](http://img.shields.io/badge/license-MIT-lightgrey.svg?style=flat
)](https://github.com/soffes/ssziparchive/blob/master/LICENSE.txt)

> SSZipArchive is a simple utility class for zipping and unzipping files.

**Important**: 2.0 is currently in the development phase and is not stable.

## What's new in 2.0?

We are planning on improving the API for use in modern Objective-C and Swift projects. Introducing nullability, fixing major bugs and over-all improving the API for better use in your projects!

#### Improvements:

- Unzipping files;
- Unzipping encrypted files;
- Create new zip files;
- Append to existing zip files;
- Zipping files;
- Zipping up NSData instances. (with a filename)

#### New:

- Nullability;
- Carthage support;
- Continuous integration testing with Travis;
- Swift version.

### License
SSZipArchive is protected under the [MIT license](https://github.com/samsoffes/ssziparchive/raw/master/LICENSE) and our slightly modified version of [Minizip](http://www.winimage.com/zLibDll/minizip.html) 1.1 is licensed under the [Zlib license](http://www.zlib.net/zlib_license.html).

## Acknowledgments
Big thanks to [aish](http://code.google.com/p/ziparchive) for creating [ZipArchive](http://code.google.com/p/ziparchive). The project that inspired SSZipArchive. Thank you [@randomsequence](https://github.com/randomsequence) for implementing the creation support tech and to [@johnezang](https://github.com/johnezang) for all his amazing help along the way.
