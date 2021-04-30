#!/usr/bin/env python3

#######################################################
#
# Stuck? Ask for help at questions.belle2.org
#
# This tutorial demonstrates how to apply helix error
# correction using HelixErrorScalerModule.
# The following  decay chain (and c.c. decay chain):
#
# B0 -> J/psi K_S0
#        |
#        +-> mu- mu+
#
# is reconstructed w/ and w/o the correction and the
# B0 decay vertex is fitted
#
# Note: This example is build upon
# B2A403-KFit-VertexFit.py
#
# Contributors: H. Tanigawa (December 2020)
#
######################################################

import basf2 as b2
import modularAnalysis as ma
from vertex import kFit
from stdCharged import stdMu
from stdV0s import stdKshorts
import variables.collections as vc
import variables.utils as vu

# create path
my_path = b2.create_path()

# load input ROOT file
ma.inputMdst(environmentType='default',
             filename=b2.find_file('B02JpsiKs_Jpsi2mumu_Ks2pipi.root', 'examples', False),
             path=my_path)

# load standard mu and K_S0 particle lists
stdMu('loose', path=my_path)
stdKshorts(path=my_path)

# create a new list of mu with scaled error
ma.scaleError('mu+:scaled', 'mu+:loose', path=my_path)

# reconstruct B0 -> J/psi K_S0 decay
ma.reconstructDecay('J/psi:default -> mu+:loose mu-:loose', '3.05 < M < 3.15', path=my_path)
ma.reconstructDecay('B0:default -> J/psi:default K_S0:merged', '5.27 < Mbc < 5.29 and abs(deltaE)<0.1', path=my_path)
ma.reconstructDecay('J/psi:scaled -> mu+:scaled mu-:scaled', '3.05 < M < 3.15', path=my_path)
ma.reconstructDecay('B0:scaled -> J/psi:scaled K_S0:merged', '5.27 < Mbc < 5.29 and abs(deltaE)<0.1', path=my_path)

# perform B0 vertex fit only using the muons
kFit('B0:default', 0., decay_string='B0 -> [J/psi -> ^mu+ ^mu-] K_S0', path=my_path)
kFit('B0:scaled', 0., decay_string='B0 -> [J/psi -> ^mu+ ^mu-] K_S0', path=my_path)

# MC matching
ma.matchMCTruth('B0:default', path=my_path)
ma.matchMCTruth('B0:scaled', path=my_path)

# select variables to be saved
B0_vars = vc.mc_truth + vc.vertex + vc.mc_vertex
B0_vars += ['LBoost', 'LBoostErr', 'mcLBoost']
helices = [a+b for a in ['d0', 'phi0', 'omega', 'z0', 'tanLambda'] for b in ['', 'Err', 'Pull']]
B0_vars += vu.create_aliases_for_selected(helices, 'B0 -> [J/psi -> ^mu+ ^mu-] K_S0')

# save variables
outputfile = 'scaleHelixError_output.root'
ma.variablesToNtuple('B0:default', B0_vars,
                     filename=outputfile, treename='B0_default', path=my_path)
ma.variablesToNtuple('B0:scaled', B0_vars,
                     filename=outputfile, treename='B0_scaled', path=my_path)

# process the events
b2.process(my_path)

# print out the summary
print(b2.statistics)
