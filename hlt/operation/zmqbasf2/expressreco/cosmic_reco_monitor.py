##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################
import basf2
from softwaretrigger import constants
from softwaretrigger.processing import setup_basf2_and_db, start_zmq_path, finalize_zmq_path, add_expressreco_processing


args = setup_basf2_and_db(event_distribution_mode=constants.EventDistributionModes.zmqbasf2)

path, _reco_path = start_zmq_path(args, location=constants.Location.expressreco,
                                  event_distribution_mode=constants.EventDistributionModes.zmqbasf2)
add_expressreco_processing(path, run_type=constants.RunTypes.cosmic)
finalize_zmq_path(path, args, location=constants.Location.expressreco)

basf2.print_path(path)
basf2.process(path)
