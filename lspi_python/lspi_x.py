#!/usr/bin/python


from pylab import *
from mpl_toolkits.mplot3d.axes3d import Axes3D
from matplotlib import font_manager
from os import system
from sys import stdout

from mdp import mdp   # import mdp class from mdp.py file



#############################################
# A few functions for drawing nice graphs

# Drawing the samples used by lspi
   
def plot_samples(mdp,samples,dimx=0,dimy=1):
    figure(figsize=(5,5))
    ioff()
    for (s,a,r,s2) in samples:
        if s2==[]:
            plot( [s[dimx]],[s[dimy]],'ro')
        else:
            plot([s[dimx],s2[dimx]], [s[dimy],s2[dimy]])
        axis([mdp.space[dimx][dimx],mdp.space[dimx][dimy],mdp.space[dimy][dimx],mdp.space[dimy][dimy]])
    draw()
    ion()


# Drawing the value, the optimal policy, and some trajectories

def plot_value_and_policy(fig, value, traj, precomp):
        
    res,X,Y,phi,dimx,dimy=precomp

    # value policy data preparation
    Z=[]
    pi=[]
    print "Generating the figure",
    stdout.flush()
    c=0
    for i in xrange(res):
        print ".",
        stdout.flush()
        Z.append([])
        pi.append([])
        for j in xrange(res):
            z=value.value_and_greedy_action([X[i][j],Y[i][j]],phi[c])
            Z[i].append( z[0] )
            pi[i].append( z[1] )
            c=c+1

    # 3D value drawing
    title('Value')
    #pcolor(X, Y, Z, cmap=cm.bone)
    #colorbar()
    ax = Axes3D(fig,rect=[0,.5,.47,.47],elev=70,azim=-80)
    ax.plot_surface(X,Y,array(Z),cmap=cm.bone, rstride=1, cstride=1,linewidth=0, antialiased=False)

    # Policy drawing
    subplot(222)
    title('Policy')
    pcolor(X, Y, pi, cmap=cm.gray)

    # Trajectory drawing    
    for t in traj:        
        lx,ly=[],[]
        for (s,a,r,s2) in t:            
            lx.append(s[0])
            ly.append(s[1])
        plot(lx,ly)
        axis([value.mdp.space[dimx][0],value.mdp.space[dimx][1],value.mdp.space[dimy][0],value.mdp.space[dimy][1]]) 

    print


# Main drawing function 

def make_figure(fig,i,value,ldw,lerr,lberr,lperf,traj,precomp):

    ioff()
    fig.clf()
    suptitle('Iteration %d'%i,fontsize=20)

    plot_value_and_policy(fig,value,traj,precomp)            
    subplot(425)        
    title('Weight variation')
    plot(ldw)        
    yscale('log')
    subplot(427)
    title('Errors')
    plot(lerr,label="Fit error")
    plot(lberr,label="Bellman error")
    best=lberr.index(min(lberr))
    plot([best],[lberr[best]],'ro')
    yscale('log')
    legend(prop=font_manager.FontProperties(size=7),loc=0)
    subplot(224)
    title('Performance')
    plot(lperf)
    plot([best],[lperf[best]],'ro')
    draw()
    ion()



