//
//  SSZipArchiveTests.m
//  ZipArchive-iOS
//
//  Created by Joel Fischer (joelfischerr) on 03.07.20.
//  Copyright © 2020 Joel Fischer. All rights reserved.
//

#import <XCTest/XCTest.h>
#import "SSZipArchive.h"

@interface SSZipArchiveTests : XCTestCase

@end

@implementation SSZipArchiveTests

int twentyMb = 20 * 1024 * 1024;
NSString *filenName = @"testFile.zip";

- (void)setUp {
    // Put setup code here. This method is called before the invocation of each test method in the class.
}

// Removes all temporary files. All paths are hardcoded. Does not discover new temporary files automatically.
- (void)tearDown {
    // Put teardown code here. This method is called after the invocation of each test method in the class.
    
    NSString *filePath = [[[NSBundle bundleForClass:[self class]] resourcePath] stringByAppendingPathComponent:filenName];
    [[NSFileManager defaultManager] removeItemAtPath:filePath error:nil];
    
    filePath = [[[NSBundle bundleForClass:[self class]] resourcePath] stringByAppendingPathComponent:@"Unpacked"];
    [[NSFileManager defaultManager] removeItemAtPath:filePath error:nil];
    
}

// This tests whether the payload size of the zip file containing 6Gb of files with compression 0 is correct.
- (void)testPayloadSizeCorrect {
    long long int iterations = 300;
    
    NSNumber *goldenSize = [NSNumber numberWithLongLong:iterations * twentyMb];
    NSData *data = [self get20MbNSData];
    NSString *filePath = [[[NSBundle bundleForClass:[self class]] resourcePath] stringByAppendingPathComponent:filenName];
    
    SSZipArchive *archive = [[SSZipArchive alloc] initWithPath:filePath];
    [archive open];
    
    for (int i = 0; i < iterations; i++) {
        NSString *fileName = [NSString stringWithFormat:@"File_%i", i];
        [archive writeData:data filename:fileName compressionLevel:0 password:@"TestPW" AES:true];
    }
    
    bool close = [archive close];
    
    NSNumber *fileSize = [SSZipArchive payloadSizeForArchiveAtPath:filePath error:nil];
    
    XCTAssertTrue(fileSize.longLongValue == goldenSize.longLongValue,
                  "Payload size should be equal to the sum of the size of all files included.");
    XCTAssertTrue(close, "Should be able to close the archive.");
}

// This creates a zip file containing 300 20Mb sized files with multiple compression levels. It then unpacks the file and checks whether
// the same number of files is present in the unpacked folder.
- (void)testAllFilesPresent {
    int iterations = 300;
    NSString *unpackPath = [[[NSBundle bundleForClass:[self class]] resourcePath] stringByAppendingPathComponent:@"Unpacked/testFile"];
    
    NSArray *compressionLevels = [NSArray arrayWithObjects:
                                  [NSNumber numberWithFloat:0],
                                  [NSNumber numberWithFloat:0.5],
                                  [NSNumber numberWithFloat:1],
                                  nil];

    for (NSNumber *compressionLevel in compressionLevels) {
        NSData *data = [self get20MbNSData];
        NSString *filePath = [[[NSBundle bundleForClass:[self class]] resourcePath] stringByAppendingPathComponent:filenName];
        
        SSZipArchive *archive = [[SSZipArchive alloc] initWithPath:filePath];
        [archive open];
        
        for (int i = 0; i < iterations; i++) {
            NSString *fileName = [NSString stringWithFormat:@"File_%i", i];
            [archive writeData:data filename:fileName compressionLevel:compressionLevel.floatValue password:@"TestPW" AES:true];
        }
        
        bool close = [archive close];
        
        [SSZipArchive unzipFileAtPath:filePath toDestination:unpackPath overwrite:true password:@"TestPW" error:nil];
        
        long int noFiles = [[NSFileManager defaultManager] contentsOfDirectoryAtPath:unpackPath error:nil].count;
        
        XCTAssertTrue(close, "Should be able to close the archive.");
        XCTAssertTrue(iterations == noFiles, "All files should be present in the exported directory");
    }
}

// Returns 20Mb of random data
-(NSData*)get20MbNSData {
  NSMutableData* theData = [NSMutableData dataWithCapacity:twentyMb];
  for (long long int i = 0; i < twentyMb/4; i++) {
    u_int32_t randomBits = arc4random();
    [theData appendBytes:(void *)&randomBits length:4];
  }

  return theData;
}

@end
