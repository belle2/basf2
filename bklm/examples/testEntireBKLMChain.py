#!/usr/bin/env python
# -*- coding: utf-8 -*-

########################################################
#
#  This steering file shows how to correctly test
#  the reconstruction chain of the BKLM only
#  (no KLMClusters or Muid related modules are used).
#
#  Contact: Giacomo De Pietro (2018)
#           giacomo.depietro@roma3.infn.it
#
########################################################

import basf2
import simulation as sim
from ROOT import Belle2

components = ['BKLM']
bklm_dataObjects = ['BKLMDigits',
                    'BKLMDigitOutOfRanges',
                    'BKLMDigitRaws',
                    'BKLMDigitEventInfos',
                    'BKLMHit1ds',
                    'BKLMHit2ds',
                    'BKLMTracks']
use_KKMC = False

# Create the main path
main = basf2.create_path()

# Set EventInfoSetter and add a progress bar
main.add_module('EventInfoSetter',
                expList=0,
                runList=1,
                evtNumList=1000)
main.add_module('Progress')
main.add_module('ProgressBar')

if use_KKMC:  # Use KKMC to generate generic mu+mu- events
    main.add_module('KKGenInput',
                    tauinputFile=Belle2.FileSystem.findFile('data/generators/kkmc/mu.input.dat'),
                    KKdefaultFile=Belle2.FileSystem.findFile('data/generators/kkmc/KK2f_defaults.dat'),
                    taudecaytableFile='',
                    kkmcoutputfilename='output.kkmc.dat')
else:  # Use ParticleGun to generate 4GeV mu+ and mu-
    main.add_module('ParticleGun',
                    nTracks=1,
                    pdgCodes=[13, -13],
                    momentumGeneration='fixed',
                    momentumParams=[4],
                    thetaGeneration='uniform',
                    thetaParams=[37, 130])

# Add simulation
sim.add_simulation(path=main,
                   components=components)

# Pack and unpack data
main.add_module('BKLMRawPacker')
main.add_module('BKLMUnpacker')

# Add the digit analyzer module (optional)
main.add_module('BKLMDigitAnalyzer',
                outputRootName='bklmHitmap')

# Add the reconstruction
reco = basf2.register_module('BKLMReconstructor')
reco.param('Prompt window (ns)', 2000)
main.add_module(reco)

# Add the self-tracking
main.add_module('BKLMTracking',
                StudyEffiMode=True,
                outputName='bklmEfficiency_test.root')

# Save the dataobjects in a .root output
main.add_module('RootOutput',
                outputFileName='bklm_dataObjects_test.root',
                branchNames=bklm_dataObjects,
                branchNamesPersistent='FileMetaData')

# Process the path
basf2.process(main)
print(basf2.statistics)
