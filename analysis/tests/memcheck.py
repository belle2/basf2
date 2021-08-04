#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""This script is used in the nightly build to check for memory issues with valgrind.
It is run as a test to make sure the memcheck does not fail because of issues in the script."""

from b2test_utils import skip_test_if_light
skip_test_if_light()  # light builds don't contain simulation, reconstruction etc; skip before trying to import # noqa

from basf2 import set_random_seed, set_log_level, LogLevel, create_path, Module, find_file, process, statistics
from simulation import add_simulation
from reconstruction import add_reconstruction, add_mdst_output
from ROOT import Belle2
from modularAnalysis import reconstructDecay, rankByHighest, buildRestOfEvent, buildContinuumSuppression, matchMCTruth, \
    variablesToNtuple
from stdCharged import stdPi, stdMu
from vertex import treeFit, TagV
from flavorTagger import flavorTagger
from rawdata import add_packers, add_unpackers
import glob
import sys
import os

# set the random seed to get reproducible results
set_random_seed(1)

# suppress messages and warnings during processing:
set_log_level(LogLevel.ERROR)

# create path
main = create_path()

# specify number of events to be generated
main.add_module('EventInfoSetter')


class StopModule(Module):
    """Module to stop the event processing in case the argument InitOnly is given"""

    def event(self):
        """Set the event meta data to end of data"""

        eventMetaData = Belle2.PyStoreObj('EventMetaData')
        eventMetaData.setEndOfData()


if len(sys.argv) > 1 and sys.argv[1] == 'InitOnly':
    main.add_module(StopModule())

# print event numbers
evtmetainfo = main.add_module('EventInfoPrinter')
evtmetainfo.set_log_level(LogLevel.INFO)

# generate BBbar events, with Bsig -> J/psi K0S
main.add_module('EvtGenInput', userDECFile=find_file('decfiles/dec/1111440100.dec'))

# detector simulation
bg = None
if 'BELLE2_BACKGROUND_DIR' in os.environ:
    bg = glob.glob(os.environ['BELLE2_BACKGROUND_DIR'] + '/*.root')
add_simulation(main, bkgfiles=bg)

# reconstruction
add_reconstruction(main)

# mdst output
add_mdst_output(main, True)

# dst output
main.add_module('RootOutput', outputFileName='dst.root')


# use standard final state particle lists
stdPi('loose', path=main)
stdMu('loose', path=main)

# reconstruct Ks -> pi+ pi- decay and keep only candidates with 0.4 < M(pipi) < 0.6 GeV
reconstructDecay('K_S0:pipi -> pi+:loose pi-:loose', cut='0.4 < M < 0.6', path=main)

# reconstruct J/psi -> mu+ mu- decay and keep only candidates with 3.0 < M(mumu) < 3.2 GeV
reconstructDecay('J/psi:mumu -> mu+:loose mu-:loose', cut='3.0 < M < 3.2', path=main)

# reconstruct B0 -> J/psi Ks decay and keep only candidates with 5.2 < M(J/PsiKs) < 5.4 GeV
reconstructDecay('B0:jpsiks -> J/psi:mumu K_S0:pipi', cut='5.2 < M < 5.4', path=main)

# perform B0 kinematic vertex fit and keep candidates only passing C.L. value of the fit > 0.0 (no cut)
treeFit('B0:jpsiks', 0.0, path=main)

# order candidates by chi2 probability
rankByHighest('B0:jpsiks', 'chiProb', path=main)

# build the rest of the event associated to the B0
buildRestOfEvent('B0:jpsiks', path=main)

# calculate continuum suppression variables
buildContinuumSuppression('B0:jpsiks', '', path=main)

# perform MC matching (MC truth association).
matchMCTruth('B0:jpsiks', path=main)

# do flavor tagging
flavorTagger('B0:jpsiks', path=main)

# calculate the Tag Vertex and Delta t (in ps), breco: type of MC association.
TagV('B0:jpsiks', 'breco', path=main)

# select variables that we want to store to ntuple
fs_vars = ['kaonID', 'muonID', 'dr', 'dz', 'pValue', 'isSignal', 'mcErrors', 'genMotherID']
b_vars = ['nTracks', 'Mbc', 'deltaE', 'p', 'E', 'useCMSFrame(p)', 'useCMSFrame(E)',
          'isSignal', 'mcErrors', 'nROE_KLMClusters', 'qrOutput(FBDT)', 'TagVLBoost', 'TagVz', 'TagVzErr', 'mcDeltaT'] + \
    ['daughter(0,daughter(0,%s))' % var for var in fs_vars]

# save variables to ntuple
variablesToNtuple('B0:jpsiks', variables=b_vars, filename='ntuple.root', treename='B0tree', path=main)

# do raw data packing and unpacking
add_packers(main)
add_unpackers(main)
for module in main.modules():
    if module.type() == 'SVDUnpacker':
        module.param('silentlyAppend', True)

# gather profiling information
main.add_module('Profile', outputFileName='vmem_profile.png', rssOutputFileName='rss_profile.png').set_log_level(LogLevel.INFO)

# execute all
process(main)

# Print call statistics
print(statistics)
