// swift-tools-version:5.0
// The swift-tools-version declares the minimum version of Swift required to build this package.

import PackageDescription

let package = Package(
    name: "ZipArchive",
    platforms: [
      .macOS(.v10_10),
      .iOS(.v8)
    ],
    products: [
        .executable(
            name: "ziparc",
            targets: ["ziparc"]),
        .library(
            name: "ZipArchive",
            targets: ["ZipArchive"]),
    ],
    targets: [
        .target(
            name: "ziparc",
            dependencies: ["ZipArchive"],
            path: "ziparc"),
        .target(
            name: "ZipArchive",
            path: "SSZipArchive")
    ]
)

