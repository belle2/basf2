#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import basf2
import simulation

main = basf2.create_path()

main.add_module('EventInfoSetter')
main.add_module('Cosmics')
main.add_module('Gearbox')
main.add_module('Geometry', excludedComponents=['MagneticField'])

simulation.add_simulation(main)

main.add_module('RootOutput')
main.add_module('Progress')

basf2.process(main)
print(basf2.statistics)
