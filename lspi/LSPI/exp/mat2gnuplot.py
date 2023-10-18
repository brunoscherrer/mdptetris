#!/usr/bin/python

import sys

iteration = 0
for line in sys.stdin.readlines():

  if len(line) <= 1:
    sys.stdout.write("\n\n")
    iteration = 0
  else:
    if line[0] != '#':
      sys.stdout.write("%d %s" % (iteration, line))
      iteration = iteration + 1

