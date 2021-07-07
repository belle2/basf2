#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import basf2
from cdctrigger import add_cdc_trigger

"""
generate tracks with particle gun, simulate CDC and CDC trigger, save the output.
"""

# ------------ #
# user options #
# ------------ #

# general options
seed = 10000
evtnum = 100
particlegun_params = {
    'pdgCodes': [-13, 13],
    'nTracks': 2,
    'momentumGeneration': 'inversePt',
    'momentumParams': [0.3, 10.],
    'thetaGeneration': 'uniform',
    'thetaParams': [35, 123],
    'phiGeneration': 'uniform',
    'phiParams': [0, 360],
    'vertexGeneration': 'uniform',
    'xVertexParams': [0, 0.0],
    'yVertexParams': [0, 0.0],
    'zVertexParams': [-50.0, 50.0]}

# ------------------------- #
# create path up to trigger #
# ------------------------- #

# set random seed
basf2.set_random_seed(seed)
# suppress messages and warnings during processing:
basf2.set_log_level(basf2.LogLevel.ERROR)

main = basf2.create_path()

main.add_module('EventInfoSetter', evtNumList=evtnum)
main.add_module('Progress')
main.add_module('Gearbox')
main.add_module('Geometry', components=['BeamPipe',
                                        'PXD', 'SVD', 'CDC',
                                        'MagneticFieldConstant4LimitedRCDC'])
particlegun = basf2.register_module('ParticleGun')
particlegun.param(particlegun_params)
main.add_module(particlegun)
main.add_module('FullSim')
main.add_module('CDCDigitizer')

# ---------------------- #
# CDC trigger and output #
# ---------------------- #

add_cdc_trigger(main)
main.add_module('RootOutput', outputFileName='cdctrigger.root')

# Process events
basf2.process(main)

# Print call statistics
print(basf2.statistics)
