#!/usr/bin/env python3
# -*- coding: utf-8 -*-
import basf2
from softwaretrigger.path_functions import setup_basf2_and_db, create_hlt_path, add_expressreco_processing,
from softwaretrigger.path_functions import finalize_hlt_path, DEFAULT_EXPRESSRECO_COMPONENTS

args = setup_basf2_and_db()
path = create_hlt_path(args)

# no reconstruction for PXD and SVD for early global cosmic runs option
reco_components = DEFAULT_EXPRESSRECO_COMPONENTS
reco_components.remove("PXD")
reco_components.remove("SVD")

add_expressreco_processing(path, run_type="collision", do_reconstruction=True, reco_components=reco_components)

finalize_hlt_path(path, args)
basf2.print_path(path)
basf2.process(path)
