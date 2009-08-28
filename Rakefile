begin
  require 'rake/gempackagetask'
  require 'rake/extensiontask'
rescue LoadError
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
EXT_DEST_DIR      = "lib/ext"
CLEAN.include FileList["src/**/{Makefile,mkmf.log}"],
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
