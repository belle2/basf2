#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import basf2 as b2
from beamparameters import add_beamparameters

# This is tracking/vxdCaTracking/extendedExamples/scripts/setup_modules.py
# If later the use of bg is wanted, you can as well import setup_bg
from VXDTF.setup_modules import (setup_sim,
                                 setup_realClusters,
                                 setup_mcTF)

# 0 means really random. Set to different seed to have reproducable simulation.
b2.set_random_seed(0)

# Extremely "non-verbose". Should be OK, as this are well tested modules...
# Can be overridden with the "-l LEVEL" flag for basf2.
b2.set_log_level(b2.LogLevel.ERROR)

# ---------------------------------------------------------------------------------------
# Creating the main path, that will be executed in the end:
main = b2.create_path()

# EventInfoSetter, EventInfoPrinter, Progress:

# Default is 1 event. To make more use the "-n NUMBER" flag for basf2.
eventinfosetter = b2.register_module('EventInfoSetter')

# Just some info about what is going on...
eventinfoprinter = b2.register_module('EventInfoPrinter')
progress = b2.register_module('Progress')

main.add_module(eventinfosetter)
main.add_module(eventinfoprinter)
main.add_module(progress)

# beam parameters
# To find out about the add_... functions: start basf2, import the function and use help(add_...)
beamparameters = add_beamparameters(main, "Y4S")
b2.print_params(beamparameters)

# We might want to have particle gun(s) and EVTGen.
# Still in that case the ParticleGun modules better come first:
param_pGun = {
    'pdgCodes': [13, -13],                   # 13 = muon --> negatively charged!
    'nTracks': 5,                          # 20 tracks is a lot, but we don't use beam background in this script.
    'momentumGeneration': 'uniformPt',
    'momentumParams': [0.1, 0.15],           # 2 values: [min, max] in GeV
    'thetaGeneration': 'uniform',
    'thetaParams': [60., 85.],               # 2 values: [min, max] in degree
    'phiGeneration': 'uniform',
    'phiParams': [0., 90.],                  # [min, max] in degree
    'vertexGeneration': 'uniform',
    'xVertexParams': [-0.1, 0.1],            # in cm...
    'yVertexParams': [-0.1, 0.1],
    'zVertexParams': [-0.5, 0.5],
}

particlegun = b2.register_module('ParticleGun')
particlegun.logging.log_level = b2.LogLevel.WARNING
particlegun.param(param_pGun)
main.add_module(particlegun)

# Now we might want to add EvtGen:
if False:
    evtgenInput = b2.register_module('EvtGenInput')
    evtgenInput.logging.log_level = b2.LogLevel.WARNING
    main.add_module(evtgenInput)

# Gearbox to access stuff from the data folders, and Geometry:
gearbox = b2.register_module('Gearbox')
main.add_module(gearbox)

geometry = b2.register_module('Geometry')
geometry.param('components', ['BeamPipe', 'MagneticFieldConstant4LimitedRSVD',  # Important: look at B field!
                              'PXD', 'SVD'])
main.add_module(geometry)

# Geant 4 Simulation: this setup could be fed with ignoring the energy deposit as parameter...
# read careful the description as you otherwise might not get any hits.
g4sim = setup_sim()
main.add_module(g4sim)


# Digitization and Clusterization -------------------------------------------------------
if True:
    setup_realClusters(main, usePXD=True)  # usePXD=True: needed since 2gftc-converter does not work without it

else:                    # Use simple clusterizer, that takes TrueHits.
    simpleClusterizer = b2.register_module('VXDSimpleClusterizer')
    simpleClusterizer.param('setMeasSigma', 0)
    simpleClusterizer.param('onlyPrimaries', True)
    useEDeposit = True
    if useEDeposit is False:
        simpleClusterizer.param('energyThresholdU', -0.0001)
        simpleClusterizer.param('energyThresholdV', -0.0001)
        simpleClusterizer.param('energyThreshold', -0.0001)
    main.add_module(simpleClusterizer)

# Setting up the MC based track finder is necessary to collect information etc.
setup_mcTF(path=main, nameOutput='mcTracks', usePXD=False, logLevel=b2.LogLevel.INFO)

# Module to write the DataStore into a Root file. Name of output file can be overriden with "-o NAME" flag.
rootOutput = b2.register_module('RootOutput')
rootOutput.param('outputFileName', "MyRootFile.root")
main.add_module(rootOutput)

# Final words:
b2.log_to_file('createSim.log', append=False)
b2.process(main)
print(b2.statistics)
