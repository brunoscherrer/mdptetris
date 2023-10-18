#!/usr/bin/python

# This script executes lambda policy iteration 10 times on a 10x10 board

import os

number_of_times=10
base_name = "exp8/"

# parameters

bias=["0", "1", "2"]
lambdas=["0.0", "0.3", "0.5", "0.7", "0.9"]
eog=["","-avoid_gameover_actions"]


for n in range(number_of_times):
    for e in eog:
        for l in lambdas:
            for b in bias:

                name = "%s%s_%s_%s" % (base_name, l, b, e)
                name2 = "%s.%d" % (name, n)
                final_feature_file = "%s.features" % (name2)
                statistics_file = "%s.stat" % (name2)
                
                cmd = "mdptetris_lambda_pi -bias_end_of_game %s -nb_iterations 100 -height 10 -lambda %s %s -initial_features ../../features/bertsekas_initial.dat -final_features %s -statistics_file %s" % (b, l, e, final_feature_file,statistics_file)
                print cmd
                os.popen(cmd)
                
                cmd="cat %s >> %s" % (statistics_file, name)
                print cmd
                os.popen(cmd)
            
