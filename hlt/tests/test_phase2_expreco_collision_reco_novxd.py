#!/usr/bin/env python3
# -*- coding: utf-8 -*-
from softwaretrigger.path_functions import add_expressreco_processing, DEFAULT_EXPRESSRECO_COMPONENTS
from softwaretrigger.test_support import create_test_path, finalize_test_path

path, tempfolder = create_test_path(runtype="collision", expNum=1002)

reco_components = DEFAULT_EXPRESSRECO_COMPONENTS
reco_components.remove("PXD")
reco_components.remove("SVD")

# no reconstruction or software trigger added at all
add_expressreco_processing(path, run_type="collision", reco_components=reco_components)

finalize_test_path(path, tempfolder, has_softwaretriggerresult=False)
