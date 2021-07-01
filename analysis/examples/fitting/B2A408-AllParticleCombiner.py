#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

#############################################################################
#
# Stuck? Ask for help at questions.belle2.org
#
# This tutorial demonstrates how to combine all particles from an input
# particle list, in this particular example to determine the PV position. The
# following  decay chain (and c.c. decay chain)
#
# D*+ -> D0 pi+
#        |
#        +-> K- pi+
#
# is reconstructed beforehand and the D0 and D*+ decay vertices are fitted.
#
# Note: This example is build upon B2A301-Dstar2D0Pi-Reconstruction.py
#
# Note: This example uses the signal MC sample created with release-01-00-00
#
# Contributors: A. Zupanc (June 2014)
#               I. Komarov (December 2017)
#               F. Meier (March 2019)
#
#############################################################################

import basf2 as b2
from modularAnalysis import applyCuts
from modularAnalysis import buildRestOfEvent
from modularAnalysis import combineAllParticles
from modularAnalysis import fillParticleList
from modularAnalysis import inputMdst
from modularAnalysis import reconstructDecay
from modularAnalysis import matchMCTruth
from modularAnalysis import signalSideParticleFilter
from modularAnalysis import variablesToNtuple
from modularAnalysis import variableToSignalSideExtraInfo
from vertex import raveFit
from vertex import treeFit
from stdCharged import stdPi, stdK
from variables import variables
import variables.collections as vc
import variables.utils as vu

# create path
my_path = b2.create_path()

# load input ROOT file
inputMdst(environmentType='default',
          filename=b2.find_file('ccbar_sample_to_test.root', 'examples', False),
          path=my_path)

# use standard final state particle lists
#
# creates "pi+:all" ParticleList (and c.c.)
stdPi('all', path=my_path)
# creates "pi+:loose" ParticleList (and c.c.)
stdPi('loose', path=my_path)
# creates "K+:loose" ParticleList (and c.c.)
stdK('loose', path=my_path)

# reconstruct D0 -> K- pi+ decay
# keep only candidates with 1.8 < M(Kpi) < 1.9 GeV
reconstructDecay('D0:kpi -> K-:loose pi+:loose', '1.8 < M < 1.9', path=my_path)

# perform D0 vertex fit
# reject candidates with C.L. value of the fit < 0.0
treeFit('D0:kpi', 0.0, path=my_path)

# reconstruct D*+ -> D0 pi+ decay
# keep only candidates with Q = M(D0pi) - M(D0) - M(pi) < 20 MeV
reconstructDecay('D*+:all -> D0:kpi pi+:all', '0.0 <= Q < 0.02', path=my_path)

# perform MC matching (MC truth association)
matchMCTruth('D*+:all', path=my_path)

# perform D*+ vertex fit
# reject candidates with C.L. value of the fit < 0.0
treeFit('D*+:all', 0.0, path=my_path)

# build rest of event
buildRestOfEvent('D*+:all', path=my_path)

# define two new paths
roe_path = b2.create_path()
deadEndPath = b2.create_path()

# execute roe path only if there is a signal particle in the event, otherwise stop processing
signalSideParticleFilter('D*+:all', '', roe_path, deadEndPath)

# select all particles in the rest of the event
fillParticleList('pi+:fromPV', 'isInRestOfEvent == 1', path=roe_path)

# perform MC matching for particles in the rest of the event
matchMCTruth('pi+:fromPV', path=roe_path)

# define a list of weakly decaying particles
WeaklyDecayingParticleNames = ['KL0', 'KS0', 'D+', 'D0', 'D_s+', 'n0', 'Sigma-',
                               'Lambda0', 'Sigma+', 'Xi-', 'Omega-', 'Lambda_c+', 'Xi_c0', 'Xi_c+', 'Omega_c0']
WeaklyDecayingParticles = [
    '130',
    '310',
    '411',
    '421',
    '431',
    '2112',
    '3112',
    '3122',
    '3222',
    '3312',
    '3334',
    '4122',
    '4132',
    '4232',
    '4332']

# select particles which are not originating from a weak decay
PVParticlesCuts = ''
DepthOfDecayChain = 5
for i in range(0, DepthOfDecayChain):
    PVParticlesCuts += '[genMotherPDG('
    PVParticlesCuts += str(i)
    PVParticlesCuts += ') == 10022'
    for j in range(0, i):
        for WeaklyDecayingParticle in WeaklyDecayingParticles:
            PVParticlesCuts += ' and abs(genMotherPDG('
            PVParticlesCuts += str(j)
            PVParticlesCuts += ')) != '
            PVParticlesCuts += WeaklyDecayingParticle
    PVParticlesCuts += ']'
    if i < (DepthOfDecayChain - 1):
        PVParticlesCuts += ' or '
applyCuts('pi+:fromPV', PVParticlesCuts, path=roe_path)

# combine all particles in the rest of the event and fit them to a common vertex
combineAllParticles(['pi+:fromPV'], 'vpho:PV', path=roe_path)
raveFit('vpho:PV', conf_level=0, constraint='iptube', path=roe_path)

# save information about the calculated PV position
PVVtxDictionary = {'x': 'PVx',
                   'y': 'PVy',
                   'z': 'PVz',
                   'chiProb': 'PV_Pvalue',
                   'nParticlesInList(pi+:fromPV)': 'nPiPV'
                   }
variableToSignalSideExtraInfo('vpho:PV', PVVtxDictionary, path=roe_path)
variables.addAlias('PVXFit', 'extraInfo(PVx)')
variables.addAlias('PVYFit', 'extraInfo(PVy)')
variables.addAlias('PVZFit', 'extraInfo(PVz)')
variables.addAlias('PVFit_Pvalue', 'extraInfo(PV_Pvalue)')
variables.addAlias('nPiFromPV', 'extraInfo(nPiPV)')

pv_vars = ['PVXFit', 'PVYFit', 'PVZFit', 'PVFit_Pvalue', 'nPiFromPV']

# execute ROE path
my_path.for_each('RestOfEvent', 'RestOfEvents', roe_path)

# Select variables that we want to store to ntuple
dstar_vars = vc.inv_mass + vc.mc_truth + pv_vars

fs_hadron_vars = vu.create_aliases_for_selected(
    vc.pid + vc.track + vc.mc_truth,
    'D*+ -> [D0 -> ^K- ^pi+] ^pi+')

d0_vars = vu.create_aliases_for_selected(
    vc.inv_mass + vc.mc_truth,
    'D*+ -> ^D0 pi+', 'D0')

# Saving variables to ntuple
output_file = 'B2A408-AllParticleCombiner.root'
variablesToNtuple('D*+:all', dstar_vars + d0_vars + fs_hadron_vars,
                  filename=output_file, treename='dsttree', path=my_path)

# Process the events
b2.process(my_path)

# print out the summary
print(b2.statistics)
