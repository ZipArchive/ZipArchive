#import <Foundation/Foundation.h>
#import <ZipArchive.h>

/**
 * Test delegate by collecting its calls
 */
@interface CollectingDelegate : NSObject <SSZipArchiveDelegate>
@property(nonatomic, retain) NSMutableArray<NSString *> *files;
@end
