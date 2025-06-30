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
from sys import argv


args_list = [
    "--input", argv[1],
    "--output", argv[2],
    "--dqm", argv[3],
    "--udp-hostname", argv[4],
    "--udp-port", argv[5],
]
args = setup_basf2_and_db(zmq=True, args_list=args_list)

path, reco_path = start_zmq_path(args, location=constants.Location.expressreco)
add_expressreco_processing(reco_path, run_type=constants.RunTypes.beam, do_reconstruction=False)
finalize_zmq_path(path, args, location=constants.Location.expressreco)

basf2.print_path(path)
basf2.process(path)
