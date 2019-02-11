#!/usr/bin/env python3
# -*- coding: utf-8 -*-
import basf2
from softwaretrigger.path_functions import setup_basf2_and_db
from softwaretrigger.path_functions import create_expressreco_path, add_expressreco_processing, finalize_expressreco_path

# set basf2 settings and configure local DB access
args = setup_basf2_and_db(dbfile="/dev/shm/LocalDB.rel0101/database.txt")

# create the expressreco path and add the Ring Buffer input and DQM modules
path = create_expressreco_path(args)

# add expressreco without reconstruction
add_expressreco_processing(path, run_type="cosmics", do_reconstruction=False)

finalize_expressreco_path(path, args, show_progress_bar=True)
basf2.print_path(path)
basf2.process(path)
