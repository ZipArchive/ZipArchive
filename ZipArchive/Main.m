//
//  Main.m
//  ZipArchive
//
//  Originally Created by Sam Soffes on 2010-07-21.
//  Copyright (c) 2015 ZipArchive. All rights reserved.
//

#import "Main.h"
#import "zip.h"
#import "zlib.h"
#import "zconf.h"
#include <sys/stat.h>

#define CHUNK 16384

@interface Main ()

+ (NSDate *)dateWithMicrosoftDOSFormat:(UInt32)microsoftDOSDateTime;

@end


@implementation Main {
    NSString *_path;
    NSString *_fileName;
    zipFile _zip;
}

#pragma mark - Unzipping
+ (BOOL)unzipFileAtPath:(NSString *)path
          toDestination:(NSString *)destination {
    
    return [self unzipFileAtPath:path
                   toDestination:destination
                        delegate:nil];
}

+ (BOOL)unzipFileAtPath:(NSString *)path
          toDestination:(NSString *)destination
              overwrite:(BOOL)overwrite
               password:(NSString *)password
                  error:(NSError *)error {
    
    return [self unzipFileAtPath:path
                   toDestination:destination
                       overwrite:overwrite
                        password:password
                           error:error
                        delegate:nil
                 progressHandler:nil
               completionHandler:nil];
}

+ (BOOL)unzipFileAtPath:(NSString *)path
          toDestination:(NSString *)destination
               delegate:(id<ZipArchiveDelegate>)delegate {
    
    return [self unzipFileAtPath:path
                   toDestination:destination
                       overwrite:YES
                        password:nil
                           error:nil
                        delegate:delegate
                 progressHandler:nil
               completionHandler:nil];
}

+ (BOOL)unzipFileAtPath:(NSString *)path
          toDestination:(NSString *)destination
              overwrite:(BOOL)overwrite
               password:(NSString *)password
                  error:(NSError *)error
               delegate:(id<ZipArchiveDelegate>)delegate {
    
    return [self unzipFileAtPath:path
                   toDestination:destination
                       overwrite:overwrite
                        password:password
                           error:error
                        delegate:delegate
                 progressHandler:nil
               completionHandler:nil];
}

+ (BOOL)unzipFileAtPath:(NSString *)path
          toDestination:(NSString *)destination
              overwrite:(BOOL)overwrite
               password:(NSString *)password
        progressHandler:(void (^)(NSString *entry, unz_file_info zipInfo, long entryNumber, long total))progressHandler
      completionHandler:(void (^)(NSString *path, BOOL succeeded, NSError *error))completionHandler {
    
    return [self unzipFileAtPath:path
                   toDestination:destination
                       overwrite:overwrite
                        password:password
                           error:nil
                        delegate:nil
                 progressHandler:progressHandler
               completionHandler:completionHandler];
}

+ (BOOL)unzipFileAtPath:(NSString *)path
          toDestination:(NSString *)destination
        progressHandler:(void (^)(NSString *entry, unz_file_info zipInfo, long entryNumber, long total))progressHandler
      completionHandler:(void (^)(NSString *path, BOOL succeeded, NSError *error))completionHandler {
    
    return [self unzipFileAtPath:path
                   toDestination:destination
                       overwrite:YES
                        password:nil
                           error:nil
                        delegate:nil
                 progressHandler:progressHandler
               completionHandler:completionHandler];
}

