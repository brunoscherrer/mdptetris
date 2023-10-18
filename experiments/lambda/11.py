#!/usr/bin/python

# This script executes lambda policy iteration 10 times on a 10x10 board

import os

prog="mdptetris_play_games"
number_of_times=1
base_name = "exp11/"

# List of parameters 
parameters=[ \
    [  "", ["../../features/dellacherie_initial.dat"], ""], \
    [  "", ["1000"], ""], \
    [  "", ["4","5","6","7","8","9","10"], "%s_" ], \
    [  "", ["4", "5","6","7","8","9","10","11","12","13","14","15","16","17","18","19","20"], "%s" ] \
    ]

              
# Launch a command in a shell
def command(cmd):
    print cmd
    os.popen(cmd)


# recursively generate all parameters and filenames and launch the corresponding commands
def run_with_all_parameters(parameters,cmd,filename,suffix):
    if (parameters==[]):
        command(cmd + " " + filename)
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

