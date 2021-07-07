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
# Execute as: basf2 cluster_position_collector_testbeam.py -n 1000000
#
# The collector will create source files for training of clusterkind 0 by simulating
# 10 million clusters.

import basf2 as b2
from ROOT import Belle2
import math
import os
import shutil


# remove PXD+SVD+MagneticField and add VTX
other_parts = [
    'COIL',
    'STR',
    'ServiceGapsMaterial',
    'BeamPipe',
    'Cryostat',
    'FarBeamLine',
    'HeavyMetalShield',
    'VXDService',
    # 'MagneticField', # FIXME: add interface to control this.
]

if __name__ == "__main__":

    import argparse
    parser = argparse.ArgumentParser(description="Generate training data for computing cluster shape corrections")
    args = parser.parse_args()

    # Now let's create a path to simulate our events.
    main = b2.create_path()

    # Now let's add modules to simulate our events.
    eventinfosetter = main.add_module("EventInfoSetter")
    histoman = main.add_module(
        'HistoManager',
        histoFileName='VTXClusterPositionCollectorOutput_kind_{:d}.root'.format(0))
    gearbox = main.add_module("Gearbox")
    main.add_module('Geometry', excludedComponents=['PXD', 'SVD', 'CDC', 'ECL', 'ARICH', 'TOP', 'KLM'] + other_parts,
                    additionalComponents=['VTX-CMOS-testbeam-2020-03-12'],
                    useDB=False)

    main.add_module('ParticleGun', pdgCodes=[11],
                    momentumGeneration="fixed", momentumParams=[5.2],
                    thetaGeneration='uniform', thetaParams=[80, 100],
                    phiGeneration='uniform', phiParams=[-90, 90],
                    # normal distributed vertex generation
                    vertexGeneration='normal',
                    xVertexParams=[-0.02, 0.0],
                    yVertexParams=[0.28, 0.3],
                    zVertexParams=[0.95, 0.3]
                    )

    main.add_module("FullSim")
    vtxdigi = main.add_module("VTXDigitizer")
    vtxclu = main.add_module("VTXClusterizer")
    main.add_module('VTXClusterPositionCollector', granularity="all", clusterKind=0)
    main.add_module("Progress")

    b2.process(main)
    print(b2.statistics)
