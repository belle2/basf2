#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import os
import sys

import basf2
import modularAnalysis as ma
import variables as va
import variables.collections as vc
import variables.utils as vu
from vertex import kFit
from b2biiConversion import convertBelleMdstToBelleIIMdst
from b2biiMonitors import addBeamParamsConversionMonitors
from b2biiMonitors import addTrackConversionMonitors
from b2biiMonitors import addNeutralsConversionMonitors

os.environ['USE_GRAND_REPROCESS_DATA'] = '1'
os.environ['PGUSER'] = 'g0db'

# If you want to create the monitoring histograms (recommended in the beginning of your analysis), just provide any argument
monitoring = False
if len(sys.argv) == 1:
    basf2.B2WARNING("In the beginning of an analysis it is recommended to study the monitoring histograms.\n"
                    "These tell you if the conversion works as expected.\n"
                    "If you want to create them, just provide any argument to this script.")
else:
    monitoring = True

print('BELLE2_EXTERNALS_DIR     = ' + str(os.getenv('BELLE2_EXTERNALS_DIR')))
print('BELLE2_EXTERNALS_SUBDIR  = ' + str(os.getenv('BELLE2_EXTERNALS_SUBDIR')))
print('BELLE2_EXTERNALS_OPTION  = ' + str(os.getenv('BELLE2_EXTERNALS_OPTION')))
print('BELLE2_EXTERNALS_VERSION = ' + str(os.getenv('BELLE2_EXTERNALS_VERSION')))
print('BELLE2_LOCAL_DIR         = ' + str(os.getenv('BELLE2_LOCAL_DIR')))
print('BELLE2_RELEASE           = ' + str(os.getenv('BELLE2_RELEASE')))
print('BELLE2_OPTION            = ' + str(os.getenv('BELLE2_OPTION')))
print('BELLE_POSTGRES_SERVER    = ' + str(os.getenv('BELLE_POSTGRES_SERVER')))
print('USE_GRAND_REPROCESS_DATA = ' + str(os.getenv('USE_GRAND_REPROCESS_DATA')))
print('PANTHER_TABLE_DIR        = ' + str(os.getenv('PANTHER_TABLE_DIR')))
print('PGUSER                   = ' + str(os.getenv('PGUSER')))

# Convert
mypath = basf2.create_path()
inputfile = basf2.find_file('b2bii_input_evtgen_exp_07_BptoD0pip-D0toKpipi0-0.mdst', 'examples', False)
convertBelleMdstToBelleIIMdst(inputfile, path=mypath)

# Reconstruct
if monitoring:
    # Create monitoring histograms if requested
    addBeamParamsConversionMonitors(path=mypath)
    addTrackConversionMonitors(path=mypath)
    addNeutralsConversionMonitors(path=mypath)

# Only charged final state particles need to be loaded. The neutral particles
# gamma, pi0, K_S0, K_L0, and Lambda0 are already loaded to the 'gamma:mdst',
# 'pi0:mdst', 'K_S0:mdst', 'K_L0:mdst', and 'Lambda0:mdst' particle lists,
# respectively.
ma.fillParticleList('pi+:all', '', path=mypath)
ma.fillParticleList('K+:all', '', path=mypath)
ma.fillParticleList('mu+:all', '', path=mypath)
ma.fillParticleList('e+:all', '', path=mypath)

# Let's have a look at the pi0 candidates in 'pi0:mdst' and print the values of some variables
# In order to access the MC information we need to run the MC matching first
ma.matchMCTruth('pi0:mdst', path=mypath)
ma.printVariableValues('gamma:mdst', ['mcPDG', 'E', 'clusterE9E25'], path=mypath)
ma.printVariableValues('pi0:mdst', ['mcPDG', 'p', 'M', 'InvM'], path=mypath)

# The advantage of the pre-loaded V0s (which are the only ones that you should
# use in B2BII analyses) is that the momenta and the position of the daughter
# tracks are determined wrt. a pivot at the decay vertex of the V0. In
# addition, K_S0:mdst (Lambda0:mdst) has goodKs (goodLambda) and nisKsFinder
# outputs attached as extra info. It can be used to select good candidates.
ma.cutAndCopyList('K_S0:good', 'K_S0:mdst', cut='goodBelleKshort', path=mypath)

# It makes sense to perform a vertex fit of the K_S0 candidates and accept
# only those candidates where the vertex fit converged
kFit('K_S0:good', 0, path=mypath)

# Again, let's print a few variable values:
ma.matchMCTruth('K_S0:good', path=mypath)
ma.printVariableValues('K_S0:good', ['mcPDG', 'M', 'InvM', 'p', 'px', 'py', 'pz',
                                     'extraInfo(goodKs)', 'extraInfo(ksnbVLike)', 'extraInfo(ksnbNoLam)'], path=mypath)

# The Belle PID variables are: atcPIDBelle(sigHyp, bkgHyp), muIDBelle, and eIDBelle
va.variables.addAlias('Lkpi', 'atcPIDBelle(3,2)')
va.variables.addAlias('Lppi', 'atcPIDBelle(4,2)')
va.variables.addAlias('Lpk', 'atcPIDBelle(4,3)')

# Since we did not apply any PID requirement the 'pi+:all' particle list
# contains all type of charged final state particles. Have a look at the
# printOut and notice how the true identity correlates with the corresponding
# PID values.
ma.printVariableValues('pi+:all', ['mcPDG', 'p', 'Lkpi', 'Lppi', 'muIDBelle',
                                   'muIDBelleQuality', 'eIDBelle', 'nSVDHits'], path=mypath)

# Now, let's really reconstruct a B decay with an intermediate D meson:
ma.reconstructDecay('D0:Kpipi0 -> K-:all pi+:all pi0:mdst', '1.7 < M < 2.0', path=mypath)
ma.reconstructDecay('B+:D0pi -> anti-D0:Kpipi0 pi+:all', '4.8 < M < 5.5', path=mypath)

ma.matchMCTruth('B+:D0pi', path=mypath)

# create and fill flat Ntuple with MCTruth and kinematic information
kinematics_and_truth = vc.kinematics + vc.mc_truth
variables = vu.create_aliases_for_selected(kinematics_and_truth, '^B+ -> [ ^D0 -> ^K- ^pi+ ^pi0] ^pi+')

belle1pid = ['eIDBelle', 'muIDBelleQuality', 'muIDBelle', 'Lkpi', 'Lppi', 'Lpk']
variables += vu.create_aliases_for_selected(belle1pid, 'B+ -> [ D0 -> ^K- ^pi+ pi0] ^pi+')

ma.variablesToNtuple('B+:D0pi', variables, filename='B2BII_ConvertAndReconstruct_Example.root', path=mypath)

# progress
mypath.add_module('Progress')

basf2.process(mypath)

# Print call statistics
print(basf2.statistics)
