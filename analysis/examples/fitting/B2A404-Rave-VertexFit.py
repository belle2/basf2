#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

##########################################################################
#                                                                        #
# Stuck? Ask for help at questions.belle2.org                            #
#                                                                        #
# This tutorial demonstrates how to perform Vertex fits                  #
# using Rave. The following  decay chain (and c.c. decay                 #
# chain):                                                                #
#                                                                        #
# D*+ -> D0 pi+                                                          #
#        |                                                               #
#        +-> K- pi+                                                      #
#                                                                        #
# is reconstructed and the D0 and D*+ decay vertices are                 #
# fitted.                                                                #
#                                                                        #
# Note: This example is build upon                                       #
# B2A301-Dstar2D0Pi-Reconstruction.py                                    #
#                                                                        #
##########################################################################

import basf2 as b2
from modularAnalysis import inputMdst
from modularAnalysis import reconstructDecay
from modularAnalysis import matchMCTruth
from vertex import raveFit
from stdCharged import stdPi, stdK
from modularAnalysis import variablesToNtuple
import variables.collections as vc
import variables.utils as vu

# create path
my_path = b2.create_path()

# load input ROOT file
inputMdst(environmentType='default',
          filename=b2.find_file('B02pi0D0_D2kpi_B2Dstarpi_Dstar2Dpi_D2kpi.root', 'examples', False),
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
# keep candidates only passing C.L. value of the fit > 0.0 (no cut)
raveFit('D0:kpi', 0.0, path=my_path)

# reconstruct D*+ -> D0 pi+ decay
# keep only candidates with Q = M(D0pi) - M(D0) - M(pi) < 20 MeV
# and D* CMS momentum > 2.5 GeV
reconstructDecay('D*+ -> D0:kpi pi+:all',
                 '0.0 <= Q < 0.02 and 2.5 < useCMSFrame(p) < 5.5', path=my_path)

# perform MC matching (MC truth association)
matchMCTruth('D*+', path=my_path)

# perform D*+ kinematic vertex fit using the D0 and the pi+
# keep candidates only passing C.L. value of the fit > 0.0 (no cut)
raveFit('D*+', 0.0, path=my_path)

# Select variables that we want to store to ntuple

dstar_vars = vc.inv_mass + vc.mc_truth + \
    vc.mc_flight_info + vc.flight_info

fs_hadron_vars = vu.create_aliases_for_selected(
    vc.pid + vc.track + vc.mc_truth,
    'D*+ -> [D0 -> ^K- ^pi+] ^pi+')

d0_vars = vu.create_aliases_for_selected(
    vc.inv_mass + vc.mc_truth,
    'D*+ -> ^D0 pi+', 'D0')


# Saving variables to ntuple
output_file = 'B2A404-Rave-VertexFit.root'
variablesToNtuple('D*+', dstar_vars + d0_vars + fs_hadron_vars,
                  filename=output_file, treename='dsttree', path=my_path)


# Process the events
b2.process(my_path)

# print out the summary
print(b2.statistics)
