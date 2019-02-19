#!/usr/bin/env python3
# -*- coding: utf-8 -*-
import basf2
from softwaretrigger.path_functions import setup_basf2_and_db, create_hlt_path, add_hlt_processing, finalize_hlt_path

# set basf2 settings and configure local DB access
args = setup_basf2_and_db(dbfile="/dev/shm/LocalDB.rel0101/database.txt")

# create the HLT path and add the Ring Buffer input and DQM modules
path = create_hlt_path(args)

# add software reconstruction and trigger for collisions and filter out events with the HLT
add_hlt_processing(path, run_type="collision", softwaretrigger_mode="hlt_filter")

finalize_hlt_path(path, args)
basf2.print_path(path)
basf2.process(path)
