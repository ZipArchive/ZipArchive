//
//  PermissionsTests.swift
//  PermissionsTests
//
//  Created by Bradly Andalman on 12/5/25.
//

import Testing
import ZipArchive

struct PermissionsTests {
    enum TestError: Error {
        case couldNotEncodeStringAsData
        case couldNotWriteDataToArchive
        case unzipExitedWithError
        case fileIsNotReadable
        case fileIsNotExecutable
    }
    
    // See https://github.com/ZipArchive/ZipArchive/issues/761
    @Test func testWriteDataPermissions() async throws {
        // Make a temporary directory for our archive
        let tempdirURL = try makeTemporaryDirectory(basename: "writeDataPermissions")
        let zipURL = tempdirURL.appendingPathComponent("testWriteDataPermissions.zip")

        // Create the data
        let dataFilename = "content.txt"
        let content = "content"
        guard let data = content.data(using: .utf8) else {
            throw TestError.couldNotEncodeStringAsData
        }
                
        // Create the archive
        try createArchiveFromData(url: zipURL, dataFilename: dataFilename, data: data)
                
        // Extract the contents of the archive using `unzip`
        // We'll just unzip to our tempdir
        try runUnzip(zipURL: zipURL, destURL: tempdirURL)
        
        // Make sure that the file we unzipped ("content.txt") is readable
        let fm = FileManager.default
        let dataFilenameURL = tempdirURL.appendingPathComponent(dataFilename)
        guard fm.isReadableFile(atPath: dataFilenameURL.path()) else {
            throw TestError.fileIsNotReadable
        }
        
        // Remove the temporary directory
        try FileManager.default.removeItem(at: tempdirURL)
    }
    
    // See https://github.com/ZipArchive/ZipArchive/issues/689
    @Test func testMaintainExecutablePermissions() async throws {
        // Make a temporary directory
        let tempdirURL = try makeTemporaryDirectory(basename: "maintainExecutablePermissions")
        
        // Write a simple executable file to this directory
        let script = "#!/bin/sh\n"
        let executableFilename = "runme.sh"
        let executableURL = tempdirURL.appendingPathComponent(executableFilename)
        try script.write(to: executableURL, atomically: true, encoding: .utf8)

        // Make the file executable
        var attributes = [FileAttributeKey: Any]()
        attributes[.posixPermissions] = 0o755
        try FileManager.default.setAttributes(attributes, ofItemAtPath: executableURL.path())
        
        // Zip the file
        let zipURL = tempdirURL.appendingPathComponent("maintainExecutablePermissions.zip")
        try createArchiveFromFile(fileURL: executableURL, zipURL: zipURL)
        
        // Extract the contents of the archive using `unzip`
        let unzippedURL = tempdirURL.appendingPathComponent("unzipped")
        try runUnzip(zipURL: zipURL, destURL: unzippedURL)
        
        // Make sure that the file we unzipped is executable
        let fm = FileManager.default
        let unzippedFileURL = unzippedURL.appendingPathComponent(executableFilename)
        guard fm.isExecutableFile(atPath: unzippedFileURL.path()) else {
            throw TestError.fileIsNotExecutable
        }
                        
        // Remove the temporary directory
        try FileManager.default.removeItem(at: tempdirURL)
    }
    
    func runUnzip(zipURL: URL, destURL: URL) throws {
        let process = Process()
        process.executableURL = URL(fileURLWithPath: "/usr/bin/unzip")
        process.arguments = [
            "-o",
            zipURL.path,
            "-d", destURL.path()
        ]
        
        // Run `unzip`
        try process.run()
        process.waitUntilExit()
        
        // Check exit status
        guard process.terminationStatus == 0 else {
            throw TestError.unzipExitedWithError
        }
    }
    
    func createArchiveFromData(url: URL, dataFilename: String, data: Data) throws {
        // Create the zip archive
        let archive = SSZipArchive(path: url.path())
        
        // Open the archive
        archive.open()
        
        // Write the data
        guard archive.write(data, filename: "content.txt", compressionLevel: 0,
                            password: nil, aes: false) else {
            throw TestError.couldNotWriteDataToArchive
        }
        
        // Close the archive
        archive.close()
    }
    
    func createArchiveFromFile(fileURL: URL, zipURL: URL) throws {
        // Create the zip archive
        let archive = SSZipArchive(path: zipURL.path())
        
        // Open the archive
        archive.open()
        
        // Write the file
        guard archive.writeFile(fileURL.path(), withPassword: nil) else {
            throw TestError.couldNotWriteDataToArchive
        }
        
        // Close the archive
        archive.close()
    }
    
    func makeTemporaryDirectory(basename: String) throws -> URL {
        let fm = FileManager.default
        let tempRoot = fm.temporaryDirectory
        var candidate = tempRoot.appendingPathComponent(basename, isDirectory: true)

        var counter = 0
        while fm.fileExists(atPath: candidate.path) {
            counter += 1
            candidate = tempRoot.appendingPathComponent("\(basename)-\(counter)",
                                                        isDirectory: true)
        }

        try fm.createDirectory(
            at: candidate,
            withIntermediateDirectories: true,
            attributes: nil
        )

        return candidate
    }
}
