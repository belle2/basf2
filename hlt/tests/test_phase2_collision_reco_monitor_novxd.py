#!/usr/bin/env python3
# -*- coding: utf-8 -*-
from softwaretrigger.path_functions import add_hlt_processing, DEFAULT_HLT_COMPONENTS
from softwaretrigger.test_support import create_test_path, finalize_test_path

path, tempfolder = create_test_path(runtype="collision")

reco_components = DEFAULT_HLT_COMPONENTS
reco_components.remove("SVD")

# no reconstruction or software trigger added at all
add_hlt_processing(path, run_type="collision", softwaretrigger_mode="monitoring", reco_components=reco_components)

finalize_test_path(path, tempfolder)
