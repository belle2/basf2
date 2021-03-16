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

import basf2 as b2
from generators import add_cosmics_generator

from cdc.cr import getDataPeriod, getTriggerType, getMapperAngle
from cdc.cr import add_cdc_cr_simulation
from cdc.cr import add_GCR_Trigger_simulation


# Set the global log level
b2.set_log_level(b2.LogLevel.INFO)

# Set database
b2.reset_database()
b2.use_database_chain()
# For GCR, July and August 2017.
b2.use_central_database("GT_gen_data_003.04_gcr2017-08", b2.LogLevel.WARNING)


def sim(exp, run, evt, st, topInCounter=False, magneticField=True, fieldMapper=False):
    """
    exp : Experimental number
    run : Run number
    evt : Number of events to be generated
    st : stream ID
    """

    main_path = b2.create_path()

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
        b2.B2INFO('skip tsim')

    output = b2.register_module('RootOutput',
                                outputFileName='gcr.cdc.{0:04d}.{1:06d}.{2:04d}.root'.format(int(exp), int(run), int(st)))
    main_path.add_module(output)
    b2.print_path(main_path)
    b2.process(main_path)
    print(b2.statistics)


if __name__ == "__main__":

    import argparse
    parser = argparse.ArgumentParser()
    parser.add_argument('exp', help='Experimental number')
    parser.add_argument('run', help='Run number')
    parser.add_argument('evt', help='Number of events to be generated')
    parser.add_argument('st', help='Stream ID')

    args = parser.parse_args()
    sim(args.exp, args.run, args.evt, args.st, topInCounter=False, magneticField=True, fieldMapper=True)
