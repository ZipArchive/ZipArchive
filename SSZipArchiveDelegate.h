//
//  SSZipArchiveDelegate.h
//  
//
//  Created by Johannes Ekberg on 2012-01-14.
//  Copyright (c) 2012 MacaroniCode Software. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "minizip/unzip.h"
@class SSZipArchive;

@protocol SSZipArchiveDelegate <NSObject>

@optional
- (void)zipArchiveWillUnzipFile:(NSString *)path globalInfo:(unz_global_info)header;
- (void)zipArchiveWillUnzipFileNumber:(NSInteger)number outOf:(NSInteger)total fromFile:(NSString *)path fileInfo:(unz_file_info)header;

@end
