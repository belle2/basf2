#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import basf2
import ROOT
import datetime
import os
import sys
import subprocess
from basf2 import *
from ROOT import Belle2
from tracking import add_cdc_cr_track_finding
from caf.framework import Calibration, CAF
from caf import backends
from cdc.cr import *

probcut = float(sys.argv[1]) if len(sys.argv) == 2 else 0.001
data_period = 'phase2'

reset_database()
use_database_chain()
use_central_database("332_COPY-OF_GT_gen_prod_004.11_Master-20171213-230000", LogLevel.INFO)
use_central_database("MagneticFieldPhase2QCSoff")
use_local_database("localDB/database.txt", "localDB")
# use_local_database("/home/belle/muchida/basf2/work/caf/gcr2/test7/localDB/database.txt")


# The backend creates a file with the input data file paths inside it and places it in the job's working directory.
# This function reads the file automatically to gte a list of input file paths
from caf.backends import get_input_data

input_data = get_input_data()
output_file_name = 'CollectorOutput.root'

# Compose basf2 module path #
#############################
main_path = basf2.create_path()
main_path.add_module('RootInput',
                     #                   entrySequences=['0:10'],
                     #                   entrySequences=['0:2000'],
                     inputFileNames=input_data)

main_path.add_module('HistoManager', histoFileName=output_file_name)

main_path.add_module('Gearbox')
main_path.add_module('Geometry', useDB=True)
main_path.add_module('CDCUnpacker')

set_cdc_cr_parameters(data_period)
add_cdc_cr_reconstruction(main_path,
                          pval2ndTrial=probcut)

main_path.add_module('CDCCalibrationCollector',
                     granularity="all",
                     EventT0Extraction=True,
                     BField=True,
                     calExpectedDriftTime=True
                     )
basf2.print_path(main_path)
basf2.process(main_path)
print(statistics)
