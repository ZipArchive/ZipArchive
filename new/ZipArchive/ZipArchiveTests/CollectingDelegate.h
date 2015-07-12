//
//  CollectingDelegate.h
//  ZipArchive
//
//  Originally Created by Chris on 2012-01-12.
//  Copyright (c) 2015 ZipArchive. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "Main.h"

/**
 * Test delegate by collecting its calls
 */

@interface CollectingDelegate : NSObject <ZipArchiveDelegate>

@property(nonatomic, retain) NSMutableArray *files;

@end