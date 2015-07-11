//
//  CollectingDelegate.m
//  ZipArchive
//
//  Originally Created by Chris on 2012-01-12.
//  Copyright (c) 2015 ZipArchive. All rights reserved.
//

#import "CollectingDelegate.h"

@implementation CollectingDelegate
@synthesize files = _files;

- (instancetype)init {
    self = [super init];
    if (self) {
        self.files = [NSMutableArray array];
    }
    
    return self;
}

- (void)zipArchiveDidUnzipArchiveFile:(NSString *)zipFile entryPath:(NSString *)entryPath destinationPath:(NSString *)destinationPath {
    [self.files addObject:entryPath];
}

@end