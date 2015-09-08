//
//  ZipArchiveTests.m
//  ZipArchiveTests
//
//  Created by Sam Soffes on 2010-03-11.
//  Copyright (c) 2015 ZipArchive. All rights reserved.
//

#import "ZipArchive/Main.h"
#import "CollectingDelegate.h"
#import <XCTest/XCTest.h>
#import <CommonCrypto/CommonDigest.h>

@interface CancelDelegate : NSObject <ZipArchiveDelegate>

@property (nonatomic, assign) int numberOfFilesUnzipped;
@property (nonatomic, assign) int numberOfFilesToUnzip;
@property (nonatomic, assign) BOOL didUnzipArchive;
@property (nonatomic, assign) int loaded;
@property (nonatomic, assign) int total;

@end

@implementation CancelDelegate

- (void)zipArchiveDidUnzipFileAtIndex:(NSInteger)fileIndex totalFiles:(NSInteger)totalFiles archivePath:(NSString *)archivePath fileInformation:(unz_file_info)fileInformation {
    _numberOfFilesUnzipped = (int)fileIndex + 1;
}

- (BOOL)zipArchiveShouldUnzipFileAtIndex:(NSInteger)fileIndex totalFiles:(NSInteger)totalFiles archivePath:(NSString *)archivePath fileInformation:(unz_file_info)fileInformation {
    return _numberOfFilesUnzipped < _numberOfFilesToUnzip;
}

- (void)zipArchiveDidUnzipArchiveAtPath:(NSString *)path zipInformation:(unz_global_info)zipInformation unzippedPath:(NSString *)unzippedPath {
    _didUnzipArchive = YES;
}

- (void)zipArchiveProgressEvent:(unsigned long long)loaded total:(unsigned long long)total {
    _loaded = (int)loaded;
    _total = (int)total;
}

@end

@interface ZipArchiveTests : XCTestCase <ZipArchiveDelegate>
@end

@implementation ZipArchiveTests {
    NSMutableArray *progressEvents;
}

- (void)setUp {
    [super setUp];
    
    progressEvents = [NSMutableArray array];
}

- (void)tearDown {
    [super tearDown];
    
    [[NSFileManager defaultManager] removeItemAtPath:[self _cachesPath:nil] error:nil];
}

- (void)testZipping {
    // Use extracted files from [-testUnzipping]
    NSString *inputPath = [self _cachesPath:@"Regular"];
    NSArray *inputPaths = @[[inputPath stringByAppendingPathComponent:@"Readme.markdown"],
                            [inputPath stringByAppendingPathComponent:@"LICENSE"]];
    NSString *outputPath = [self _cachesPath:@"Zipped"];
    NSString *archivePath = [outputPath stringByAppendingPathComponent:@"CreatedArchive.zip"];
    
    [Main createZipFileAtPath:archivePath
             withFilesAtPaths:inputPaths];
    
    // TODO: - Make sure the files are actually unzipped. They are, but the test could be better.
    XCTAssertTrue([[NSFileManager defaultManager] fileExistsAtPath:archivePath], @"Archive Created.");
}

- (void)testDirectoryZipping {
    NSString *inputPath = [self _cachesPath:@"Unicode"];
    NSString *outputPath = [self _cachesPath:@"FolderZipped"];
    NSString *archivePath = [outputPath stringByAppendingPathComponent:@"ArchiveWithFolders.zip"];
    
    [Main createZipFileAtPath:archivePath
      withContentsOfDirectory:inputPath];
    
    XCTAssertTrue([[NSFileManager defaultManager] fileExistsAtPath:archivePath], @"Folder Archive created");
}

