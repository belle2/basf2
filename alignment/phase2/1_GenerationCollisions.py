#!/usr/bin/env python
# -*- coding: utf-8 -*-

# *****************************************************************************

# title           : 1_Generation.py
# description     : Generation & simulation collision events
# author          : Jakub Kandra (jakub.kandra@karlov.mff.cuni.cz)
# date            : 28. 12. 2017

# *****************************************************************************

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

# create path
main = create_path()

# specify number of events to be generated
# the experiment number for phase2 MC has to be 1002, otherwise the wrong payloads (for VXDTF2 the SectorMap) are loaded
eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param('evtNumList', [10000])  # we want to process 1000 events
eventinfosetter.param('runList', [0])  # from run number 1
eventinfosetter.param('expList', [1002])  # and experiment number 1002 (Phase2)
main.add_module(eventinfosetter)

# in case you need to fix seed of random numbers
# set_random_seed('d33fa68eab781f3dcb069fb23425885fcd92d3432e6433a14894e5d7bba34272')

main.add_module("Gearbox", fileName='/geometry/Beast2_phase2.xml')

# Register the geometry module
geometry = register_module('Geometry')
components = [
    'BeamPipe',
    'MagneticField',
    'PXD',
    'SVD',
    'CDC',
    'EKLM',
    'BKLM']

"""
'TOP',
'ARICH',
'ECL'
"""

geometry.param('components', components)
main.add_module(geometry)

# generate BBbar events
main.add_module('EvtGenInput')

# generate di-muon events
# kkgeninput = register_module('KKGenInput')
# kkgeninput.param('tauinputFile', Belle2.FileSystem.findFile('data/generators/kkmc/mu.input.dat'))
# kkgeninput.param('KKdefaultFile', Belle2.FileSystem.findFile('data/generators/kkmc/KK2f_defaults.dat'))
# kkgeninput.param('taudecaytableFile', '')
# kkgeninput.param('kkmcoutputfilename', 'kkmc_mumu.txt')

# main.add_module(kkgeninput)

# detector simulation
add_simulation(main, components=components)
# add_simulation(main, bkgfiles=bg)

# trigger simulation
add_tsim(main)

main.add_module("RootOutput")

# process events and print call statistics
process(main)
print(statistics)
