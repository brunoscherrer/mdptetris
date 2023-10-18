#!/usr/bin/python

# This script executes lambda policy iteration 10 times on a 10x10 board

import os

prog="./lambda_policy_iteration"
number_of_times=10
base_name = "exp9/"

# List of parameters 
parameters =[ \
    [  "-bias_end_of_game",  ["0","1","2"], "b%s_"  ], \
    [  "-lambda",            ["0.0","0.1","0.2","0.3","0.4","0.5","0.6","0.7","0.8","0.9"], "l%s_" ], \
    [  "",                   ["", "-avoid_gameover_actions"], ["na","a"] ], \
    [  "-nb_iterations",     ["100"], "" ], \
    [  "-height",            ["10"], "" ] \
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
    run_with_all_parameters(parameters,prog,base_name,".%d" % (n))
            
