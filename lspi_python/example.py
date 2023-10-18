#!/usr/bin/python


from mountain_car import mountain_car     # import the mountain_car MDP
from mdp import fourier_based_value       # import the polynom based value
from lspi import lspi                     # import the algorithm 
from lspi_x import plot_samples

# make the figure appear
from pylab import ion,savefig
ion()




#initialize model
m=mountain_car()

# initialize projection basis, fourier up to 5x5
v=fourier_based_value(m,5)

# generate random starting states for empirical evaluation of policies
t=m.random_states(50)

# generate 100 trajectories of length 10
s=m.generate_trajectories(100,10)
plot_samples(m,s)
savefig("samples.eps")

# run LSPI with lambda=0.9 for at most 200 iterations, using the Bellman residual projection method, and evaluating the policy with at most 200 steps, generating a video "example.mpg"
lspi(0.5,s, v, 200, "BR",t,200,[],[],"example")

