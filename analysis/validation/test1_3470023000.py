#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
  <input>../3470023000.dst.root</input>
  <output>../3470023000.ntup.root</output>
  <contact>Jake Bennett; jvbennett@cmu.edu</contact>
</header>
"""

###############################################################
#
# Analysis Code (make Ntuple for validation of tau->a0(->eta pi)nu)
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
from stdPi0s import *

# set the input files
inputMdstList('default', '../3470023000.dst.root')

stdPi('all')
stdPi0s('eff50Fit')

# leption ID from tag-side tau (mu-ID:85% eff./6.2% pi fake, e-ID:95% eff/1.7% pi fake)
cutAndCopyList(
    'pi+:lepton',
    'pi+:all',
    'chiProb>0.001 and abs(d0)<2 and abs(z0)<4 and nCDCHits>=20 and nSVDHits>=6 and [ muonID>0.697 or electronID>0.597 ]')
# pion ID (85% eff./5.9% kaon fake rate)
cutAndCopyList('pi+:good', 'pi+:all', 'chiProb>0.001 and abs(d0)<2 and abs(z0)<4 and nCDCHits>=20 and nSVDHits>=6 and pionID>0.918')

# reconstruction : tau -> eta pi, eta->pi+pi-pi0
reconstructDecay('eta:ppp0 -> pi+:good pi-:good pi0:eff50Fit', '0.3 < M < 0.8')
reconstructDecay('a_0-:etappp0 -> eta:ppp0 pi-:good', '0.1 < M < 2')
reconstructDecay('Z0:etappp0 -> a_0-:etappp0 pi+:lepton', '0.1 < M < 15')

# reconstruction : tau -> eta pi, eta->gamma gamma
reconstructDecay('eta:gg -> gamma:pi0eff50 gamma:pi0eff50', '0.3 < M < 0.8')
reconstructDecay('a_0-:etagg -> eta:gg pi-:good', '0.1 < M < 2')
reconstructDecay('Z0:etagg -> a_0-:etagg pi+:lepton', '0.1 < M < 15')

matchMCTruth('pi+:lepton')
matchMCTruth('pi+:good')
matchMCTruth('pi0:eff50Fit')
matchMCTruth('eta:ppp0')
matchMCTruth('eta:gg')
matchMCTruth('a_0-:etappp0')
matchMCTruth('a_0-:etagg')

# define Ntuple tools
toolsEtappp0 = ['EventMetaData', 'Z0:etappp0']
toolsEtappp0 += ['InvMass', '^Z0 -> [ ^a_0- -> [ ^eta -> ^pi+ ^pi- [ ^pi0 -> ^gamma ^gamma ]  ] ^pi- ] ^pi+']
toolsEtappp0 += ['Kinematics', '^Z0 -> [ ^a_0- -> [ ^eta -> ^pi+ ^pi- [ ^pi0 -> ^gamma ^gamma ] ] ^pi- ] ^pi+']
toolsEtappp0 += ['CMSKinematics', '^Z0 -> [ ^a_0- -> [ ^eta -> ^pi+ ^pi- [ ^pi0 -> ^gamma ^gamma ] ] ^pi- ] ^pi+']
toolsEtappp0 += ['MCKinematics', '^Z0 -> [ ^a_0- -> [ ^eta -> ^pi+ ^pi- [ ^pi0 -> ^gamma ^gamma ] ] ^pi- ] ^pi+']
toolsEtappp0 += ['MCTruth', '^Z0 -> [ ^a_0- -> [ ^eta -> ^pi+ ^pi- [ ^pi0 -> ^gamma ^gamma ] ] ^pi- ] ^pi+']
toolsEtappp0 += ['TrackHits', 'Z0 -> [ a_0- -> [ eta -> ^pi+ ^pi- [ ^pi0 -> gamma gamma ] ] ^pi- ] ^pi+']
toolsEtappp0 += ['CustomFloats[isSignal]', 'Z0 -> [ ^a_0- -> [ ^eta -> pi+ pi- pi0  ] pi- ] pi+']

toolsEtagg = ['EventMetaData', 'Z0:etagg']
toolsEtagg += ['InvMass', '^Z0 -> [ ^a_0- -> [ ^eta -> ^gamma ^gamma ] ^pi- ] ^pi+']
toolsEtagg += ['Kinematics', '^Z0 -> [ ^a_0- -> [ ^eta -> ^gamma ^gamma ] ^pi- ] ^pi+']
toolsEtagg += ['CMSKinematics', '^Z0 -> [ ^a_0-g -> [ ^eta -> ^gamma ^gamma ] ^pi- ] ^pi+']
toolsEtagg += ['MCKinematics', '^Z0 -> [ ^a_0- -> [ ^eta -> ^gamma ^gamma ] ^pi- ] ^pi+']
toolsEtagg += ['MCTruth', '^Z0 -> [ ^a_0- -> [ ^eta -> ^gamma ^gamma ] ^pi- ] ^pi+']
toolsEtagg += ['TrackHits', 'Z0 -> [ a_0- -> [ eta -> gamma gamma ] ^pi- ] ^pi+']
toolsEtagg += ['CustomFloats[isSignal]', 'Z0 -> [ ^a_0- -> [ ^eta -> gamma gamma  ] pi- ] pi+']

# save stuff to root file
ntupleFile('../3470023000.ntup.root')
ntupleTree('tautau_3pi', 'Z0:etappp0', toolsEtappp0)
ntupleTree('tautau_gg', 'Z0:etagg', toolsEtagg)

# Process the events
process(analysis_main)

# print out the summary
print(statistics)
