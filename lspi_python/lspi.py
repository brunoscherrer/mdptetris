#!/usr/bin/python


from pylab import *
from sys import stdout
from os import system

from mdp import mdp   # import mdp class from mdp.py file

from lspi_x import make_figure   # to make nice drawings



##############################################################################
# Computation of feature vectors for all (s,a) for all (s,s2) of the data set

def precomputation_of_features_dataset(samples,value):
    
    print "Precomputation of feature vectors for the data set",
    stdout.flush()
    
    nb_actions=value.mdp.nb_actions
    phi=[]
    phi2=[]
    ns=len(samples)
    i=0
    for (s,a,r,s2) in samples:
        l=[]
        l2=[]
        for a in xrange(nb_actions):
            l.append(value.feature_vector(s,a))
            if s2!=[]:
                l2.append(value.feature_vector(s2,a))
        phi.append(l)
        phi2.append(l2)

        i=i+1
        if i%(ns/50)==0:
            print ".",
            stdout.flush()
    print
            
    return phi,phi2
            

def precomputation_of_features_figure(value,res=50,dimx=0,dimy=1):

    print "Precomputation of feature vectors for the figures",
    stdout.flush()

    m=value.mdp
    nb_actions=m.nb_actions

    phi=[]
    x = arange(m.space[dimx][0], m.space[dimx][1], m.range[dimx]/res)
    y = arange(m.space[dimy][0], m.space[dimy][1], m.range[dimy]/res)
    X,Y = meshgrid(x, y)
    for i in xrange(res):
        print ".",
        stdout.flush()                
        for j in xrange(res):
            l=[]
            for a in xrange(nb_actions):
                l.append(value.feature_vector([X[i][j],Y[i][j]],a))
            phi.append(l)
    print
            
    return res,X,Y,phi,dimx,dimy
            

###########################
# LS(lambda)PI  Algorithm


def lspi_one_iteration(lbda, samples, phi_samples, phi2_samples, value, projmethod):

    gamma=value.mdp.gamma
    
    A=zeros((value.nb_features, value.nb_features))
    b=zeros(value.nb_features)

    print "Going through the samples to build the linear system",
    l=len(samples)

    i=0
    for (s,a,r,s2) in samples:        

        phi=phi_samples[i][a] # phi(s,a)
        
        if s2!=[]: # if non terminal

            phi2=phi2_samples[i][value.greedy_action(s2,phi2_samples[i])] # phi(s2,greedy(v)(s2))

            #    TD projection
            if projmethod=="TD":                                     
                A += matrix(phi).H * (phi-lbda*gamma*phi2)
                b += phi*(r+(1-lbda)*gamma*dot(phi2,value.w))

            #     BR projection   
            else:                
                A += matrix(phi-lbda*gamma*phi2).H * (phi-lbda*gamma*phi2) # ATTENTION: biased version, only works for deterministic problems
                b += (phi-lbda*gamma*phi2) * (r+(1-lbda)*gamma*dot(phi2,value.w))
                
        else:   # if terminal state
            A += matrix(phi).H * phi
            b += phi*r
            
        i=i+1
        if i%(l/10)==0:
            print ".",
            stdout.flush()
            
    print "Solving the system...",
    stdout.flush()    
    w=solve(A,b)
    
    # compute fit error, i.e:
    # 1/nb_samples * sum_{s,a,r,s'} Phi(s,a)w' - lambda*gamma*Phi(s',pi_w(s'))w' - r - (1-lambda)*gamma*Phi(s',pi_w(s'))w
    err=0
    i=0
    for (s,a,r,s2) in samples:
        phi=phi_samples[i][a]
        if s2!=[]:
            phi2=phi2_samples[i][value.greedy_action(s2,phi2_samples[i])]            
            err += square (dot (phi-lbda*gamma*phi2,w) - r - (1-lbda)*gamma*dot(phi2,value.w))
        else:
            err += square (dot (phi,w) - r)
        i=i+1
    err/=l

    # store  the new value
    value.w=w
    
    # compute the bellman error, i.e:
    # 1/nb_samples * sum_{s,a,r,s'} Phi(s,a) w'-r- gamma * Phi(s',pi_w'(s'))w' ]^2
    berr=0
    i=0
    for (s,a,r,s2) in samples:
        psi=phi_samples[i][a]
        if s2!=[]:
            psi=psi-gamma*phi2_samples[i][value.greedy_action(s2,phi2_samples[i])]
        berr += square (dot (psi,w) - r)
        i=i+1
    berr/=l
    
    return err,berr
        


def lspi(lbda, samples, value, nb_iterations=30, projmethod="TD", test_start_states=[], max_traj_length=200, precomputed_features_dataset=[], precomputed_features_figure=[], filename=""):

    print "*** LSPI, lambda=%f, %i samples, %i features, proj=%s, test from %i states, iterates at most %i times ***"%(lbda, len(samples), value.nb_features, projmethod, len(test_start_states),nb_iterations)

    # Precomputations
    if precomputed_features_dataset==[]:
        stdout.flush()
        phi_samples,phi2_samples=precomputation_of_features_dataset(samples,value)
    else:
        phi_samples,phi2_samples=precomputed_features_dataset

    if precomputed_features_figure==[]:
        precomputed_features_figure=precomputation_of_features_figure(value)


    fig=figure(figsize=(8,8),facecolor="white")
    lw,ldw,lerr,lberr,lperf=[],[],[],[],[]

    for i in xrange(nb_iterations):

        # Doing one iteration
        print "Iteration",i,":"
        w=value.w[:] # copy
        err,berr = lspi_one_iteration(lbda, samples, phi_samples, phi2_samples, value, projmethod)
        dw=sqrt(sum(square(value.w-w))/value.nb_features)
        print "dw=",dw,"err=",err,"berr=",berr                                      

        # Storing data
        ldw.append(dw)
        lerr.append(err)
        lberr.append(berr)

        # Testing empirically the new greedy policy        
        lt, perf = value.evaluate_policy(test_start_states,max_traj_length)
        print "perf=",perf
        lperf.append(perf)
        
        # show figure and save images
        make_figure(fig,i,value,ldw[0:i+1],lerr[0:i+1],lberr[0:i+1],lperf[0:i+1],lt,precomputed_features_figure)
        fname = '%s_tmp%03d.png'%(filename,i)
        savefig(fname)
        
        if dw<1e-6:            
            break
    
    # Make the animation and clean the temporary files        
    for j in xrange(i+1,i+10): # ten still images when finished
        fname2 = '%s_tmp%03d.png'%(filename,j)
        system("cp %s %s"%(fname,fname2))
    print 'Making movie animation.mpg'
    system("mencoder 'mf://%s_tmp*.png' -mf type=png:fps=2 -ovc lavc -lavcopts vcodec=wmv2 -oac copy -o %s.mpg"%(filename,filename))
    system("rm %s_tmp*.png"%(filename))

    return lberr, lperf
