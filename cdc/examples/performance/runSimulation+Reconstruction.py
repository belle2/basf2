#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Simple example script to Simulate cosmics events and reconstruct only with CDC.
Usage :
basf2 runSimulation+Reconstruction.py <run> <nevt> <period>
run: Run number
nevt: Number of events to be generated
period : Data taking period (set 201607, 201608a, 201608b, 201609)
"""

from basf2 import *
from ROOT import Belle2
import datetime
from tracking import add_cdc_cr_track_finding
import os.path
import sys
from cdc.cr.utilities import *


# DB setting.
use_central_database("cdc_cr_mc", LogLevel.WARNING)
set_experiment_name(0, "cdc_crt_e000000")
# use_local_database("cdc_crt1/database.txt", "cdc_crt1", False)

# Set the global log level
set_log_level(LogLevel.INFO)


def main(run, evt, period, mode):

    main_path = create_path()
    emptyPath = create_path()

    if mode is 'rec':
        inputFilename = 'output_mc_run' + run + '.root'
        main_path.add_module('RootInput', inputFileNames=[inputFilename])
    else:
        main_path.add_module('EventInfoSetter',
                             evtNumList=[int(evt)],
                             runList=[int(run)])

    main_path.add_module('Progress')

    gearbox = register_module('Gearbox',
                              override=[
                                  ("/Global/length", "8.", "m"),
                                  ("/Global/width", "8.", "m"),
                                  ("/Global/height", "1.5", "m"),
                                  ("/DetectorComponent[@name='CDC']//GlobalPhiRotation", "1.875", "deg"),
                              ])
    main_path.add_module(gearbox)

    main_path.add_module('Geometry',
                         components=['CDC']
                         )

    set_cdc_cr_parameters(period)

    if mode is not 'rec':
        # Simulation.
        add_cdc_cr_simulation(main_path, emptyPath)

    # output
    if mode is 'sim':
        output = register_module('RootOutput')
        outputFilename = 'output_mc_run' + run + '.root'
        output.param('outputFileName', outputFilename)
        main_path.add_module(output)
    else:
        # Reconstruction.
        add_cdc_cr_reconstruction(main_path)

        # Simple analyis module.
        main_path.add_module('CDCCosmicAnalysis',
                             Output='output_run' + run + '.root')
    print_path(main_path)
    process(main_path)
    print(statistics)

if __name__ == "__main__":

    import argparse
    parser = argparse.ArgumentParser(description='CDC CR simulation and reconstruction.')
    parser.add_argument('run', help='Run number')
    parser.add_argument('evt', help='Number of events to be generated')
    parser.add_argument('period', help='Data period')
    args = parser.parse_args()

    date = datetime.datetime.today()
    print(date.strftime('Start at : %d-%m-%y %H:%M:%S\n'))
    main(args.run, args.evt, args.period, '')
    date = datetime.datetime.today()
    print(date.strftime('End at : %y-%m-%d %H:%M:%S\n'))
