#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################
import basf2
import hbasf2
from softwaretrigger import constants
from softwaretrigger.processing import setup_basf2_and_db, start_zmq_path, finalize_zmq_path, add_hlt_processing


args = setup_basf2_and_db(event_distribution_mode=constants.EventDistributionModes.zmq)

path, reco_path = start_zmq_path(args, location=constants.Location.hlt,
                                 event_distribution_mode=constants.EventDistributionModes.zmq)
add_hlt_processing(reco_path, run_type=constants.RunTypes.cosmic, do_reconstruction=False)
finalize_zmq_path(path, args, location=constants.Location.hlt)

basf2.print_path(path)
hbasf2.process(path, [args.dqm, args.output], True)
