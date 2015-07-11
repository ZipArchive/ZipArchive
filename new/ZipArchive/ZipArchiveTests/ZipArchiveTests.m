//
//  ZipArchiveTests.m
//  ZipArchiveTests
//
//  Created by Sam Soffes on 2010-03-11.
//  Copyright (c) 2015 ZipArchive. All rights reserved.
//

#import "Main.h"
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
    _numberOfFilesUnzipped = fileIndex + 1;
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
    [Main createZipFileAtPath:archivePath withFilesAtPaths:inputPaths];
    
    // TODO: - Make sure the files are actually unzipped. They are, but the test could be better.
    XCTAssertTrue([[NSFileManager defaultManager] fileExistsAtPath:archivePath], @"Archive Created.");
}

- (void)testDirectoryZipping {
    NSString *inputPath = [self _cachesPath:@"Unicode"];
    NSString *outputPath = [self _cachesPath:@"FolderZipped"];
    NSString *archivePath = [outputPath stringByAppendingPathComponent:@"ArchiveWithFolders.zip"];
    [Main createZipFileAtPath:archivePath withContentsOfDirectory:inputPath];
    
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
        [Main createZipFileAtPath:archivePath withFilesAtPaths:inputPaths];
        
        long long threshold = 510000;
        long long fileSize = [[[NSFileManager defaultManager] attributesOfItemAtPath:archivePath error:nil][NSFileSize] longLongValue];
        
        XCTAssertTrue(fileSize > threshold, @"Zipping failed at %lld", fileSize);
    }
}

- (void)testUnzipping {
    NSString *zipPath = [[NSBundle bundleForClass:[self class]] pathForResource:@"TestArchive" ofType:@"zip"];
    NSString *outputPath = [self _cachesPath:@"Regular"];
    [Main unzipFileAtPath:zipPath toDestination:outputPath delegate:self];
    
    NSFileManager *fileManager = [NSFileManager defaultManager];
    NSString *testPath = [outputPath stringByAppendingPathComponent:@"Readme.markdown"];
    XCTAssertTrue([fileManager fileExistsAtPath:testPath], @"README Unzipped.");
    
    testPath = [outputPath stringByAppendingPathComponent:@"LICENSE"];
    XCTAssertTrue([fileManager fileExistsAtPath:testPath], @"LICENSE Unzipped.");
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

- (void)testSmallFileUnzipping {
    NSString *zipPath = [[NSBundle mainBundle] pathForResource:@"hello" ofType:@"zip"];
    NSString *outputPath = [self _cachesPath:@"Regular"];
    [Main unzipFileAtPath:zipPath toDestination:outputPath delegate:self];
    
    NSFileManager *fileManager = [NSFileManager defaultManager];
    NSString *testPath = [outputPath stringByAppendingPathComponent:@"REadme.markdown"];
    XCTAssertTrue([fileManager fileExistsAtPath:testPath], @"README Unzipped");
    
    testPath = [outputPath stringByAppendingPathComponent:@"LICENSE"];
    XCTAssertTrue([fileManager fileExistsAtPath:testPath], @"LICENSE Unzipped");
}

@end