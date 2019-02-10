#!/usr/bin/env python3
# -*- coding: utf-8 -*-
from softwaretrigger.path_functions import add_hlt_dqm
from softwaretrigger.path_functions import add_pxd_fullframe_phase3_early, add_roi_payload_assembler
from softwaretrigger.test_support import create_test_path, finalize_test_path

path, tempfolder = create_test_path(runtype="cosmics", expNum=1003, location="hlt")

add_hlt_dqm(path, run_type="cosmics", standalone=True)
add_pxd_fullframe_phase3_early(path)
add_roi_payload_assembler(path)

finalize_test_path(path, tempfolder, has_softwaretriggerresult=False)
