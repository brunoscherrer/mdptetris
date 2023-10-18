#!/usr/bin/python

# This script executes lambda policy iteration 10 times on a 10x10 board

import os

prog="mdptetris_lambda_pi"
number_of_times=10
base_name = "exp10/"

# List of parameters 
parameters =[ \
    [  "-initial_features",  ["../../features/bertsekas_initial.dat"], ""], \
    [  "-lambda",            ["0.0","0.3","0.5","0.7"], "l%s_" ], \
    [  "",                   ["", "-avoid_gameover_actions"], ["na_","a_"] ], \
    [  "-nb_iterations",     ["500"], "" ], \
    [  "-height",            ["10"], "" ], \
    [  "-stepsize",          ["100000 1000000", "500000 1000000","10 10"], ["s.1", "s.5", "s10_10"] \
    ]
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
for n in [0,1,2,3,4]:
    run_with_all_parameters(parameters,prog,base_name,".%d" % (n))
            
