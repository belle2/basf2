#!/usr/bin/env python
# -*- coding: utf-8 -*-

import basf2
from ROOT import Belle2

import beamparameters
import simulation

main = basf2.create_path()

main.add_module("EventInfoSetter")
# beamparameters.add_beamparameters(main, "Y4S")
main.add_module('Gearbox')
main.add_module('Geometry')
main.add_module('EvtGenInput')
simulation.add_simulation(main)
main.add_module("RootOutput")
main.add_module("Progress")

basf2.process(main)
print(basf2.statistics)
