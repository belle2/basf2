#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Simple example script to reconstruct cosmics events
Usage :
basf2 runRec.py <run>
run: Run number
"""

from basf2 import *
from ROOT import Belle2
import datetime
from tracking import add_cdc_cr_track_finding
from reconstruction import *
from simulation import *
import os.path
import sys
from cdc.cr import *


def add_Sam_n_Jan_top_modules(path, components=None):
    """
    Add the TOP reconstruction to the path.

    :param path: The path to add the modules to.
    :param components: The components to use or None to use all standard components.
    """
    # TOP reconstruction
    if components is None or 'TOP' in components:
        top_cm = register_module('TOPChannelMasker')
        path.add_module(top_cm)
        top_rec = register_module('TOPReconstructorPDF')
        path.add_module(top_rec)


reset_database()
use_database_chain()
use_local_database(Belle2.FileSystem.findFile("data/framework/database.txt"))
# use_local_database("cdc_crt/database.txt", "cdc_crt")
use_central_database("GT_gen_data_002.11_gcr2017-07", LogLevel.WARNING)

# Set the global log level
set_log_level(LogLevel.INFO)


def main(run, period, mode):

    main_path = create_path()
    empty_path = create_path()

    inputFilename = '/hsm/belle2/bdata/Data/Raw/e0001/r' + run + '/sub00/cosmic.0001.' + run + '.HLT1.f*.root'
    main_path.add_module('RootInput', inputFileNames=[inputFilename])

    # Get run number.
    run_number = int(run)

    # Set the period of data taking.
    data_period = getDataPeriod(run_number)

    globalPhiRotation = getPhiRotation()

    gearbox = register_module('Gearbox',
                              fileName="/geometry/GCR_Summer2017.xml",
                              override=[
                                  ("/Global/length", "8.", "m"),
                                  ("/Global/width", "8.", "m"),
                                  ("/Global/height", "8.", "m"),
                              ])
    main_path.add_module(gearbox)

    main_path.add_module('Geometry')
    components = ['CDC', 'TOP', 'ECL', 'BKLM']

    main_path.add_module('Progress')

    # Set CDC CR parameters.
    set_cdc_cr_parameters(data_period)

    # Add CDC CR reconstruction.
    add_cdc_cr_reconstruction(main_path, eventTimingExtraction=True)

    # Add further reconstruction modules
    add_dedx_modules(main_path, pruneTracks=False, components=components)
    for m in main_path.modules():
        if m.name() == 'CDCDedxPID':
            m.param('enableDebugOutput', True)

    add_ext_module(main_path)

    add_Sam_n_Jan_top_modules(main_path)
    # add_arich_modules(main_path)

    add_ecl_modules(main_path)
    add_ecl_track_matcher_module(main_path)
    add_ecl_eip_module(main_path)
    add_ecl_mc_matcher_module(main_path)

    add_klm_modules(main_path, components=components)
    add_klm_mc_matcher_module(main_path, components=components)

    add_muid_module(main_path)
    add_pid_module(main_path)

    # Add output
    add_mdst_output(main_path, mc=True, filename='mdst.cosmic.0001.' + run + '.root',
                    additionalBranches=['CDCDedxTracks', 'ECLDigits', 'ECLCalDigits', 'RecoTracks'])

    #
    output = register_module('RootOutput')
    outputFilename = 'dst.cosmic.0001.' + run + '.root'
    output.param('outputFileName', outputFilename)
    main_path.add_module(output)

    print_path(main_path)
    process(main_path)
    print(statistics)

if __name__ == "__main__":

    import argparse
    parser = argparse.ArgumentParser()
    parser.add_argument('run', help='Run number')
    parser.add_argument('--period', dest='period', default='normal', help='Data period')

    args = parser.parse_args()

    date = datetime.datetime.today()
    print(date.strftime('Start at : %d-%m-%y %H:%M:%S\n'))
    main(args.run, args.period, '')
    date = datetime.datetime.today()
    print(date.strftime('End at : %y-%m-%d %H:%M:%S\n'))
