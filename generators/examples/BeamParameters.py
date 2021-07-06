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
from beamparameters import add_beamparameters

# small example to show how to set BeamParameters

main = basf2.create_path()
main.add_module("EventInfoSetter", expList=0, runList=1, evtNumList=10000)

# add beamparameters with named preset
beamparameters = add_beamparameters(main, "Y4S")

# override preset however you like
beamparameters.param("covHER", [0.01, 0.01, 0.01])
beamparameters.param("smearDirection", False)

# print parameters to be used
basf2.print_params(beamparameters)

# add generator, simulation, analysis, and so forth
main.add_module("RootOutput", outputFileName="BeamParameters.root")

basf2.process(main)
