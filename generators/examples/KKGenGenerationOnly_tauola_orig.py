#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import basf2 as b2
from beamparameters import add_beamparameters

b2.set_random_seed(12345)
b2.set_log_level(b2.LogLevel.INFO)

# main path
main = b2.create_path()

# event info setter
main.add_module("EventInfoSetter", expList=0, runList=1, evtNumList=1000000)

# beam parameters
beamparameters = add_beamparameters(main, "Y4S")
# beamparameters.param("generateCMS", True)
# beamparameters.param("smearVertex", False)

# to run the framework the used modules need to be registered
kkgeninput = b2.register_module('KKGenInput')
kkgeninput.param('tauinputFile', b2.find_file('data/generators/kkmc/tauola_orig.input.dat'))
kkgeninput.param('KKdefaultFile', b2.find_file('data/generators/kkmc/KK2f_defaults.dat'))
# kkgeninput.param('taudecaytableFile', b2.find_file('data/generators/kkmc/tau_decaytable.dat'))
# above line makes decay table to be read by Pythia; uncomment next line to make tau decay table to be read by Tauola
kkgeninput.param('taudecaytableFile', '')
kkgeninput.param('kkmcoutputfilename', 'kkmc_tautau_orig.txt')

# run
main.add_module("Progress")
main.add_module(kkgeninput)
main.add_module("RootOutput", outputFileName="kkmc_tautau_orig.root")
main.add_module("HepMCOutput", OutputFilename='kkmc_tautau_orig.hepmc', StoreVirtualParticles=True)
# main.add_module("PrintTauTauMCParticles", logLevel=LogLevel.INFO, onlyPrimaries=False)
# main.add_module("PrintMCParticles", logLevel=LogLevel.INFO, onlyPrimaries=False)

# generate events
b2.process(main)

# show call statistics
print(b2.statistics)
