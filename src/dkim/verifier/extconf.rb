require 'mkmf'
# link against pdkim
if have_library("pdkim1", "pdkim_init_verify")
    create_makefile("verifier")
else
    puts "no pdkim library found"
end