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

# suppress messages and warnings during processing:
basf2.set_log_level(basf2.LogLevel.INFO)

# main path
main = basf2.create_path()

# event info setter
main.add_module("EventInfoSetter", expList=0, runList=1, evtNumList=100)

# beam parameters
beamparameters = add_beamparameters(main, "Y1S")
# beamparameters.param("generateCMS", True)
# beamparameters.param("smearVertex", False)
# beamparameters.param("smearEnergy", False)
# print_params(beamparameters)

# EvtGen
evtgen = basf2.register_module('EvtGenInput')
# parent particle name from evt.pdl: Upsilon = Y(1S)
evtgen.param('ParentParticle', "Upsilon")
evtgen.set_log_level(basf2.LogLevel.INFO)

# run
main.add_module("Progress")
main.add_module("Gearbox")
main.add_module(evtgen)
main.add_module("RootOutput", outputFileName="evtgen_upsilon1s.root")
main.add_module("PrintMCParticles", logLevel=basf2.LogLevel.DEBUG, onlyPrimaries=False)

# generate events
basf2.process(main)

# show call statistics
print(basf2.statistics)
