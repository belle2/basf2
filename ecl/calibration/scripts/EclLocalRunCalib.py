#!/usr/bin/env python3
# -*- coding: utf-8 -*-

########################################################
#
# Local run Digit Calibration Script
#
# This script is used to run ECL local calibration
#
# Author: The Belle II Collaboration
# Contributors: Sergei Gribanov (S.S.Gribanov@inp.nsk.su) (BINP),
# Vitaly Vorobyev (vvorob@inp.nsk.su) (BINP)
#
########################################################

import basf2
from ROOT.Belle2 import FileSystem
import argparse


def connectToDB(isCentral, dbtag):
    """!@brief Connect to data base (DB).

    @param isCentral used to choose central
    DB (True) or local DB (False)

    @param dbtag is used to select DB tag in
    a case of cental DB or to select path to
    DB in a case of local DB.
    """

    basf2.reset_database()
    if isCentral:
        basf2.use_central_database(dbtag, basf2.LogLevel.DEBUG)
    else:
        basf2.use_local_database(FileSystem.findFile(dbtag))


def parseArguments():
    """!@brief Get list of command line arguments.
    """
    parser = argparse.ArgumentParser(description='EcL local calibration script')
    parser.add_argument('--filename',
                        action='store',
                        dest='filename',
                        type=str,
                        help='Path to local calibration run.')
    parser.add_argument('--centraldb',
                        action='store_true',
                        dest='centraldb',
                        default=False,
                        help='Use central DB. '
                        'In default case a local DB is used.')
    parser.add_argument('--dbtag',
                        action='store',
                        dest='dbtag',
                        type=str,
                        default='localdb/database.txt',
                        help='Central DB tag or path to local DB.')
    parser.add_argument('--reference',
                        action='store_true',
                        dest='reference',
                        default=False,
                        help='Enable reference mode.')
    parser.add_argument('--verbose',
                        action='store_true',
                        dest='verbose',
                        default=False,
                        help='Enable verbose mode')
    parser.add_argument('--maxdev',
                        action='store',
                        dest='maxdev',
                        type=int,
                        default=3,
                        help='Set maxDev parameter.')
    parser.add_argument('--minCounts',
                        action='store',
                        dest='mincounts',
                        type=int,
                        default=1000,
                        help='Set minCounts parameter.')
    parser.add_argument('--maxtimeoffset',
                        action='store',
                        dest='maxtimeoffset',
                        type=float,
                        default=0.001,
                        help='Max time offset')
    parser.add_argument('--maxamploffset',
                        action='store',
                        dest='maxamploffset',
                        type=float,
                        default=0.001,
                        help='Max amplitude offset')
    parser.add_argument('--maxrejectcount',
                        action='store',
                        dest='maxrejectcount',
                        type=int,
                        default=5,
                        help='Maximum reject count')

    return parser.parse_args()


def runCalibration():
    """!@ Run ECL local calibration procedure
    """
    args = parseArguments()
    connectToDB(args.centraldb, args.dbtag)

    print('REFMODE = %d' % (args.reference,))
    print('VERBMODE = %d' % (args.reference,))

    # set log level
    basf2.set_log_level(basf2.LogLevel.INFO)

    # add event infosetter
    inputmodule = basf2.register_module('SeqRootInput')
    inputmodule.param("inputFileName", args.filename)

    # register unpacker module
    eclunpacker = basf2.register_module('ECLUnpacker')
    # set parameter storeTrigTime = 1 for eclunpacker
    # to save trigger phase information in the datastore
    eclunpacker.param('storeTrigTime', 1)

    # Local run calibration
    calibrator = basf2.register_module('ECLLocalRunCalibrator')
    calibrator.param('maxDev', args.maxdev)
    calibrator.param('minCounts', args.mincounts)
    calibrator.param('refMode', args.reference)
    calibrator.param('verbose', args.verbose)
    calibrator.param('maxTimeOffset', args.maxtimeoffset)
    calibrator.param('maxAmplOffset', args.maxamploffset)
    calibrator.param('maxRejectCount', args.maxrejectcount)

    # create main path
    main = basf2.create_path()
    main.add_module(inputmodule)
    main.add_module(eclunpacker)
    main.add_module(calibrator)

    # Show progress of processing
    progress = basf2.register_module('ProgressBar')
    main.add_module(progress)

    basf2.process(main)
    print(basf2.statistics)


if __name__ == '__main__':
    runCalibration()
