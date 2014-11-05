Pod::Spec.new do |s|
  s.name            = 'SSZipArchive'
  s.version         = '0.2.2.1'
  s.summary         = 'Utility class for zipping and unzipping files on iOS and Mac.'
  s.homepage        = 'https://github.com/samsoffes/ssziparchive'
  s.author          = { 'Sam Soffes' => 'sam@samsoff.es' }
  s.source          = { :git => 'https://github.com/samsoffes/ssziparchive.git', :tag => s.version.to_s }
  s.description     = 'SSZipArchive is a simple utility class for zipping and unzipping files on iOS and Mac.'
  s.source_files    = 'SSZipArchive.{h,m}', 'minizip/*.{h,c}'
  s.library         = 'z'
  s.preserve_paths  = ['Tests', '.gitignore']
  s.license      = { :type => 'MIT', :file => 'LICENSE' }
  s.header_mappings_dir = '.'
end