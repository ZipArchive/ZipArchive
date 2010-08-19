//
//  SSZipArchive.h
//  SSZipArchive
//
//  Created by Sam Soffes on 7/21/10.
//  Copyright Sam Soffes 2010. All rights reserved.
//
//  Based on ZipArchive by aish
//  http://code.google.com/p/ziparchive
//

@interface SSZipArchive : NSObject {

}

+ (BOOL)unzipFileAtPath:(NSString *)path toDestination:(NSString *)destination;
+ (BOOL)unzipFileAtPath:(NSString *)path toDestination:(NSString *)destination overwrite:(BOOL)overwrite password:(NSString *)password error:(NSError **)error;

@end
