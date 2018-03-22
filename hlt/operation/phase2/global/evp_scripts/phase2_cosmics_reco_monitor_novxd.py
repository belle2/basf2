#!/usr/bin/env python3
# -*- coding: utf-8 -*-
import basf2
from softwaretrigger.path_functions import setup_basf2_and_db, create_hlt_path, add_hlt_processing, finalize_hlt_path
from softwaretrigger.path_functions import DEFAULT_HLT_COMPONENTS


args = setup_basf2_and_db()
path = create_hlt_path(args)

reco_components = DEFAULT_HLT_COMPONENTS
reco_components.remove("SVD")

# no reconstruction or software trigger added at all
add_hlt_processing(path, run_type="cosmics", softwaretrigger_mode="monitoring",
                   reco_components=reco_components,
                   data_taking_period="phase2")

finalize_hlt_path(path, args)
basf2.print_path(path)
basf2.process(path)
