#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#####################################################################
#                                                                   #
# eclLocalRunCalibration.py                                         #
#                                                                   #
# This script is used to run ECL local calibration                  #
#                                                                   #
# Author: The Belle II Collaboration                                #
# Contributors: Sergei Gribanov (S.S.Gribanov@inp.nsk.su) (BINP),   #
# Vitaly Vorobyev (vvorob@inp.nsk.su) (BINP)                        #
#                                                                   #
#####################################################################
import basf2
from ROOT.Belle2 import FileSystem
import argparse


def parseArguments():
    """!@brief Get list of command line arguments.
    """
    parser = argparse.ArgumentParser(description='ECL local calibration script')
    parser.add_argument('--filename',
                        action='store',
                        dest='filename',
                        type=str,
                        help='Path to local calibration run')
    parser.add_argument('--lowrun',
                        action='store',
                        dest='lowrun',
                        type=int,
                        default=-1,
                        help='Low run of validity interval')
    parser.add_argument('--highrun',
                        action='store',
                        dest='highrun',
                        type=int,
                        default=-1,
                        help='High run of validity interval')
    parser.add_argument('--mintime',
                        action='store',
                        dest='mintime',
                        type=float,
                        default=-1000.,
                        help='Set minTime parameter')
    parser.add_argument('--maxtime',
                        action='store',
                        dest='maxtime',
                        type=float,
                        default=1000.,
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
                        default=1.e+05,
                        help='Set moaxAmpl parameter')
    parser.add_argument('--nofdevs',
                        action='store',
                        dest='nofdevs',
                        type=int,
                        default=3,
                        help='Set nOfStdDevs parameter')
    parser.add_argument('--centraldb',
                        action='store_true',
                        dest='centraldb',
                        default=False,
                        help='Use central DB. '
                        'In default case a local DB is used.')
    parser.add_argument('--changeprev',
                        action='store_true',
                        dest='changeprev',
                        default=False,
                        help='Change interval of validity '
                        'of a previous payload.')
    parser.add_argument('--addref',
                        action='store_true',
                        dest='addref',
                        default=False,
                        help='Mark run as reference '
                        'if it is needed to'
                        'do that right after calibration.')
    parser.add_argument('--dbname',
                        action='store',
                        dest='dbname',
                        type=str,
                        default='localdb/database.txt',
                        help='Central DB tag or path to local DB')
    parser.add_argument('--fillhisto',
                        action='store_true',
                        dest='fillhisto',
                        default=False,
                        help='Set mode, used '
                        'to fill time and amplitude '
                        'histograms for a certain cellid.')
    parser.add_argument('--cellid',
                        action='store',
                        dest='cellid',
                        type=int,
                        default=0,
                        help='Cell id, used to '
                        ' obtain time and '
                        'amplitude histograms '
                        'for a certain cellid.')
    parser.add_argument('--histofname',
                        action='store',
                        dest='histfname',
                        type=str,
                        default='histo_time_ampl.root',
                        help='Path to amplitude and time'
                        'histograms for a certain '
                        'cellid.')

    return parser.parse_args()


def runCalibration():
    """!@ Run ECL local calibration procedure
    """
    args = parseArguments()
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
    calibrator.param('nOfStdDevs', args.nofdevs)
    calibrator.param('isLocal', (not args.centraldb))
    calibrator.param('changePrev', args.changeprev)
    calibrator.param('addRef', args.addref)
    calibrator.param('dbName', args.dbname)
    calibrator.param('lowRun', args.lowrun)
    calibrator.param('highRun', args.highrun)
    calibrator.param('fillHisto', args.fillhisto)
    calibrator.param('cellid', args.cellid)
    calibrator.param('histoFileName', args.histfname)
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
