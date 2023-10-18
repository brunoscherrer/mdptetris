#!/usr/bin/python

# Mountain Car MDP class

from pylab import *

from mdp import mdp   # import mdp class from mdp.py file




### "Sutton" mountain car

class mountain_car(mdp):   # inherits from mdp class
    
    def __init__(self):
        
        self.space=[ [-1.2,0.5],[-0.07,0.07] ]
        self.nb_actions=3
        self.gamma=1.0
        mdp.__init__(self)

    def samples_nextstate_and_reward(self,s,a):
        "returns a reward and a new state"

        s2=[0,0]

        # speed variation
        if a==2:
            s2[1]=s[1]+0.001
        elif a==0:
            s2[1]=s[1]-0.001
        else: #a==1
            s2[1]=s[1]
        s2[1]-=0.0025*cos(3*s[0])
        s2[1]=min(0.07,max(-0.07,s2[1]),s2[1]) # bound between -0.07 and 0.07

        # change of position
        s2[0]=s[0]+s[1]
        if s2[0]>0.5:
            r=0
            s2=[] # terminal state
        elif s2[0]<-1.2:
            s2[0]=-1.2
            s2[1]=0          # speed is reset to 0
            r=-1
        else:
            r=-1

        return s2,r


# A variant a la "Munos and Moore"
class mountain_car2(mdp):   # inherits from mdp class
    
    def __init__(self):
        
        self.space=[ [-1.2,0.5],[-0.07,0.07] ]
        self.nb_actions=2
        self.gamma=0.99
        mdp.__init__(self)

    def samples_nextstate_and_reward(self,s,a):
        "returns a reward and a new state"

        s2=[0,0]

        # speed variation
        if a==1:
            s2[1]=s[1]+0.001
        else: #a==0
            s2[1]=s[1]-0.001
        s2[1]-=0.0025*cos(3*s[0])
        s2[1]=min(0.07,max(-0.07,s2[1]),s2[1]) # bound between -0.07 and 0.07

        # change of position
        s2[0]=s[0]+s[1]
        if s2[0]>0.5:
            r=1-2*abs(s2[1])/0.07  # reward depends on speed
            s2=[] # good terminal state
        elif s2[0]<-1.2:
            r=-1
            s2=[] # bad terminal state
        else:
            r=0

        return s2,r
