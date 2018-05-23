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

"""
Steering file to generate local run calibration payloads.
Usage examples:
  1. Generating payloads in localdb:
  ./EclLocalRunCalib.py \
  --filename /hsm/belle2/bdata/group/detector/ECL/0003/01854/ecl.0003.01854.HLT1.f00000.sroot
"""

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
    parser = argparse.ArgumentParser(description='ECL local calibration script')
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
    parser.add_argument('--mintime',
                        action='store',
                        dest='mintime',
                        type=float,
                        default=-500.,
                        help='Set minTime parameter')
    parser.add_argument('--maxtime',
                        action='store',
                        dest='maxtime',
                        type=float,
                        default=500.,
                        help='Set maxTime parameter')
    parser.add_argument('--minampl',
                        action='store',
                        dest='minampl',
                        type=float,
                        default=0.,
                        help='Set minAmpl parameter')
    parser.add_argument('--maxampl',
                        action='store',
                        dest='maxampl',
                        type=float,
                        default=3.e+04,
                        help='Set maxAmpl parameter')
    parser.add_argument('--maxtimeoffset',
                        action='store',
                        dest='maxtimeoffset',
                        type=float,
                        default=0.3,
                        help='Set maxTimeOffset parameter')
    parser.add_argument('--maxamploffset',
                        action='store',
                        dest='maxamploffset',
                        type=float,
                        default=1.e-3,
                        help='Set maxAmplOffset parameter')
    parser.add_argument('--mincounts',
                        action='store',
                        dest='mincounts',
                        type=int,
                        default=1000,
                        help='Set initNOfEvents parameter')
    parser.add_argument('--nofdev',
                        action='store',
                        dest='nofdev',
                        type=int,
                        default=3,
                        help='Set nOfStdDevs parameter')
    parser.add_argument('--debugfile',
                        action='store',
                        dest='debugfile',
                        type=str,
                        default="",
                        help='Set nOfStdDevs parameter')

    return parser.parse_args()


def runCalibration():
    """!@ Run ECL local calibration procedure
    """
    args = parseArguments()
    connectToDB(args.centraldb, args.dbtag)
    print('REFMODE = %d' % (args.reference,))
    print('VERBMODE = %d' % (args.reference,))
    # set log level
    basf2.set_log_level(basf2.LogLevel.DEBUG)
    # add event infosetter
    inputmodule = basf2.register_module('SeqRootInput')
    inputmodule.param("inputFileName", args.filename)
    # register unpacker module
    eclunpacker = basf2.register_module('ECLUnpacker')
    # to save trigger phase information in the datastore
    eclunpacker.param('storeTrigTime', 1)
    # Local run calibration
    calibrator = basf2.register_module('ECLLocalRunCalibrator')
    calibrator.log_level = basf2.LogLevel.DEBUG
    calibrator.param('minTime', args.mintime)
    calibrator.param('maxTime', args.maxtime)
    calibrator.param('minAmpl', args.minampl)
    calibrator.param('maxAmpl', args.maxampl)
    calibrator.param('maxTimeOffset', args.maxtimeoffset)
    calibrator.param('maxAmplOffset', args.maxamploffset)
    calibrator.param('nOfStdDevs', args.nofdev)
    calibrator.param('initNOfEvents', args.mincounts)
    calibrator.param('refMode', args.reference)
    calibrator.param('verbose', args.verbose)
    calibrator.param('debugFile', args.debugfile)
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