+ (BOOL)unzipFileAtPath:(NSString *)path
          toDestination:(NSString *)destination
              overwrite:(BOOL)overwrite
               password:(NSString *)password
                  error:(NSError *)error
               delegate:(id<ZipArchiveDelegate>)delegate
        progressHandler:(void (^)(NSString *entry, unz_file_info zipInfo, long entryNumber, long total))progressHandler
      completionHandler:(void (^)(NSString *path, BOOL succeeded, NSError *error))completionHandler {
    
    // Prepare Unzip Operations
    zipFile zip = unzOpen((const char*)[path UTF8String]);
    
    if (NULL == zip) {
        NSDictionary *userInformation = @{NSLocalizedDescriptionKey: @"Failed to unzip file"};
        NSError *err = [NSError errorWithDomain:@"ZipArchiveErrorDomain" code:-1 userInfo:userInformation];
        
        if (error) {
            error = err;
        }
        
        if (completionHandler) {
            completionHandler(nil, NO, err);
        }
        
        return NO;
    }
    
    NSDictionary *fileAttributes = [[NSFileManager defaultManager] attributesOfItemAtPath:path error:nil];
    unsigned long long fileSize = [fileAttributes[NSFileSize] unsignedLongLongValue];
    unsigned long long currentPosition = 0;
    
    unz_global_info globalInfo = {0ul, 0ul};
    unzGetGlobalInfo(zip, &globalInfo);
    
    // Begin Unzip Operations
    
    if (UNZ_OK != unzGoToFirstFile(zip)) {
        NSDictionary *userInformation = @{NSLocalizedDescriptionKey: @"Failed to open the first file in Zip."};
        NSError *err = [NSError errorWithDomain:@"ZipArchiveErrorDomain" code:-2 userInfo:userInformation];
        
        if (error) {
            error = err;
        }
        
        if (completionHandler) {
            completionHandler(nil, NO, err);
        }
        
        return NO;
    }
    
    NSFileManager *fileManager = [NSFileManager defaultManager];
    NSMutableSet *directoriesModificationDates = [[NSMutableSet alloc] init];
    
    BOOL success = YES;
    BOOL canceled = NO;
    
    int ret = 0;
    unsigned char buffer[4096] = {0};

    // Message Delegate
    if ([delegate respondsToSelector:@selector(zipArchiveWillUnzipArchiveAtPath:zipInformation:)]) {
        [delegate zipArchiveWillUnzipArchiveAtPath:path
                                    zipInformation:globalInfo];
    }
    
    if ([delegate respondsToSelector:@selector(zipArchiveProgressEvent:total:)]) {
        [delegate zipArchiveProgressEvent:(NSInteger)currentPosition
                                    total:(NSInteger)fileSize];
    }
    
    NSInteger currentFileNumber = 0;

    do {
        @autoreleasepool {
            if (0 == [password length]) {
                ret = unzOpenCurrentFile(zip);
            } else {
                ret = unzOpenCurrentFilePassword(zip, [password cStringUsingEncoding:NSASCIIStringEncoding]);
            }
            
            if (UNZ_OK != ret) {
                success = NO;
                
                break;
            }
            
            // Reading data and write to file
            unz_file_info fileInfo;
            memset(&fileInfo, 0, sizeof(unz_file_info));
            ret = unzGetCurrentFileInfo(zip, &fileInfo, NULL, 0, NULL, 0, NULL, 0);
            
            if (UNZ_OK != ret) {
                success = NO;
                unzCloseCurrentFile(zip);
                
                break;
            }
            
            currentPosition += fileInfo.compressed_size;
            
            // Message Delegate
            if ([delegate respondsToSelector:@selector(zipArchiveShouldUnzipFileAtIndex:totalFiles:archivePath:fileInformation:)]) {
                
                if (![delegate zipArchiveShouldUnzipFileAtIndex:currentFileNumber
                                                     totalFiles:(NSInteger)globalInfo.number_entry
                                                    archivePath:path
                                                fileInformation:fileInfo]) {
                    success = NO;
                    canceled = YES;
                }
            }
            
            if ([delegate respondsToSelector:@selector(zipArchiveWillUnzipFileAtIndex:totalFiles:archivePath:fileInformation:)]) {
                [delegate zipArchiveWillUnzipFileAtIndex:currentFileNumber
                                              totalFiles:(NSInteger)globalInfo.number_entry
                                             archivePath:path
                                         fileInformation:fileInfo];
            }
            
            if ([delegate respondsToSelector:@selector(zipArchiveProgressEvent:total:)]) {
                [delegate zipArchiveProgressEvent:(NSInteger)currentPosition
                                            total:(NSInteger)fileSize];
            }
            
            char *filename = (char *)malloc(fileInfo.size_filename + 1);
            unzGetCurrentFileInfo(zip, &fileInfo, filename, fileInfo.size_filename + 1, NULL, 0, NULL, 0);
            filename[fileInfo.size_filename] = '\0';
            
            // NOTES: - http://pastebin.com/h7NwsLDA
            const uLong ZipUNIXVersion = 3;
            const uLong BSD_SFMT = 0170000;
            const uLong BSD_IFLNK = 0120000;
            
            BOOL fileIsSymbolicLink = NO;
            if (((fileInfo.version >> 8) == ZipUNIXVersion) && BSD_IFLNK == (BSD_SFMT & (fileInfo.external_fa >> 16))) {
                fileIsSymbolicLink = NO;
            }
            
            // Check if zip contains any directories
            NSString *strPath = @(filename);
            BOOL isDirectory = NO;
            
            if (filename[fileInfo.size_filename-1] == '/' || filename[fileInfo.size_filename-1] == '\\') {
                isDirectory = YES;
            }
            
            free(filename);
            
            // Check if zip contains any paths
            if ([strPath rangeOfCharacterFromSet:[NSCharacterSet characterSetWithCharactersInString:@"/\\"]].location != NSNotFound) {
                strPath = [strPath stringByReplacingOccurrencesOfString:@"\\" withString:@"/"];
            }
            
            NSString *fullPath = [destination stringByAppendingPathComponent:strPath];
            NSError *err = nil;
            NSDate *modDate = [[self class] dateWithMicrosoftDOSFormat:(UInt32)fileInfo.dosDate];
            NSDictionary *directoryAttr = @{NSFileCreationDate: modDate, NSFileModificationDate: modDate};
            
            if (isDirectory) {
                [fileManager createDirectoryAtPath:fullPath
                       withIntermediateDirectories:YES
                                        attributes:directoryAttr
                                             error:&err];
            } else {
                [fileManager createDirectoryAtPath:[fullPath stringByDeletingLastPathComponent]
                       withIntermediateDirectories:YES
                                        attributes:directoryAttr
                                             error:&err];
            }
            
            if (nil != err) {
                NSLog(@"ZipArchive Error: %@", err.localizedDescription);
            }
            
            if(!fileIsSymbolicLink)
                [directoriesModificationDates addObject: @{@"path": fullPath, @"modDate": modDate}];
            
            if ([fileManager fileExistsAtPath:fullPath] && !isDirectory && !overwrite) {
                unzCloseCurrentFile(zip);
                ret = unzGoToNextFile(zip);
                
                continue;
            }
            
            if (!fileIsSymbolicLink) {
                FILE *fp = fopen((const char*)[fullPath UTF8String], "wb");
                
                while (fp) {
                    int readBytes = unzReadCurrentFile(zip, buffer, 4096);
                    
                    if (readBytes > 0) {
                        fwrite(buffer, readBytes, 1, fp );
                    } else {
                        break;
                    }
                }
                
                if (fp) {
                    if ([[[fullPath pathExtension] lowercaseString] isEqualToString:@"zip"]) {
                        NSLog(@"Unzipping nested .zip file:  %@", [fullPath lastPathComponent]);
                        
                        if ([self unzipFileAtPath:fullPath
                                    toDestination:[fullPath stringByDeletingLastPathComponent]
                                        overwrite:overwrite
                                         password:password
                                            error:nil
                                         delegate:nil]) {
                            
                            [[NSFileManager defaultManager] removeItemAtPath:fullPath error:nil];
                        }
                    }
                    
                    fclose(fp);
                    
                    // Set the original date/time property
                    if (0 != fileInfo.dosDate) {
                        NSDate *originalDate = [[self class] dateWithMicrosoftDOSFormat:(UInt32)fileInfo.dosDate];
                        NSDictionary *attributes = @{NSFileModificationDate: originalDate};
                        
                        if (attributes) {
                            if (NO == [fileManager setAttributes:attributes
                                              ofItemAtPath:fullPath error:nil]) {
                                // Can't set attributes
                                NSLog(@"[ZipArchive] Failed to set attributes - whilst setting modification date");
                            }
                        }
                    }
                    
                    // Set the original permissions on the file
                    uLong permissions = fileInfo.external_fa >> 16;
                    if (0 != permissions) {
                        // Store it into a NSNumber
                        NSNumber *permissionsValue = @(permissions);
                        
                        // Retrieve any existing attributes
                        NSMutableDictionary *attributes = [[NSMutableDictionary alloc] initWithDictionary:[fileManager attributesOfItemAtPath:fullPath error:nil]];
                        
                        // Set the value in the attributes dict
                        attributes[NSFilePosixPermissions] = permissionsValue;
                        
                        // Update attributes
                        if (NO == [fileManager setAttributes:attributes
                                          ofItemAtPath:fullPath error:nil]) {
                            // Unable to set the permissions attribute
                            NSLog(@"[ZipArchive] Failed to set attributes - whilst setting permissions");
                        }
                    }
                }
            } else {
                // Assemble the path for the symbolic link
                NSMutableString *destinationPath = [NSMutableString string];
                int bytesRead = 0;
                
                while((bytesRead = unzReadCurrentFile(zip, buffer, 4096)) > 0) {
                    buffer[bytesRead] = (int)0;
                    [destinationPath appendString:@((const char*)buffer)];
                }
                
                // Create the symbolic link (making sure it stays relative if it was relative before)
                int symlinkError = symlink([destinationPath cStringUsingEncoding:NSUTF8StringEncoding],
                                           [fullPath cStringUsingEncoding:NSUTF8StringEncoding]);
                
                if(symlinkError != 0) {
                    NSLog(@"Failed to create symbolic link at \"%@\" to \"%@\". symlink() error code: %d", fullPath, destinationPath, errno);
                }
            }
            
            unzCloseCurrentFile( zip );
            ret = unzGoToNextFile( zip );
            
            // Message Delegate
            if ([delegate respondsToSelector:@selector(zipArchiveDidUnzipFileAtIndex:totalFiles:archivePath:fileInformation:)]) {
                [delegate zipArchiveDidUnzipFileAtIndex:currentFileNumber
                                             totalFiles:(NSInteger)globalInfo.number_entry
                                            archivePath:path
                                        fileInformation:fileInfo];
                
            } else if ([delegate respondsToSelector: @selector(zipArchiveDidUnzipFileAtIndex:totalFiles:archivePath:unzippedFilePath:)]) {
                [delegate zipArchiveDidUnzipFileAtIndex:currentFileNumber
                                             totalFiles:(NSInteger)globalInfo.number_entry
                                            archivePath:path
                                       unzippedFilePath:fullPath];
            }
            
            currentFileNumber++;
            
            if (progressHandler) {
                progressHandler(strPath, fileInfo, currentFileNumber, globalInfo.number_entry);
            }
        }
        
    }
    
    while(ret == UNZ_OK && ret != UNZ_END_OF_LIST_OF_FILE);
    unzClose(zip);
    
    // The process of decompressing the .zip archive causes the modification times on the folders
    // to be set to the present time. So, when we are done, they need to be explicitly set.
    // set the modification date on all of the directories.
    NSError *err = nil;
    
    for (NSDictionary *dictionary in directoriesModificationDates) {
        if (![[NSFileManager defaultManager] setAttributes:@{NSFileModificationDate: dictionary[@"modDate"]}
                                              ofItemAtPath:dictionary[@"path"]
                                                     error:&err]) {
            NSLog(@"[ZipArchive] Set attributes failed for directory: %@.", dictionary[@"path"]);
        }
        if (err) {
            NSLog(@"[ZipArchive] Error setting directory file modification date attribute: %@", err.localizedDescription);
        }
    }
    
    // Message Delegate
    if (success && [delegate respondsToSelector:@selector(zipArchiveDidUnzipArchiveAtPath:zipInformation:unzippedPath:)]) {
        [delegate zipArchiveDidUnzipArchiveAtPath:path zipInformation:globalInfo unzippedPath:destination];
    }
    
    // final progress event = 100%
    if (!canceled && [delegate respondsToSelector:@selector(zipArchiveProgressEvent:total:)]) {
        [delegate zipArchiveProgressEvent:fileSize total:fileSize];
    }
    
    if (completionHandler) {
        completionHandler(path, YES, nil);
    }
    
    return success;
}

