#!/usr/bin/env python3

import basf2
from basf2 import conditions
from basf2 import create_path, register_module
from analysisDQM import get_hadB_path
import sys


conditions.prepend_globaltag("online")
conditions.prepend_globaltag("data_prompt_rel08")
conditions.prepend_globaltag("AIRFLOW_data_staging_calibration_prompt")


rec_path_1 = create_path()

# baseDir = "/gpfs/group/belle2/dataprod/Data/release-08-01-08/DB00003234/bucket37"
# fName = baseDir+"/e0030/4S/r02311/skim/hadron/mdst/sub00/mdst.physics.0030.02311.HLT11.hlt_hadron.f00002.root"
fName = sys.argv[1]
rec_path_1.add_module('RootInput', inputFileName=fName)

get_hadB_path(rec_path_1)

histo = register_module("HistoManager")
rec_path_1.add_module(histo)
histo.param('histoFileName', sys.argv[2])

basf2.process(rec_path_1)
