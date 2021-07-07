#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import basf2
import modularAnalysis as ma
from variables import variables as vm
import variables.collections as vc
import variables.utils as vu


myMain = basf2.create_path()

# Y(4S) -> B0:tag B0:sig
# anti-B0:tag -> D*+ pi-; D*+ -> D0 pi+; D0 -> K- pi+
# B0:sig -> anti-D0 pi0; anti-D0 -> K+ pi-
ma.inputMdst(environmentType='default',
             filename=basf2.find_file('B02pi0D0_D2kpi_B2Dstarpi_Dstar2Dpi_D2kpi.root', 'examples', False),
             path=myMain)


# create final state particle lists
kaons = ('K-', '')
pions = ('pi+', '')
photons = ('gamma', '')

ma.fillParticleLists([kaons, pions, photons], path=myMain)

# reconstruct pi0 -> gamma gamma decay
ma.reconstructDecay('pi0 -> gamma gamma', '0.05 < M < 1.7', path=myMain)

# reconstruct D0 -> K- pi+ decay (and c.c.)
ma.reconstructDecay('D0 -> K- pi+', '1.800 < M < 1.900', path=myMain)

# reconstruct D*+ -> D0 pi+ (and c.c.)
ma.reconstructDecay('D*+ -> D0 pi+', '0.0 <= Q < 0.02', path=myMain)

# reconstruct Btag -> D*+ pi- (and c.c.)
ma.reconstructDecay('anti-B0:tag -> D*+ pi-', '5.000 < M < 6.000', path=myMain)

# reconstruct Bsig -> D0 pi0 (and c.c.)
ma.reconstructDecay('B0:sig -> anti-D0 pi0', '0.000 < M < 6.000', path=myMain)

# reconstruct Y(4S) -> Btag Bsig
ma.reconstructDecay('Upsilon(4S):B0barB0 -> anti-B0:tag B0:sig', '0.000 < M < 11.000', dmID=1, path=myMain)
ma.reconstructDecay('Upsilon(4S):B0B0 -> B0:tag B0:sig', '0.000 < M < 11.000', dmID=2, path=myMain)
ma.copyLists('Upsilon(4S):all', ['Upsilon(4S):B0barB0', 'Upsilon(4S):B0B0'], path=myMain)

# perform MC matching
ma.matchMCTruth('Upsilon(4S):all', myMain)

# create and fill RestOfEvent for Btag and Y(4S) particles
ma.buildRestOfEvent('B0:tag', path=myMain)
ma.buildRestOfEvent('Upsilon(4S):all', path=myMain)

# define variables for Btag ntuple
commonVariables = vc.mc_truth + vc.deltae_mbc
BvariableList = commonVariables + vc.roe_multiplicities

# define variables for Upsilon(4S) ntuple
Y4SvariableList = vc.mc_truth + vc.roe_multiplicities + vc.recoil_kinematics + vc.extra_energy
vm.addAlias('dmID', 'extraInfo(decayModeID)')
Y4SvariableList += ['dmID']
Y4SvariableList += vu.create_aliases(commonVariables, 'daughter(0, {variable})', 'Btag')
Y4SvariableList += vu.create_aliases(commonVariables, 'daughter(1, {variable})', 'Bsig')

# write flat ntuples
ma.variablesToNtuple('B0:tag', variables=BvariableList, filename='ROE_BtagBsig.root', treename='btag', path=myMain)
ma.variablesToNtuple('Upsilon(4S):all', variables=Y4SvariableList, filename='ROE_BtagBsig.root', treename='btagbsig', path=myMain)

basf2.process(myMain)
print(basf2.statistics)
