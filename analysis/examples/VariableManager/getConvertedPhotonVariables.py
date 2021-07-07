#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# Doxygen should skip this script
# @cond

"""
This script loads a specified input ROOT file, reconstructs photon conversions and
saves converted-photon variables and some of their generator-level counterparts for comparison.

Execute script with:
  $> basf2 getConvertedPhotonVariables.py -i [input_ROOT_file]
"""

import basf2 as b2
import modularAnalysis as ma
from variables import variables as vm

path = b2.Path()
ma.inputMdst(environmentType='default',
             filename=b2.find_file('mdst14.root', 'validation', True),
             path=path)


# variables aliases
vm.addAlias('cpDR', 'convertedPhotonDelR(0,1)')
vm.addAlias('cpDZ', 'convertedPhotonDelZ(0,1)')
vm.addAlias('cpDTL', 'convertedPhotonDelTanLambda(0,1)')
vm.addAlias('cpX', 'convertedPhotonX(0,1)')
vm.addAlias('cpY', 'convertedPhotonY(0,1)')
vm.addAlias('cpZ', 'convertedPhotonZ(0,1)')
vm.addAlias('cpRho', 'convertedPhotonRho(0,1)')
vm.addAlias('cpM', 'convertedPhotonInvariantMass(0,1)')
vm.addAlias('cpPx', 'convertedPhotonPx(0,1)')
vm.addAlias('cpPy', 'convertedPhotonPy(0,1)')
vm.addAlias('cpPz', 'convertedPhotonPz(0,1)')

vm.addAlias('mcX', 'daughter(0,mcProductionVertexX)')
vm.addAlias('mcY', 'daughter(0,mcProductionVertexY)')
vm.addAlias('mcZ', 'daughter(0,mcProductionVertexZ)')


# tuple variables
tuple_vars = [
    'cpDR',
    'cpDZ',
    'cpDTL',
    'cpX',
    'cpY',
    'cpZ',
    'cpRho',
    'cpM',
    'cpPx',
    'cpPy',
    'cpPz',
    'isSignal',
    'mcX',
    'mcY',
    'mcZ',
    'mcPX',
    'mcPY',
    'mcPZ']


ma.fillParticleList("e+:loose", cut="p > 0.3", path=path)
ma.reconstructDecay(
    'gamma:conv -> e+:loose e-:loose',
    cut='cpM < 0.1 and cpDR > -0.15 and cpDR < 0.15 and cpDZ > -0.05 and cpDZ < 0.05',
    path=path)
ma.matchMCTruth('gamma:conv', path=path)
ma.variablesToNtuple('gamma:conv', tuple_vars, filename='output.root', treename='photonVars', path=path)


# progress
progress = b2.register_module('Progress')
path.add_module(progress)
b2.process(path=path)

# Print call statistics
print(b2.statistics)


# @endcond
