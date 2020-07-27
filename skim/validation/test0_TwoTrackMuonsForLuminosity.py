#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# Descriptor: e+e- --> mu+mu-(ISR)

"""
<header>
  <output>../TwoTrackMuonsForLuminosity.dst.root</output>
  <contact>zhouxy@buaa.edu.cn</contact>
</header>
"""

import basf2
from ROOT import Belle2
from simulation import add_simulation
from reconstruction import add_reconstruction, add_mdst_output

# Set the global log level
basf2.set_log_level(basf2.LogLevel.INFO)

main = basf2.create_path()

main.add_module("EventInfoSetter", expList=0, runList=1, evtNumList=10000)

# generate events
main.add_module('KKGenInput',
                tauinputFile=Belle2.FileSystem.findFile('data/generators/kkmc/mu.input.dat'),
                KKdefaultFile=Belle2.FileSystem.findFile('data/generators/kkmc/KK2f_defaults.dat'),
                taudecaytableFile='',
                kkmcoutputfilename='kkmc_mumu.dat')

# detector simulation
add_simulation(main)

# reconstruction
add_reconstruction(main)

# Finally add mdst output
output_filename = "../TwoTrackMuonsForLuminosity.dst.root"
add_mdst_output(main, filename=output_filename)

# generate events
basf2.process(main)

# show call statistics
print(basf2.statistics)
