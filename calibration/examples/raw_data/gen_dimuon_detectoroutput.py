#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
from simulation import add_simulation
from reconstruction import add_reconstruction
from reconstruction import add_mdst_output
from beamparameters import add_beamparameters
from rawdata import add_packers, add_raw_output, add_raw_seqoutput
from ROOT import Belle2
from modularAnalysis import printMCParticles, printDataStore

set_random_seed(5433)

use_central_database('production')

# Show the Trigger cuts and their properties that are in the database
# from softwaretrigger import db_access
#
# B2RESULT("Currently, there are the following cuts in the global condition database:")
#  for base_identifier, cut_identifier in db_access.get_all_cuts_in_database():
#     B2RESULT(base_identifier + " " + cut_identifier)
#     cut = db_access.download_cut_from_db(base_identifier, cut_identifier, False)
#     B2RESULT("Cut condition: " + cut.decompile())
#     B2RESULT("Cut is a reject cut: " + str(cut.isRejectCut()))

main = create_path()

# specify number of events to be generated
eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param('evtNumList', [100])
eventinfosetter.param('runList', [1])
eventinfosetter.param('expList', [1])
main.add_module(eventinfosetter)

# set the BeamParameters for running at Y(4S)
beamparameters = add_beamparameters(main, "Y4S")

kkgeninput = register_module('KKGenInput')
kkgeninput.param('tauinputFile', Belle2.FileSystem.findFile('data/generators/kkmc/mu.input.dat'))
kkgeninput.param('KKdefaultFile', Belle2.FileSystem.findFile('data/generators/kkmc/KK2f_defaults.dat'))
kkgeninput.param('taudecaytableFile', '')
kkgeninput.param('kkmcoutputfilename', 'kkmc_mumu.txt')
main.add_module(kkgeninput)

# Print out list of generated particles to check decay is correct
# printMCParticles(path=main)

# detector simulation,  digitisers and clusterisers
add_simulation(main)

# reconstruction for HLT
add_reconstruction(main)

# pack raw data
add_packers(main)

# Check what is currently in the datastore before pruning for output
# printDataStore(path=main)

# SROOT raw data output

# If you want MC Truth Relations later
add_raw_seqoutput(main, 'raw.sroot', additionalObjects=['SoftwareTriggerResult', 'MCParticles'])
# add_raw_seqoutput(main, 'raw.sroot', additionalObjects=['SoftwareTriggerResult'])

# Go!
process(main)

# Print call statistics
print(statistics)
