#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
import basf2 as b2
from cdc import cr

# The backend creates a file with the input data file paths inside it and places it in the job's working directory.
# This function reads the file automatically to get a list of input file paths
from caf.backends import get_input_data

probcut = float(sys.argv[1]) if len(sys.argv) == 2 else 0.001
data_period = 'phase2'

b2.reset_database()
b2.use_database_chain()
b2.use_central_database("332_COPY-OF_GT_gen_prod_004.11_Master-20171213-230000", b2.LogLevel.INFO)
b2.use_central_database("MagneticFieldPhase2QCSoff")
b2.use_local_database("/home/belle/muchida/basf2/release/cdc/examples/caf/localDB/database.txt")
# use_local_database("/home/belle/muchida/basf2/work/caf/gcr2/test7/localDB/database.txt")

input_data = get_input_data()
output_file_name = 'CollectorOutput.root'

# Compose basf2 module path #
#############################
main_path = b2.create_path()
main_path.add_module('RootInput',
                     # entrySequences=['0:10'],
                     entrySequences=['0:2000'],
                     inputFileNames=input_data)

main_path.add_module('HistoManager', histoFileName=output_file_name)

main_path.add_module('Gearbox')
main_path.add_module('Geometry', useDB=True)
main_path.add_module('CDCUnpacker')

cr.set_cdc_cr_parameters(data_period)
cr.add_cdc_cr_reconstruction(main_path,
                             pval2ndTrial=probcut)

main_path.add_module('CDCCalibrationCollector',
                     granularity="all",
                     EventT0Extraction=True,
                     BField=True,
                     calExpectedDriftTime=True
                     )
b2.print_path(main_path)
b2.process(main_path)
print(b2.statistics)
