#!/usr/bin/env ruby

binaryDir = ENV["BINARY_DIR"]

# arguments are a string that must match the test name
substrpat = ARGV.length ? ARGV[0] : ""

thisDir = File.dirname(__FILE__)
casesDir = File.join(thisDir, "cases")
lexBinary = File.join(binaryDir,  "test",  "bins", "lex", "lex_test")
if !File.executable? lexBinary
  throw "Can't find lex test binary: #{lexBinary}"
end
parseBinary = File.join(binaryDir, "test", "bins", "parse", "parse_test")
if !File.executable? parseBinary
  throw "Can't find parse test binary: #{parseBinary}"
end

passed = 0
total = 0
files = Dir.glob(File.join(casesDir, "*.orderly"))
puts "1..#{files.length * 2}"
puts "#Running parse/lex tests: "
puts "#(containing '#{substrpat}' in name)" if substrpat && substrpat.length > 0

files.each { |f| 
  next if substrpat && substrpat.length > 0 && !f.include?(substrpat)
  [
    [ "Lexing", f.sub(/orderly$/, "lexed"), lexBinary ],
    [ "Parsing", f.sub(/orderly$/, "parsed"), parseBinary ]
  ].each { |testType|
    total += 1
    what, wantFile, program = *testType
    gotFile = wantFile + ".got"
    got = ""
    IO.popen(program, "w+") { |lb|
      File.open(f, "r").each {|l| lb.write(l)}
      lb.close_write
      got = lb.read
    }

    explanation = "#{what} <#{f.sub(/^.*?([^\/]+)\.orderly$/, '\1')}>"
    if !File.exist? wantFile
      puts "not ok #{total} - # TODO no file #{explanation}"    
      passed += 1
    else
      want = IO.read(wantFile)
      if (got == want)
        puts "ok #{total} - #{explanation}"
        passed += 1
      else
        puts "not ok #{total}"
        puts "#<<<want<<<"
        puts want.gsub(/^/,"#")
        puts "#========"
        puts got.gsub(/^/,"#")
        puts "#>>got>>"
      end
    end
  }
}

puts "##{passed}/#{total} tests successful"
exit passed == total

