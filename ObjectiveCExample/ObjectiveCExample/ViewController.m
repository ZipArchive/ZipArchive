//
//  ViewController.m
//  ObjectiveCExample
//
//  Created by Sean Soper on 10/23/15.
//
//

#import "ViewController.h"
#import "SSZipArchive.h"

@interface ViewController ()

@property (weak, nonatomic) IBOutlet UIButton *zipButton;
@property (weak, nonatomic) IBOutlet UIButton *unzipButton;
@property (weak, nonatomic) IBOutlet UIButton *resetButton;
@property (weak, nonatomic) IBOutlet UILabel *file1;
@property (weak, nonatomic) IBOutlet UILabel *file2;
@property (weak, nonatomic) IBOutlet UILabel *file3;
@property (copy, nonatomic) NSString *zipPath;

@end

@implementation ViewController

#pragma mark - Life Cycle
- (void)viewDidLoad {
  [super viewDidLoad];
    
    _file1.text = @"";
    _file2.text = @"";
    _file3.text = @"";
}

- (void)didReceiveMemoryWarning {
  [super didReceiveMemoryWarning];
}

#pragma mark - IBAction
- (IBAction)zipPressed:(id)sender {
  NSString *sampleDataPath = [[NSBundle mainBundle].bundleURL
                              URLByAppendingPathComponent:@"Sample Data"
                              isDirectory:YES].path;
  _zipPath = [self tempZipPath];
  BOOL success = [SSZipArchive createZipFileAtPath:_zipPath
                           withContentsOfDirectory:sampleDataPath];
  if (success) {
    _unzipButton.enabled = YES;
    _zipButton.enabled = NO;
  }
}

- (IBAction)unzipPressed:(id)sender {
  if (!_zipPath) {
    return;
  }
  NSString *unzipPath = [self tempUnzipPath];
  if (!unzipPath) {
    return;
  }
  BOOL success = [SSZipArchive unzipFileAtPath:_zipPath
                                 toDestination:unzipPath];
  if (!success) {
    return;
  }
  NSError *error = nil;
  NSMutableArray<NSString *> *items = [[[NSFileManager defaultManager]
                                        contentsOfDirectoryAtPath:unzipPath
                                        error:&error] mutableCopy];
  if (error) {
    return;
  }
  [items enumerateObjectsUsingBlock:^(NSString * _Nonnull obj, NSUInteger idx, BOOL * _Nonnull stop) {
    switch (idx) {
      case 0: {
        self.file1.text = obj;
        break;
      }
      case 1: {
        self.file2.text = obj;
        break;
      }
      case 2: {
        self.file3.text = obj;
        break;
      }
      default: {
        NSLog(@"Went beyond index of assumed files");
        break;
      }
    }
  }];
  _unzipButton.enabled = NO;
  _resetButton.enabled = YES;
}

- (IBAction)resetPressed:(id)sender {
  _file1.text = @"";
  _file2.text = @"";
  _file3.text = @"";
  _zipButton.enabled = YES;
  _unzipButton.enabled = NO;
  _resetButton.enabled = NO;
}

#pragma mark - Private
- (NSString *)tempZipPath {
  NSString *path = [NSString stringWithFormat:@"%@/\%@.zip",
                    NSSearchPathForDirectoriesInDomains(NSCachesDirectory, NSUserDomainMask, YES)[0],
                    [NSUUID UUID].UUIDString];
  return path;
}

- (NSString *)tempUnzipPath {
  NSString *path = [NSString stringWithFormat:@"%@/\%@",
                    NSSearchPathForDirectoriesInDomains(NSCachesDirectory, NSUserDomainMask, YES)[0],
                    [NSUUID UUID].UUIDString];
  NSURL *url = [NSURL fileURLWithPath:path];
  NSError *error = nil;
  [[NSFileManager defaultManager] createDirectoryAtURL:url
                           withIntermediateDirectories:YES
                                            attributes:nil
                                                 error:&error];
  if (error) {
    return nil;
  }
  return url.path;
}

@end
