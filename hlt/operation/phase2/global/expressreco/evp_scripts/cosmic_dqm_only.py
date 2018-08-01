#!/usr/bin/env python3
# -*- coding: utf-8 -*-
import basf2
from softwaretrigger.path_functions import setup_basf2_and_db
from softwaretrigger.path_functions import create_expressreco_path, add_expressreco_processing, finalize_expressreco_path

db_on_ram = "/dev/shm/LocalDB.rel0101/database.txt"
args = setup_basf2_and_db(dbfile=db_on_ram)
path = create_expressreco_path(args)

add_expressreco_processing(path, run_type="cosmics", do_reconstruction=False,
                           data_taking_period="phase2")

finalize_expressreco_path(path, args, show_progress_bar=True)
basf2.print_path(path)
basf2.process(path)
