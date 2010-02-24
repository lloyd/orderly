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
tests = Dir.glob(File.join(casesDir, "*.orderly"))
puts "1..#{tests.length}"
puts "#Running error reporting tests: "
puts "#(containing '#{substrpat}' in name)" if substrpat && substrpat.length > 0



tests.each { |f| 
  total += 1
  next if substrpat && substrpat.length > 0 && !f.include?(substrpat)

  program = checkBinary
  what = "Checking" 
  wantFile = f.sub(/orderly$/, "output")
  gotFile = wantFile + ".got"
  got = ""
  explanation = "#{what} <#{f.sub(/^.*?([^\/]+)\.orderly$/, '\1')}> "
  IO.popen(program, "w+") { |lb|
    File.open(f, "r").each {|l| lb.write(l)}
    lb.close_write
    got = lb.read
  }
  if !File.exist? wantFile
    todoFile = f + ".todo"
    if File.exist? todoFile
      todoexp = IO.read(todoFile)
      explanation += "# TODO - #{todoexp}"    
      passed += 1
    else
      explanation += "'goldfile' doesn't exist: #{wantFile} (left #{gotFile})"
      File.open(gotFile, "w+") { |gf| gf.write got }
    end
    want = ""
  else
    want = IO.read(wantFile)
  end
  if (got == want)
    puts "ok #{total} - #{explanation}"
    passed += 1
  else
      puts "not ok #{total} - #{explanation}"
      puts "#<<<want<<<"
      puts want.gsub(/^/,"#")
      puts "#========"
      puts got.gsub(/^/,"#")
      puts "#>>got>>"
  end
}

exit passed == total
