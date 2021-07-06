#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import basf2 as b2
b2.logging.log_level = b2.LogLevel.WARNING

# -------------------------
# here we register modules
# -------------------------

progress = b2.register_module('Progress')
paramloader = b2.register_module('Gearbox')
geometry = b2.register_module('Geometry')
geometry.param('Components', ['TOP'])
input = b2.register_module('RootInput')
topdigi = b2.register_module('TOPDigitizer')
topback = b2.register_module('TOPBackground')

param_back = {'TimeOfSimulation': 10.0, 'Type': 'Touschek_HER',
              'Output': 'Background_Touschek_HER.root'}
# TimeOfSimulation in us
# Type the backgound type just for labeing

topback.param(param_back)
# --------------------------------------
# here we set the parameters of modules
# --------------------------------------

input.param('inputFileName', '/home/belle/petric/Touschek_HER.root')

# create path
main = b2.create_path()

# add modules to path

main.add_module(input)
main.add_module(progress)
main.add_module(paramloader)
main.add_module(geometry)
main.add_module(topdigi)
main.add_module(topback)

b2.process(main)

print(b2.statistics)
