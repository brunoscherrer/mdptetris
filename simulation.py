#!/usr/bin/python

import os, sys

# This module executes a simulation (for example LPI or CE) and returns the mean score of the best iteration
# (i.e. the best line in the statistic file).
# The command should not include the parameter -statistics_file because it is added automatically.

def execute(cmd, statistics_file):
    cmd = "%s -statistics_file %s" % (cmd, statistics_file)
    sys.stdout.write("%s\n" % cmd)
    sys.stdout.flush()

    # execute the command
    os.popen(cmd)

    # read the statistic file to get the best episode realised, and make the concatenation
    f = open(statistics_file)
    simulation_score = 0
    for line in f:
        
        # get the score of each line (i.e. each iteration of the execution) and keep the best one
        if (line[0] != '#'): # do not consider the lines with comments
            score = float(line.split()[1]) # the mean score of the episode is the second word of the line
            if score > simulation_score:
                simulation_score = score
                
    # print the best score of this execution
    sys.stdout.write("%d\n" % simulation_score)
    sys.stdout.flush()
 
    return simulation_score
