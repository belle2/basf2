#!/usr/bin/env python3
# -*- coding: utf-8 -*-

'''
An example script to reconstruct osmics events with standalone CDC.
Usage :
basf2 runReconstruction.py <input> <output>
input: Input root file (after CDC unpacker).
       These data are usually stored in
       /ghi/fs01/belle2/bdata/group/detector/CDC/unpacked/
output : Output root file, which contains helix parameters.
         N.B. this is not the basf2 root file!
         To see the helix parameters.
         Please use compare2Tracks.C for example.
'''
import basf2
from basf2 import *
import ROOT
import os
import os.path
import argparse

from reconstruction import add_cosmics_reconstruction
from tracking import add_cdc_cr_track_finding
from ROOT import Belle2
from cdc.cr import *

reset_database()
use_database_chain()
use_local_database(Belle2.FileSystem.findFile("data/framework/database.txt"))
use_local_database("cdc_crt/database.txt", "cdc_crt")
# use_central_database("cdc_cr_test1", LogLevel.WARNING)


def rec(input, output, topInCounter=True, magneticField=False):

    main_path = basf2.create_path()
    logging.log_level = LogLevel.INFO

    # Get run number.
    run_number = getRunNumber(input)

    # Set the peiod of data taking.
    data_period = getDataPeriod(run_number)

    globalPhiRotation = getPhiRotation()

    if os.path.exists('output') is False:
        os.mkdir('output')

    # RootInput
    main_path.add_module('RootInput',
                         inputFileNames=input)

    # gearbox & geometry needs to be registered any way
    main_path.add_module('Gearbox',
                         override=[
                             ("/DetectorComponent[@name='CDC']//GlobalPhiRotation", str(globalPhiRotation), "deg")
                         ])
    #
    if magneticField is False:
        main_path.add_module('Geometry',
                             components=['CDC'])
    else:
        main_path.add_module('Geometry',
                             components=['CDC', 'MagneticFieldConstant4LimitedRCDC'])

    main_path.add_module('Progress')

    # Add CDC CR reconstruction.
    add_cosmics_reconstruction(main_path, pre_general_run_setup=data_period,
                               eventTimingExtraction=False)

    # Simple analysi module.
    output = "/".join(['output', output])
    main_path.add_module('CDCCosmicAnalysis',
                         noBFit=not magneticField,
                         Output=output)

    #    main_path.add_module("RootOutput", outputFileName='full.root')
    basf2.print_path(main_path)
    basf2.process(main_path)
    print(basf2.statistics)


if __name__ == "__main__":
    # Make the parameters accessible form the outside.

    parser = argparse.ArgumentParser()
    parser.add_argument('input', help='Input file to be processed (unpacked CDC data).')
    parser.add_argument('output', help='Output file you want to store the results.')
    args = parser.parse_args()
    rec(args.input, args.output, topInCounter=True, magneticField=False)
