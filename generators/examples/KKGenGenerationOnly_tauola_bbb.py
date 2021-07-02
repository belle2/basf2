#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

from basf2 import *
from beamparameters import add_beamparameters

set_random_seed(12345)
set_log_level(LogLevel.INFO)

# main path
main = create_path()

# event info setter
main.add_module("EventInfoSetter", expList=0, runList=1, evtNumList=1000000)

# beam parameters
beamparameters = add_beamparameters(main, "Y4S")
# beamparameters.param("generateCMS", True)
# beamparameters.param("smearVertex", False)

# to run the framework the used modules need to be registered
kkgeninput = register_module('KKGenInput')
kkgeninput.param('tauinputFile', find_file('data/generators/kkmc/tauola_bbb.input.dat'))
kkgeninput.param('KKdefaultFile', find_file('data/generators/kkmc/KK2f_defaults.dat'))
# kkgeninput.param('taudecaytableFile', find_file('data/generators/kkmc/tau_decaytable.dat'))
# above line makes decay table to be read by Pythia; uncomment next line to make tau decay table to be read by Tauola
kkgeninput.param('taudecaytableFile', '')
kkgeninput.param('kkmcoutputfilename', 'kkmc_tautau_bbb.txt')

# run
main.add_module("Progress")
main.add_module(kkgeninput)
main.add_module("RootOutput", outputFileName="kkmc_tautau_bbb.root")
main.add_module("HepMCOutput", OutputFilename='kkmc_tautau_bbb.hepmc', StoreVirtualParticles=True)
# main.add_module("PrintTauTauMCParticles", logLevel=LogLevel.INFO, onlyPrimaries=False)
# main.add_module("PrintMCParticles", logLevel=LogLevel.INFO, onlyPrimaries=False)

# generate events
process(main)

# show call statistics
print(statistics)
