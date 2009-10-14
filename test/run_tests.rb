#!/usr/bin/env ruby

thisDir = File.dirname(__FILE__)
casesDir = File.join(thisDir, "cases")
lexBinary = File.join(thisDir, "..", "build", "test", "bins", "lex", "lex_test")
if !File.executable? lexBinary
  throw "Can't find lex test binary: #{lexBinary}"
end

passed = 0
total = 0

puts "Running parse/lex tests:"

Dir.glob(File.join(casesDir, "*.orderly")).each { |f| 
  got = ""
  IO.popen(lexBinary, "w+") { |lb|
    File.open(f, "r").each {|l| lb.write(l)}
    lb.close_write
    got = lb.read
  }
  wantFile = f.sub(/orderly$/, "lexed")

  print "<#{f.sub(/^.*?([^\/]+)\.orderly$/, '\1')}>:\t "
  if !File.exist? wantFile
    puts "FAIL"    
    puts "'goldfile' doesn't exist: #{wantFile}"
    puts got
  else
    want = IO.read(wantFile)
    if (got == want)
      puts "ok"
      passed += 1
    else
      puts "FAIL"
      # XXX: give more information
    end
  end
  total += 1
}

puts "#{passed}/#{total} tests successful"
exit passed == total

