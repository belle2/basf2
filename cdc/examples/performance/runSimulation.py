#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Simple example script to simulate cosmics events only with CDC.
Usage :
basf2 runSimulation.py <exp> <run> <nevt> <st>
exp: Experimental number
run: Run number
nevt: Number of events to be generated
st: Stream ID
"""

from basf2 import *
from ROOT import Belle2
import datetime
from generators import add_cosmics_generator

from tracking import add_cdc_cr_track_finding
import os.path
import sys
from cdc.cr import getDataPeriod


# Set the global log level
set_log_level(LogLevel.INFO)

# Set database
reset_database()
use_database_chain()
use_local_database(Belle2.FileSystem.findFile("data/framework/database.txt"))
use_central_database("GT_gen_data_002.11_gcr2017-07", LogLevel.WARNING)


def sim(exp, run, evt, st, topInCounter=True, magneticField=False):
    """
    exp : Experimental number
    run : Run number
    evt : Number of events to be generated
    st : stream ID
    """

    main_path = create_path()

    main_path.add_module('EventInfoSetter',
                         expList=[int(exp)],
                         evtNumList=[int(evt)],
                         runList=[int(run)])

    main_path.add_module('Progress')

    period = getDataPeriod(int(run))

    if not magneticField:
        components = ['CDC']
    else:
        components = ['CDC', 'MagneticFieldConstant4LimitedRCDC']

    add_cosmics_generator(main_path, components=components, pre_general_run_setup=period, top_in_counter=topInCounter)

    output = register_module('RootOutput',
                             outputFileName='gcr.cdc.{0:04d}.{1:06d}.{2:04d}.root'.format(int(exp), int(run), int(st)))
    main_path.add_module(output)
    print_path(main_path)
    process(main_path)
    print(statistics)

if __name__ == "__main__":

    import argparse
    parser = argparse.ArgumentParser()
    parser.add_argument('exp', help='Experimental number')
    parser.add_argument('run', help='Run number')
    parser.add_argument('evt', help='Number of events to be generated')
    parser.add_argument('st', help='Stream ID')

    args = parser.parse_args()
    sim(args.exp, args.run, args.evt, args.st, topInCounter=False, magneticField=True)
