require 'mkmf'
# link against pdkim
if have_library("pdkim1", "pdkim_init_verify")
    create_makefile("verifier")
else
  puts "PDKIM library not found - please install it from http://duncanthrax.net/pdkim/"
end