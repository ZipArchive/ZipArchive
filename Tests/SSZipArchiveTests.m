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
- (NSString *)_createOutputPath;
@end

@implementation SSZipArchiveTests

- (void)testBasicZipping {
	NSString *outputPath = [self _createOutputPath];
    
    NSArray *inputPaths = [NSArray arrayWithObjects:
                           [outputPath stringByAppendingPathComponent:@"Readme.markdown"],
                           [outputPath stringByAppendingPathComponent:@"LICENSE"],
                           nil];
    NSString *archivePath = [outputPath stringByAppendingPathComponent:@"CreatedArchive.zip"];
    [SSZipArchive createZipFileAtPath:archivePath withFilesAtPaths:inputPaths];
 
	NSFileManager *fileManager = [NSFileManager defaultManager];        
	STAssertTrue([fileManager fileExistsAtPath:archivePath], @"Archive created");    
}

- (void)testBasicUnzipping {
	NSString *zipPath = [[NSBundle bundleForClass:[self class]] pathForResource:@"TestArchive" ofType:@"zip"];
	NSString *outputPath = [self _createOutputPath];
	
	[SSZipArchive unzipFileAtPath:zipPath toDestination:outputPath];
	
	NSFileManager *fileManager = [NSFileManager defaultManager];    
	NSString *testPath = [outputPath stringByAppendingPathComponent:@"Readme.markdown"];
	STAssertTrue([fileManager fileExistsAtPath:testPath], @"Readme unzipped");
	
	testPath = [outputPath stringByAppendingPathComponent:@"LICENSE"];
	STAssertTrue([fileManager fileExistsAtPath:testPath], @"LICENSE unzipped");
	
//	zipPath = [[NSBundle bundleForClass:[self class]] pathForResource:@"LargeArchive" ofType:@"zip"];
//	outputPath = [[self _cachesPath] stringByAppendingPathComponent:@"large"];
//	[SSZipArchive unzipFileAtPath:zipPath toDestination:outputPath];
}


#pragma mark - Private

- (NSString *)_createOutputPath {
	NSString *outputPath = [[self _cachesPath] stringByAppendingPathComponent:@"basic"];
    
	NSFileManager *fileManager = [NSFileManager defaultManager];
	if (![fileManager fileExistsAtPath:outputPath]) {
		[fileManager createDirectoryAtPath:outputPath withIntermediateDirectories:YES attributes:nil error:nil];
	}
    
//    NSLog(@"outputPath: %@", outputPath);
    
    return outputPath;
}

- (NSString *)_cachesPath {
	return [[NSSearchPathForDirectoriesInDomains(NSCachesDirectory, NSUserDomainMask, YES) lastObject]
			stringByAppendingPathComponent:@"com.samsoffes.ssziparchive.tests"];
}

@end
