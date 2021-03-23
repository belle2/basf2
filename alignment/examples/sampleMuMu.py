#!/usr/bin/env python
# -*- coding: utf-8 -*-

import basf2

# from beamparameters import add_beamparameters
from generators import add_kkmc_generator
from simulation import add_simulation
from L1trigger import add_tsim

main = basf2.create_path()

main.add_module("EventInfoSetter")
# add_beamparameters(main, "Y4S")
main.add_module('Gearbox')
main.add_module('Geometry')

add_kkmc_generator(main, 'mu-mu+')

add_simulation(main)
add_tsim(main)

main.add_module("RootOutput")
main.add_module("Progress")

basf2.process(main)
print(basf2.statistics)
