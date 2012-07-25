#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
from basf2 import *
logging.log_level = LogLevel.WARNING

# -------------------------
# here we register modules
# -------------------------

progress = register_module('Progress')
paramloader = register_module('Gearbox')
geometry = register_module('Geometry')
geometry.param('Components', ['TOP'])
input = register_module('SimpleInput')
topdigi = register_module('TOPDigi')
param_digi = {'PhotonFraction': 0.3}
topdigi.param(param_digi)
topback = register_module('TOPBackground')

param_back = {'TimeOfSimulation': 200.0, 'Type': 'Touschek_HER',
              'Output': 'Touschek_HER.root'}
# TimeOfSimulation in us
# Type the backgound type just for labeing

topback.param(param_back)
# --------------------------------------
# here we set the parameters of modules
# --------------------------------------

input.param('inputFileName',
            '/gpfs/home/belle/nakayama/basf2_opt/release_201207_MCgen/Work_MCgen/hadd/Touschek_HER.root'
            )

# create path
main = create_path()

# add modules to path

main.add_module(input)
main.add_module(progress)
main.add_module(paramloader)
main.add_module(geometry)
main.add_module(topdigi)
main.add_module(topback)

process(main)

print statistics
