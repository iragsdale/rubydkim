#!/usr/bin/ruby
# Reads an email in from standard input and signs it
require "dkim"

domain, selector, keyfile = ARGV
key = File.readlines(keyfile).join

# read the email
signer = DKIM::Signer.new(domain, selector, key)
message = ""
STDIN.each_line do |line|
  line.gsub!(/\n/, "\r\n")
  signer.feed(line)
  message << line
end

signature = signer.finish

# print out the signature then the email
puts "#{signature.signature_header}\n#{message}"