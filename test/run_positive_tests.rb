#!/usr/bin/env ruby

binaryDir = ENV["BINARY_DIR"]

# arguments are a string that must match the test name
substrpat = ARGV.length ? ARGV[0] : ""

thisDir = File.dirname(__FILE__)
casesDir = File.join(thisDir, "positive_cases")
reformatBinary = File.join(binaryDir, "reformatter", "orderly_reformat")
if !File.executable? reformatBinary
  throw "Can't find orderly_reformat binary: #{reformatBinary}"
end

passed = 0
skipped = 0
total = 0
cur = 0
#puts "TAP version 13"
tests = Hash.new

Dir.glob(File.join(casesDir, "*")).each { |f| 
  next if f !~ /\.orderly$/ && f !~ /\.jsonschema$/ 
  next if substrpat && substrpat.length > 0 && !f.include?(substrpat)
  key = f.sub(/\.[^.]*$/, "")
  tests[key] = Array.new if !tests.has_key? key
  tests[key].push f
}
total = tests.length * 4
puts "1..#{total}"
puts "#Running round trip tests: "
puts "#(containing '#{substrpat}' in name)" if substrpat && substrpat.length > 0


tests.each { |k,v|
  if v.length != 2
    skipped += 4
    cur += 1
    puts "not ok #{cur} - #{k} (either jsonschema or orderly missing) # TODO "
    cur += 1
    puts "not ok #{cur} - #{k} (either jsonschema or orderly missing) # TODO "
    cur += 1
    puts "not ok #{cur} - #{k} (either jsonschema or orderly missing) # TODO "
    cur += 1
    puts "not ok #{cur} - #{k} (either jsonschema or orderly missing) # TODO "
  else 
    order = [ "orderly", k + ".orderly" ]
    chaos = [ "jsonschema", k + ".jsonschema" ]

    [
      [ order, order ],
      [ order, chaos ],             
      [ chaos, order ],
      [ chaos, chaos ]
    ].each { |x|
      explanation = "'#{k.sub(/^.*\/([^\/]*)$/, '\1')}' (#{x[0][0]}->#{x[1][0]})"
      cmd = "#{reformatBinary} -i #{x[0][0]} -o #{x[1][0]}"
      got = nil
      cur += 1
      IO.popen(cmd, "w+") { |lb|      
        File.open(x[0][1], "r").each {|l| lb.write(l)}
        lb.close_write
        got = lb.read
      }
      want = IO.read(x[1][1])
      if (got == want)
        puts "ok #{cur} - #{explanation}"
        passed += 1
      else 
        puts "not ok #{cur} - #{explanation}"
        puts "#<<<want<<<"
        puts want.gsub(/^/,"#")
        puts "#========"
        puts got.gsub(/^/,"#")
        puts "#>>got>>"
      end
    }
  end
}

puts "# (#{passed}+#{skipped} skipped)/#{total} tests successful"
exit ((ENV["NO_SKIPPING"]) ? passed == total : (passed + skipped) == total)
