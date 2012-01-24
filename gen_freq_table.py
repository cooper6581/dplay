#!/usr/bin/env python

import sys

f = open(sys.argv[1])
data = []
for line in f.readlines():
  nums = line.rstrip().split(',')
  for n in nums:
      data.append(n)

f.close()

print len(data)

for d in data:
  print "%s," % (d,),