- (void)testMultipleZipping {
    NSArray *inputPaths = @[[[NSBundle mainBundle] pathForResource:@"0" ofType:@"m4a"],
                            [[NSBundle mainBundle] pathForResource:@"1" ofType:@"m4a"],
                            [[NSBundle mainBundle] pathForResource:@"2" ofType:@"m4a"],
                            [[NSBundle mainBundle] pathForResource:@"3" ofType:@"m4a"],
                            [[NSBundle mainBundle] pathForResource:@"4" ofType:@"m4a"],
                            [[NSBundle mainBundle] pathForResource:@"5" ofType:@"m4a"],
                            [[NSBundle mainBundle] pathForResource:@"6" ofType:@"m4a"],
                            [[NSBundle mainBundle] pathForResource:@"7" ofType:@"m4a"]];
    NSString *outputPath = [self _cachesPath:@"Zipped"];
    
    for (int test = 0; test < 1000; test++) {
        NSString *archivePath = [outputPath stringByAppendingPathComponent:[NSString stringWithFormat:@"queue_test_%d.zip", test]];
        
        [Main createZipFileAtPath:archivePath
                 withFilesAtPaths:inputPaths];
        
        long long threshold = 510000;
        long long fileSize = [[[NSFileManager defaultManager] attributesOfItemAtPath:archivePath error:nil][NSFileSize] longLongValue];
        
        XCTAssertTrue(fileSize > threshold, @"Zipping failed at %lld", fileSize);
    }
}

- (void)testUnzipping {
    NSString *zipPath = [[NSBundle bundleForClass:[self class]] pathForResource:@"TestArchive" ofType:@"zip"];
    NSString *outputPath = [self _cachesPath:@"Regular"];
    
    [Main unzipFileAtPath:zipPath
            toDestination:outputPath
                 delegate:self];
    
    NSFileManager *fileManager = [NSFileManager defaultManager];
    NSString *testPath = [outputPath stringByAppendingPathComponent:@"Readme.markdown"];
    XCTAssertTrue([fileManager fileExistsAtPath:testPath], @"README Unzipped.");
    
    testPath = [outputPath stringByAppendingPathComponent:@"LICENSE"];
    XCTAssertTrue([fileManager fileExistsAtPath:testPath], @"LICENSE Unzipped.");
}

- (void)testSmallFileUnzipping {
    NSString *zipPath = [[NSBundle mainBundle] pathForResource:@"hello" ofType:@"zip"];
    NSString *outputPath = [self _cachesPath:@"Regular"];
    
    [Main unzipFileAtPath:zipPath
            toDestination:outputPath
                 delegate:self];
    
    NSFileManager *fileManager = [NSFileManager defaultManager];
    NSString *testPath = [outputPath stringByAppendingPathComponent:@"REadme.markdown"];
    XCTAssertTrue([fileManager fileExistsAtPath:testPath], @"README Unzipped");
    
    testPath = [outputPath stringByAppendingPathComponent:@"LICENSE"];
    XCTAssertTrue([fileManager fileExistsAtPath:testPath], @"LICENSE Unzipped");
}

- (void)testUnzippingProgress {
    NSString *zipPath = [[NSBundle bundleForClass:[self class]] pathForResource:@"TestArchive" ofType:@"zip"];
    NSString *outputPath = [self _cachesPath:@"Progress"];
    
    [progressEvents removeAllObjects];
    [Main unzipFileAtPath:zipPath
            toDestination:outputPath
                 delegate:self];
    
    // 4 Events: The first, then for each of the two files one, then the final event
    XCTAssertTrue(4 == [progressEvents count], @"Expected 4 progress events");
    XCTAssertTrue(0 == [progressEvents[0] intValue]);
    XCTAssertTrue(619 == [progressEvents[1] intValue]);
    XCTAssertTrue(1114 == [progressEvents[2] intValue]);
    XCTAssertTrue(1436 == [progressEvents[3] intValue]);
}

- (void)testUnzippingWithTest {
    NSString *zipPath = [[NSBundle bundleForClass:[self class]] pathForResource:@"TestPasswordArchive" ofType:@"zip"];
    NSString *outputPath = [self _cachesPath:@"Password"];
    NSError *error = nil;
    
    [Main unzipFileAtPath:zipPath
            toDestination:outputPath
                overwrite:YES
                 password:@"passw0rd"
                    error:&error
                 delegate:self];
    
    NSFileManager *fileManager = [NSFileManager defaultManager];
    NSString *testPath = [outputPath stringByAppendingPathComponent:@"Readme.markdown"];
    XCTAssertTrue([fileManager fileExistsAtPath:testPath], @"Readme Unzipped");
    
    testPath = [outputPath stringByAppendingPathComponent:@"LICENSE"];
    XCTAssertTrue([fileManager fileExistsAtPath:testPath], @"LICENSE Unzipped");
}

