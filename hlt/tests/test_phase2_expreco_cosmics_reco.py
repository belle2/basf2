#!/usr/bin/env python3
# -*- coding: utf-8 -*-
from softwaretrigger.path_functions import add_expressreco_processing
from softwaretrigger.test_support import create_test_path, finalize_test_path

path, tempfolder = create_test_path(runtype="cosmics", expNum=1002, location="expressreco")

# no reconstruction or software trigger added at all
add_expressreco_processing(path, run_type="cosmics", clean_wrapped_path=True)

# also has softwaretriggerresult, because the ST modules
# are always added, even in the default reco
finalize_test_path(path, tempfolder, has_softwaretriggerresult=False)
