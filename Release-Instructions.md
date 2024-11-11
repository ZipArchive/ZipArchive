# New ZipArchive release

The following steps should be taken by project maintainers when they create a new release.

1. Create a new release and tag for the release.

    - Tags should be in the form of Major.Minor.Revision 
    
     *(Xcode SPM tools do not pick up vMajor.Minor.Revision used in older releases. After 2.4.2 please do not include the v in the tag name)*

    - Release names should be  more human readable: Version Major.Minor.Revision

2. Update the podspec and test it

    - *pod lib lint SSZipArchive.podspec*

3. Push the pod to the trunk

    - *pod trunk push SSZipArchive.podspec*


Note: We are no longer releasing a Carthage release as of 2.2.3. Developers are encouraged to build one themselves.


# Minizip-ng (formally minizip) update

The following steps should be taken by project maintainers when they update minizip files.

1. Source is at https://github.com/zlib-ng/minizip-ng.
2. Have cmake:
`brew install cmake`
3. Run cmake on minizip repo with our desired configuration:
`cmake . -DMZ_BZIP2=OFF -DMZ_LIBCOMP=OFF -DMZ_LZMA=OFF -DMZ_OPENSSL=OFF -DMZ_ZLIB=ON -DMZ_ZSTD=OFF`
4. Look at the file `./CMakeFiles/minizip.dir/DependInfo.cmake` it will give you the following information:
- The list of C files that we need to include.

5. Look at the file `./CMakeFiles/minizip.dir/flags.make` it will give you the following information:
- The list of compiler flags that we need to include (we have to either include zlib-ng or use ZLIB_COMPAT)

HAVE_ARC4RANDOM_BUF HAVE_ICONV HAVE_INTTYPES_H HAVE_PKCRYPT HAVE_STDINT_H HAVE_WZAES HAVE_ZLIB ZLIB_COMPAT

Ignoring the ones starting with an underscore, like: "_BSD_SOURCE" "_DARWIN_C_SOURCE" "_DEFAULT_SOURCE" "_POSIX_C_SOURCE=200809L"

6. Set those flags in SSZipArchive.podspec (for CocoaPods) and in ZipArchive.xcodeproj (for Carthage)

7. Replace the .h and .c files with the latest ones, except for `compat/*.{c,h}`, which are customized to expose some struct in SSZipCommon.h.

Note: we can also use `cmake -G Xcode . -DMZ_BZIP2=OFF -DMZ_LIBCOMP=OFF -DMZ_LZMA=OFF -DMZ_OPENSSL=OFF -DMZ_ZLIB=ON -DMZ_ZSTD=OFF` to get the list of files to include in an xcodeproj of its own.
