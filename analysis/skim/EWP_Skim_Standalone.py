#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# EWP standalone skim steering
# P. Urquijo, F. Tenchini 6/Jan/2015
#
######################################################

from basf2 import *
from modularAnalysis import *
from stdCharged import *
from stdPi0s import *
from stdPhotons import *
from stdV0s import *
from stdLightMesons import *

set_log_level(LogLevel.INFO)

filelist = \
    ['/ghi/fs01/belle2/bdata/MC/release-00-07-02/DBxxxxxxxx/MC7/prod00000787/s00/e0000/4S/r00000/mixed/sub00/' +
     'mdst_000001_prod00000787_task00000001.root'
     ]  # This is an MC7 file used for testing since we don't have MC8 yet; please modify accordingly
inputMdstList('default', filelist)

stdPi0s('loose')
stdKshorts()
stdPi('95eff')
stdK('95eff')
stdMu('95eff')  # this benchmark is missing, will load mu+:all
stdE('95eff')  # this benchmark is missing, will load e+:all
stdPhotons('loose')
stdPhotons('all')  # only used for eta lists

# rename to use in EWP_List.py
# this is slightly inefficient, but makes the code more modular
cutAndCopyList('pi0:ewp', 'pi0:loose', '', True)
cutAndCopyList('pi+:ewp', 'pi+:95eff', '', True)
cutAndCopyList('K+:ewp', 'K+:95eff', '', True)
#

# Loose cuts to compensate for missing leptons benchmarks;
cutAndCopyList('e+:ewp', 'e+:all', 'pt > 0.4 and abs(d0)<2 and abs(z0)<4 and chiProb > 0.001', True)
cutAndCopyList('mu+:ewp', 'mu+:all', 'pt > 0.7 and abs(d0)<2 and abs(z0)<4 and chiProb > 0.001', True)
#

# 20 MeV width; 'all' is 0.45<M<0.55; this selection could be optimised
cutAndCopyList('K_S0:ewp', 'K_S0:all', '0.4776 < M < 0.5176', True)

# We don't have light meson standard particle lists yet
reconstructDecay('eta:ewp1 -> gamma:all gamma:all', '0.4 < M < 0.6', 1, True)
reconstructDecay('eta:ewp2 -> pi0:ewp pi-:ewp pi+:ewp', '0.4 < M < 0.6', 2, True)
copyLists('eta:ewp', ['eta:ewp1', 'eta:ewp2'], True)
reconstructDecay('rho0:ewp -> pi-:ewp pi+:ewp', '0.47 < M < 1.07', 1, True)
reconstructDecay('rho+:ewp -> pi0:ewp pi+:ewp', '0.47 < M < 1.07', 1, True)
reconstructDecay('phi:ewp -> K+:ewp K-:ewp', '0.97 < M < 1.1', 1, True)
reconstructDecay('omega:ewp -> pi0:ewp pi-:ewp pi+:ewp', '0.73 < M < 0.83', 1, True)
#

# EWP Skim
from EWP_List import *
XgammaList = B2XgammaList()
skimOutputUdst('BtoXgamma_Standalone', XgammaList)
summaryOfLists(XgammaList)

XllList = B2XllList()
skimOutputUdst('BtoXll_Standalone', XllList)
summaryOfLists(XllList)

# printDataStore()

process(analysis_main)

# print out the summary
print(statistics)
