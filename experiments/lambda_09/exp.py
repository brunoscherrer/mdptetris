#!/usr/bin/python

# This script executes lambda policy iteration 10 times on a 10x10 board and 10x20 board, varying lambda and the -evaluate_gameover_with_features parameter

import os

prog="mdptetris_lambda_pi"
number_of_times=10
base_name = "./"

# List of parameters 
parameters =[ \
    [  "-initial_features",  ["../../data/features/bertsekas_initial.dat"], ""], \
    [  "-lambda",             ["0.0","0.1","0.2","0.3","0.4","0.5","0.6","0.7","0.8","0.9","1.0"], "%s" ], \
    [  "-nb_iterations",     ["200"], "" ], \
    [  "-nb_games_batch",     ["100"], "" ], \
    [  "-height",     ["19"], "" ], \
    [  "", ["-gameover_evaluation"], ["b"] ], \
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
            
