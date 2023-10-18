#!/usr/bin/python

# This script executes lambda policy iteration 10 times on a 10x10 board

import simulation
import os

number_of_times=10
base_name = "exp7/"

# parameters

lambdas=[0.3, 0.5, 0.7, 0.9]
eog=["","-avoid_gameover_actions"]


for n in range(number_of_times):
    for i in eog:
        for l in lambdas:

            name = "%sl=%f%s" % (base_name, l, i)
            name2 = "%s.%d" % (name, n)
            final_feature_file = "%s.features" % (name2)
            statistics_file = "%s.stat" % (name2)
        
            cmd = "mdptetris_lambda_pi -bias_end_of_game -nb_iterations 100 -height 10 -lambda %f %s -initial_features ../../features/bertsekas_initial.dat -final_features %s -statistics_file %s" % (l, i, final_feature_file,statistics_file)
            print cmd
            os.popen(cmd)
            
            cmd="cat %s >> %s" % (statistics_file, name)
            print cmd
            os.popen(cmd)
            