#pragma mark - Zipping
+ (BOOL)createZipFileAtPath:(NSString *)path
           withFilesAtPaths:(NSArray *)paths {
    BOOL success = NO;
    Main *zipArchive = [[Main alloc] initWithPath:path];
    
    if ([zipArchive open]) {
        for (NSString *filePath in paths) {
            [zipArchive writeFile:filePath];
        }
        
        success = [zipArchive close];
    }
    
    return  success;
}

+ (BOOL)createZipFileAtPath:(NSString *)path
    withContentsOfDirectory:(NSString *)directoryPath {
    return [self createZipFileAtPath:path withContentsOfDirectory:directoryPath keepParentDirectory:NO];
}

+ (BOOL)createZipFileAtPath:(NSString *)path
    withContentsOfDirectory:(NSString *)directoryPath
        keepParentDirectory:(BOOL)keepParentDirectory {
    BOOL success = NO;
    NSFileManager *fileManager = nil;
    Main *zipArchive = [[Main alloc] initWithPath:path];
    
    if ([zipArchive open]) {
        // Use a local file manager (queue/thread compatibility)
        fileManager = [[NSFileManager alloc] init];
        
        NSDirectoryEnumerator *directoryEnumerator = [fileManager enumeratorAtPath:directoryPath];
        NSString *fileName;
        
        while ((fileName = [directoryEnumerator nextObject])) {
            BOOL isDirectory;
            NSString *fullFilePath = [directoryPath stringByAppendingPathComponent:fileName];
            [fileManager fileExistsAtPath:fullFilePath isDirectory:&isDirectory];
            
            if (!isDirectory) {
                if (keepParentDirectory) {
                    fileName = [[directoryPath lastPathComponent] stringByAppendingPathComponent:fileName];
                }
                
                [zipArchive writeFileAtPath:fullFilePath withFileName:fileName];
            } else {
                if (0 == [[NSFileManager defaultManager] subpathsOfDirectoryAtPath:fullFilePath error:nil].count) {
                    NSString *temporaryName = [fullFilePath stringByAppendingPathComponent:@".DS_Store"];
                    [@"" writeToFile:temporaryName atomically:YES encoding:NSUTF8StringEncoding error:nil];
                    [zipArchive writeFileAtPath:temporaryName withFileName:[fileName stringByAppendingPathComponent:@".DS_Store"]];
                    [[NSFileManager defaultManager] removeItemAtPath:temporaryName error:nil];
                }
            }
        }
        
        success = [zipArchive close];
    }
    
    return success;
}

