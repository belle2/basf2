#!/usr/bin/env python3
# -*- coding: utf-8 -*-

################################################################################################
# This steering file is for the investigation of trigger menu by using L1 Emulator
# Contributor: Chunhua LI
##################################################################################################

from basf2 import *
from simulation import add_simulation
from reconstruction import add_reconstruction
from modularAnalysis import inputMdstList
from modularAnalysis import fillParticleList
from modularAnalysis import analysis_main
from modularAnalysis import ntupleFile
from modularAnalysis import ntupleTree
from L1emulator import *

logging.log_level = LogLevel.ERROR
emptypath = create_path()

filelistSIG = \
    [
        #    '../../../../release-head-head/MC/Analysis_L1emulator/MC/tauTomunn-tau1-prong/BGx0-noVXD/rootFiles/*root'
        '../../../../release-head-head/MC/Analysis_L1emulator/MC/Bhabha/BGx0-noVXD/rootFiles/*root'
        #    ' ../../../../release-head-head/MC/Analysis_L1emulator/MC/tau1prong-1prong/BGx0-noVXD/rootFiles/*root'
    ]

inputMdstList(filelistSIG)

# create charged tracks list
fillParticleList('pi+:HLT', 'pt>0.2')

# create gamma list
fillParticleList('gamma:HLT', 'E>0.1')

# add L1 Emulation
add_L1Emulation(analysis_main)


# define Ntuple
toolsY = ['EventMetaData', 'pi+']
# the trigger results are stored in the variable "Summary[50]" in Ntuple LETRG. The values are 0 by default.
# Summary[0]: If the event pass at least one trigger path, the value is the weight of the event which is assigned in generator
# Summary[1],[2],[3],[4] are corresponding to the trigger result of eclBhabha, BhabhaVeto, SBhabhaVeto, and ggVeto.
# Summary[5-50] are the trigger results of the trigger pathes.
toolsY += ['LETRG', 'pi+']
# The CDC, ECL, and KLM information used in TRGi are stored in Ntuples LECDC, LEECL, LEKLM
# The defination of variables in these ntuples could be found in link
toolsY += ['LECDC', 'pi+']
toolsY += ['LEECL', 'pi+']
toolsY += ['LEKLM', 'pi+']
# These ntuples are event level based, the parameters 'pi+' means nothing here.
ntupleFile('Bhabha.root')
ntupleTree('LETRG', '', toolsY)

process(analysis_main)

print(statistics)
