#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# This steering file steers the collectors for the training of the PXD cluster
# position and angle estimator running the CAF.
#
# Execute as: basf2 cluster_position_angle_collector.py -n 10000000 -- --clusterkind=0
#
# The collector will create source files for training of clusterkind 0 by simulating
# 10 million clusters.
#
# Full set of training sources for PXD requires starting the script 4x wiht clusterkinds
# 0-3.
#
# author: benjamin.schwenker@pyhs.uni-goettingen.de

from basf2 import *
from ROOT import Belle2
import math
import os
import shutil

if __name__ == "__main__":

    import argparse
    parser = argparse.ArgumentParser(description="Generate training data for computing cluster shape corrections")
    parser.add_argument('--clusterkind', dest='clusterkind', default=0, type=int,
                        help='ClusterKinds 0, 1, 2, 3 are for z55, z60, z70, z85 pixels')
    parser.add_argument('--momentum', dest='momentum', default=1.0, type=float, help='Momentum of particle gun')
    parser.add_argument('--pdgCode', dest='pdgCode', default=-211, type=int, help='PDG code for particle gun')
    args = parser.parse_args()

    # In order to create source for a specific kind of clusters, we position the particle gun below
    # a specific part of the sensors.
    #
    # Kinds 0-3 are clusters where all pixels have a specific pixel pitch type. Higher kinds (not yet implemented)
    # are for cases where sensor borders are touched are the cluster neighbors masked pixels.
    #
    # For setting of vertex close to surface of for PXD kinds of pixels set:
    # 55 um pitch: sensor 1.3.2 , vertex: x: [-0.2050,-0.2], y: [1.35], z: [0.7,0.7055]
    # 60 um pitch: sensor 1.3.2 , vertex: x: [-0.2050,-0.2], y: [1.35], z: [-1.5,-1.5060]
    # 70 um pitch: sensor 2.4.2 , vertex: x: [-0.2050,-0.2], y: [2.18], z: [0.9,0.9070]
    # 85 um pitch: sensor 2.4.2 , vertex: x: [-0.2050,-0.2], y: [2.18], z: [-2.0,-2.0085]

    vertex_x = [-0.2050, -0.2]
    vertex_y = [1.35]
    vertex_z = [0.7, 0.7055]

    if args.clusterkind == 0:
        pass
    elif args.clusterkind == 1:
        vertex_z = [-1.5060, -1.5]
    elif args.clusterkind == 2:
        vertex_y = [2.18]
        vertex_z = [0.9, 0.9070]
    elif args.clusterkind == 3:
        vertex_y = [2.18]
        vertex_z = [-2.0085, -2.0]

    # Now let's create a path to simulate our events.
    main = create_path()

    # Now let's add modules to simulate our events.
    eventinfosetter = main.add_module("EventInfoSetter")
    histoman = main.add_module(
        'HistoManager',
        histoFileName='PXDClusterPositionAndAngleCollectorOutput_kind_{:d}.root'.format(
            args.clusterkind))
    gearbox = main.add_module("Gearbox")
    geometry = main.add_module("Geometry")
    geometry.param({"components": ['MagneticField', 'PXD']})
    particlegun = main.add_module("ParticleGun")
    particlegun.param({"nTracks": 1,
                       "pdgCodes": [args.pdgCode],
                       "momentumGeneration": 'discrete',
                       "momentumParams": [args.momentum] + [1] * len([args.momentum]),
                       "thetaGeneration": 'uniformCos',
                       "thetaParams": [0, 180],
                       "phiGeneration": 'uniform',
                       "phiParams": [0, 180],
                       "xVertexGeneration": 'uniform',
                       "xVertexParams": vertex_x,
                       "yVertexGeneration": 'fixed',
                       "yVertexParams": vertex_y,
                       "zVertexGeneration": 'uniform',
                       "zVertexParams": vertex_z,
                       "independentVertices": False,
                       })
    main.add_module("FullSim")
    pxddigi = main.add_module("PXDDigitizer")
    pxdclu = main.add_module("PXDClusterizer")
    main.add_module('PXDClusterPositionAndAngleCollector', granularity="all", clusterKind=args.clusterkind)
    main.add_module("Progress")

    process(main)
    print(statistics)
