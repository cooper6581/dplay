#!/usr/bin/env python

import sys

f = open(sys.argv[1])
data = []
blah = []

for line in f.readlines():
  if (line[0] == ';'):
    raw = line.rstrip().split()
    if(len(raw) > 2):
      if(raw[1] == 'Tuning'):
        if blah:
          data.append(blah)
        blah = []
        blah.append(raw[2])
  else:
    nums = line.rstrip().split(',')
    for n in nums:
      blah.append(n)
data.append(blah)
f.close()

for n in xrange(1,37):
  for t in xrange(0,16):
    print data[t][n],
  print
    
