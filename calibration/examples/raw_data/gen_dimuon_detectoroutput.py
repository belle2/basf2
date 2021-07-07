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
from generators import add_kkmc_generator
from simulation import add_simulation
from reconstruction import add_reconstruction
from beamparameters import add_beamparameters
from rawdata import add_packers, add_raw_seqoutput

b2.set_random_seed(5433)

b2.use_central_database('production')

# Show the Trigger cuts and their properties that are in the database
# from softwaretrigger import db_access
#
# B2RESULT("Currently, there are the following cuts in the global condition database:")
#  for base_identifier, cut_identifier in db_access.get_all_cuts_in_database():
#     B2RESULT(base_identifier + " " + cut_identifier)
#     cut = db_access.download_cut_from_db(base_identifier, cut_identifier, False)
#     B2RESULT("Cut condition: " + cut.decompile())
#     B2RESULT("Cut is a reject cut: " + str(cut.isRejectCut()))

main = b2.create_path()

# specify number of events to be generated
eventinfosetter = b2.register_module('EventInfoSetter')
eventinfosetter.param('evtNumList', [100])
eventinfosetter.param('runList', [1])
eventinfosetter.param('expList', [1])
main.add_module(eventinfosetter)

# set the BeamParameters for running at Y(4S)
beamparameters = add_beamparameters(main, "Y4S")

add_kkmc_generator(main, 'mu-mu+')

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
b2.process(main)

# Print call statistics
print(b2.statistics)
