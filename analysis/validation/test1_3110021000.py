#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
  <input>../3110021000.dst.root</input>
  <output>../3110021000.ntup.root</output>
  <contact>Jake Bennett; jvbennett@cmu.edu</contact>
</header>
"""

###############################################################
#
# Analysis Code (make Ntuple for validation of ee->pipigamma)
# contributor : Maeda Yosuke (KMI, Nagoya Univ.)
#               maeday@hepl.phys.nagoya-u.ac.jp  (March 2018)
#
################################################################

import os
from basf2 import *
from modularAnalysis import *
from variables import variables
from ROOT import Belle2
from stdCharged import *
from stdPhotons import *

# set the input files
inputMdstList('default', '../3110021000.dst.root')

stdPi('all')
stdPhotons('tight')

# pion
cutAndCopyList('pi+:good', 'pi+:all', 'chiProb>0.001 and abs(d0)<2 and abs(z0)<4 and nCDCHits>=20 and nSVDHits>=6 and p>1')
# pion ID
cutAndCopyList('gamma:good', 'gamma:tight', 'useCMSFrame(E)>3')

# reconstruction : rho
reconstructDecay('rho0 -> pi+:good pi-:good', '0.1 < M < 10')
reconstructDecay('Z0 -> rho0 gamma:good', '5 < M < 15')


matchMCTruth('pi+:good')
matchMCTruth('gamma:good')

# define Ntuple tools
tools = ['EventMetaData', 'Z0']
tools += ['InvMass', '^Z0 -> [ ^rho0 -> ^pi+ ^pi- ] ^gamma']
tools += ['Kinematics', '^Z0 -> [ ^rho0 -> ^pi+ ^pi- ] ^gamma']
tools += ['CMSKinematics', '^Z0 -> [ ^rho0 -> ^pi+ ^pi- ] ^gamma']
tools += ['MCKinematics', '^Z0 -> [ ^rho0 -> ^pi+ ^pi- ] ^gamma']
tools += ['MCTruth', '^Z0 -> [ ^rho0 -> ^pi+ ^pi- ] ^gamma']
tools += ['TrackHits', 'Z0 -> [ rho0 -> ^pi+ ^pi- ] ^gamma']
tools += ['PID', 'Z0 -> [ rho0 -> ^pi+ ^pi- ] gamma']

# save stuff to root file
ntupleFile('../3110021000.ntup.root')
ntupleTree('isr', 'Z0', tools)

# Process the events
process(analysis_main)

# print out the summary
print(statistics)
