//
//  ViewController.m
//  Example
//
//  Created by Douglas Bumby on 2015-07-11.
//  Copyright (c) 2015 Cosmic Labs. All rights reserved.
//

#import "ViewController.h"

@interface ViewController ()

@end

@implementation ViewController

- (void)viewDidLoad {
    [super viewDidLoad];

    // Unzip Operation
    NSString *zipPath = @"path_to_your_zip_file";
    NSString *destinationPath = @"path_to_the_folder_where_you_want_it_unzipped";
    
    [Main unzipFileAtPath:zipPath
            toDestination:destinationPath];
    
    // Zip Operation
    NSString *zippedPath = @"path_where_you_want_the_file_created";
    NSArray *inputPaths = @[[[NSBundle mainBundle] pathForResource:@"photo1" ofType:@"jpg"],
                            [[NSBundle mainBundle] pathForResource:@"photo1" ofType:@"jpg"]];
    
    [Main createZipFileAtPath:zippedPath
             withFilesAtPaths:inputPaths];
    
    // Zip Directory
    [Main createZipFileAtPath:zippedPath
      withContentsOfDirectory:inputPaths];
}

@end