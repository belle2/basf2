#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# ---------------------------------------------------------------------------------------
# CAF calibration script: module alignment
# data type: cdst (dimuon or bhabha)
#
# usage: basf2 run_alignment.py expNo runFirst runLast [sample]
#        sample = dimuon/bhabha (D = dimuon)
#
# Note: input data must be already well calibrated (including moduleT0 and commonT0)
#
# author: M. Staric
# ---------------------------------------------------------------------------------------

import basf2
import sys
import os
import glob
from caf import backends
from caf.framework import Calibration, CAF, Collection
from caf.strategies import SequentialRunByRun, SingleIOV, SimpleRunByRun
from ROOT import Belle2
from ROOT.Belle2 import TOP
from basf2 import B2ERROR

# ----- those parameters need to be adjusted before running -----------------------------
#
globalTag = 'data_reprocessing_prompt_rel4_patchb'
data_dir = '/group/belle2/dataprod/Data/release-03-02-02/DB00000654/proc9/'
dimuon_skim_dir = 'offskim/offskim_mumutop/cdst/sub00'
bhabha_skim_dir = 'skim/hlt_bhabha/cdst/sub00'
main_output_dir = 'top_calibration'
default_sample = 'dimuon'
fixedParameters = ['dn/n']  # to list all names: basf2 -m TOPAlignmentCollector
#
# ---------------------------------------------------------------------------------------

# Argument parsing
argvs = sys.argv
if len(argvs) < 4:
    print("usage: basf2", argvs[0], "experiment runFirst runLast [sample]")
    print("       sample = dimuon/bhabha (D = dimuon)")
    sys.exit()
experiment = int(argvs[1])
run_first = int(argvs[2])
run_last = int(argvs[3])
sample = default_sample

if len(argvs) > 4:
    sample = argvs[4]

if sample == 'dimuon':
    skim_dir = dimuon_skim_dir
elif sample == 'bhabha':
    skim_dir = bhabha_skim_dir
else:
    B2ERROR("Invalid sample name: " + sample)
    sys.exit()

# Make list of files
inputFiles = []
expNo = 'e' + '{:0=4d}'.format(experiment)
for run in range(run_first, run_last + 1):
    runNo = 'r' + '{:0=5d}'.format(run)
    filename = f"{data_dir}/{expNo}/*/{runNo}/{skim_dir}/cdst*.root"
    inputFiles += glob.glob(filename)

if len(inputFiles) == 0:
    B2ERROR('No cdst files found in ' + data_dir + ' for exp=' + str(experiment) +
            ' runFirst=' + str(run_first) + ' runLast=' + str(run_last) +
            ' (skim_dir=' + skim_dir + ')')
    sys.exit()

# Output folder
run_range = 'r' + '{:0=5d}'.format(run_first) + '-' + '{:0=5d}'.format(run_last)
output_dir = f"{main_output_dir}/alignment-{sample}-{expNo}-{run_range}"

# Temporary fix for BII-5431
if not os.path.isdir(main_output_dir):
    os.makedirs(main_output_dir)
    print('New folder created: ' + main_output_dir)

# Suppress messages during processing
# basf2.set_log_level(basf2.LogLevel.WARNING)


def module_alignment():
    ''' alignment of TOP modules '''

    # Define calibration
    cal = Calibration(name='TOP_alignment')
    cal.use_central_database(globalTag)
    cal.strategies = SingleIOV

    # Add collections
    for slot in range(1, 17):
        # Create path
        main = basf2.create_path()

        # Basic modules: Input, converters, geometry, unpacker
        main.add_module('RootInput')
        main.add_module('TOPGeometryParInitializer')
        main.add_module('TOPChannelMasker')

        # Collector module
        collector = basf2.register_module('TOPAlignmentCollector')
        collector.param('sample', sample)
        collector.param('parFixed', fixedParameters)
        collector.param('targetModule', slot)

        # Define collection
        collection = Collection(collector=collector, input_files=inputFiles,
                                pre_collector_path=main, max_files_per_collector_job=-1)
        collection.use_central_database(globalTag)
        collection.backend_args = {"queue": "l"}
        # Add collection to calibration
        cal.add_collection(name='slot_' + '{:0=2d}'.format(slot), collection=collection)

    # Algorithm
    algorithm = TOP.TOPAlignmentAlgorithm()
    cal.algorithms = algorithm

    return cal


# Define calibrations
cal = module_alignment()
cal.backend_args = {"queue": "l"}

# Add calibrations to CAF
cal_fw = CAF()
cal_fw.add_calibration(cal)
cal_fw.output_dir = output_dir
cal_fw.backend = backends.LSF()

# Run calibration
cal_fw.run()
