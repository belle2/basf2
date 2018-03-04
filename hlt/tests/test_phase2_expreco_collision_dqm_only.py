#!/usr/bin/env python3
# -*- coding: utf-8 -*-
from softwaretrigger.path_functions import add_expressreco_processing
from softwaretrigger.test_support import create_test_path, finalize_test_path

path, tempfolder = create_test_path(runtype="collision", expNum=1002, location="expressreco")

# no reconstruction or software trigger added at all
add_expressreco_processing(path, run_type="collision", do_reconstruction=False, clean_wrapped_path=True)

finalize_test_path(path, tempfolder, has_softwaretriggerresult=False)