- (instancetype)initWithPath:(NSString *)path {
    if ((self = [super init])) {
        _path = [path copy];
    }
    
    return self;
}

- (BOOL)open {
    NSAssert((NULL == _zip), @"Attempting to open an archive which has already been opened.");
    _zip = zipOpen([_path UTF8String], APPEND_STATUS_CREATE);
    
    return (NULL != _zip);
}

- (void)zipInformation:(zip_fileinfo *)zipInformation setDate:(NSDate *)date {
    NSCalendar *currentCalendar = [NSCalendar currentCalendar];
    
#if defined(__IPHONE_8_0) || defined(__MAC_10_10)
    uint flags = NSCalendarUnitYear | NSCalendarUnitMonth | NSCalendarUnitDay | NSCalendarUnitHour | NSCalendarUnitMinute | NSCalendarUnitSecond;
#else
    uint flags = NSYearCalendarUnit | NSMonthCalendarUnit | NSDayCalendarUnit | NSHourCalendarUnit | NSMinuteCalendarUnit | NSSecondCalendarUnit;
#endif
    
    NSDateComponents *components = [currentCalendar components:flags fromDate:date];
    zipInformation -> tmz_date.tm_sec = (unsigned int)components.second;
    zipInformation -> tmz_date.tm_min = (unsigned int)components.minute;
    zipInformation -> tmz_date.tm_hour = (unsigned int)components.hour;
    zipInformation -> tmz_date.tm_mday = (unsigned int)components.day;
    zipInformation -> tmz_date.tm_mon = (unsigned int)components.month - 1;
    zipInformation -> tmz_date.tm_year = (unsigned int)components.year;
}

