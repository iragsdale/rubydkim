require 'mkmf'
# link against pdkim
if have_library("pdkim1", "pdkim_init_sign")
    create_makefile("signer")
else
  puts "PDKIM library not found - please install it from http://duncanthrax.net/pdkim/"
end