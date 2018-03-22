#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#############################################################
# Simple steering file to demonstrate how to run Track DQM on Phase2 geometry
#############################################################

from basf2 import *
from ROOT import Belle2
from simulation import add_simulation
from reconstruction import add_reconstruction
from L1trigger import add_tsim
import glob

# background (collision) files
# bg = glob.glob('./BG/*.root')
# on KEKCC: (choose one of the sets)
# bg = /group/belle2/BGFile/OfficialBKG/15thCampaign/phase2/set*/*.root
bg = None

# number of events to generate, can be overriden with -n
num_events = 100

# create path
main = create_path()

# specify number of events to be generated
# the experiment number for phase2 MC has to be 1002, otherwise the wrong payloads (for VXDTF2 the SectorMap) are loaded
main.add_module("EventInfoSetter", expList=1002, runList=1, evtNumList=num_events)

# in case you need to fix seed of random numbers
# set_random_seed('d33fa68eab781f3dcb069fb23425885fcd92d3432e6433a14894e5d7bba34272')

# generate BBbar events
# main.add_module('EvtGenInput')

kkgeninput = register_module('KKGenInput')
kkgeninput.param('tauinputFile', Belle2.FileSystem.findFile('data/generators/kkmc/mu.input.dat'))
kkgeninput.param('KKdefaultFile', Belle2.FileSystem.findFile('data/generators/kkmc/KK2f_defaults.dat'))
kkgeninput.param('taudecaytableFile', '')
kkgeninput.param('kkmcoutputfilename', 'kkmc_mumu.txt')

main.add_module(kkgeninput)

# detector simulation
add_simulation(main, bkgfiles=bg)

# trigger simulation
add_tsim(main)

main.add_module("RootOutput")

# process events and print call statistics
process(main)
print(statistics)
