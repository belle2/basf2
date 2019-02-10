#!/usr/bin/env python3
# -*- coding: utf-8 -*-
from softwaretrigger.path_functions import add_hlt_dqm
from softwaretrigger.test_support import create_test_path, finalize_test_path

path, tempfolder = create_test_path(runtype="collision", expNum=1003, location="hlt")

add_hlt_dqm(path, run_type="collision", standalone=True)

finalize_test_path(path, tempfolder, has_softwaretriggerresult=False)