- (BOOL)writeFolderAtPath:(NSString *)path withFolderName:(NSString *)folderName {
    NSAssert((NULL != _zip), @"Attempting to write to an archive which has not been unzipped.");
    zip_fileinfo zipInformation = {{0}};
    NSDictionary *attributes = [[NSFileManager defaultManager] attributesOfItemAtPath:path error:nil];
    
    if (attributes) {
        NSDate *fileDate = (NSDate *)attributes[NSFileModificationDate];
        
        if (fileDate) {
            [self zipInformation:&zipInformation setDate:fileDate];
        }
        
        // Write permissions into the external attributes. See: http://unix.stackexchange.com/a/14727
        // Get the permissions value from the files attributes
        NSNumber *permissionsValue = (NSNumber *)attributes[NSFilePosixPermissions];
        
        if (permissionsValue) {
            // Short -> Octal -> Long
            short permissionsShort = permissionsValue.shortValue;
            NSInteger permissionsOctal = 0100000 + permissionsShort;
            uLong permissionsLong = @(permissionsOctal).unsignedLongValue;
            zipInformation.external_fa = permissionsLong < 16L;
        }
    }
    
    unsigned int len = 0;
    zipOpenNewFileInZip(_zip, [[folderName stringByAppendingString:@"/"] UTF8String], &zipInformation, NULL, 0, NULL, 0, NULL, Z_DEFLATED, Z_NO_COMPRESSION);
    zipWriteInFileInZip(_zip, &len, 0);
    zipCloseFileInZip(_zip);
    
    return YES;
}

