#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""
This is a log-type unit test on compatibility of the ROE and the reconstructed
V0 objects created by V0Finder. It runs over a mdst file which contains two muons,
converted gamma, K_S0 and Lambda0. It uses two muons as a signal side, so ROE should
contain only 3 V0 objects. The mdst file was generated by scripts in the
external repository: https://gitlab.desy.de/belle2/software/examples-data-creation/roe-unittest-mc
There is no larger equivalent of this test.
"""

import b2test_utils
from basf2 import set_random_seed, create_path, process

# make logging more reproducible by replacing some strings
b2test_utils.configure_logging_for_tests()
set_random_seed("1337")
testinput = [b2test_utils.require_file('analysis/tests/pgun-roe-mdst.root')]
fsp_signal_side = 'mu+'
fsp_tag_side = 'pi-'
###############################################################################
# a new ParticleLoader for each fsp
testpath = create_path()
testpath.add_module('RootInput', inputFileNames=testinput)
testpath.add_module('ParticleLoader', decayStrings=[fsp_signal_side])
testpath.add_module('ParticleListManipulator', outputListName=fsp_signal_side,
                    inputListNames=[fsp_signal_side + ':all'], cut='isSignal == 1')
testpath.add_module('ParticleLoader', decayStrings=[fsp_tag_side])
testpath.add_module('ParticleListManipulator', outputListName=fsp_tag_side, inputListNames=[fsp_tag_side + ':all'])
testpath.add_module('ParticleStats', particleLists=[fsp_signal_side])

signal_side_name = 'B0'
testpath.add_module('ParticleCombiner',
                    decayString=signal_side_name + ' -> mu+ mu-',
                    cut='')

testpath.add_module('RestOfEventBuilder', particleList=signal_side_name,
                    particleListsInput=[fsp_tag_side])
mask = ('cleanMask', '', '', '')
testpath.add_module('RestOfEventInterpreter', particleList=signal_side_name,
                    ROEMasks=mask)

###############################################################################
roe_path = create_path()

v0list = ['gamma:conv -> e+ e-',
          'Lambda0 -> p+ pi-',
          'K_S0 -> pi+ pi-']
cut = "daughter(0, isSignal) > 0 and daughter(1, isSignal) >0"
for v0 in v0list:
    roe_path.add_module('ParticleLoader', decayStrings=[v0], addDaughters=True)
    roe_path.add_module('ParticleListManipulator', outputListName=v0.split()[0],
                        inputListNames=[v0.split()[0].split(":")[0] + ':V0'], cut='-0.1 < dM < 0.1')
    roe_path.add_module('ParticleSelector',
                        decayString=v0.split(' ->', 1)[0],
                        cut=cut)

    roe_path.add_module('RestOfEventUpdater',
                        particleList=v0.split(' ->', 1)[0],
                        updateMasks=[mask[0]])
roe_path.add_module('RestOfEventPrinter',
                    maskNames=[mask[0]],
                    unpackComposites=False,
                    fullPrint=False)
testpath.for_each('RestOfEvent', 'RestOfEvents', path=roe_path)
###############################################################################
testpath.add_module('ParticlePrinter', listName=signal_side_name, fullPrint=False,
                    variables=['nROE_Composites(cleanMask)'])

process(testpath, 2)
