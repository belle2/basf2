#!/usr/bin/env python3
# -*- coding: utf-8 -*-
import basf2
from softwaretrigger.path_functions import setup_basf2_and_db, create_expressreco_path, add_expressreco_processing
from softwaretrigger.path_functions import finalize_expressreco_path, DEFAULT_EXPRESSRECO_COMPONENTS

db_on_ram = "/dev/shm/LocalDB.rel0101/database.txt"
args = setup_basf2_and_db(dbfile=db_on_ram)

path = create_expressreco_path(args)

# no reconstruction for PXD and SVD for early global cosmic runs option
reco_components = DEFAULT_EXPRESSRECO_COMPONENTS
reco_components.remove("PXD")
reco_components.remove("SVD")

add_expressreco_processing(
    path,
    run_type="cosmics",
    do_reconstruction=True,
    data_taking_period="phase2",
    reco_components=reco_components,
    make_crashsafe=False)

finalize_expressreco_path(path, args, show_progress_bar=True)
basf2.print_path(path)
basf2.process(path)
