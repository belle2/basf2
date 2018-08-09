#!/usr/bin/env python3
# -*- coding: utf-8 -*-
from basf2 import *
from svd import *
from tracking import *
from reconstruction import add_reconstruction
import glob
import argparse
import sys
import os
import ROOT
from ROOT.Belle2 import FileSystem
from ROOT.Belle2 import SVDCoGCalibrationsImporter


use_central_database("332_COPY-OF_GT_gen_prod_004.11_Master-20171213-230000")

parser = argparse.ArgumentParser(description="Simulation on/off, Phase 2/3")
parser.add_argument('--simulate-on', dest='simulate', action='store_const', const=True, default=False, help='Turn on simulation')
parser.add_argument('--phase2-on', dest='p2', action='store_const', const=True, default=False, help='Use P2 in simulation')
parser.add_argument(
    '--dbloading-only',
    dest='dbload',
    action='store_const',
    const=True,
    default=False,
    help='Perform only DB loadings')
parser.add_argument('--calsensors-on', dest='calsensors', action='store_const', const=True,
                    default=False, help='Fill DB with sensors-related info instead of area-realted ones')
args = parser.parse_args()


main = create_path()

if (not args.dbload):
    numEvents = 3000
    filename = "SVDCoGCalibration_Data.root"

    if (args.simulate):
        from simulation import add_simulation
        set_random_seed(1)
        eventinfosetter = register_module('EventInfoSetter')
        if (args.p2):
            eventinfosetter.param('expList', [1002])
        else:
            eventinfosetter.param('expList', [0])
        eventinfosetter.param('runList', [1])
        eventinfosetter.param('evtNumList', [numEvents])
        main.add_module(eventinfosetter)
        main.add_module('EventInfoPrinter')
        main.add_module('EvtGenInput')
        # bkgFiles = glob.glob('/sw/belle2/bkg/*.root')
        bkgFiles = None
        add_simulation(main, bkgfiles=bkgFiles, usePXDDataReduction=False)
        if (args.p2):
            filename = "SVDCoGCalibration_P2.root"
        else:
            filename = "SVDCoGCalibration_P3.root"
    else:
        # rooinput = register_module('RootInput',inputFileNames=["run77/SVDRootOutput_cosmic.0002.00077.HLT3.f0000*"])
        rooinput = register_module('RootInput')
        main.add_module(rooinput)
        main.add_module("Gearbox", fileName="/geometry/Beast2_phase2.xml")
        main.add_module('Geometry')
        add_svd_unpacker(main)

    # reconstruct SVD raw data
    add_svd_reconstruction(main, useNN=False, useCoG=True)
    for mod in main.modules():
        if mod.name() == 'SVDCoGTimeEstimator':
            # mod.param('ShaperDigits', 'SVDShaperDigitsNotHotINroi')
            mod.param('Correction_StripCalPeakTime', True)
            mod.param('Correction_TBTimeWindow', False)
            mod.param('Correction_ShiftMeanToZero', False)
            mod.param('Correction_ShiftMeanToZeroTBDep', False)

    svdcalibcog = register_module('SVDCoGCalibrator')
    svdcalibcog.param('outputFileName', filename)
    svdcalibcog.param('useTracks', False)
    main.add_module(svdcalibcog)
    main.add_module('Progress')
    print_path(main)
    process(main)
    print(statistics)

    print("---")
    print("Run 'CompareCoGCalibrationHistos.C' macro passing to that the " + filename + " root file:")
    print("> root -b")
    print("root > .L CompareCoGCalibrationHistos.C")
    print("root > CompareCoGCalibrationHistos(" + filename + ")")
    print("You can download the macro from https://stash.desy.de/projects/B2G/repos/svd_qatools/browse")
    print("---")

use_local_database("temporaryLocalDB/database.txt")
dbImporter = SVDCoGCalibrationsImporter()
dbImporter.importSVDTimeShiftCorrections(not args.calsensors, filename)
