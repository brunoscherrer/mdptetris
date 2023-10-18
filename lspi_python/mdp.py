#!/usr/bin/python

# Classes for mdp, samples, feature based representation


from pylab import *
from sys import stdout

##################################################################"
# ABSTRACT MDP class for continuous valued multidimensional MDP


class mdp:   

    def __init__(self):
        self.nb_dim=len(self.space)
        self.range=[]      # initialize the range from self.space
        for x in self.space:
            self.range.append(x[1]-x[0])

    def random_state(self):
        s=[]
        for j in xrange(self.nb_dim): # random position
            s.append( self.space[j][0]+random()*self.range[j] )
        return s

    def random_states(self,n):
        l=[]
        for i in xrange(n):
            l.append(self.random_state())
        return l

    def random_action(self):
        return randint(self.nb_actions)

    def generate_trajectories(self, start_states, max_traj_length, value=[]):

        if type(start_states) is list:
            ssl=start_states
        else:
            ssl=[]
            for i in xrange(start_states):
                ssl.append(self.random_state())

        l=[]
        for ss in ssl:
            s=ss[:]
            for j in xrange(max_traj_length):
                if value==[]:
                    a=self.random_action()
                else:
                    a=value.greedy_action(s)
                s2,r=self.samples_nextstate_and_reward(s,a)
                l.append((s,a,r,s2))
                s=s2
                if s==[]:
                    break
        return l

    def normalize_state(self,s,r=1.0): #normalize each dimension between 0 and 1    
        x=[0]*self.nb_dim
        for i in xrange(self.nb_dim):
            x[i]= r*(s[i]-self.space[i][0])/self.range[i] 
        return x


###############################################
# ABSTRACT Feature based value representation

class feature_based_value:   

    def __init__(self,mdp):
        self.mdp=mdp
        self.w=[0.0]*self.nb_features

    def qvalue(self,s,a,precomputed_features=[]): # l may be a vector of precomputed vectors for all actions in s
        if precomputed_features==[]:
            return dot( self.w, self.feature_vector(s,a) )            # Q(s,a)=\Phi(s,a) w
        else:
            return dot( self.w, precomputed_features[a] )

    def value_and_greedy_action(self,s,precomputed_features=[]): 
        
        value=self.qvalue(s,0,precomputed_features)
        amax=0
        for a in xrange(1,self.mdp.nb_actions):
            qv=self.qvalue(s,a,precomputed_features)
            if qv>value:
                value=qv
                amax=a
        return value,amax

    def value(self,s,precomputed_features=[]):
        return self.value_and_greedy_action(s,precomputed_features)[0]

    def greedy_action(self,s,precomputed_features=[]):
        return self.value_and_greedy_action(s,precomputed_features)[1]

    def evaluate_policy(self,test_start_states,max_traj_length):
        if test_start_states==[]:
            return [],0
        else:
            print "Evaluating the policy from",len(test_start_states),"states",
            stdout.flush()
            traj_list=[]
            gamma=self.mdp.gamma
            c=0
            sum_reward=0
            for ss in test_start_states:                
                traj=self.mdp.generate_trajectories([ss], max_traj_length, self)
                traj_list.append(traj)
                d=1.0    
                for (s,a,r,s2) in traj:
                    sum_reward += d*r
                    d *= gamma
                c=c+1
                print ".",
                stdout.flush()
            perf=sum_reward/c
            return traj_list,perf
            



##########################
# Fourier representation

class fourier_based_value(feature_based_value):
    
    def __init__(self,mdp,fd):        
        self.fourier_depth=fd
        self.nb_fourier_vectors=pow(self.fourier_depth,mdp.nb_dim)
        self.nb_features=mdp.nb_actions*self.nb_fourier_vectors        
        feature_based_value.__init__(self,mdp)
        self.fourier_vectors=self.generate_fourier_vectors()
        
        
    def generate_fourier_vectors_rec(self,l,c,n):
        if n==self.mdp.nb_dim:
            l.append(array(c))
        else:
            for i in xrange(self.fourier_depth):
                l=self.generate_fourier_vectors_rec(l,c+[i],n+1)
        return l

    def generate_fourier_vectors(self):
        return self.generate_fourier_vectors_rec([],[],0)

    
    def feature_vector(self,s,a):    
        phi=zeros(self.nb_features)
        idx=self.nb_fourier_vectors*a
        x=self.mdp.normalize_state(s,pi) # between 0 and pi
        for i in xrange(0,self.nb_fourier_vectors):            
            phi[idx+i]=1.0
            for j in xrange(self.mdp.nb_dim):
                phi[idx+i] *= cos( self.fourier_vectors[i][j]*x[j] )
        return phi




############################
# Polynomial representation

class polynom_based_value(feature_based_value):
    
    def __init__(self,mdp,order):        
        self.order=order        
        self.polynom_vectors=self.generate_polynom_vectors(mdp.nb_dim)
        self.nb_polynom_vectors=len(self.polynom_vectors)
        self.nb_features=mdp.nb_actions*self.nb_polynom_vectors
        feature_based_value.__init__(self,mdp)
                
    def generate_polynom_vectors_rec(self,l,c,n,nb_dim):
        if n==nb_dim:
            if sum(c)<=self.order:
                l.append(array(c))
        else:
            for i in xrange(self.order+1):
                l=self.generate_polynom_vectors_rec(l,c+[i],n+1,nb_dim)
        return l

    def generate_polynom_vectors(self,nb_dim):
        return self.generate_polynom_vectors_rec([],[],0,nb_dim)

    def feature_vector(self,s,a):    
        phi=zeros(self.nb_features)
        idx=self.nb_polynom_vectors*a
        x=self.mdp.normalize_state(s,1) # between 0 and 1
        for i in xrange(0,self.nb_polynom_vectors):
            phi[idx+i]=1.0
            for j in xrange(0,self.mdp.nb_dim):
                phi[idx + i] *=  pow(x[j],self.polynom_vectors[i][j])
        return phi



