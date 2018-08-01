#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Stuck? Ask for help at questions.belle2.org
#
# This Tutorial demonstrates how to use the
# MCDecayString NtupleTool.
#
# It is based on B2A301-Dstar2D0Pi-Reconstruction.py
# but adds the MCDecayString and uses an MC8 root file
# stored at KEK as input.
# The file is also available on the grid:
# /belle/MC/release-00-08-00/DB00000208/MC8/prod00000972/
#   s00/e0000/4S/r00000/ccbar/sub00/mdst_001680_prod00000972_task00001682.root
#
# The tutorial demonstrates how to reconstruct the
# following  decay chain (and c.c. decay chain):
#
# D*+ -> D0 pi+
#        |
#        +-> K- pi+
#
# Contributors: A. Zupanc  (June 2014 - B2A301)
#               M. Barrett (November 2017)
#               I. Komarov (December 2017)
#
######################################################

from basf2 import *
from modularAnalysis import inputMdstList
from modularAnalysis import reconstructDecay
from modularAnalysis import matchMCTruth
from modularAnalysis import analysis_main
from modularAnalysis import ntupleFile
from modularAnalysis import ntupleTree
from stdCharged import *

# Add MC9 signal samples
filelistSIG = [('/group/belle2/tutorial/release_01-00-00/mdst_000001_prod00002171_task00000001.root')]
inputMdstList('default', filelistSIG)

# use standard final state particle lists
#
# creates "pi+:all" ParticleList (and c.c.)
stdPi('all')
# creates "pi+:loose" ParticleList (and c.c.)
stdLoosePi()
# creates "K+:loose" ParticleList (and c.c.)
stdLooseK()

# reconstruct D0 -> K- pi+ decay
# keep only candidates with 1.8 < M(Kpi) < 1.9 GeV
reconstructDecay('D0:kpi -> K-:loose pi+:loose', '1.8 < M < 1.9')

# reconstruct D*+ -> D0 pi+ decay
# keep only candidates with Q = M(D0pi) - M(D0) - M(pi) < 20 MeV
# and D* CMS momentum > 2.5 GeV
reconstructDecay('D*+ -> D0:kpi pi+:all', '0.0 < Q < 0.020 and 2.5 < useCMSFrame(p) < 5.5')

# perform MC matching (MC truth asociation)
matchMCTruth('D*+')

# add the ParticleMCDecayString module to the analysis
# More details on the format of the MC decay string can be found here:
# https://confluence.desy.de/display/BI/Physics+MCDecayString
analysis_main.add_module('ParticleMCDecayString', listName='D*+', conciseString=False, fileName='my_hashmap.root')
from variables import variables
variables.addAlias('decayHash', 'extraInfo(DecayHash)')
variables.addAlias('decayHashExtended', 'extraInfo(DecayHashExtended)')

# create and fill flat Ntuple with MCTruth, kinematic information and D0 FlightInfo
from variableCollections import event_variables, kinematic_variables, cluster_variables,\
    track_variables, mc_variables, pid_variables, convert_to_daughter_vars, convert_to_gd_vars,\
    flight_info, mc_flight_info, vertex, mc_vertex

charged_particle_variables = kinematic_variables + track_variables + mc_variables + pid_variables

from modularAnalysis import variablesToNTuple
output_file = 'B2A504-Dstar2D0Pi-Reconstruction.root'
variablesToNTuple(filename=output_file,
                  decayString='D*+',
                  treename='dsttree',
                  ['decayHash', 'decayHashExtended'] + event_variables +
                  kinematic_variables + mc_variables +
                  convert_to_daughter_vars(charged_particle_variables + mc_variables, 1) +
                  convert_to_daughter_vars(kinematic_variables + mc_variables, 0) +
                  convert_to_gd_vars(charged_particle_variables, 0, 0) +
                  convert_to_gd_vars(charged_particle_variables, 0, 1))

# Process the events
process(analysis_main)

# print out the summary
print(statistics)
