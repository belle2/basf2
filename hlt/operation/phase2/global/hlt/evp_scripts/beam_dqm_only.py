#!/usr/bin/env python3
# -*- coding: utf-8 -*-
import basf2
from softwaretrigger.path_functions import setup_basf2_and_db, add_hlt_dqm, create_hlt_path, finalize_hlt_path

args = setup_basf2_and_db(dbfile="/dev/shm/LocalDB.rel0101/database.txt")
path = create_hlt_path(args)

# no reconstruction or software trigger added at all
add_hlt_dqm(path, run_type="collision", standalone=True)

finalize_hlt_path(path, args)
basf2.print_path(path)
basf2.process(path)
