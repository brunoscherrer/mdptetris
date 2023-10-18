#!/usr/bin/python

# -avoid_gameover_features
# plus de valeurs de rho, de nb_samples_generated
# avec du bruit constant

import simulation, os

a_nb_evaluating_games = [1, 2, 5, 10]
a_nb_samples_generated = [60, 100, 200, 300]
a_rho = ["0.05", "0.1", "0.2", "0.3", "0.4"]
a_noise = ["none", "constant 4"]

for n in range(10): # execute each simulation 10 times
    for nb_evaluating_games in a_nb_evaluating_games:
        for nb_samples_generated in a_nb_samples_generated:
            for noise in a_noise:
                for rho in a_rho:
                    # compute the path
                    path = "simulations/exp_ce/%d_%d" % (nb_samples_generated, nb_evaluating_games)
                    if not os.path.isdir(path):
                        os.makedirs(path, 0755)
                        
                    if (noise == "none"):
                        short_noise = ""
                    else:
                        short_noise = "c"                            
                        
                    # compute the file names and the command
                    final_feature_file = "%s/f%s%s-%d" % (path, short_noise, rho, n)
                    statistics_file = "%s/s%s%s-%d" % (path, short_noise, rho, n)
                    statistics_file_concatenation = "%s/s%s%s" % (path, short_noise, rho)
                    
                    cmd = "./cross_entropy -height 10 -avoid_gameover_actions -nb_episodes 80 -initial_features ce_bertsekas.dat -final_features %s -nb_evaluating_games %d -nb_vectors_generated %d -rho %s -noise %s" % (final_feature_file, nb_evaluating_games, nb_samples_generated, rho, noise)
                    
                    # execute the simulation
                    score = simulation.execute(cmd, statistics_file)
                    
                    # copy the statistic file into the concatenation file
                    src = open(statistics_file)
                    dst = open(statistics_file_concatenation, "a")
                    dst.write("# Command: %s\n" % cmd)
                    for line in src:
                        dst.write("%s" % line)
