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
import variables.collections as vc
import variables.utils as vu
import stdCharged as stdc
from stdV0s import stdKshorts
from stdPi0s import stdPi0s

# create path
my_path = b2.create_path()

# load input ROOT file
ma.inputMdst(environmentType='default',
             filename=b2.find_file('Dst2D0pi.root', 'examples', False),
             path=my_path)


# use standard final state particle lists
#
# creates "pi+:all" ParticleList (and c.c.)
stdc.stdPi(listtype='all', path=my_path)
# creates "pi+:loose" ParticleList (and c.c.)
stdc.stdPi(listtype='loose', path=my_path)
# creates "K+:loose" ParticleList (and c.c.)
stdc.stdK(listtype='loose', path=my_path)

# reconstruct D0 -> K- pi+ decay
# keep only candidates with 1.8 < M(Kpi) < 1.9 GeV
ma.reconstructDecay(decayString='D0:kpi -> K-:loose pi+:loose', cut='1.8 < M < 1.9', path=my_path)

# reconstruct D*+ -> D0 pi+ decay
# keep only candidates with Q = M(D0pi) - M(D0) - M(pi) < 20 MeV
# and D* CMS momentum > 2.5 GeV
ma.reconstructDecay(decayString='D*+ -> D0:kpi pi+:all', cut='0.0 < Q < 0.2', path=my_path)

# perform MC matching (MC truth asociation)
ma.matchMCTruth(list_name='D*+', path=my_path)

# Select variables that we want to store to ntuple
dstar_vars = vc.inv_mass + vc.mc_truth

fs_hadron_vars = vu.create_aliases_for_selected(
    list_of_variables=vc.pid + vc.track + vc.mc_truth,
    decay_string='D*+ -> [D0 -> ^K- ^pi+] ^pi+')

d0_vars = vu.create_aliases_for_selected(
    list_of_variables=vc.inv_mass + vc.mc_truth,
    decay_string='D*+ -> ^D0 pi+',
    prefix='D0')


# Saving variables to ntuple
output_file = 'B2A301-Dstar2D0Pi-Reconstruction.root'
ma.variablesToNtuple('D*+', dstar_vars + d0_vars + fs_hadron_vars,
                     filename=output_file, treename='dsttree', path=my_path)

# Process the events
b2.process(my_path)

# print out the summary
print(b2.statistics)
