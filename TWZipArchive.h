//
//  TWZipArchive.h
//  TWZipArchive
//
//  Created by Sam Soffes on 7/21/10.
//  Copyright Tasteful Works 2010. All rights reserved.
//
//  Based on ZipArchive by aish
//  http://code.google.com/p/ziparchive
//

@interface TWZipArchive : NSObject {

}

+ (BOOL)unzipFileAtPath:(NSString *)path toDestination:(NSString *)destination;
+ (BOOL)unzipFileAtPath:(NSString *)path toDestination:(NSString *)destination overwrite:(BOOL)overwrite password:(NSString *)password;

@end
