#!/usr/bin/env python3
# -*- coding: utf-8 -*-
import basf2
from softwaretrigger import constants
from softwaretrigger.processing import setup_basf2_and_db, start_path, finalize_path
from pxd import add_pxd_percentframe_phase3_early, add_roi_payload_assembler

args = setup_basf2_and_db()

path = start_path(args, location=constants.Location.hlt)
add_pxd_percentframe_phase3_early(path, fraction=0.1, random_position=True)
add_roi_payload_assembler(path, ignore_hlt_decision=True)
finalize_path(path, args, location=constants.Location.hlt)

basf2.print_path(path)
basf2.process(path)
