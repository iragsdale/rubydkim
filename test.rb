require "lib/dkim"

domain = "cloudsmtp.com"
selector = "sample"
key = File.readlines('sample_key').to_s
message = "From: testing@cloudsmtp.com
To: mario@lubowitz.co.uk
Subject: Inventore dolorem odit alias.
Date: Thu, 27 Aug 2009 16:19:48 -0500

Voluptate et eveniet architecto quaerat accusamus. Pariatur eum maxime veritatis. Facilis vitae est nisi ea veniam.

Et et ea illum ut non qui. Impedit quod eveniet et quia velit facere doloremque tempore. Ad qui beatae repellendus pariatur. Hic id magni ea et.
"
# sign the message
signer = DKIM::Signer.new(domain, selector, key)
signer.feed(message)
signature = signer.finish

signed = "#{signature.signature_header}\n#{message}"
puts "signed:"
puts signed
puts

verifier = DKIM::Verifier.new
verifier.feed(signed)
signatures = verifier.finish

puts "signatures: #{signatures.inspect}"
puts "signatures.first.passed? #{signatures.first.passed?}"