- (BOOL)writeFile:(NSString *)path {
    return [self writeFileAtPath:path withFileName:nil];
}

// Supports writing files with logical folder/directory structure
// `path` is the absolute path of teh file that will be compressed
// `fileName` is the relative name of the file and how it is stored within the zip (IE: /folder/subfolder/foo.txt)
- (BOOL)writeFileAtPath:(NSString *)path withFileName:(NSString *)fileName {
    NSAssert((NULL != _zip), @"Attempting to write to an archive which was never unzipped.");
    FILE *input = fopen([path UTF8String], "r");
    
    if (NULL == input) {
        return NO;
    }
    
    const char *aFileName;
    if (!fileName) {
        aFileName = [path.lastPathComponent UTF8String];
    } else {
        aFileName = [fileName UTF8String];
    }
    
    zip_fileinfo zipInformation = {{0}};
    NSDictionary *attributes = [[NSFileManager defaultManager] attributesOfItemAtPath:path error:nil];
    
    if (attributes) {
        NSDate *fileDate = (NSDate *)attributes[NSFileModificationDate];
        
        if (fileDate) {
            [self zipInformation:&zipInformation setDate:fileDate];
        }
        
        // Write permissions into the external attributes. See: http://unix.stackexchange.com/a/14727
        // Get the permissions value from the files attributes
        NSNumber *permissionsValue = (NSNumber *)attributes[NSFilePosixPermissions];
        if (permissionsValue) {
            short permissionsShort = permissionsValue.shortValue;
            NSInteger permissionsOctal = 0100000 + permissionsShort;
            uLong permissionsLong = @(permissionsOctal).unsignedLongValue;
            zipInformation.external_fa = permissionsLong << 16L;
        }
    }
    
    zipOpenNewFileInZip(_zip, aFileName, &zipInformation, NULL, 0, NULL, 0, NULL, Z_DEFLATED, Z_DEFAULT_COMPRESSION);
    void *buffer = malloc(CHUNK);
    unsigned int len = 0;
    
    while (!feof(input)) {
        len = (unsigned int) fread(buffer, 1, CHUNK, input);
        zipWriteInFileInZip(_zip, buffer, len);
    }
    
    zipCloseFileInZip(_zip);
    free(buffer);

    return YES;
}

