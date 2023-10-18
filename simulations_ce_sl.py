#!/usr/bin/python

# This script executes cross entropy 10 times with the original
# parameters of Szita & Lorincz, trying to reproduce their results.

import simulation

path = "simulations/sl"

noises = ["none", "constant 4", "linear 5 10"]
noise_names = ["none", "constant", "linear"]

for n in range(10): # execute each simulation 10 times
    for i in range(3):
        
        # compute the file names and the command
        noise_name = noise_names[i]
        noise = noises[i]
        final_feature_file = "%s/features_%s.%d" % (path, noise_name, n)
        statistics_file = "%s/statistics_%s.%d" % (path, noise_name, n)
        statistics_file_concatenation = "%s/statistics_%s" % (path, noise_name)
        
        cmd = "./cross_entropy -avoid_gameover_actions -nb_episodes 80 -initial_features ce_bertsekas.dat -final_features %s -noise %s" % (final_feature_file, noise)
        
        # execute the simulation
        score = simulation.execute(cmd, statistics_file)
        
        # copy the statistic file into the concatenation file
        src = open(statistics_file)
        dst = open(statistics_file_concatenation, "a")
        for line in src:
            dst.write("%s\n" % line)
            
