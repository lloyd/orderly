#!/usr/bin/env ruby

# arguments are a string that must match the test name
substrpat = ARGV.length ? ARGV[0] : ""

thisDir = File.dirname(__FILE__)
casesDir = File.join(thisDir, "cases")
lexBinary = File.join(thisDir, "..", "build", "test", "bins", "lex", "lex_test")
if !File.executable? lexBinary
  throw "Can't find lex test binary: #{lexBinary}"
end
parseBinary = File.join(thisDir, "..", "build", "test", "bins", "parse", "parse_test")
if !File.executable? parseBinary
  throw "Can't find parse test binary: #{parseBinary}"
end
parseBinary = File.join(thisDir, "..", "build", "test", "bins", "parse", "parse_test")
if !File.executable? parseBinary
  throw "Can't find parse test binary: #{parseBinary}"
end

passed = 0
total = 0

puts "Running parse/lex tests: "
puts "(containing '#{substrpat}' in name)" if substrpat && substrpat.length > 0

Dir.glob(File.join(casesDir, "*.orderly")).each { |f| 
  next if substrpat && substrpat.length > 0 && !f.include?(substrpat)
  [
    [ "Lexing", f.sub(/orderly$/, "lexed"), lexBinary ],
    [ "Parsing", f.sub(/orderly$/, "parsed"), parseBinary ]
  ].each { |testType|
    what, wantFile, program = *testType
    gotFile = wantFile + ".got"
    got = ""
    IO.popen(program, "w+") { |lb|
      File.open(f, "r").each {|l| lb.write(l)}
      lb.close_write
      got = lb.read
    }

    print "#{what} <#{f.sub(/^.*?([^\/]+)\.orderly$/, '\1')}>:\t "
    if !File.exist? wantFile
      puts "skipped"    
      passed += 1
    else
      want = IO.read(wantFile)
      if (got == want)
        puts "ok"
        passed += 1
      else
        puts "FAIL"
        puts "<<<want<<<"
        puts want
        puts "========"
        puts got
        puts ">>got>>"
      end
    end
    total += 1
  }
}

puts "#{passed}/#{total} tests successful"
exit passed == total

