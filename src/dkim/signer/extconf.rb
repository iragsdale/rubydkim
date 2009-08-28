require 'mkmf'
# link against pdkim
if have_library("pdkim1", "pdkim_init_sign")
    create_makefile("signer")
else
    puts "no pdkim library found"
end