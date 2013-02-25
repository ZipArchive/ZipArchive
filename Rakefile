desc 'Run the tests'
task :test do
  system 'xcodebuild -project Tests/SSZipArchive.xcodeproj -scheme SSZipArchiveTests TEST_AFTER_BUILD=YES'
end

task :default => :test
