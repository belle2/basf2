#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##############################################################################
# Use e+e- --> mu+ mu- events to study ECL alignment
#
# usage:  basf2 -i inputData.root muPairAlignment.py [outputName.root]
#
# (c) Christopher Hearty, 2020 (hearty@physics.ubc.ca)
#
##############################################################################

import sys
import basf2 as b2
from modularAnalysis import inputMdst
from modularAnalysis import fillParticleList
from variables import variables
from modularAnalysis import reconstructDecay
from modularAnalysis import applyEventCuts
from modularAnalysis import rankByHighest
from modularAnalysis import cutAndCopyList
from modularAnalysis import variablesToEventExtraInfo

# ..create path
mypath = b2.create_path()

inputMdst('default', '/group/belle2/dataprod/MC/MC13a_local/mumu/mumu_eph3_2295.root', path=mypath)

# ..Muon list. Require that there be two high-p tracks, and none with clusterE>1 GeV
fillParticleList('mu-:highp', 'useCMSFrame(p) > 3.5 and abs(d0) < 0.5 and abs(z0) < 4 and nCDCHits>0 and nVXDHits>0', path=mypath)
cutAndCopyList('mu-:highE', 'mu-:highp', 'clusterE>1.', path=mypath)

variables.addAlias('nhighp', 'nParticlesInList(mu-:highp)')
variables.addAlias('nhighE', 'nParticlesInList(mu-:highE)')
applyEventCuts('[nhighE==0] and [nhighp==2]', mypath)

# ..Combine two muons, requiring back to back and high invariant mass
variables.addAlias('deltaPhiCMS', 'formula(57.2957795*abs(daughterDiffOfPhiCMS(0, 1)))')
variables.addAlias('sumThetaCMS', 'formula(57.2957795*(daughter(0, useCMSFrame(theta)) + daughter(1, useCMSFrame(theta))))')
event_cut = '175 < sumThetaCMS < 185 and deltaPhiCMS > 175 and 9 < M < 11'
reconstructDecay('vpho:0 -> mu-:highp mu+:highp', event_cut, path=mypath)

# ..Require a good muon pair
variables.addAlias('nPair', 'nParticlesInList(vpho:0)')
applyEventCuts('nPair==1', mypath)

# ..Record the highest and lowest muonID of the two muons in the event
cutAndCopyList('mu-:muonID', 'mu-:highp', '', path=mypath)
rankByHighest('mu-:muonID', 'muonID', path=mypath)

cutAndCopyList('mu-:bestID', 'mu-:muonID', 'extraInfo(muonID_rank)==1', path=mypath)
variablesToEventExtraInfo(particleList='mu-:bestID', variables={'muonID': 'bestID'}, path=mypath)
variables.addAlias('bestMuonID', 'eventExtraInfo(bestID)')

cutAndCopyList('mu-:worstID', 'mu-:muonID', 'extraInfo(muonID_rank)==2', path=mypath)
variablesToEventExtraInfo(particleList='mu-:worstID', variables={'muonID': 'worstID'}, path=mypath)
variables.addAlias('worstMuonID', 'eventExtraInfo(worstID)')

# ..Variables to store (per muon)
variables.addAlias('muThetaLab', 'formula(57.2957795*theta)')
variables.addAlias('muPhiLab', 'formula(57.2957795*phi)')
variables.addAlias('muPcms', 'useCMSFrame(p)')
variables.addAlias('muptLab', 'pt')
variables.addAlias('e1Uncorr', 'formula(clusterHighestE * clusterUncorrE / clusterE)')
variables.addAlias('clustThetaLab', 'formula(57.2957795*clusterTheta)')
variables.addAlias('clustPhiLab', 'formula(57.2957795*clusterPhi)')

varsToStore = [
    'charge',
    'p',
    'muPcms',
    'muptLab',
    'muThetaLab',
    'muPhiLab',
    'clusterE',
    'e1Uncorr',
    'clustThetaLab',
    'clustPhiLab',
    'HighLevelTrigger',
    'muonID',
    'bestMuonID',
    'worstMuonID']

# ..store to ntuple
outputName = "muPairAlignment.root"
narg = len(sys.argv)
if(narg == 2):
    outputName = sys.argv[1]
mypath.add_module('VariablesToNtuple', particleList='mu-:highp', variables=varsToStore, fileName=outputName)


# ..Process
b2.process(mypath)
print(b2.statistics)
