Pod::Spec.new do |s|
  s.name         = 'SSZipArchive'
  s.version      = '3.0'
  s.summary      = 'Utility class for zipping and unzipping files on iOS, tvOS, watchOS, and Mac.'
  s.description  = 'SSZipArchive is a simple utility class for zipping and unzipping files on iOS, tvOS, watchOS, and Mac.'
  s.homepage     = 'https://github.com/ZipArchive/ZipArchive'
  s.license      = { :type => 'MIT', :file => 'LICENSE.txt' }
  s.authors      = { 'Sam Soffes' => 'sam@soff.es', 'Joshua Hudson' => nil, 'Antoine Cœur' => nil }
  s.source       = { :git => 'https://github.com/ZipArchive/ZipArchive.git', :tag => "v#{s.version}" }
  s.ios.deployment_target = '4.0'
  s.tvos.deployment_target = '9.0'
  s.osx.deployment_target = '10.8'
  s.watchos.deployment_target = '2.0'
  s.xcconfig = { "GCC_PREPROCESSOR_DEFINITIONS" => 'HAVE_AES HAVE_CRYPT HAVE_ZLIB' }
  s.source_files =  'SSZipArchive/minizip/*.{c,h}', 'SSZipArchive/minizip/aes/*.{c,h}', 'SSZipArchive/*.{m,h}'
  s.public_header_files = 'SSZipArchive/minizip/*.h', 'SSZipArchive/*.h'
  s.library = 'z'


end


