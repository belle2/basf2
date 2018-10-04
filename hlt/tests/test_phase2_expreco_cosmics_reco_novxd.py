#!/usr/bin/env python3
# -*- coding: utf-8 -*-
from softwaretrigger.path_functions import add_expressreco_processing, DEFAULT_EXPRESSRECO_COMPONENTS
from softwaretrigger.test_support import create_test_path, finalize_test_path

path, tempfolder = create_test_path(runtype="cosmics", expNum=1002, location="expressreco")

reco_components = DEFAULT_EXPRESSRECO_COMPONENTS
reco_components.remove("PXD")
reco_components.remove("SVD")
add_expressreco_processing(path, run_type="cosmics", reco_components=reco_components)

finalize_test_path(path, tempfolder, has_softwaretriggerresult=False)
