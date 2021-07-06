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

basf2.set_log_level(basf2.LogLevel.INFO)

# main path
main = basf2.create_path()

# event info setter
main.add_module("EventInfoSetter", expList=0, runList=1, evtNumList=100)

# beam parameters
beamparameters = add_beamparameters(main, "Y4S")

# to run the framework the used modules need to be registered
kkgeninput = basf2.register_module('KKGenInput')
kkgeninput.param('tauinputFile', basf2.find_file('data/generators/kkmc/mu.input.dat'))
kkgeninput.param('KKdefaultFile', basf2.find_file('data/generators/kkmc/KK2f_defaults.dat'))
kkgeninput.param('taudecaytableFile', '')
kkgeninput.param('kkmcoutputfilename', 'kkmc_mumu.txt')

# run
main.add_module("Progress")
main.add_module(kkgeninput)
main.add_module("RootOutput", outputFileName="kkmc_mumu.root")
main.add_module("PrintMCParticles", logLevel=basf2.LogLevel.INFO, onlyPrimaries=False)

# generate events
basf2.process(main)

# show call statistics
print(basf2.statistics)
