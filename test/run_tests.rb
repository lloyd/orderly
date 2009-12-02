#!/usr/bin/env ruby

rv = 0 

mypath = File.dirname(__FILE__)

system(File.join(mypath, "run_cases.rb"))
rv += $?.to_i
system(File.join(mypath, "run_negative_cases.rb"))
rv += $?.to_i

puts "TESTS FAILED (#{rv})!" if rv > 0
exit rv


