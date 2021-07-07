#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# Test whether we can create and read a sequential root file with 5 events

from basf2 import create_path, process, set_random_seed, statistics, LogLevel, conditions
import os
import multiprocessing
from b2test_utils import skip_test_if_light

skip_test_if_light()  # light builds don't contain particle gun
set_random_seed(42)

# create a path
main = create_path()

# generate 5 events
main.add_module("EventInfoSetter", evtNumList=[5])

# Add particle gun to shoot some pions and kaons at the ARICH
main.add_module('ParticleGun', pdgCodes=[211, -211, 321, -321],
                momentumGeneration="fixed", momentumParams=[3],
                thetaGeneration="uniform", thetaParams=[17, 25])

# Add everything we need to simulate just ARICH
main.add_module('Progress')
main.add_module('Gearbox')
main.add_module('Geometry', useDB=False, components=['MagneticField', 'ARICH'], logLevel=LogLevel.ERROR)
main.add_module('FullSim', logLevel=LogLevel.ERROR)
# Add seqoutput but ignore write rate
main.add_module('SeqRootOutput', outputFileName='seqout_test.sroot', logLevel=LogLevel.WARNING)

# Run in sub process to avoid side effects
sub = multiprocessing.Process(target=process, args=(main,))
sub.start()
sub.join()

# Read file again
conditions.disable_globaltag_replay()
readpath = create_path()
readpath.add_module('SeqRootInput', inputFileName='seqout_test.sroot')
readpath.add_module('Progress')
process(readpath)

# remove input file
os.remove('seqout_test.sroot')

# and make sure everything is fine
assert statistics.modules[1].name == 'Progress'
assert statistics.modules[1].calls(statistics.EVENT) == 5
