// swift-tools-version:5.9
// The swift-tools-version declares the minimum version of Swift required to build this package.

import PackageDescription

let package = Package(
    name: "ZipArchive",
    platforms: [
        .iOS("15.5"),
        .tvOS("15.4"),
        .macOS(.v10_15),
        .visionOS("1.0"),
        .watchOS("8.4"),
        .macCatalyst("13.0"),
    ],
    products: [
        .library(name: "ZipArchive", targets: ["ZipArchive"]),
    ],
    targets: [
        .target(
            name: "ZipArchive",
            path: "SSZipArchive",
            resources: [
                .process("Supporting Files/PrivacyInfo.xcprivacy")],
            cSettings: [
                .define("HAVE_ARC4RANDOM_BUF"),
                .define("HAVE_ICONV"),
                .define("HAVE_INTTYPES_H"),
                .define("HAVE_PKCRYPT"),
                .define("HAVE_STDINT_H"),
                .define("HAVE_WZAES"),
                .define("HAVE_ZLIB"),
                .define("ZLIB_COMPAT"),
                .headerSearchPath("minizip"),
            ],
            linkerSettings: [
                .linkedLibrary("z"),
                .linkedLibrary("iconv"),
                .linkedFramework("Security"),
            ]
        )
    ]
)
