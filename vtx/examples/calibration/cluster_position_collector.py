#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# This steering file steers the collectors for the training of the VTX cluster
# position estimator running the CAF.
#
# Execute as: basf2 cluster_position_collector.py -n 15000000
#
# The collector will create source files for training of clusterkind 0 by simulating
# 10 million clusters.

import basf2 as b2
from ROOT import Belle2
import math
import os
import shutil

if __name__ == "__main__":

    import argparse
    parser = argparse.ArgumentParser(description="Generate training data for computing cluster shape corrections")
    parser.add_argument('--momentum', dest='momentum', default=0.5, type=float, help='Momentum of particle gun')
    parser.add_argument('--pdgCode', dest='pdgCode', default=-211, type=int, help='PDG code for particle gun')
    args = parser.parse_args()

    # In order to create source for a specific kind of clusters, we position the particle gun below
    # a specific part of the sensors.
    # For position the gun vertex close to surface of one sensor:
    vertex_x = [-0.2050, -0.2]
    vertex_y = [1.35]
    vertex_z = [0.7, 0.7055]

    # Now let's create a path to simulate our events.
    main = b2.create_path()

    # Now let's add modules to simulate our events.
    eventinfosetter = main.add_module("EventInfoSetter")
    histoman = main.add_module(
        'HistoManager',
        histoFileName='VTXClusterPositionCollectorOutput_kind_{:d}.root'.format(0))
    gearbox = main.add_module("Gearbox")
    main.add_module('Geometry', excludedComponents=['PXD', 'SVD', 'CDC', 'ECL', 'ARICH', 'TOP', 'KLM'],
                    additionalComponents=['VTX-CMOS-5layer'],
                    useDB=False)

    particlegun = main.add_module("ParticleGun")
    particlegun.param({"nTracks": 1,
                       "pdgCodes": [args.pdgCode],
                       "momentumGeneration": 'discrete',
                       "momentumParams": [args.momentum] + [1] * len([args.momentum]),
                       "thetaGeneration": 'uniformCos',
                       "thetaParams": [0, 90],
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
    vtxdigi = main.add_module("VTXDigitizer")
    vtxclu = main.add_module("VTXClusterizer")
    main.add_module('VTXClusterPositionCollector', granularity="all", clusterKind=0)
    main.add_module("Progress")

    b2.process(main)
    print(b2.statistics)