- (void)testUnzippingTruncatedFileFix {
    NSString *zipPath = [[NSBundle bundleForClass:[self class]] pathForResource:@"IncorrectHeaders" ofType:@"zip"];
    NSString *outputPath = [self _cachesPath:@"IncorrectHeaders"];
    
    [Main unzipFileAtPath:zipPath
            toDestination:outputPath
                 delegate:self];
    
    NSString *intendedReadmeTxtMD5 = @"31ac96301302eb388070c827447290b5";
    
    NSString *filePath = [outputPath stringByAppendingPathComponent:@"IncorrectHeaders/Readme.txt"];
    NSData *data = [NSData dataWithContentsOfFile:filePath];
    
    NSString *actualReadmeTxtMD5 = [self _calculateMD5Digest:data];
    XCTAssertTrue([actualReadmeTxtMD5 isEqualToString:intendedReadmeTxtMD5], @"Readme.txt MD5 digest should match original.");
}

- (void)testUnzippingWithSymlinkedFileInside {
    NSString *zipPath = [[NSBundle bundleForClass:[self class]] pathForResource:@"SymbolicLink" ofType:@"zip"];
    NSString *outputPath = [self _cachesPath:@"SymbolicLink"];
    
    [Main unzipFileAtPath:zipPath
            toDestination:outputPath
                 delegate:self];
    
    NSString *testSymlinkFolder = [outputPath stringByAppendingPathComponent:@"SymbolicLink/Github.app"];
    NSString *testSymlinkFile = [outputPath stringByAppendingPathComponent:@"SymbolicLink/Icon.icns"];
    NSError *error = nil;
    NSString *symlinkFolderPath = [[NSFileManager defaultManager] destinationOfSymbolicLinkAtPath:testSymlinkFolder error:&error];
    BOOL symbolicLinkToFolderPersists = ((nil != symlinkFolderPath) && [symlinkFolderPath isEqualToString:@"/Applications/Github.app"]) && (nil == error);
    
    error = nil;
    
    NSString *symlinkFilePath = [[NSFileManager defaultManager] destinationOfSymbolicLinkAtPath:testSymlinkFile error:&error];
    BOOL symbolicLinkToFilePersists = ((nil != symlinkFilePath) && [symlinkFilePath isEqualToString:@"/Applications/Github.app/Contents/Resources/AppIcon.icns"]) && (nil == error);
    
    XCTAssertTrue(symbolicLinkToFilePersists && symbolicLinkToFolderPersists, @"Symbolic links should persist from the original archive to the outputted files.");
}

- (void)testUnzippingWithRelativeSymlink {
    NSString *zipPath = [[NSBundle bundleForClass:[self class]] pathForResource:@"RelativeSymbolicLink" ofType:@"zip"];
    NSString *outputPath = [self _cachesPath:@"RelativeSymbolicLink"];
    
    [Main unzipFileAtPath:zipPath
            toDestination:outputPath
                 delegate:self];
    
    // Determine where the symlinks are
    NSString *subfolderName = @"symlinks";
    NSString *testBasePath = [NSString pathWithComponents:@[outputPath]];
    NSString *testSymlinkFolder = [NSString pathWithComponents:@[testBasePath, subfolderName, @"folderSymlink"]];
    NSString *testSymlinkFile = [NSString pathWithComponents:@[testBasePath, subfolderName, @"fileSymlink"]];
    
    // Determine where the files they link to are
    NSString *parentDirectory = @"../";
    NSString *testSymlinkFolderTarget = [NSString pathWithComponents:@[parentDirectory, @"symlinkedFolder"]];
    NSString *testSymlinkFileTarget = [NSString pathWithComponents:@[parentDirectory, @"symlinkedFile"]];
    
    NSError *error = nil;
    
    NSString *symlinkFolderPath = [[NSFileManager defaultManager] destinationOfSymbolicLinkAtPath:testSymlinkFolder error:&error];
    BOOL symbolicLinkToFolderPersists = ((nil != symlinkFolderPath) && [symlinkFolderPath isEqualToString:testSymlinkFolderTarget]) && (nil == error);
    
    error = nil;
    
    NSString *symlinkFilePath = [[NSFileManager defaultManager] destinationOfSymbolicLinkAtPath:testSymlinkFile error:&error];
    BOOL symbolicLinkToFilePersists = ((nil != symlinkFilePath) && [symlinkFilePath isEqualToString:testSymlinkFileTarget]) && (nil == error);
    
    XCTAssertTrue(symbolicLinkToFilePersists && symbolicLinkToFolderPersists, @"Relative symbolic links should persist from the original archive to the outputted files (and also remain relative).");
}

