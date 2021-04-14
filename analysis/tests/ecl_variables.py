#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
A test of the ECL variables using the latest mDST test file from the mdst package.

.. note::
   Please update the print out of this test when the new mDST test file is released.

"""

import b2test_utils
from glob import glob
import basf2 as b2

# make logging more reproducible by replacing some strings
b2test_utils.configure_logging_for_tests()
b2.set_random_seed("1337")

testinput = [b2test_utils.require_file(sorted(glob(f"{b2.find_file('mdst/tests')}/mdst-v0*.root"))[-1])]

fsps = ['gamma:all']

testpath = b2.create_path()
testpath.add_module('RootInput', inputFileNames=testinput)
#: Cluster-related variables
ecl_vars = [
    'clusterPulseShapeDiscriminationMVA',
    'clusterHasPulseShapeDiscrimination',
    'clusterNumberOfHadronDigits',
    'clusterDeltaLTemp',
    'minC2TDist',
    'nECLClusterTrackMatches',
    'clusterZernikeMVA',
    'clusterReg',
    'clusterAbsZernikeMoment40',
    'clusterAbsZernikeMoment51',
    'clusterBelleQuality',
    'clusterClusterID',
    'clusterConnectedRegionID',
    'clusterE1E9',
    'clusterE9E21',
    'clusterE9E25',
    'clusterEoP',
    'clusterErrorE',
    'clusterErrorPhi',
    'clusterErrorTheta',
    'clusterErrorTiming',
    'clusterHighestE',
    'clusterHasFailedErrorTiming',
    'clusterHasFailedTiming',
    'clusterHasNPhotons',
    'clusterHasNeutralHadron',
    'clusterLAT',
    'clusterNHits',
    'clusterPhi',
    'clusterR',
    'clusterSecondMoment',
    'clusterTheta',
    'clusterTiming',
    'clusterTrackMatch',
    'goodBelleGamma',
    'minC2TDistID',
    'minC2TDistVar(p)',
    'minC2TDistVar(cosTheta,pi+:all)'
]

# Load pi-:all for minC2TDistVar
testpath.add_module('ParticleLoader', decayStrings=['pi+:all'])

for fsp in fsps:
    testpath.add_module('ParticleLoader', decayStrings=[fsp])

    testpath.add_module('ParticlePrinter', listName=fsp, fullPrint=False,
                        variables=ecl_vars)
b2.process(testpath, 1)
