
import numpy as np

from scipy.spatial.distance import pdist
from scipy.spatial.distance import squareform
import pylab as pl
import csv
import os as os

from math import floor
from math import sqrt

from sklearn import cluster, datasets
from sklearn.metrics import euclidean_distances
from sklearn.neighbors import kneighbors_graph
from sklearn.preprocessing import StandardScaler

period = [2*3.14159265, 2*3.14159265]
periodNP = np.array([2*3.14159265, 2*3.14159265])

def loadData(filename):
    for line in csv.reader(open(filename), delimiter=' '):
        if line:
            yield line

def periodicDistance(xs, ys, ps):
    ''' given two d dimensional points x and y, of which some dimensions may be periodic, compute the
    periodic difference '''
    
    assert len(xs) == len(ys)
    assert len(ys) == len(ps)

    dist = 0;
    for (x,y,p) in zip(xs, ys, ps):
        diff = x - y
        if p != 0:
            diff -= floor(diff/p + 0.5) * p
        dist += diff*diff
    return sqrt(dist)
        
def periodicDistanceNP(xs,ys):
    diff = xs - ys
    for i in xrange(diff.shape[0]):
       if(periodNP[i] != 0):
            diff[i] -= np.floor(diff[i]/periodNP[i]+0.5)*periodNP[i]
                   
    return np.sqrt(np.dot(diff, diff))
            
def testPeriodicDistance():
    a = 0.1
    b = 0.9
    p = 1.0
    
    alist = [a]
    blist = [b]
    plist = [p]
    
    print periodicDistance(alist,blist,plist)

def computePairwiseDistances(data, period):
    distances = []
    count = 0
    for x in data:
        count += 1
        print count
        rowDistances = []
        for y in data:
            rowDistances.append(periodicDistance(x,y, period))
        distances.append(rowDistances) 
    return distances
         



         
#read in input file for terran 
fourClusterDataset = []
for data in loadData("cdata0_full_nonperiodic.txt"):
#for data in loadData("cdata0.txt"):
    fourClusterDataset.append([float(x) for x in data])
    
NPFourClusterDataset = np.array(fourClusterDataset)

n_samples = 2000;

noisy_moons = datasets.make_moons(n_samples=n_samples, noise=.05)
noisy_circles = datasets.make_circles(n_samples=n_samples, factor=.5,
                                      noise=.05)
no_structure = np.random.rand(n_samples, 2), None

dataset_count = 0
count = 1

pl.figure(figsize=(20, 12))

for dataset in [NPFourClusterDataset, noisy_moons[0], noisy_circles[0], no_structure[0]]:

    if dataset_count != 0:
        dataset = StandardScaler().fit_transform(dataset)

    terranAssignment = []
    for assign in loadData("assignment.log"):
        terranAssignment.append(int(assign[0]))
    
    period = [2*3.14159265, 2*3.14159265]
    
    bandwidth_small = cluster.estimate_bandwidth(dataset, quantile=0.2)
    print bandwidth_small
    bandwidth_large = cluster.estimate_bandwidth(dataset, quantile=0.3)
    print bandwidth_large
    
    dbscan_low  = cluster.DBSCAN(eps=0.1, metric='euclidean')
    dbscan_high = cluster.DBSCAN(eps=0.2, metric='euclidean')
    spectral = cluster.SpectralClustering(n_clusters=2, 
                                        eigen_solver='arpack',
                                        affinity="nearest_neighbors")  
    affinity = cluster.AffinityPropagation(damping=.9, preference=-200)
    ms_small = cluster.MeanShift(bandwidth=bandwidth_small, bin_seeding=True)
    ms_large = cluster.MeanShift(bandwidth=bandwidth_large, bin_seeding=True)        
    two_means = cluster.KMeans(n_clusters=2)
    four_means = cluster.KMeans(n_clusters=4)
                
    dbscan_low.fit(dataset)
    dbscan_high.fit(dataset)
    spectral.fit(dataset)
    ms_small.fit(dataset)
    ms_large.fit(dataset)
    two_means.fit(dataset)
    four_means.fit(dataset)
    
    #affinity is really slow
    #affinity.fit(dataset)
    
    pl.subplots_adjust(left=.001, right=.999, bottom=.001, top=.96, wspace=.01,
                    hspace=.01)

    methodCount = 0
    #for algorithm in [dbscan, spectral, affinity]:
    algorithms = [dbscan_low, dbscan_high, spectral, ms_small, ms_large, two_means, four_means, 1]
    
    for algorithm in algorithms:
    
        pl.subplot(4, len(algorithms), count)    
                  
        y_pred = []
        
        titleName = ''
        
        if methodCount % len(algorithms) == 0:
            titleName = 'DBSCAN e = 0.1'   
        elif methodCount % len(algorithms) == 1:
            titleName = 'DBSCAN e = 0.2'  
        elif methodCount % len(algorithms) == 2:
            titleName = 'Spectral k=2'
        elif methodCount % len(algorithms) == 3:
            titleName = 'Mean Shift q=0.2'
        elif methodCount % len(algorithms) == 4:
            titleName = 'Mean Shift q=0.3'
        elif methodCount % len(algorithms) == 5:
            titleName = 'KMeans k=2'
        elif methodCount % len(algorithms) == 6:
            titleName = 'KMeans k=4'
        else:
            titleName = 'Terran'
             
        if dataset_count == 0:
            pl.title(titleName, size=15)
             
        if hasattr(algorithm, 'labels_'):
            y_pred = algorithm.labels_.astype(np.int)
        else:
            if dataset_count == 0:
                y_pred = np.array(terranAssignment)
            else:
                y_pred = np.zeros(n_samples).astype(np.int)
            
        colors = np.array([x for x in 'bgrcmykbgrcmykbgrcmykbgrcmyk'])
        colors = np.hstack([colors] * 20)
        
        
        pl.scatter(dataset[:, 0], dataset[:, 1], color=colors[y_pred].tolist(), s=1)
        
        if dataset_count == 0:
            pl.xlim(-2, 4.5)
            pl.ylim(-3.9, 5)
        else:
            pl.xlim(-2, 2)
            pl.ylim(-2, 2)
            
        pl.xticks(())
        pl.yticks(())
        count += 1
        methodCount += 1
    
    dataset_count += 1
    
pl.show()