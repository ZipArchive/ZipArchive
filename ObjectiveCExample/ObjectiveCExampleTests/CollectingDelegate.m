//
// Created by chris on 8/1/12.
//
// To change the template use AppCode | Preferences | File Templates.
//


#import "CollectingDelegate.h"

@implementation CollectingDelegate {
    
}
@synthesize files = _files;


- (instancetype)init {
    self = [super init];
    if (self) {
        self.files = [NSMutableArray array];
    }
    return self;
}

- (void)zipArchiveDidUnzipArchiveFile:(NSString *)zipFile entryPath:(NSString *)entryPath destPath:(NSString *)destPath {
    [self.files addObject:entryPath];
}




@end