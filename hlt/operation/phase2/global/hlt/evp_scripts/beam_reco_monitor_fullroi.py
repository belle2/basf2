#!/usr/bin/env python3
# -*- coding: utf-8 -*-
import basf2
from softwaretrigger.path_functions import setup_basf2_and_db, create_hlt_path, add_hlt_processing, finalize_hlt_path

args = setup_basf2_and_db(dbfile="/dev/shm/LocalDB.rel0101/database.txt")
path = create_hlt_path(args)

# add software reconstruction and trigger for collisions and don't filter out events with the HLT
# furhtermore, the ROIs for the full sensors will be generated
add_hlt_processing(path, run_type="collision", softwaretrigger_mode="monitoring", roi_take_fullframe=True)

finalize_hlt_path(path, args)
basf2.print_path(path)
basf2.process(path)
