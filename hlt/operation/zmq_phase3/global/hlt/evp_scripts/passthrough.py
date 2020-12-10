#!/usr/bin/env python3
# -*- coding: utf-8 -*-
import basf2
import hbasf2
from softwaretrigger import constants
from softwaretrigger.processing import finalize_zmq_path, setup_basf2_and_db, start_zmq_path
from pxd import add_pxd_percentframe_phase3_early, add_roi_payload_assembler

args = setup_basf2_and_db(zmq=True)

path, reco_path = start_zmq_path(args, location=constants.Location.hlt)
add_pxd_percentframe_phase3_early(reco_path, fraction=0.1, random_position=True)
add_roi_payload_assembler(reco_path, ignore_hlt_decision=True)
finalize_zmq_path(path, args, location=constants.Location.hlt)

basf2.print_path(path)
hbasf2.process(path, [args.dqm, args.output], True)