- (void)testZippingAndUnzippingForDate {
    NSString *inputPath = [self _cachesPath:@"Regular"];
    NSArray *inputPaths = @[[inputPath stringByAppendingPathComponent:@"Readme.markdown"]];
    
    NSDictionary *originalFileAttributes = [[NSFileManager defaultManager] attributesOfItemAtPath:[inputPath stringByAppendingPathComponent:@"Readme.markdown"] error:nil];
    
    NSString *outputPath = [self _cachesPath:@"ZippedDate"];
    NSString *archivePath = [outputPath stringByAppendingPathComponent:@"CreatedArchive.zip"];
    
    [Main
     createZipFileAtPath:archivePath
     withFilesAtPaths:inputPaths];
    
    [Main unzipFileAtPath:archivePath
            toDestination:outputPath
                 delegate:self];
    
    NSDictionary *createdFileAttributes = [[NSFileManager defaultManager] attributesOfItemAtPath:[outputPath stringByAppendingPathComponent:@"Readme.markdown"] error:nil];
    
    XCTAssertEqualObjects(originalFileAttributes[NSFileCreationDate], createdFileAttributes[@"NSFileCreationDate"], @"Orginal file creationDate should match created one");
}

- (void)testZippingAndUnzippingForPermissions {
    // File we're going to test permissions on before and after zipping
    NSString *targetFile = @"/Contents/MacOS/TestProject";
    
    // ZIPPING
    NSString *inputFile = [[NSBundle mainBundle] pathForResource:@"PermissionsTestApp" ofType:@"app"];
    NSString *targetFilePreZipPath = [inputFile stringByAppendingPathComponent:targetFile];
    NSDictionary *preZipAttributes = [[NSFileManager defaultManager] attributesOfItemAtPath:targetFilePreZipPath error:nil];
    
    NSString *outputDirectory = [self _cachesPath:@"PermissionsTest"];
    NSString *archivePath = [outputDirectory stringByAppendingPathComponent:@"TestAppArchive.zip"];
    
    [Main createZipFileAtPath:archivePath
      withContentsOfDirectory:inputFile];
    
    // UNZIPPING
    [Main unzipFileAtPath:archivePath
            toDestination:outputDirectory];
    
    NSString *targetFilePath = [outputDirectory stringByAppendingPathComponent:@"/Contents/MacOS/TestProject"];
    NSDictionary *fileAttributes = [[NSFileManager defaultManager] attributesOfItemAtPath:targetFilePath error:nil];
    
    XCTAssertEqual(fileAttributes[NSFilePosixPermissions], preZipAttributes[NSFilePosixPermissions], @"File permissions should be retained during compression and de-compression");
}

- (void)testUnzippingWithCancel {
    NSString *zipPath = [[NSBundle bundleForClass:[self class]] pathForResource:@"TestArchive" ofType:@"zip"];
    NSString *outputPath = [self _cachesPath:@"Cancel1"];
    
    CancelDelegate *delegate = [[CancelDelegate alloc] init];
    delegate.numberOfFilesToUnzip = 1;
    
    [Main unzipFileAtPath:zipPath
            toDestination:outputPath
                 delegate:delegate];
    
    XCTAssertEqual(delegate.numberOfFilesUnzipped, 1);
    XCTAssertFalse(delegate.didUnzipArchive);
    XCTAssertNotEqual(delegate.loaded, delegate.total);
    
    outputPath = [self _cachesPath:@"Cancel2"];
    
    delegate = [[CancelDelegate alloc] init];
    delegate.numberOfFilesToUnzip = 1000;
    
    [Main unzipFileAtPath:zipPath
            toDestination:outputPath
                 delegate:delegate];
    
    XCTAssertEqual(delegate.numberOfFilesUnzipped, 2);
    XCTAssertTrue(delegate.didUnzipArchive);
    XCTAssertEqual(delegate.loaded, delegate.total);
}

