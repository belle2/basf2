#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import basf2 as b2
import pdg as pdg
from beamparameters import add_beamparameters

b2.set_random_seed(12345)
b2.set_log_level(b2.LogLevel.INFO)

# create alpha particle
mass = 1.0  # in GeV
width = 0.004  # in GeV
pdg.add_particle('alpha', 94144, mass, width, 0, 0)

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
kkgeninput.param('tauinputFile', b2.find_file('tauola_bbb.lepton_alpha-mu_mu.dat'))
kkgeninput.param('KKdefaultFile', b2.find_file('data/generators/kkmc/KK2f_defaults.dat'))
# kkgeninput.param('taudecaytableFile', b2.find_file('data/generators/kkmc/tau_decaytable.dat'))
# above line makes decay table to be read by Pythia; uncomment next line to make tau decay table to be read by Tauola
kkgeninput.param('taudecaytableFile', '')
kkgeninput.param('kkmcoutputfilename', 'kkmc_tautau_bbb.txt')

# run
main.add_module("Progress")
main.add_module(kkgeninput, logLevel=b2.LogLevel.INFO)
main.add_module("RootOutput", outputFileName="kkmc_tautau_bbb.root")
main.add_module("HepMCOutput", OutputFilename='kkmc_tautau_bbb.hepmc', StoreVirtualParticles=True)
# main.add_module("PrintTauTauMCParticles", logLevel=LogLevel.INFO, onlyPrimaries=False)
main.add_module("PrintMCParticles", logLevel=b2.LogLevel.INFO, onlyPrimaries=False, showStatus=True)
main.add_module(
    "TauDecayMode",
    printmode="all",
    file_minus=b2.find_file('data/analysis/modules/TauDecayMode/map_tauminus.txt'),
    file_plus=b2.find_file('data/analysis/modules/TauDecayMode/map_tauplus.txt'))


# generate events
b2.process(main, calculateStatistics=True)

# show call statistics
print(b2.statistics)
