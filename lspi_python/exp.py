#!/usr/bin/python

# Experiments with LSPI

# Optimization
import psyco
psyco.full()


from pylab import *
from pickle import dump,load

from mdp import mdp, polynom_based_value,fourier_based_value  # import mdp class from mdp.py file
from mountain_car import mountain_car,mountain_car2     # import the mountain_car MDP
from navigation import navigation         # impott the navigation MDP

from lspi import lspi,precomputation_of_features_dataset,precomputation_of_features_figure                # import the algorithm
from lspi_x import plot_samples

from matplotlib import font_manager
from sys import argv

######################################################
# Functions for saving/loading/plotting the learning curves


def save_learning_curves(filename,results):
    f=open(filename,"w")
    dump(results,f)
    f.close()

def load_learning_curves(filename):
    f=open(filename,"r")
    results=load(f)
    f.close()
    return results

def plot_learning_curves(results):
    figure(figsize=(10,5))
    subplot(121)
    for r in results:
        plot(r[2],label=r[0])
    legend(prop=font_manager.FontProperties(size=7),bbox_to_anchor=(1.05, 1), loc=2, borderaxespad=0.)
    draw()

def sort_learning_curves_real_perf(results,filter=""):    
    l=[]
    for r in results:
        if r[0].find(filter) != -1:
            l.append((r[0],max(r[1])))
    return sorted(l, key=lambda x: x[1])

def sort_learning_curves(results,filter="",sort=1):    
    l=[]
    for r in results:
        if r[0].find(filter) != -1:            
            l.append((r[0],r[1][r[2].index(min(r[2]))],min(r[2])))
    return sorted(l, key=lambda x: x[sort],reverse=(sort==2))

def sort_learning_curves_br(results,filter=""):    
    return sort_learning_curves(results,filter,2)


def list_divergence(results,filter=""):    
    l=[]
    for r in results:
        if r[0].find(filter) != -1 and r[2][-1]>1e3:            
            l.append(r[0])
    return sorted(l, key=lambda x: x[1])


##########################################################
# experiment 1


def exp1():

    print "Generating experiments %s to %s"%(argv[1],argv[2])
    dir='./exp1/'

    m=mountain_car()

    test_states=[]
    for x in arange(m.space[0][0]+m.range[0]/14,m.space[0][1],m.range[0]/7):
        for y in arange(m.space[1][0]+m.range[1]/14,m.space[1][1],m.range[1]/7):
            test_states.append([x,y])
           
    nb_samples=500
    for i in xrange(int(argv[1]),int(argv[2])+1):
        results=[]
        samples=m.generate_trajectories(nb_samples,1)
        f=open("%ssamples_%d"%(dir,i),'w')
        dump(samples,f)
        f.close()
        for fd in [3, 4, 5, 6, 7, 8, 9, 10]:
            v=fourier_based_value(m,fd)
            prec_feat_d=precomputation_of_features_dataset(samples,v)
            prec_feat_f=precomputation_of_features_figure(v)
            for l in [0.0, 0.3,0.6,0.9,1.0]:
                for pm in ["TD","BR"]:

                    filename='%smc-fd=%d_l=%s_pm=%s_%d'%(dir, fd,str(l),pm,i)

                    # Running the LSPI algorithm from v=0
                    v.w=zeros(v.nb_features) 
                    br_curve, learning_curve = lspi(l,samples,v,200,pm,test_states,200,prec_feat_d,prec_feat_f,filename)
                    
                    print "Updating allresults file with new perf curves"
                    results.append( (filename, learning_curve, br_curve) )
                    save_learning_curves("%sexp1_%d"%(dir,i),results)

                    close() # close the figure
                        



# put interactive mode on
ion()
#exp1()


