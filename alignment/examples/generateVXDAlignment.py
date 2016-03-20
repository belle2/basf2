#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import sys
from basf2 import *
from ROOT import Belle2

from alignment.utils import AlignmentGeneratorConfig

alignment = AlignmentGeneratorConfig()

# Generate "empty" alignment with zeros
alignment.set_all('0.0.0', 0.0)

"""
You can also generate alignment different from zero, e.g.
to generate uniform distribution from -100um to +100um
for u-shift of sensor 1 in all ladders of layer 1, do:

 alignment.gen_u_uniform('1.0.1', 0.01)
"""

main = create_path()
main.add_module('EventInfoSetter')
main.add_module('Gearbox')
main.add_module('Geometry', components=['PXD', 'SVD'])
main.add_module('AlignmentGenerator', payloadIov=[0, 0, -1, -1], payloadName="", data=alignment.get_data(), createPayload=True)
main.add_module('Progress')

process(main)
print(statistics)
