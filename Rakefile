begin
  require 'rake/gempackagetask'
rescue LoadError
  puts "got error: #{$!}"
end
require 'rake/clean'
require 'rbconfig'
include Config

PKG_NAME          = 'DKIM'
PKG_VERSION       = File.read('VERSION').chomp
PKG_FILES         = FileList["**/*"].exclude(/CVS|pkg|tmp|coverage|Makefile/).exclude(/\.(so|bundle|o|#{CONFIG['DLEXT']})$/)
EXT_ROOT_DIR      = 'src/dkim'
EXT_SIGNER_DIR    = "#{EXT_ROOT_DIR}/signer"
EXT_SIGNER_SRC    = "#{EXT_SIGNER_DIR}/signer.c"
EXT_SIGNER_DL     = "#{EXT_ROOT_DIR}/signer.#{CONFIG['DLEXT']}"
EXT_VERIFIER_DIR    = "#{EXT_ROOT_DIR}/verifier"
EXT_VERIFIER_SRC    = "#{EXT_VERIFIER_DIR}/verifier.c"
EXT_VERIFIER_DL     = "#{EXT_ROOT_DIR}/verifier.#{CONFIG['DLEXT']}"
EXT_DEST_DIR      = "lib"
CLEAN.include FileList["src/**/{Makefile,mkmf.log}"], FileList["pkg"],
  FileList["{src,lib}/**/*.{so,bundle,#{CONFIG['DLEXT']},o,obj,pdb,lib,manifest,exp,def}"]

task :default => [ EXT_SIGNER_DL, EXT_VERIFIER_DL ]

desc "Compiling extension"
task :compile_ext => [ EXT_SIGNER_DL, EXT_VERIFIER_DL ]

file EXT_SIGNER_DL => EXT_SIGNER_SRC do
  cd EXT_SIGNER_DIR do
    ruby 'extconf.rb'
    system 'make'
  end
  cp "#{EXT_SIGNER_DIR}/signer.#{CONFIG['DLEXT']}", EXT_DEST_DIR
end

file EXT_VERIFIER_DL => EXT_VERIFIER_SRC do
  cd EXT_VERIFIER_DIR do
    ruby 'extconf.rb'
    system 'make'
  end
  cp "#{EXT_VERIFIER_DIR}/verifier.#{CONFIG['DLEXT']}", EXT_DEST_DIR
end

# creating the gems
if defined?(Gem) and defined?(Rake::GemPackageTask)
  
  spec_ext = Gem::Specification.new do |s|
    s.name = 'rubydkim'
    s.version = PKG_VERSION
    s.summary = "A gem for creating & verifying DKIM signatures"
    s.description = "This is a DKIM implementation as a Ruby extension in C."
    s.add_dependency('dnsruby')

    s.files = PKG_FILES

    s.extensions = FileList['src/**/extconf.rb']

    s.require_paths << 'lib'

    s.bindir = "bin"
    s.executables = [ "dkim_sign.rb", "dkim_verify.rb" ]
    s.default_executable = "dkim_verify.rb"

    s.has_rdoc = false

    s.author = "Ian Ragsdale"
    s.email = "ian.ragsdale@gmail.com"
    s.homepage = "http://github.com/iragsdale/rubydkim"
  end
  
  desc "Generate gemspec"
  task :gemspec do
    File.open("#{spec_ext.name}.gemspec", "w") do |file|
      file.puts spec_ext.to_ruby
    end
  end
  
  Rake::GemPackageTask.new(spec_ext) do |pkg|
    pkg.need_tar      = true
    pkg.package_files = PKG_FILES
  end
  
end
