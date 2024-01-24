#!/usr/bin/env python3
##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""
A test of the ECL variables using a recently produced mDST file.
The mDST file contains 3 B0 -> [ J/psi -> mu+ mu- ] [ K_S0 -> pi+ pi- ] events, BGx0.
"""

import b2test_utils
import basf2 as b2

# make logging more reproducible by replacing some strings
b2test_utils.configure_logging_for_tests()
b2.set_random_seed("1337")

testinput = [b2test_utils.require_file(f"{b2.find_file('analysis/tests')}/ecl-rec-mdst-test.root")]

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
    testpath.add_module('ParticleSelector', decayString=fsp, cut='isFromECL')
    testpath.add_module('ParticlePrinter', listName=fsp, fullPrint=False,
                        variables=ecl_vars)
b2.process(testpath, 1)
