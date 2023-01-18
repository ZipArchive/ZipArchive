//
//  SSZipEntry.h
//  ZipArchive
//
//  Created by Evgeniy Karpov on 18.01.2023.
//  Copyright © 2023 smumryak. All rights reserved.
//

#ifndef SSZipEntry_h
#define SSZipEntry_h

#import <Foundation/Foundation.h>

@interface SSZipEntry : NSObject

@property (nonatomic, strong, readonly) NSString *path;
@property (nonatomic, readonly) NSUInteger uncompressedSize;

- (instancetype)initWithPath:(NSString *)path uncompressedSize:(NSUInteger)uncompressedSize;

@end

#endif /* SSZipEntry_h */
