#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Simple example script to simulate and reconstruct cosmics events only with CDC.
Usage :
basf2 runSimulation+Reconstruction.py <exp> <run> <nevt> <st>
exp: Experimental number
run: Run number
nevt: Number of events to be generated
st: Stream ID
"""

from basf2 import *
from ROOT import Belle2
import datetime
from tracking import add_cdc_cr_track_finding
import os.path
import sys
from cdc.cr import *


# Set the global log level
set_log_level(LogLevel.INFO)

# Set database
reset_database()
use_database_chain()
use_local_database(Belle2.FileSystem.findFile("data/framework/database.txt"))
use_central_database("cdc_cr_test1", LogLevel.WARNING)


def main(exp, run, evt, st):

    main_path = create_path()
    emptyPath = create_path()

    main_path.add_module('EventInfoSetter',
                         expList=[int(exp)],
                         evtNumList=[int(evt)],
                         runList=[int(run)])

    main_path.add_module('Progress')

    period = getDataPeriod(int(run))
    set_cdc_cr_parameters(period)

    phi = getPhiRotation()

    gearbox = register_module('Gearbox',
                              override=[
                                  ("/Global/length", "8.", "m"),
                                  ("/Global/width", "8.", "m"),
                                  ("/Global/height", "1.5", "m"),
                                  ("/DetectorComponent[@name='CDC']//GlobalPhiRotation", str(phi), "deg"),
                              ])
    main_path.add_module(gearbox)

    main_path.add_module('Geometry',
                         components=['CDC']
                         )

    add_cdc_cr_simulation(main_path, emptyPath)

    # Add CDC CR reconstruction.
    add_cdc_cr_reconstruction(main_path, eventTimingExtraction=False)

    # Simple analysi module.
    outputFilename = 'output_mc_run' + run + 'st' + st + '.root'
    output = "/".join(['output', outputFilename])
    main_path.add_module('CDCCosmicAnalysis',
                         Output=output)

    # main_path.add_module("RootOutput", outputFileName=output)
    basf2.print_path(main_path)
    basf2.process(main_path)
    print(basf2.statistics)


if __name__ == "__main__":

    import argparse
    parser = argparse.ArgumentParser()
    parser.add_argument('exp', help='Experimental number')
    parser.add_argument('run', help='Run number')
    parser.add_argument('evt', help='Number of events to be generated')
    parser.add_argument('st', help='Stream ID')

    args = parser.parse_args()
    main(args.exp, args.run, args.evt, args.st)
