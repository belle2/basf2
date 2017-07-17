#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
import reconstruction

main = create_path()
main.add_module('RootInput')
main.add_module('Gearbox')
main.add_module('Geometry', excludedComponents=['MagneticField'])

reconstruction.add_cosmics_reconstruction(main, pruneTracks=False)

main.add_module('RootOutput')
main.add_module('Progress')

process(main)
print(statistics)
