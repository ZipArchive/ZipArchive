//
//  SSZipArchive+Swift.swift
//  ZipArchive
//
//  Created by William Dunay on 7/6/16.
//  Copyright © 2016 smumryak. All rights reserved.
//

import Foundation

extension SSZipArchive {
    
    static func unzipFileAtPath(_ path: String, toDestination destination: String, overwrite: Bool, password: String?, delegate: SSZipArchiveDelegate?) throws -> Bool {
        
        var success = false
        var error: NSError?
        
        success = __unzipFile(atPath: path, toDestination: destination, overwrite: overwrite, password: password, error: &error, delegate: delegate)
        if let throwableError = error {
            throw throwableError
        }
        
        return success
    }
}
