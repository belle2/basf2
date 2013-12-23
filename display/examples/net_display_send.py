#!/usr/bin/env python
# -*- coding: utf-8 -*-


import os
import random
from basf2 import *


eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param('evtNumList', [5000])


# create geometry
gearbox = register_module('Gearbox')
geometry = register_module('Geometry')
geometry.param('components', ['PXD', 'SVD', 'CDC', 'MagneticField'])

particlegun = register_module('ParticleGun')


# simulation
g4sim = register_module('FullSim')
# make the simulation less noisy
g4sim.logging.log_level = LogLevel.ERROR

tx = register_module('TxSocket')
tx.param('DestHostName', 'localhost')
#tx.param('DestPort', 1111)

# create paths
main = create_path()

# add modules to paths
main.add_module(eventinfosetter)
main.add_module(register_module('Progress'))

main.add_module(gearbox)
main.add_module(geometry)
main.add_module(particlegun)
main.add_module(g4sim)

main.add_module(tx)

process(main)
print statistics
