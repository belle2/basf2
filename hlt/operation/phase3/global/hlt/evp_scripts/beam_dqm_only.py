#!/usr/bin/env python3
# -*- coding: utf-8 -*-
import basf2
from softwaretrigger import constants
from softwaretrigger.processing import setup_basf2_and_db, start_path, finalize_path, add_hlt_processing

args = setup_basf2_and_db()

path = start_path(args, location=constants.Location.hlt)
add_hlt_processing(path, run_type=constants.RunTypes.beam, do_reconstruction=False)
finalize_path(path, args, location=constants.Location.hlt)

basf2.print_path(path)
basf2.process(path)
