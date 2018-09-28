#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Stuck? Ask for help at questions.belle2.org
#
# This tutorial demonstrates how to reconstruct the
# following  decay chain:
#
# B0 -> rho gamma
#       |
#       +-> pi+ pi-
#
#
# Contributors: A. Zupanc (June 2014)
#               I. Komarov (Demeber 2017)
#               I. Komarov (September 2018)
#
################################################################################

import basf2 as b2
import modularAnalysis as ma
import variableCollections as vc
import variableCollectionsTools as vct
from stdPhotons import stdPhotons
import stdCharged as stdc

# check if the required input file exists
import os
if not os.path.isfile(os.getenv('BELLE2_EXAMPLES_DATA') + '/B2rhogamma_rho2pipi.root'):
    b2.B2FATAL("You need the example data installed. Run `b2install-example-data` in terminal for it.")

# create path
my_path = ma.analysis_main

# load input ROOT file
ma.inputMdst(environmentType='default',
             filename='$BELLE2_EXAMPLES_DATA/B2rhogamma_rho2pipi.root',
             path=my_path)

#
# creates "gamma:tight" ParticleList
stdPhotons(listtype='tight', path=my_path)

# creates "pi+:loose" ParticleList (and c.c.)
stdc.stdLoosePi(path=my_path)

# reconstruct rho -> pi+ pi- decay
# keep only candidates with 0.6 < M(pi+pi-) < 1.0 GeV
ma.reconstructDecay(decayString='rho0 -> pi+:loose pi-:loose',
                    cut='0.6 < M < 1.0',
                    path=my_path)

# reconstruct B0 -> rho0 gamma decay
# keep only candidates with Mbc > 5.2 GeV
# and -2 < Delta E < 2 GeV
ma.reconstructDecay(decayString='B0 -> rho0 gamma:tight',
                    cut='5.2 < Mbc < 5.29 and abs(deltaE) < 2.0',
                    path=my_path)

# perform MC matching (MC truth asociation)
ma.matchMCTruth(list_name='B0', path=my_path)

# Select variables that we want to store to ntuple

gamma_vars = vc.cluster + vc.mc_truth + vc.kinematics
rho_vars = vc.cluster + vc.mc_truth + vc.kinematics + vc.inv_mass
pi_vars = vc.pid + vc.track
b_vars = vc.event_meta_data + \
    vc.kinematics + \
    vc.deltae_mbc + \
    vc.mc_truth + \
    vct.convert_to_all_selected_vars(variables_list=gamma_vars,
                                     decay_string='B0 -> rho0 ^gamma') + \
    vct.convert_to_all_selected_vars(variables_list=rho_vars,
                                     decay_string='B0 -> ^rho0 gamma') + \
    vct.convert_to_all_selected_vars(variables_list=rho_vars,
                                     decay_string='B0 -> [rho0 -> ^pi+ ^pi-] gamma')

# Saving variables to ntuple
rootOutputFile = 'B2A304-B02RhoGamma-Reconstruction.root'
ma.variablesToNtuple(decayString='B0',
                     variables=b_vars,
                     filename=rootOutputFile,
                     treename='b0',
                     path=my_path)

# Process the events
b2.process(my_path)

# print out the summary
print(b2.statistics)
