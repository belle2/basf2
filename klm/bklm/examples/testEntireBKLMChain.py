#!/usr/bin/env python
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

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
import generators as gen
import simulation as sim

bklm_dataObjects = ['BKLMDigits',
                    'BKLMDigitOutOfRanges',
                    'KLMDigitEventInfos',
                    'BKLMHit1ds',
                    'BKLMHit2ds',
                    'BKLMTracks']
use_KKMC = False

# Create the main path
main = basf2.create_path()

# Set EventInfoSetter and add a progress bar
main.add_module('EventInfoSetter',
                expList=0,
                runList=0,
                evtNumList=1000)
main.add_module('Progress')
main.add_module('ProgressBar')

if use_KKMC:  # Use KKMC to generate generic mu+mu- events
    gen.add_kkmc_generator(path=main,
                           finalstate='mu-mu+')
else:  # Use ParticleGun to generate 4GeV mu+ and mu-
    main.add_module('ParticleGun',
                    nTracks=1,
                    pdgCodes=[13, -13],
                    momentumGeneration='fixed',
                    momentumParams=[4],
                    thetaGeneration='uniform',
                    thetaParams=[37, 130])

# Add simulation
main.add_module('Gearbox')
main.add_module('Geometry',
                components=['KLM'],
                useDB=False)
sim.add_simulation(path=main,
                   components=['KLM'])

# Pack and unpack data
main.add_module('KLMPacker')
main.add_module('KLMUnpacker',
                WriteDigitRaws=True)

# Add the digit analyzer module (optional)
main.add_module('BKLMDigitAnalyzer',
                outputRootName='bklmHitmap')

# Add the reconstruction
main.add_module('KLMReconstructor')

# Add the self-tracking
main.add_module('BKLMTracking',
                StudyEffiMode=True,
                outputName='bklmEfficiency.root')

# Save the dataobjects in a .root output
main.add_module('RootOutput',
                outputFileName='bklm_dataObjects.root',
                branchNames=bklm_dataObjects,
                branchNamesPersistent='FileMetaData')

# Process the path
basf2.process(main)
print(basf2.statistics)
