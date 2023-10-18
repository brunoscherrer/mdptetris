#!/usr/bin/python

# This script executes least-squares lambda policy iteration 10 times on a 10x10 board, varying lambda

import os

prog="mdptetris_lspi"
number_of_times=1
base_name = "./"

# List of parameters 
parameters =[ \
    [  "-initial_features",  ["features/lagoudakis_initial.dat"], ""], \
    [  "-lambda",             ["0.0","0.1","0.2","0.3","0.4","0.5","0.6","0.7","0.8","0.9","0.95","0.98","0.99","0.995","0.998","0.999","1.0"], "%s" ], \
#    [  "-lambda",             ["0.0","0.1","0.2","0.3","0.4","0.5","0.6","0.7","0.8","0.9","1.0"], "%s" ], \
    [  "-height",     ["20"], "" ], \
    [  "-load_samples",     ["samples1000.h20.dat"], "" ], \
    [  "-method",     ["0", "1"], ".m%s" ], \
#    [  "-gamma",      ["0.9", "0.99"], ".g%s" ], \
#    [  "-gamma",      ["1.0"], ".g%s" ], \
    ]

              
# Launch a command in a shell
def command(cmd):
    print cmd
    os.popen(cmd)


# recursively generate all parameters and filenames and launch the corresponding commands
def run_with_all_parameters(parameters,cmd,filename,suffix):
    if (parameters==[]):
        command(cmd + " -statistics_file " + filename+suffix)
        command("cat " + filename+suffix + " >> " + filename)
    else:
        for i in range(len(parameters[0][1])):
            cmd2 = cmd + " " + parameters[0][0] + " " + parameters[0][1][i]
            if parameters[0][2].__class__ is list:
                filename2 = filename + parameters[0][2][i]
            elif parameters[0][2]!="":
                filename2 = filename + parameters[0][2] % (parameters[0][1][i])
            else:
                filename2 = filename
            run_with_all_parameters(parameters[1:],cmd2,filename2,suffix)


# repeat n times
for n in range(number_of_times):
    run_with_all_parameters(parameters,prog + " -seed %d" % (n+1), base_name,".%d" % (n))

