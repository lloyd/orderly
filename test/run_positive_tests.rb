#!/usr/bin/env ruby

# arguments are a string that must match the test name
substrpat = ARGV.length ? ARGV[0] : ""

thisDir = File.dirname(__FILE__)
casesDir = File.join(thisDir, "positive_cases")
reformatBinary = File.join(thisDir, "..", "build", "reformatter", "orderly_reformat")
if !File.executable? reformatBinary
  throw "Can't find orderly_reformat binary: #{reformatBinary}"
end

passed = 0
total = 0

puts "Running round trip tests: "
puts "(containing '#{substrpat}' in name)" if substrpat && substrpat.length > 0

tests = Hash.new

Dir.glob(File.join(casesDir, "*")).each { |f| 
  next if f !~ /\.orderly$/ && f !~ /\.jsonschema$/ 
  next if substrpat && substrpat.length > 0 && !f.include?(substrpat)
  key = f.sub(/\.[^.]*$/, "")
  tests[key] = Array.new if !tests.has_key? key
  tests[key].push f
}

tests.each { |k,v|
  if v.length != 2
    print "Running #{k}:\t "
    puts "FAIL (can't find both jsonschema and orderly files)"
    total += 4
  else 
    order = [ "orderly", k + ".orderly" ]
    chaos = [ "jsonschema", k + ".jsonschema" ]

    [
      [ order, order ],
      [ order, chaos ],             
      [ chaos, order ],
      [ chaos, chaos ]
    ].each { |x|
      print "test '#{k.sub(/^.*\/([^\/]*)$/, '\1')}' (#{x[0][0]} -> #{x[1][0]}):\t "
      cmd = "#{reformatBinary} -i #{x[0][0]} -o #{x[1][0]}"
      got = nil
      IO.popen(reformatBinary, "w+") { |lb|      
        File.open(x[0][1], "r").each {|l| lb.write(l)}
        lb.close_write
        got = lb.read
      }
      want = IO.read(x[1][1])
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
      total += 1
    }
  end
}

puts "#{passed}/#{total} tests successful"
exit passed == total
