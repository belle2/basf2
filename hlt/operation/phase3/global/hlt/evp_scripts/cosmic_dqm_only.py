#!/usr/bin/env python3
# -*- coding: utf-8 -*-
import basf2
from softwaretrigger.path_functions import setup_basf2_and_db, add_hlt_dqm, create_hlt_path, finalize_hlt_path
from softwaretrigger.path_functions import add_pxd_fullframe_phase3_early, add_roi_payload_assembler

# set basf2 settings and configure local DB access
args = setup_basf2_and_db(dbfile="/dev/shm/LocalDB.rel0101/database.txt")

# create the HLT path and add the Ring Buffer input and DQM modules
path = create_hlt_path(args)

# no reconstruction or software trigger added at all
add_hlt_dqm(path, run_type="cosmics", standalone=True)
add_roi_payload_assembler(path)

finalize_hlt_path(path, args)
basf2.print_path(path)
basf2.process(path)