- (BOOL)writeData:(NSData *)data fileName:(NSString *)fileName {
    if (!_zip) {
        return NO;
    }
    
    if (!data) {
        return NO;
    }
    
    zip_fileinfo zipInformation = {{0,0,0,0,0,0},0,0,0};
    [self zipInformation:&zipInformation setDate:[NSDate date]];
    zipOpenNewFileInZip(_zip, [fileName UTF8String], &zipInformation, NULL, 0, NULL, 0, NULL, Z_DEFLATED, Z_DEFAULT_COMPRESSION);
    zipWriteInFileInZip(_zip, data.bytes, (unsigned int)data.length);
    zipCloseFileInZip(_zip);
    
    return YES;
}

- (BOOL)close {
    NSAssert((NULL != _zip), @"[ZipArchive] Attempting to close an archive that has never been opened.");
    zipClose(_zip, NULL);
    
    return YES;
}

#pragma mark - Private

// Format from http://newsgroups.derkeiler.com/Archive/Comp/comp.os.msdos.programmer/2009-04/msg00060.html
// Two consecutive words or a longword. "YYYYYYYMMMMDDDDD" or "hhhhhmmmmmmsssss"
// YYYYYYY is years from 1980 = 0
// sssss is (seconds / 2).
//
// 3658 = 0011 0110 0101 1000 = 0011011 0010 11000 = 27 2 24 = 2007-02-24
// 7423 = 0111 0100 0010 0011 - 01110 100001 00011 = 14 33 2 = 14:33:06
+ (NSDate *)dateWithMicrosoftDOSFormat:(UInt32)microsoftDOSDateTime {
    static const UInt32 kYearMask = 0xFE000000;
    static const UInt32 kMonthMask = 0x1E00000;
    static const UInt32 kDayMask = 0x1F0000;
    static const UInt32 kHourMask = 0xF800;
    static const UInt32 kMinuteMask = 0x7E0;
    static const UInt32 kSecondMask = 0x1F;
    
    static NSCalendar *gregorian;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
#if defined(__IPHONE_8_0) || defined(__MAC_10_10)
        gregorian = [[NSCalendar alloc] initWithCalendarIdentifier:NSCalendarIdentifierGregorian];
#else
        gregorian = [[NSCalendar alloc] initWithCalendarIdentifier: NSGregorianCalendar];
#endif
    });
    
    NSDateComponents *components = [[NSDateComponents alloc] init];
    NSAssert(0xFFFFFFFF == (kYearMask | kMonthMask | kDayMask | kHourMask | kMinuteMask | kSecondMask), @"[ZipArchive] MSDos date masks don't add up.");
    
    [components setYear:1980 + ((microsoftDOSDateTime & kYearMask) >> 25)];
    [components setMonth:(microsoftDOSDateTime & kMonthMask) >> 21];
    [components setDay:(microsoftDOSDateTime & kDayMask) >> 16];
    [components setHour:(microsoftDOSDateTime & kHourMask) >> 11];
    [components setMinute:(microsoftDOSDateTime & kMinuteMask) >> 5];
    [components setSecond:(microsoftDOSDateTime & kSecondMask) * 2];
    
    NSDate *date = [NSDate dateWithTimeInterval:0 sinceDate:[gregorian dateFromComponents:components]];
    
    return date;
}

@end