// Commented out to avoid checking in several gig file into the repository. Simply add a file named
// `LargeArchive.zip` to the project and uncomment out these lines to test.
- (void)testUnzippingLargeFiles {
    NSString *zipPath = [[NSBundle bundleForClass:[self class]] pathForResource:@"LargeArchive" ofType:@"zip"];
    NSString *outputPath = [self _cachesPath:@"Large"];
    
    [Main unzipFileAtPath:zipPath
            toDestination:outputPath];
}

- (void)testShouldProvidePathOfUnzippedFileInDelegateCallback {
    CollectingDelegate *collector = [CollectingDelegate new];
    NSString *zipPath = [[NSBundle bundleForClass:[self class]] pathForResource:@"TestArchive" ofType:@"zip"];
    NSString *outputPath = [self _cachesPath:@"Regular"];
    
    [Main unzipFileAtPath:zipPath
            toDestination:outputPath
                 delegate:collector];
}

#pragma mark - ZipArchiveDelegate
- (void)zipArchiveWillUnzipArchiveAtPath:(NSString *)path
                          zipInformation:(unz_global_info)zipInformation {
    NSLog(@"zipArchiveWillUnzipArchiveAtPath: %@ zipInformation:", path);
}

- (void)zipArchiveDidUnzipArchiveAtPath:(NSString *)path
                         zipInformation:(unz_global_info)zipInformation
                           unzippedPath:(NSString *)unzippedPath {
    NSLog(@"zipArchiveDidUnzipArchiveAtPath: %@ zipInformation: unzippedPath: %@", path, unzippedPath);
}

- (BOOL)zipArchiveShouldUnzipFileAtIndex:(NSInteger)fileIndex
                              totalFiles:(NSInteger)totalFiles
                             archivePath:(NSString *)archivePath
                         fileInformation:(unz_file_info)fileInformation {
    NSLog(@"zipArchiveShouldUnzipFileAtIndex: %d totalFiles: %d archivePath: %@ fileInformation:", (int)fileIndex, (int)totalFiles, archivePath);
    
    return YES;
}

- (void)zipArchiveWillUnzipFileAtIndex:(NSInteger)fileIndex
                            totalFiles:(NSInteger)totalFiles
                           archivePath:(NSString *)archivePath
                       fileInformation:(unz_file_info)fileInformation {
    NSLog(@"zipArchiveWillUnzipFileAtIndex: %d totalFiles: %d archivePath: %@ fileInformation:", (int)fileIndex, (int)totalFiles, archivePath);
}

- (void)zipArchiveDidUnzipFileAtIndex:(NSInteger)fileIndex
                           totalFiles:(NSInteger)totalFiles
                          archivePath:(NSString *)archivePath
                      fileInformation:(unz_file_info)fileInformation {
    NSLog(@"zipArchiveDidUnzipFileAtIndex: %d totalFiles: %d archivePath: %@ fileInformation:", (int)fileIndex, (int)totalFiles, archivePath);
}

- (void)zipArchiveProgressEvent:(unsigned long long)loaded
                          total:(unsigned long long)total {
    NSLog(@"zipArchiveProgressEvent: loaded: %d total: %d", (int)loaded, (int)total);
    [progressEvents addObject:@(loaded)];
}

#pragma mark - Private
- (NSString *)_cachesPath:(NSString *)directory {
    NSString *path = [[NSSearchPathForDirectoriesInDomains(NSCachesDirectory, NSUserDomainMask, YES) lastObject]
                      stringByAppendingPathComponent:@"com.SamSoffes.ZipArchive.tests"];
    if (directory) {
        path = [path stringByAppendingPathComponent:directory];
    }
    
    NSFileManager *fileManager = [NSFileManager defaultManager];
    if (![fileManager fileExistsAtPath:path]) {
        [fileManager createDirectoryAtPath:path withIntermediateDirectories:YES attributes:nil error:nil];
    }
    
    return path;
}

- (NSString *)_calculateMD5Digest:(NSData *)data {
    unsigned char digest[CC_MD5_DIGEST_LENGTH], i;
    CC_MD5(data.bytes, (unsigned int)data.length, digest);
    NSMutableString *string = [NSMutableString string];
    
    for (i = 0; i < CC_MD5_DIGEST_LENGTH; i++) {
        [string appendFormat:@"%02x", (int)(digest[i])];
    }
    
    return [string copy];
}

@end