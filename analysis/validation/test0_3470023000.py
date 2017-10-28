#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# Descriptor: e+e- --> (tau+ --> mu nu anti-nu) (tau- --> (a0 --> eta pi) nu)

"""
<header>
  <output>../3470023000.dst.root</output>
  <contact>karim.trabelsi@kek.jp</contact>
</header>
"""

from basf2 import *
from simulation import add_simulation
from reconstruction import add_reconstruction, add_mdst_output
from HLTTrigger import add_HLT_Y4S
from ROOT import Belle2
import sys
import glob

set_random_seed(12345)

# background (collision) files
# bg = glob.glob('/group/belle2/users/harat/basf2/cvmfsv000700/BG/set1/[A-Z]*.root')  # if you run at KEKCC
bg = glob.glob('./BG/[A-Z]*.root')

# main path
main = create_path()

# event info setter
eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param('evtNumList', [100])
eventinfosetter.param('runList', [1])
eventinfosetter.param('expList', [1])
main.add_module(eventinfosetter)

# to run the framework the used modules need to be registered
kkgeninput = register_module('KKGenInput')
kkgeninput.param('tauinputFile', Belle2.FileSystem.findFile('data/generators/kkmc/tau.input.dat'))
kkgeninput.param('KKdefaultFile', Belle2.FileSystem.findFile('data/generators/kkmc/KK2f_defaults.dat'))
kkgeninput.param('taudecaytableFile', './3470023000.dat')
main.add_module(kkgeninput)

# detector simulation
# add_simulation(main, bkgfiles=bg)
add_simulation(main)

# HLT L3 simulation
main.add_module('Level3')

# reconstruction
add_reconstruction(main)

# HLT physics trigger
add_HLT_Y4S(main)

# Finally add mdst output
add_mdst_output(main, filename='../3470023000.dst.root')

# generate events
process(main)
print(statistics)
