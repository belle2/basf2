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
from modularAnalysis import generateY4S
from modularAnalysis import ntupleFile
from modularAnalysis import ntupleTree

logging.log_level = LogLevel.INFO
emptypath = create_path()

filelistSIG = \
    [
        '/home/belle/chunhua/release-2015-05-01/MC/MC-noVXD/Bhabha/BGx0-allDetectors/rootFiles/*root'
    ]

inputMdstList(filelistSIG)


# create charged tracks list
fillParticleList('pi+:LE', '')

# create gamma list
fillParticleList('gamma:LE', 'E>0.1')

# load L1Emulation module
L1emulator = register_module('L1Emulation')

# The users could define triggers with the name "TRGi", which i here could be from 1 to 50.
# TRG1,TRG2,TRG3 are for eclBhabhaVetoLE, BhabhaVetoLE and ggVetoLE,
# respectivley,which have to be defined sequentially.The corresponding
# logics with names (eclBhabhaVetoLE, BhabhaVetoLE, ggVetoLE) are
# available in other triggers.

# eclBhabhaVetoLE
L1emulator.param('TRG1', 'EC1LE>3.0 and EC2LE>1.0 and AngGGLE>170')

# BhabhaVetoLE, the logic "eclBhabhaVetoLE" could is available here
L1emulator.param('TRG2', 'eclBhabhaVetoLE==1 and NtLE==2 and NtcLE==2')
# ggVetoLE
L1emulator.param('TRG3', 'eclBhabhaVetoLE==1 and NtLE==0')

# After definition, the logics "eclBhabhaVetoLE, BhabhaVetoLE, and ggVetoLE" couldbe used in the following trigger.

# Define users' triggers. The examples are following.

# Hadron
L1emulator.param('TRG4', ' NtLE>2 or EtotLE>1.0 or NcLE>3')

# BhabhaAccept1 and BhabhaAccept2
L1emulator.param('TRG5', 'NtLE==2 and NtcLE==2 and BhabhaVetoLE==1')
L1emulator.param('TRG6', 'NtLE>=1 and NtcLE>=1 and BhabhaVetoLE==1')

# ggAccept
L1emulator.param('TRG7', 'NtLE==0 and ggVetoLE==1')

# Single Photon
L1emulator.param('TRG8', 'EC1LE>2.0 and BhabhaVetoLE==0 and ggVetoLE==0')
L1emulator.param('TRG9', 'EC1LE>2.0 and BhabhaVetoLE==0')
L1emulator.param('TRG10', 'EC1LE>1.0 and NtLE<=1 and ggVetoLE==0')

# mu pair
# Dimu, 1Trk1mu
L1emulator.param('TRG11', 'NkLE>=2 and maxAngTTLE>45')
L1emulator.param('TRG12', 'NtLE>=1 and NkLE>=1 and maxAngTTLE>45')

analysis_main.add_module(L1emulator)

# define Ntuple
toolsY = ['EventMetaData', 'pi+']
# the trigger results are stored in the variable "Summary[50]" in Ntuple LETRG.
# if Trigger "TRGi" passed, then Summary[i-1]=1, otherwise =0.
# The valuses are 0 by default if "TRGi" is not defined in the script.
toolsY += ['LETRG', 'pi+']

# The CDC, ECL, and KLM information used in TRGi are stored in Ntuples LECDC, LEECL, LEKLM
# The defination of variables in these ntuples could be found in link
toolsY += ['LECDC', 'pi+']
toolsY += ['LEECL', 'pi+']
toolsY += ['LEKLM', 'pi+']

# These ntuples are event level based, the parameters 'pi+' means nothing here.
ntupleFile('Bhabha-allDetectors-test.root')
ntupleTree('LETRG', '', toolsY)

process(analysis_main)

print statistics
