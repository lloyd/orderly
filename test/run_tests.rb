#!/usr/bin/env ruby

rv = 0 

system("./run_cases.rb")
rv += $?.to_i
system("./run_negative_cases.rb")
rv += $?.to_i

puts "TESTS FAILED!" if rv
exit rv


