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
from simulation import add_simulation

import os.path
import sys
from cdc.cr import getDataPeriod, getTriggerType, getMapperAngle
from cdc.cr import add_cdc_cr_simulation
from cdc.cr import add_GCR_Trigger_simulation


# Set the global log level
set_log_level(LogLevel.INFO)

# Set database
reset_database()
use_database_chain()
# For GCR, July and August 2017.
use_central_database("GT_gen_data_003.04_gcr2017-08", LogLevel.WARNING)


def sim(exp, run, evt, st, topInCounter=False, magneticField=True, fieldMapper=False):
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

    period = getDataPeriod(exp=int(exp),
                           run=int(run))

    mapperAngle = getMapperAngle(exp=int(exp),
                                 run=int(run))

    triggerType = getTriggerType(exp=int(exp),
                                 run=int(run))

    if fieldMapper is True:
        main_path.add_module('CDCJobCntlParModifier',
                             MapperGeometry=True,
                             MapperPhiAngle=mapperAngle)

    components = ['CDC', 'ECL', 'MagneticField'] if magneticField is True else ['CDC', 'ECL']

    add_cosmics_generator(path=main_path,
                          components=components,
                          global_box_size=[8, 8, 8],
                          accept_box=[0.7, 0.3, 0.3],  # LWH
                          keep_box=[0.7, 0.3, 0.3],
                          cosmics_data_dir='data/generators/modules/cryinput/',
                          setup_file='./cry.setup',
                          data_taking_period=period,
                          top_in_counter=topInCounter)

    # add_simulation(main_path)
    add_cdc_cr_simulation(main_path, components=components)

    if triggerType is not None:
        add_GCR_Trigger_simulation(main_path,
                                   backToBack=True if triggerType == 'b2b' else False,
                                   skipEcl=True)
    else:
        B2INFO('skip tsim')

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
    sim(args.exp, args.run, args.evt, args.st, topInCounter=False, magneticField=True, fieldMapper=True)
