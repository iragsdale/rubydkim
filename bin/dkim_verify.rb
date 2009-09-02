#!/usr/bin/ruby
# Reads an email in from standard input and tries to verify it
require "dkim"

# read the email
verifier = DKIM::Verifier.new
STDIN.each_line do |line|
  verifier.feed(line)
end

# grab the list of signatures found
signatures = verifier.finish

# display the results
if signatures.empty?
  puts "No signatures found!"
else
  signatures.each do |signature|
    dnsname = "#{signature.selector}._domainkey.#{signature.domain}"
    puts "signature: #{dnsname} passed? #{signature.passed?}"
    puts signature.inspect
  end
end