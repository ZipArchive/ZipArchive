#import <Foundation/Foundation.h>
#import "SSZipArchive.h"

/**
 * Test delegate by collecting its calls
 */
@interface CollectingDelegate : NSObject <SSZipArchiveDelegate>
@property(nonatomic, retain) NSMutableArray *files;
@end