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
import modularAnalysis as ma

b2.set_random_seed(12345)
b2.set_log_level(b2.LogLevel.INFO)

# identifier for this run
tag = "112_261"

# main path
main = b2.create_path()

# event info setter
main.add_module("EventInfoSetter", expList=0, runList=1, evtNumList=100000)

# beam parameters
beamparameters = add_beamparameters(main, "Y4S")
# beamparameters.param("generateCMS", True)
# beamparameters.param("smearVertex", False)

# to run the framework the used modules need to be registered
kkgeninput = b2.register_module('KKGenInput')
kkgeninput.param('tauinputFile', b2.find_file(f'tauola_bbb.pigamma_{tag}.dat'))
kkgeninput.param('KKdefaultFile', b2.find_file('data/generators/kkmc/KK2f_defaults.dat'))
# kkgeninput.param('taudecaytableFile', b2.find_file('data/generators/kkmc/tau_decaytable.dat'))
# above line makes decay table to be read by Pythia; uncomment next line to make tau decay table to be read by Tauola
kkgeninput.param('taudecaytableFile', '')
kkgeninput.param('kkmcoutputfilename', f'txt/kkmc_tautau_bbb_{tag}.txt')

# run
main.add_module("Progress")
main.add_module(kkgeninput, logLevel=b2.LogLevel.INFO)
main.add_module("PrintMCParticles", logLevel=b2.LogLevel.INFO, onlyPrimaries=False, showMomenta=True, showStatus=True)
main.add_module("TauDecayMode")
variables = ['tauMinusMCMode', 'tauMinusEgstar', 'tauPlusMCMode', 'tauPlusEgstar']
ma.printVariableValues('', variables, main)
ma.variablesToNtuple(decayString='', filename=f'root/kkmc_tautau_bbb_variables_{tag}.root', variables=variables, path=main)
# ma.applyEventCuts('tauPlusEgstar > 0.01', path=main)

main.add_module("RootOutput", outputFileName=f'root/kkmc_tautau_bbb_{tag}.root')
# main.add_module("HepMCOutput", OutputFilename=f'kkmc_tautau_bbb_{tag}.hepmc', StoreVirtualParticles=True)

# generate events
b2.process(main)

# show call statistics
print(b2.statistics)
