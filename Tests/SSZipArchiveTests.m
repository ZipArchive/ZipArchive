//
//  SSZipArchiveTests.m
//  SSZipArchiveTests
//
//  Created by Sam Soffes on 10/3/11.
//  Copyright (c) 2011 Sam Soffes. All rights reserved.
//

#import "SSZipArchiveTests.h"
#import "SSZipArchive.h"

@interface SSZipArchiveTests ()
- (NSString *)_cachesPath;
@end

@implementation SSZipArchiveTests

- (void)testBasicUnzipping {
	NSString *zipPath = [[NSBundle bundleForClass:[self class]] pathForResource:@"TestArchive" ofType:@"zip"];
	NSString *outputPath = [[self _cachesPath] stringByAppendingPathComponent:@"basic"];
	
	NSFileManager *fileManager = [NSFileManager defaultManager];
	if (![fileManager fileExistsAtPath:outputPath]) {
		[fileManager createDirectoryAtPath:outputPath withIntermediateDirectories:YES attributes:nil error:nil];
	}
	
	[SSZipArchive unzipFileAtPath:zipPath toDestination:outputPath];
	
	NSString *testPath = [outputPath stringByAppendingPathComponent:@"Readme.markdown"];
	STAssertTrue([fileManager fileExistsAtPath:testPath], @"Readme unzipped");
	
	testPath = [outputPath stringByAppendingPathComponent:@"LICENSE"];
	STAssertTrue([fileManager fileExistsAtPath:testPath], @"LICENSE unzipped");
	
//	zipPath = [[NSBundle bundleForClass:[self class]] pathForResource:@"LargeArchive" ofType:@"zip"];
//	outputPath = [[self _cachesPath] stringByAppendingPathComponent:@"large"];
//	[SSZipArchive unzipFileAtPath:zipPath toDestination:outputPath];
}


#pragma mark - Private

- (NSString *)_cachesPath {
	return [[NSSearchPathForDirectoriesInDomains(NSCachesDirectory, NSUserDomainMask, YES) lastObject]
			stringByAppendingPathComponent:@"com.samsoffes.ssziparchive.tests"];
}

@end
