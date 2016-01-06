#!/usr/bin/env python
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
        #    '../../../../release-head-head/MC/Analysis_L1emulator/MC/eeee_pre/BGx0-noVXD/rootFiles-nopreselection/*root'
    ]

inputMdstList(filelistSIG)

# create charged tracks list
fillParticleList('pi+:HLT', 'pt>0.2')

# create gamma list
fillParticleList('gamma:HLT', 'E>0.1')


# Users can develop their own trigger path as shown below
# The four veto logics  eclBhabha, BhabhaVeto, SBhabhaVeto, and ggVeto have to be defined firstly.

# eclBhabhaVetoHLT
L1Emulation('eclBhabhaVeto', 'EC1HLT>0 and EC2HLT>0.0 and EC12HLT>7.0 and AngGGCMSHLT>1.745', analysis_main, [1])

# BhabhaVetoHLT, the logic "eclBhabhaVetoHLT" could is available here
L1Emulation('BhabhaVeto', 'eclBhabhaVetoHLT==1 and NtHLT==2 and NtcHLT==2', analysis_main, [1])

# Bhabha veto with single track
L1Emulation(
    'SBhabhaVeto',
    'EC1HLT > 0 and EC2HLT > 0.0 and EC12HLT > 5.0 and AngGGCMSHLT > 1.745 and' +
    ' NtHLT == 1 and NtcHLT == 1 and ENeutralHLT > 0.5 and PT1HLT > 2.5 and ET1HLT > 2.0',
    analysis_main,
    [1])

# ggVetoHLT
L1Emulation('ggVeto', 'eclBhabhaVetoHLT==1 and NtHLT==0', analysis_main, [1])

# User could also use the following default veto definations instead of the above four custom definations.
# L1Emulation_Veto(analysis_main)

# Then user could define the physical trigger path, the total pathes should be less than 50
# parameters 1: the name of the path given by user 2: the trigger logic 3: the path  4: the scale factor. if there
# is no scale for the path, set [1]. if you set the array with more than one component, e.g. [1000,200,50,10], the
# trigger path will be scaled as a function of the election's theta in lab frame by default.
L1Emulation('TwoTracks', 'NltHLT==2 and BhabhaVetoHLT==0', analysis_main, [1])
L1Emulation('TwoTracksScale', 'NltHLT==2', analysis_main, [2000])
L1Emulation('OneTrkOneMu', 'NtHLT>=1 and Layer1KLMHLT>=3 and maxAngTMHLT>0.785', analysis_main, [1])
L1Emulation('Hadron1', 'NtHLT>=3 and NltHLT>=2', analysis_main, [1])
L1Emulation('Neutral', 'NcHLT>=3 and AngGGCMSHLT>0.349 and AngGGCMSHLT<2.967 and eclBhabhaVetoHLT==0', analysis_main, [1])

# Finally, load L1EmulationPrint module to print the reigger result on screen.
L1EmulationPrint(analysis_main)

# define Ntuple
toolsY = ['EventMetaData', 'pi+']
# the trigger results are stored in the variable "Summary[50]" in Ntuple LETRG. The values are 0 by default.
# Summary[0]: If the event pass at least one trigger path, the value is the weight of the event which is assigned in generator
# Summary[1],[2],[3],[4] are corresponding to the trigger result of eclBhabha, BhabhaVeto, SBhabhaVeto, and ggVeto.
# Summary[5-50] are the trigger results of the trigger pathes.
toolsY += ['LETRG', 'pi+']
# The CDC, ECL, and KLM information are stored in Ntuples LECDC, LEECL, LEKLM
# The defination of variables in these ntuples could be found in link
toolsY += ['LECDC', 'pi+']
toolsY += ['LEECL', 'pi+']
toolsY += ['LEKLM', 'pi+']


# These ntuples are event level based, the parameters 'pi+' means nothing here.
ntupleFile('BB.root')
ntupleTree('LETRG', '', toolsY)

process(analysis_main)

print(statistics)
