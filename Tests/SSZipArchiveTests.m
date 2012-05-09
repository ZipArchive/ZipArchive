//
//  SSZipArchiveTests.m
//  SSZipArchiveTests
//
//  Created by Sam Soffes on 10/3/11.
//  Copyright (c) 2011 Sam Soffes. All rights reserved.
//

#import "SSZipArchive.h"
#import <SenTestingKit/SenTestingKit.h>
#import <CommonCrypto/CommonDigest.h>

@interface SSZipArchiveTests : SenTestCase <SSZipArchiveDelegate>

- (NSString *)_cachesPath:(NSString *)directory;

- (NSString *)_calculateMD5Digest:(NSData *)data;

@end

@implementation SSZipArchiveTests

- (void)setUp {
	[[NSFileManager defaultManager] removeItemAtPath:[self _cachesPath:nil] error:nil];
}


- (void)testZipping {
	NSString *outputPath = [self _cachesPath:@"Zipped"];
	NSArray *inputPaths = [NSArray arrayWithObjects:
						   [outputPath stringByAppendingPathComponent:@"Readme.markdown"],
						   [outputPath stringByAppendingPathComponent:@"LICENSE"],
	nil];
	NSString *archivePath = [outputPath stringByAppendingPathComponent:@"CreatedArchive.zip"];
	[SSZipArchive createZipFileAtPath:archivePath withFilesAtPaths:inputPaths];

	// TODO: Make sure the files are actually unzipped. They are, but the test should be better.
	STAssertTrue([[NSFileManager defaultManager] fileExistsAtPath:archivePath], @"Archive created");
}


- (void)testUnzipping {
	NSString *zipPath = [[NSBundle bundleForClass:[self class]] pathForResource:@"TestArchive" ofType:@"zip"];
	NSString *outputPath = [self _cachesPath:@"Regular"];
	
	[SSZipArchive unzipFileAtPath:zipPath toDestination:outputPath delegate:self];
	
	NSFileManager *fileManager = [NSFileManager defaultManager];
	NSString *testPath = [outputPath stringByAppendingPathComponent:@"Readme.markdown"];
	STAssertTrue([fileManager fileExistsAtPath:testPath], @"Readme unzipped");
	
	testPath = [outputPath stringByAppendingPathComponent:@"LICENSE"];
	STAssertTrue([fileManager fileExistsAtPath:testPath], @"LICENSE unzipped");
}


- (void)testUnzippingWithPassword {
	NSString *zipPath = [[NSBundle bundleForClass:[self class]] pathForResource:@"TestPasswordArchive" ofType:@"zip"];
	NSString *outputPath = [self _cachesPath:@"Password"];
	
	NSError *error = nil;
	[SSZipArchive unzipFileAtPath:zipPath toDestination:outputPath overwrite:YES password:@"passw0rd" error:&error delegate:self];
	
	NSFileManager *fileManager = [NSFileManager defaultManager];
	NSString *testPath = [outputPath stringByAppendingPathComponent:@"Readme.markdown"];
	STAssertTrue([fileManager fileExistsAtPath:testPath], @"Readme unzipped");
	
	testPath = [outputPath stringByAppendingPathComponent:@"LICENSE"];
	STAssertTrue([fileManager fileExistsAtPath:testPath], @"LICENSE unzipped");
}

- (void)testUnzippingTruncatedFileFix {
    NSString* zipPath = [[NSBundle bundleForClass:[self class]] pathForResource:@"IncorrectHeaders" ofType:@"zip"];
    NSString* outputPath = [self _cachesPath:@"IncorrectHeaders"];
    
    [SSZipArchive unzipFileAtPath:zipPath toDestination:outputPath delegate:self];
    
    NSString* intendedReadmeTxtMD5 = @"31ac96301302eb388070c827447290b5";
    
    NSString* filePath = [outputPath stringByAppendingPathComponent:@"IncorrectHeaders/Readme.txt"];
    NSData* data = [NSData dataWithContentsOfFile:filePath];
    
    NSString* actualReadmeTxtMD5 = [self _calculateMD5Digest:data];
    STAssertTrue([actualReadmeTxtMD5 isEqualToString:intendedReadmeTxtMD5], @"Readme.txt MD5 digest should match original.");
}


// Commented out to avoid checking in several gig file into the repository. Simply add a file named
// `LargeArchive.zip` to the project and uncomment out these lines to test.
//
//- (void)testUnzippingLargeFiles {
//	NSString *zipPath = [[NSBundle bundleForClass:[self class]] pathForResource:@"LargeArchive" ofType:@"zip"];
//	NSString *outputPath = [self _cachesPath:@"Large"];
//	
//	[SSZipArchive unzipFileAtPath:zipPath toDestination:outputPath];
//}


#pragma mark - SSZipArchiveDelegate

- (void)zipArchiveWillUnzipArchiveAtPath:(NSString *)path zipInfo:(unz_global_info)zipInfo {
	NSLog(@"*** zipArchiveWillUnzipArchiveAtPath: `%@` zipInfo:", path);
}


- (void)zipArchiveDidUnzipArchiveAtPath:(NSString *)path zipInfo:(unz_global_info)zipInfo unzippedPath:(NSString *)unzippedPath {
	NSLog(@"*** zipArchiveDidUnzipArchiveAtPath: `%@` zipInfo: unzippedPath: `%@`", path, unzippedPath);
}


- (void)zipArchiveWillUnzipFileAtIndex:(NSInteger)fileIndex totalFiles:(NSInteger)totalFiles archivePath:(NSString *)archivePath fileInfo:(unz_file_info)fileInfo {
	NSLog(@"*** zipArchiveWillUnzipFileAtIndex: `%ld` totalFiles: `%ld` archivePath: `%@` fileInfo:", fileIndex, totalFiles, archivePath);
}


- (void)zipArchiveDidUnzipFileAtIndex:(NSInteger)fileIndex totalFiles:(NSInteger)totalFiles archivePath:(NSString *)archivePath fileInfo:(unz_file_info)fileInfo {
	NSLog(@"*** zipArchiveDidUnzipFileAtIndex: `%ld` totalFiles: `%ld` archivePath: `%@` fileInfo:", fileIndex, totalFiles, archivePath);
}


#pragma mark - Private

- (NSString *)_cachesPath:(NSString *)directory {
	NSString *path = [[NSSearchPathForDirectoriesInDomains(NSCachesDirectory, NSUserDomainMask, YES) lastObject]
					   stringByAppendingPathComponent:@"com.samsoffes.ssziparchive.tests"];
	if (directory) {
		path = [path stringByAppendingPathComponent:directory];
	}
	
	NSFileManager *fileManager = [NSFileManager defaultManager];
	if (![fileManager fileExistsAtPath:path]) {
		[fileManager createDirectoryAtPath:path withIntermediateDirectories:YES attributes:nil error:nil];
	}
	
	return path;
}

- (NSString *)_calculateMD5Digest:(NSData *)data
{
    unsigned char buffer[CC_MD5_DIGEST_LENGTH];
    CC_MD5([data bytes], [data length], buffer);
    
    NSMutableString* digest = [NSMutableString string];
    
    for(int i = 0; i < CC_MD5_DIGEST_LENGTH; ++i)
        [digest appendFormat:@"%02x", buffer[i]];
    
    return digest;
}

@end
