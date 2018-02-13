#!/usr/bin/env python3
# -*- coding: utf-8 -*-
import basf2
from softwaretrigger.path_functions import setup_basf2_and_db, create_hlt_path, add_hlt_processing, finalize_hlt_path


setup_basf2_and_db()
path = create_hlt_path()

# add software reconstruction and trigger for collisions and filter out events with the HLT
add_hlt_processing(path, run_type="collison", softwaretrigger_mode="monitor")

finalize_hlt_path(path)
basf2.print_path(path)
basf2.process(path)
