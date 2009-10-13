#!/usr/bin/env ruby

thisDir = File.dirname(__FILE__)
casesDir = File.join(thisDir, "cases")
lexBinary = File.join(thisDir, "..", "build", "test", "bins", "lex", "lex_test")
if !File.executable? lexBinary
  throw "Can't find lex test binary: #{lexBinary}"
end

passed = 0
total = 0

Dir.glob(File.join(casesDir, "*.orderly")).each { |f| 
  got = ""
  IO.popen(lexBinary, "w+") { |lb|
    File.open(f, "r").each {|l| lb.write(l)}
    lb.close_write
    got = lb.read
  }
  want = IO.read(f.sub(/orderly$/, "lexed"))
  print "Lexing test <#{f.sub(/^.*?([^\/]+)\.orderly$/, '\1')}>: "
  if (got == want)
    puts "ok"
    passed += 1
  else
    puts "fail"
    # XXX: give more information
  end
  total += 1
}

puts "#{passed}/#{total} tests successful"
exit passed == total

