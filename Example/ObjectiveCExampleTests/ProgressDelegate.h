//
//  ProgressDelegate.h
//  ObjectiveCExample
//
//  Created by Antoine Cœur on 04/10/2017.
//

#import <Foundation/Foundation.h>
#if COCOAPODS
#import <SSZipArchive/SSZipArchive.h>
#else
#import <ZipArchive/SSZipArchive.h>
#endif

@interface ProgressDelegate : NSObject <SSZipArchiveDelegate>
{
@public
    NSMutableArray *progressEvents;
}

@end
