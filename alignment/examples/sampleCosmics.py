#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import basf2
import simulation

main = basf2.create_path()

main.add_module('EventInfoSetter')
main.add_module('Cosmics')
main.add_module('Gearbox')
main.add_module('Geometry')

simulation.add_simulation(main)

main.add_module('RootOutput')
main.add_module('Progress')

basf2.process(main)
print(basf2.statistics)
