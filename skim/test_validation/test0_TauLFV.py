#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# Descriptor:  tau LFV decays to mg,mmm,mKs,eg,eee,eKs

"""
<header>
  <output>TauLFV.dst.root, kkmc_tautau.txt</output>
  <contact>kenji@hepl.phys.nagoya-u.ac.jp</contact>
</header>
"""

from basf2 import *
from simulation import add_simulation
from reconstruction import add_reconstruction, add_mdst_output
from ROOT import Belle2
import glob

set_random_seed(12345)

# background (collision) files
# bg = glob.glob('./BG/[A-Z]*.root')

# main path
main = create_path()

# specify number of events to be generated
eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param('evtNumList', [10000])
eventinfosetter.param('runList', [1])
eventinfosetter.param('expList', [0])
main.add_module(eventinfosetter)

# beam parameters
from beamparameters import add_beamparameters
beamparameters = add_beamparameters(main, "Y4S")

# generator
kkgeninput = register_module('KKGenInput')
kkgeninput.param('tauinputFile', Belle2.FileSystem.findFile('data/generators/kkmc/tau.input.dat'))
kkgeninput.param('KKdefaultFile', Belle2.FileSystem.findFile('data/generators/kkmc/KK2f_defaults.dat'))
kkgeninput.param('taudecaytableFile', Belle2.FileSystem.findFile('tau_LFV_p.dat'))
kkgeninput.param('kkmcoutputfilename', 'kkmc_tautau.txt')
main.add_module(kkgeninput)

# detector simulation
# add_simulation(main, bkgfiles=bg)
add_simulation(main)

# reconstruction
add_reconstruction(main)

# Finally add mdst output
output_filename = "TauLFV.dst.root"
add_mdst_output(main, filename=output_filename)

# process events and print call statistics
process(main)
print(statistics)
