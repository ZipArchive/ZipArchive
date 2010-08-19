//
//  SSZipArchive.m
//  SSZipArchive
//
//  Created by Sam Soffes on 7/21/10.
//  Copyright Sam Soffes 2010. All rights reserved.
//

#import "SSZipArchive.h"
#include "minizip/zip.h"
#include "minizip/unzip.h"
#import "zlib.h"
#import "zconf.h"

@interface SSZipArchive (PrivateMethods)
+ (NSDate *)_dateFor1980;
@end


@implementation SSZipArchive

+ (BOOL)unzipFileAtPath:(NSString *)path toDestination:(NSString *)destination {
	return [self unzipFileAtPath:path toDestination:destination overwrite:YES password:nil error:nil];
}

+ (BOOL)unzipFileAtPath:(NSString *)path toDestination:(NSString *)destination overwrite:(BOOL)overwrite password:(NSString *)password error:(NSError **)error {
	// Begin opening
	zipFile zip = unzOpen((const char*)[path UTF8String]);	
	if (zip == NULL) {
		NSDictionary *userInfo = [NSDictionary dictionaryWithObject:@"failed to open zip file" forKey:NSLocalizedDescriptionKey];
		if (error) {
			*error = [NSError errorWithDomain:@"SSZipArchiveErrorDomain" code:-1 userInfo:userInfo];
		}
		return NO;
	}
	
	unz_global_info  globalInfo = {0};
	unzGetGlobalInfo(zip, &globalInfo);
	
	// Begin unzipping
	if (unzGoToFirstFile(zip) != UNZ_OK) {
		NSDictionary *userInfo = [NSDictionary dictionaryWithObject:@"failed to open first file in zip file" forKey:NSLocalizedDescriptionKey];
		if (error) {
			*error = [NSError errorWithDomain:@"SSZipArchiveErrorDomain" code:-2 userInfo:userInfo];
		}
		return NO;
	}
	
	BOOL success = YES;
	int ret;
	unsigned char buffer[4096] = {0};
	NSFileManager *fileManager = [NSFileManager defaultManager];
	NSDate *nineteenEighty = [self _dateFor1980];
	
	do {
		if ([password length] == 0) {
			ret = unzOpenCurrentFile(zip);
		} else {
			ret = unzOpenCurrentFilePassword(zip, [password cStringUsingEncoding:NSASCIIStringEncoding]);
		}
		
		if (ret != UNZ_OK) {
			success = NO;
			break;
		}
		
		// Reading data and write to file
		int read;
		unz_file_info fileInfo = {0};
		ret = unzGetCurrentFileInfo(zip, &fileInfo, NULL, 0, NULL, 0, NULL, 0);
		if (ret != UNZ_OK) {
			success = NO;
			unzCloseCurrentFile(zip);
			break;
		}
		
		char *filename = (char *)malloc(fileInfo.size_filename + 1);
		unzGetCurrentFileInfo(zip, &fileInfo, filename, fileInfo.size_filename + 1, NULL, 0, NULL, 0);
		filename[fileInfo.size_filename] = '\0';
		
		// Check if it contains directory
		NSString *strPath = [NSString stringWithCString:filename encoding:NSUTF8StringEncoding];
		BOOL isDirectory = NO;
		if (filename[fileInfo.size_filename-1] == '/' || filename[fileInfo.size_filename-1] == '\\') {
			isDirectory = YES;
		}
		free(filename);
		
		// Contains a path
		if ([strPath rangeOfCharacterFromSet:[NSCharacterSet characterSetWithCharactersInString:@"/\\"]].location != NSNotFound) {
			strPath = [strPath stringByReplacingOccurrencesOfString:@"\\" withString:@"/"];
		}
		
		NSString* fullPath = [destination stringByAppendingPathComponent:strPath];
		
		if (isDirectory) {
			[fileManager createDirectoryAtPath:fullPath withIntermediateDirectories:YES attributes:nil error:nil];
		} else {
			[fileManager createDirectoryAtPath:[fullPath stringByDeletingLastPathComponent] withIntermediateDirectories:YES attributes:nil error:nil];
		}
		
		if ([fileManager fileExistsAtPath:fullPath] && !isDirectory && !overwrite) {
			unzCloseCurrentFile(zip);
			ret = unzGoToNextFile(zip);
			continue;
		}
		
		FILE *fp = fopen((const char*)[fullPath UTF8String], "wb");
		while (fp) {
			read = unzReadCurrentFile(zip, buffer, 4096);

			if (read > 0) {
				fwrite(buffer, read, 1, fp );
			} else {
				break;
			}
		}
		
		if (fp) {
			fclose(fp);
			
			// Set the orignal datetime property
			if (fileInfo.dosDate != 0) {
				NSDate *orgDate = [[NSDate alloc] initWithTimeInterval:(NSTimeInterval)fileInfo.dosDate  sinceDate:nineteenEighty];
				NSDictionary *attr = [NSDictionary dictionaryWithObject:orgDate forKey:NSFileModificationDate];
				
				if (attr) {
					if ([fileManager setAttributes:attr ofItemAtPath:fullPath error:nil] == NO) {
						// Can't set attributes 
						NSLog(@"Failed to set attributes");
					}
				}
				[orgDate release];
			}
		}
		
		unzCloseCurrentFile( zip );
		ret = unzGoToNextFile( zip );
	} while(ret == UNZ_OK && UNZ_OK != UNZ_END_OF_LIST_OF_FILE);
	
	// Close
	unzClose(zip);
	
	return success;
}


+ (NSDate *)_dateFor1980 {
	NSDateComponents *comps = [[NSDateComponents alloc] init];
	[comps setDay:1];
	[comps setMonth:1];
	[comps setYear:1980];
	NSCalendar *gregorian = [[NSCalendar alloc] initWithCalendarIdentifier:NSGregorianCalendar];
	NSDate *date = [gregorian dateFromComponents:comps];
	
	[comps release];
	[gregorian release];
	return date;
}

@end
