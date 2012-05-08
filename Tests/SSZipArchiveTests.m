//
//  SSZipArchiveTests.m
//  SSZipArchiveTests
//
//  Created by Sam Soffes on 10/3/11.
//  Copyright (c) 2011 Sam Soffes. All rights reserved.
//

#import "SSZipArchiveTests.h"
#import "SSZipArchive.h"

@interface SSZipArchiveTests () <SSZipArchiveDelegate>
- (NSString *)_cachesPath;
@end

@implementation SSZipArchiveTests

- (void)testZipping {
	NSString *outputPath = [self _cachesPath];
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
	NSString *outputPath = [self _cachesPath];
	
	[SSZipArchive unzipFileAtPath:zipPath toDestination:outputPath delegate:self];
	
	NSFileManager *fileManager = [NSFileManager defaultManager];
	NSString *testPath = [outputPath stringByAppendingPathComponent:@"Readme.markdown"];
	STAssertTrue([fileManager fileExistsAtPath:testPath], @"Readme unzipped");
	
	testPath = [outputPath stringByAppendingPathComponent:@"LICENSE"];
	STAssertTrue([fileManager fileExistsAtPath:testPath], @"LICENSE unzipped");
		
	// Commented out to avoid checking in several gig file into the repository. Simply add a file named
	// `LargeArchive.zip` to the project and uncomment out these lines to test.
//	zipPath = [[NSBundle bundleForClass:[self class]] pathForResource:@"LargeArchive" ofType:@"zip"];
//	outputPath = [[self _cachesPath] stringByAppendingPathComponent:@"large"];
//	[SSZipArchive unzipFileAtPath:zipPath toDestination:outputPath];
}


#pragma mark - Delegate

- (void)zipArchiveWillUnzipArchiveAtPath:(NSString *)path zipInfo:(unz_global_info)zipInfo {
	NSLog(@"zipArchiveWillUnzipArchiveAtPath: `%@` zipInfo:", path);
}


- (void)zipArchiveDidUnzipArchiveAtPath:(NSString *)path zipInfo:(unz_global_info)zipInfo unzippedPath:(NSString *)unzippedPath {
	NSLog(@"zipArchiveDidUnzipArchiveAtPath: `%@` zipInfo: unzippedPath: `%@`", path, unzippedPath);
}


- (void)zipArchiveWillUnzipFileAtIndex:(NSInteger)fileIndex totalFiles:(NSInteger)totalFiles archivePath:(NSString *)archivePath fileInfo:(unz_file_info)fileInfo {
	NSLog(@"zipArchiveWillUnzipFileAtIndex: `%ld` totalFiles: `%ld` archivePath: `%@` fileInfo:", fileIndex, totalFiles, archivePath);
}


- (void)zipArchiveDidUnzipFileAtIndex:(NSInteger)fileIndex totalFiles:(NSInteger)totalFiles archivePath:(NSString *)archivePath fileInfo:(unz_file_info)fileInfo {
	NSLog(@"zipArchiveDidUnzipFileAtIndex: `%ld` totalFiles: `%ld` archivePath: `%@` fileInfo:", fileIndex, totalFiles, archivePath);
}


#pragma mark - Private

- (NSString *)_cachesPath {
	return [[NSSearchPathForDirectoriesInDomains(NSCachesDirectory, NSUserDomainMask, YES) lastObject]
			stringByAppendingPathComponent:@"com.samsoffes.ssziparchive.tests"];
}

@end
