#!/usr/bin/env python3
# -*- coding: utf-8 -*-
import basf2
from softwaretrigger import constants
from softwaretrigger.processing import setup_basf2_and_db, start_path, finalize_path, add_expressreco_processing

args = setup_basf2_and_db()

path = start_path(args, location=constants.Location.expressreco)
add_expressreco_processing(path, run_type=constants.RunTypes.beam, do_reconstruction=False)
finalize_path(path, args, location=constants.Location.expressreco)

basf2.print_path(path)
basf2.process(path)
