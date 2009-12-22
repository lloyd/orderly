#!/usr/bin/env ruby

binaryDir = ENV["BINARY_DIR"]

# arguments are a string that must match the test name
substrpat = ARGV.length ? ARGV[0] : ""

thisDir = File.dirname(__FILE__)
casesDir = File.join(thisDir, "negative_cases")
checkBinary = File.join(binaryDir, "checker", "orderly_check")
if !File.executable? checkBinary
  throw "Can't find orderly_check binary: #{checkBinary}"
end

passed = 0
total = 0

puts "Running error reporting tests: "
puts "(containing '#{substrpat}' in name)" if substrpat && substrpat.length > 0

Dir.glob(File.join(casesDir, "*.orderly")).each { |f| 
  next if substrpat && substrpat.length > 0 && !f.include?(substrpat)

  program = checkBinary
  what = "Checking" 
  wantFile = f.sub(/orderly$/, "output")
  gotFile = wantFile + ".got"
  got = ""
  print "#{what} <#{f.sub(/^.*?([^\/]+)\.orderly$/, '\1')}>:\t "
  IO.popen(program, "w+") { |lb|
    File.open(f, "r").each {|l| lb.write(l)}
    lb.close_write
    got = lb.read
  }
  if !File.exist? wantFile
    puts "FAIL"    
    puts "'goldfile' doesn't exist: #{wantFile} (left #{gotFile})"
    File.open(gotFile, "w+") { |gf| gf.write got }
    puts got
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

puts "#{passed}/#{total} tests successful"
exit passed == total
