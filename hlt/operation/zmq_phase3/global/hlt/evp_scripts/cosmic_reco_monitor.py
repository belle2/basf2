#!/usr/bin/env python3
# -*- coding: utf-8 -*-
import basf2
import hbasf2
from softwaretrigger import constants
from softwaretrigger.processing import setup_basf2_and_db, start_zmq_path, finalize_zmq_path, add_hlt_processing

args = setup_basf2_and_db(zmq=True)

path, reco_path = start_zmq_path(args, location=constants.Location.hlt)
add_hlt_processing(reco_path, run_type=constants.RunTypes.cosmic,
                   softwaretrigger_mode=constants.SoftwareTriggerModes.monitor,
                   data_taking_period="early_phase3")
finalize_zmq_path(path, args, location=constants.Location.hlt)

basf2.print_path(path)
hbasf2.process(path, [args.dqm, args.output], True)
