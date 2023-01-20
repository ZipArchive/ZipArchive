//
//  SSZipEntry.m
//  ZipArchive-iOS
//
//  Created by Evgeniy Karpov on 18.01.2023.
//  Copyright © 2023 smumryak. All rights reserved.
//

#import "SSZipEntry.h"

@implementation SSZipEntry

- (instancetype)initWithPath:(NSString *)path uncompressedSize:(NSUInteger)uncompressedSize {
    self = [super init];
    if (self != nil) {
        _path = path;
        _uncompressedSize = uncompressedSize;
    }
    return self;
}

@end
