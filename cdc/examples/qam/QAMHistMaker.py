#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
import ROOT
from ROOT import Belle2
from ROOT import TFile, TH1D, TH2D, TCanvas, TFile, TChain
from ROOT import TH1F
import argparse
import glob


reset_database()
use_database_chain()
use_local_database(Belle2.FileSystem.findFile("data/framework/database.txt"))
# use_local_database("cdc_crt/database.txt", "cdc_crt")
# use_local_database("localDB/database.txt", "localDB")
# For GCR, July 2017.
use_central_database("GT_gen_data_002.11_gcr2017-07", LogLevel.WARNING)
# For GCR, August 2017.
# use_central_database("GT_gen_data_003.04_gcr2017-08", LogLevel.WARNING)


def ana(exp=1, run=3118, magneticField=True, prefix='', dest='.'):

    # Seach dst files.
    files = glob.glob(prefix + '/dst.cosmic.{0:0>4}.{1:0>5}'.format(exp, run) + '*.root')
    # create path
    main = create_path()
    # Input (ROOT file).
    main.add_module('RootInput',
                    inputFileNames=files)

    gearbox = register_module('Gearbox',
                              fileName="/geometry/GCR_Summer2017.xml",
                              override=[
                                  ("/Global/length", "8.", "m"),
                                  ("/Global/width", "8.", "m"),
                                  ("/Global/height", "8.", "m"),
                              ])
    main.add_module(gearbox)
    main.add_module('Geometry')

    output = "/".join([dest, 'output.{0:0>4}.{1:0>5}.root'.format(exp, run)])
    main.add_module('CDCCosmicAnalysis',
                    noBFit=not magneticField,
                    Output=output,
                    treeName='treeTrk',
                    phi0InRad=False)

    main.add_module('Progress')
    # process events and print call statistics
    process(main)
    print(statistics)


if __name__ == "__main__":
    parser = argparse.ArgumentParser()

    parser.add_argument('exp', help='Experimental number')
    parser.add_argument('run', help='Run number')
    args = parser.parse_args()
    '''
    ana(exp=args.exp, run=args.run, magneticField=True,
    #         prefix='/gpfs/home/belle/karim/BASF2/build-2017-08-21/cdc/examples/performance/data_reference',
    prefix='/ghi/fs01/belle2/bdata/users/karim/data/GCR1/build-2017-08-21',
    #         dest='/ghi/fs01/belle2/bdata/group/detector/CDC/qam/GCR1/build-2017-08-21'
    # dest='/ghi/fs01/belle2/bdata/group/detector/CDC/qam/GCR1/test'
    dest='.'  # Store current directory.
    )
    '''
    ROOT.gROOT.LoadMacro('createQAMHist.C')
    #    ROOT.gSystem.Load('createQAMHist_C.so')
    prefix = '/gpfs/home/belle/karim/BASF2/build-2017-08-21/cdc/examples/performance/data_reference'
    hist = ROOT.createQAMHist('/'.join([prefix, 'output.{0:0>4}.{1:0>5}.root'.format(args.exp, args.run)]),
                              'qam.{0:0>4}.{1:0>5}.root'.format(args.exp, args.run))
    #    ROOT.createQAMHist()
