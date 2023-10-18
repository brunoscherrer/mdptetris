#!/usr/bin/python

# Navigation MDP class

from pylab import *

from mdp import mdp   # import mdp class from mdp.py file



class navigation(mdp):   # inherits from mdp class
    
    def __init__(self):
        
        self.space=[ [0.0,10.0],[0.0,10.0] ]
        self.nb_actions=4
        self.gamma=0.99
        mdp.__init__(self)


    def samples_nextstate_and_reward(self,s,a):
        "returns a reward and a new state"

        step=0.1
        theta=2*pi*a/self.nb_actions
        dx=step*cos(theta)
        dy=step*sin(theta)
        
        s2=[s[0]+dx,s[1]+dy]

        r=0 # 0 reward unless...

        # ... goal or...
        if s2[0]>4 and s2[0]<6 and s2[1]>4 and s2[1]<6:
            s2=[]
            r=1
        # obstacles
        elif s2[0]<0 or s2[0]>10 or s2[1]<0 or s2[1]>10:# or (s2[0]>7 and s2[0]<8 and s2[1]>2 and s2[1]<8):# or (s2[0]>2 and s2[0]<8 and s2[1]>7 and s2[1]<8):
            s2=[]
            r=-1
            

        return s2,r


