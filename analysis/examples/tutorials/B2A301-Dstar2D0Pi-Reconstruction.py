#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Stuck? Ask for help at questions.belle2.org
#
# This tutorial demonstrates how to reconstruct the
# following  decay chain (and c.c. decay chain):
#
# D*+ -> D0 pi+
#        |
#        +-> K- pi+
#
# Contributors: A. Zupanc (June 2014)
#               I. Komarov (Demeber 2017)
#
######################################################

import basf2 as b2
import modularAnalysis as ma
import variableCollections as vc
import variableCollectionsTools as vct
import stdCharged as stdc
from stdV0s import stdKshorts
from stdPi0s import stdPi0s


# check if the required input file exists
import os
if not os.path.isfile(os.getenv('BELLE2_EXAMPLES_DATA') + '/Dst2D0pi.root'):
    b2.B2FATAL("You need the example data installed. Run `b2install-example-data` in terminal for it.")

# create path
my_path = ma.analysis_main

# load input ROOT file
ma.inputMdst(environmentType='default',
             filename='$BELLE2_EXAMPLES_DATA/Dst2D0pi.root',
             path=my_path)


# use standard final state particle lists
#
# creates "pi+:all" ParticleList (and c.c.)
stdc.stdPi(listtype='all', path=my_path)
# creates "pi+:loose" ParticleList (and c.c.)
stdc.stdLoosePi(path=my_path)
# creates "K+:loose" ParticleList (and c.c.)
stdc.stdLooseK(path=my_path)

# reconstruct D0 -> K- pi+ decay
# keep only candidates with 1.8 < M(Kpi) < 1.9 GeV
ma.reconstructDecay(decayString='D0:kpi -> K-:loose pi+:loose', cut='1.8 < M < 1.9', path=my_path)

# reconstruct D*+ -> D0 pi+ decay
# keep only candidates with Q = M(D0pi) - M(D0) - M(pi) < 20 MeV
# and D* CMS momentum > 2.5 GeV
ma.reconstructDecay(decayString='D*+ -> D0:kpi pi+:all', cut='0.0 < Q < 0.020 and 2.5 < useCMSFrame(p) < 5.5', path=my_path)

# perform MC matching (MC truth asociation)
ma.matchMCTruth(list_name='D*+', path=my_path)

# Select variables that we want to store to ntuple
dstar_vars = vc.event_meta_data + vc.inv_mass + vc.ckm_kinematics + vc.mc_truth

fs_hadron_vars = vct.convert_to_all_selected_vars(
    variables_list=vc.pid + vc.track + vc.mc_truth,
    decay_string='D*+ -> [D0 -> ^K- ^pi+] ^pi+')

d0_vars = vct.convert_to_one_selected_vars(
    variables_list=vc.inv_mass + vc.mc_truth,
    decay_string='D*+ -> ^D0 pi+', 'D0')


# Saving variables to ntuple
output_file = 'B2A301-Dstar2D0Pi-Reconstruction.root'
ma.variablesToNtuple('D*+', dstar_vars + d0_vars + fs_hadron_vars,
                     filename=output_file, treename='dsttree', path=my_path)

# Process the events
b2.process(my_path)

# print out the summary
print(b2.statistics)
