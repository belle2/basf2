#!/usr/bin/env python3
# -*- coding: utf-8 -*-
import basf2
from softwaretrigger.path_functions import setup_basf2_and_db, finalize_hlt_path, create_hlt_path
from softwaretrigger.path_functions import add_pxd_fullframe_phase2, add_roi_payload_assembler

localdb = '/dev/shm/LocalDB.rel0101/database.txt'
args = setup_basf2_and_db(dbfile=localdb)
path = create_hlt_path(args)

add_pxd_fullframe_phase2(path)
add_roi_payload_assembler(path)

finalize_hlt_path(path, args, show_progress_bar=True)

basf2.print_path(path)
basf2.process(path)
