#!/usr/bin/env python3
# -*- coding: utf-8 -*-
from softwaretrigger.path_functions import add_hlt_processing
from softwaretrigger.test_support import create_test_path, finalize_test_path

path, tempfolder = create_test_path(runtype="collision")

# no reconstruction or software trigger added at all
add_hlt_processing(path, run_type="collision", softwaretrigger_mode="filter")

finalize_test_path(path, tempfolder)
