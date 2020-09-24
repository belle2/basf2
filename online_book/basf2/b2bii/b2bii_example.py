#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import sys

import basf2
import modularAnalysis as ma
import variables as va
import variables.collections as vc
import variables.utils as vu
from b2biiConversion import convertBelleMdstToBelleIIMdst

os.environ['USE_GRAND_REPROCESS_DATA'] = '1'
os.environ['PGUSER'] = 'g0db'

# Convert
mypath = basf2.create_path()
inputfile = basf2.find_file('b2bii_input_evtgen_exp_07_BptoD0pip-D0toKpipi0-0.mdst', 'examples', False)
convertBelleMdstToBelleIIMdst(inputfile, path=mypath)

# Only charged final state particles need to be loaded. The neutral particles
# gamma, pi0, K_S0, K_L0, and Lambda0 are already loaded to the 'gamma:mdst',
# 'pi0:mdst', 'K_S0:mdst', 'K_L0:mdst', and 'Lambda0:mdst' particle lists,
# respectively.
ma.fillParticleList('pi+:all', 'atcPIDBelle(3,2)<0.4', path=mypath)
ma.fillParticleList('K+:all', 'atcPIDBelle(3,2)>0.6', path=mypath)

# Let's have a look at the pi0 candidates in 'pi0:mdst' and print the values of some variables
# In order to access the MC information we need to run the MC matching first
ma.matchMCTruth('pi0:mdst', path=mypath)

# The Belle PID variables are: atcPIDBelle(sigHyp, bkgHyp), muIDBelle, and eIDBelle
va.variables.addAlias('Lkpi', 'atcPIDBelle(3,2)')

# Now, let's really reconstruct a B decay with an intermediate D meson:
ma.reconstructDecay('D0:Kpipi0 -> K-:all pi+:all pi0:mdst', '1.7 < M < 2.0', path=mypath)
ma.reconstructDecay('B+:D0pi -> anti-D0:Kpipi0 pi+:all', '4.8 < M < 5.5', path=mypath)

ma.matchMCTruth('B+:D0pi', path=mypath)

# create and fill flat Ntuple with MCTruth and kinematic information
kinematics_and_truth = vc.kinematics + vc.mc_truth
variables = vu.create_aliases_for_selected(kinematics_and_truth, '^B+ -> [ ^D0 -> ^K- ^pi+ ^pi0] ^pi+')

belle1pid = ['eIDBelle', 'muIDBelleQuality', 'muIDBelle', 'Lkpi']
variables += vu.create_aliases_for_selected(belle1pid, 'B+ -> [ D0 -> ^K- ^pi+ pi0] ^pi+')

ma.variablesToNtuple('B+:D0pi', variables, filename='B2BII_B2D0pi_D2Kpipi0.root', path=mypath)

# progress
mypath.add_module('Progress')

basf2.process(mypath)

# Print call statistics
print(basf2.statistics)
