#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# This is a dummy script, ready to test UDST reading when ParticleLists are persistified

"""
<header>
<input>../MDSTtoUDST.udst.root</input>
<output>../UDSTtoNTUP.ntup.root</output>
<contact>Luis Pesantez; pesantez@uni-bonn.de</contact>
</header>
"""

import sys

rootFileName = '../UDSTtoNTUP'
logFileName = rootFileName + '.log'
sys.stdout = open(logFileName, 'w')

import os
from basf2 import *
from modularAnalysis import *
from stdCharged import *
from stdPhotons import *

inputMdst('default', '../MDSTtoUDST.udst.root')
loadStdCharged()
stdPhotons('all')

# Here the ParticleLists will be extracted from the UDST to dump to ntuple
ntupleFile('../UDSTtoNTUP.ntup.root')

ntupPi = ['EventMetaData', '^pi+', 'Kinematics', '^pi+']
ntupleTree('piMC', 'pi+:all', ntupPi)
fillParticleListFromMC('pi+:gen', '')
ntupleTree('piGen', 'pi+:gen', ntupPi)

ntupGamma = ['EventMetaData', '^gamma', 'Kinematics', '^gamma']
ntupleTree('gammaMC', 'gamma:all', ntupGamma)
fillParticleListFromMC('gamma:gen', '')
ntupleTree('gammaGen', 'gamma:gen', ntupGamma)

process(analysis_main)

# Print call statistics
print(statistics)
