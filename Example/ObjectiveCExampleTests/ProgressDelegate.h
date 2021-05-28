//
//  ProgressDelegate.h
//  ObjectiveCExample
//
//  Created by Antoine Cœur on 04/10/2017.
//

#import <Foundation/Foundation.h>

#import <ZipArchive.h>


@interface ProgressDelegate : NSObject <SSZipArchiveDelegate>
{
@public
    NSMutableArray *progressEvents;
}

@end
