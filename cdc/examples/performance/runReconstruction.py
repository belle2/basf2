#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

'''
An example script to reconstruct osmics events with standalone CDC.
Usage :
basf2 runReconstruction.py <input> <output>
input: Input root file (Raw data)
       GCR1 (RAW) /hsm/belle2/bdata/Data/Raw/e0001/
       GCR2 (RAW) /hsm/belle2/bdata/Data/Raw/e0002/

output : Output root file, which contains helix parameters.
         N.B. this is not the basf2 root file!
         To see the helix parameters.
         Please use compare2Tracks.C for example.
'''

import basf2 as b2
import os
import os.path
import argparse
from cdc import cr

# Set your suitable DB
b2.reset_database()
b2.use_database_chain()
b2.use_central_database("332_COPY-OF_GT_gen_prod_004.11_Master-20171213-230000", b2.LogLevel.INFO)
b2.use_central_database("MagneticFieldPhase2QCSoff")
# use_local_database("/home/belle/muchida/basf2/work/caf/gcr2/test6/localDB/database.txt")


def rec(input, output, topInCounter=False, magneticField=True,
        unpacking=True, fieldMapper=False):
    main_path = b2.create_path()
    b2.logging.log_level = b2.LogLevel.INFO

    # Get experiment and runnumber.
    exp_number, run_number = cr.getExpRunNumber(input)

    # Set the peiod of data taking.
    data_period = cr.getDataPeriod(exp=exp_number,
                                   run=run_number)

    mapperAngle = cr.getMapperAngle(exp=exp_number,
                                    run=run_number)

    # print(data_period)
    if os.path.exists('output') is False:
        os.mkdir('output')

    # RootInput
    main_path.add_module('RootInput',
                         #                          entrySequences=['0:1000'],
                         inputFileNames=input)
    if unpacking is True:
        main_path.add_module('CDCUnpacker')

    if data_period == 'gcr2017':
        gearbox = b2.register_module('Gearbox',
                                     fileName="/geometry/GCR_Summer2017.xml",
                                     override=[("/Global/length", "8.", "m"),
                                               ("/Global/width", "8.", "m"),
                                               ("/Global/height", "8.", "m"),
                                               ])
        main_path.add_module(gearbox)
    else:
        main_path.add_module('Gearbox')

    if fieldMapper is True:
        main_path.add_module('CDCJobCntlParModifier',
                             MapperGeometry=True,
                             MapperPhiAngle=mapperAngle)

    if magneticField is True:
        main_path.add_module('Geometry', useDB=True)
    else:
        main_path.add_module('Geometry',
                             components=['CDC', 'ECL'])

    main_path.add_module('Progress')

    # Add CDC CR reconstruction.
    cr.set_cdc_cr_parameters(data_period)
    cr.add_cdc_cr_reconstruction(main_path)
    # add_cosmics_reconstruction(main_path,
    #                               data_taking_period=data_period,
    #                           merge_tracks=False)

    # Simple analysis module.
    output = "/".join(['output', output])
    main_path.add_module('CDCCosmicAnalysis',
                         noBFit=not magneticField,
                         Output=output)

    #    main_path.add_module("RootOutput", outputFileName='full.root')
    b2.print_path(main_path)
    b2.process(main_path)
    print(b2.statistics)


if __name__ == "__main__":
    # Make the parameters accessible form the outside.
    parser = argparse.ArgumentParser()
    parser.add_argument('input', help='Input file to be processed (unpacked CDC data).')
    parser.add_argument('output', help='Output file you want to store the results.')
    args = parser.parse_args()
    rec(args.input, args.output, topInCounter=False, magneticField=True,
        unpacking=True, fieldMapper=False)
