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
# This tutorial demonstrates how to include the charm flavor             #
# tagging user interface into your analysis.                             #
# The following decay is recontructed:                                   #
#                                                                        #
# D0 -> K- pi+                                                           #
#                                                                        #
# The qr value, i.e. the flavor*dilution factor                          #
# is saved as extraInfo to the reconstructed D0.                         #
#                                                                        #
##########################################################################

import basf2 as b2
import modularAnalysis as ma
import charmFlavorTagger as cft
import variables.collections as vc
import variables.utils as vu


# create path
cft_path = b2.Path()

# append analysis global tag where the CFT payload is stored
b2.conditions.append_globaltag('analysis_tools_light-2302-genetta')

# load input ROOT file
ma.inputMdst(filename=b2.find_file('Dst2D0pi.root', 'examples', False),
             path=cft_path)

# create pion and kaon particle lists
ma.fillParticleList(
    "pi+:D0", "thetaInCDCAcceptance and dr < 1 and abs(dz) < 3 and pionID>0.1", path=cft_path
)
ma.fillParticleList(
    "K+:D0", "thetaInCDCAcceptance and dr < 1 and abs(dz) < 3 and kaonID>0.1", path=cft_path
)

# reconstruct D0 signal candidates
ma.reconstructDecay(
    decayString="D0:sig -> K-:D0 pi+:D0",
    cut="1.8 < InvM < 1.9",
    path=cft_path,
)

# Does the matching between reconstructed and MC particles
ma.matchMCTruth(list_name='D0:sig', path=cft_path)

# build the rest of the event associated to the D0
ma.buildRestOfEvent(target_list_name='D0:sig',
                    path=cft_path)

# Charm Flavor Tagging Function.
cft.charmFlavorTagger(
    'D0:sig',
    path=cft_path)

# Select variables that will be stored to ntuple
d_vars = vc.kinematics+vc.mc_truth+["CFT_qr", "CFT_prob"]
d_vars += vu.create_aliases_for_selected(
    list_of_variables=vc.kinematics
    + vc.mc_truth,
    decay_string="D0 -> ^K- ^pi+",
)

# Saving variables to ntuple
output_file = 'B2A913-CharmFlavorTagger.root'
ma.variablesToNtuple(decayString='D0:sig',
                     variables=d_vars,
                     filename=output_file,
                     treename='D0tree',
                     path=cft_path)

# Process the events
b2.process(cft_path)

# print out the summary
print(b2.statistics)
