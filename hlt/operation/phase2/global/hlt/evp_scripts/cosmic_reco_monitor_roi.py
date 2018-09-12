#!/usr/bin/env python3
# -*- coding: utf-8 -*-
import basf2
from softwaretrigger.path_functions import setup_basf2_and_db, create_hlt_path, add_hlt_processing, finalize_hlt_path


args = setup_basf2_and_db(dbfile="/dev/shm/LocalDB.rel0101/database.txt")
path = create_hlt_path(args)

# do cosmics reconstruction and use the Roi finder
# to generate Rois from actual SVD and CDC tracks
add_hlt_processing(path, run_type="cosmics", softwaretrigger_mode="monitoring", data_taking_period="phase2",
                   roi_take_fullframe=False)

finalize_hlt_path(path, args)
basf2.print_path(path)
basf2.process(path)
