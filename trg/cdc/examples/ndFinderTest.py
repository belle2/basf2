#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import basf2
from generators import add_phokhara_generator
from simulation import add_simulation
from reconstruction import add_reconstruction
from ROOT import Belle2
import numpy as np
import glob


def addGen(main, args):
    main.add_module('EventInfoSetter',
                    runList=[1],
                    expList=[0],
                    evtNumList=args.nevents
                    )
    main.add_module('Progress')
    main.add_module('Gearbox')
    main.add_module('Geometry')
    add_phokhara_generator(main, finalstate=args.finalState)

    if args.bkgfiles is not None:
        def strip(x): return x.split('/')[-1].split('.')[0]
        bkgraws = args.bkgfiles.strip("'")
        bkgfiles = glob.glob(bkgraws)
        print('bkgfiles', bkgfiles)
        add_simulation(main, bkgfiles=bkgfiles)
    else:
        add_simulation(main)
    add_reconstruction(main)


def addRead(main, args):
    main.add_module('RootInput', inputFileName=args.genFileName)
    main.add_module('Progress')
    main.add_module('Gearbox')
    main.add_module('Geometry')


def add_cdc_finders(path):
    """
    This function adds the CDC trigger TSF and Finder modules to a path.
    @path              modules are added to this path
    """
    # TSF
    InTS = Belle2.FileSystem.findFile("data/trg/cdc/innerLUT_Bkg_p0.70_b0.80.coe")
    OutTS = Belle2.FileSystem.findFile("data/trg/cdc/outerLUT_Bkg_p0.70_b0.80.coe")
    path.add_module('CDCTriggerTSF',
                    InnerTSLUTFile=InTS,
                    OuterTSLUTFile=OutTS,
                    TSHitCollectionName="CDCTriggerSegmentHits")
    # 2D finder
    path.add_module('CDCTrigger2DFinder')
    # 3D finder
    axialFile = Belle2.FileSystem.findFile(args.axialFile)
    stereoFile = Belle2.FileSystem.findFile(args.stereoFile)
    path.add_module("CDCTriggerNDFinder",
                    logLevel=basf2.LogLevel.DEBUG, debugLevel=1000,
                    minhits=args.minhits,
                    minweight=args.minweight,
                    minpts=args.minpts,
                    mincells=args.mincells,
                    minhits_axial=args.minhits_axial,
                    minassign=args.minassign,
                    thresh=args.thresh,
                    diagonal=args.diagonal,
                    axialFile=axialFile,
                    stereoFile=stereoFile,
                    verbose=args.verbose
                    )


class QuickCheck(basf2.Module):
    def initialize(self):
        self.storen = np.zeros(10, dtype='int')
        self.storem = np.zeros(10, dtype='int')
        self.store2d = np.zeros(10, dtype='int')
        self.store3d = np.zeros(10, dtype='int')
        self.storeunits = np.zeros(10, dtype='int')

    def getUniqRecTS(self, rec):
        sls = []
        for tshit in rec.getRelationsTo('CDCTriggerSegmentHits'):
            sls.append(tshit.getISuperLayer())
        return np.unique(sls)

    def event(self):
        recos = Belle2.PyStoreArray("RecoTracks")
        nrecos = len(recos)
        mrecos = 0
        unitsrecos = 0
        for rec in recos:
            cdcuniq = np.unique([cdchit.getICLayer() for cdchit in rec.getCDCHitList()])
            ncdc = len(cdcuniq)
            if ncdc > 25:
                mrecos += 1

            nunits = len(self.getUniqRecTS(rec))
            if nunits >= 4:
                unitsrecos += 1

        find2ds = Belle2.PyStoreArray("TRGCDC2DFinderTracks")
        n2ds = len(find2ds)

        find3ds = Belle2.PyStoreArray("CDCTrigger3DFinderTracks")
        n3ds = len(find3ds)

        [nrecos, mrecos, unitsrecos, n2ds, n3ds] = self.setMax([nrecos, mrecos, unitsrecos, n2ds, n3ds])

        self.storen[nrecos] += 1
        self.storem[mrecos] += 1
        self.store2d[n2ds] += 1
        self.store3d[n3ds] += 1
        self.storeunits[unitsrecos] += 1

    def setMax(self, valList, maxval=10):
        trimmed = []
        for value in valList:
            if value >= maxval:
                value = maxval - 1
            trimmed.append(value)
        return trimmed

    def terminate(self):
        def mystr(val):
            return '%3s' % ('%d' % val)
        headline0 = [' ' for i in range(6)]
        headline0.insert(4, 'TrackType')
        headline1 = ['  NTrack', '__|'] + ['___' for i in range(5)]
        headline2 = [' ' * 3, '|', 'Reco', 'Reco  ', 'Reco', '2DFinder', '3DFinder']
        headline3 = [' ' * 3, '|', '    ', '>25CDC', '>4TS', '        ', '        ']

        def uline(strarr, sep='     ', fmt='%.9s', fmtOut='%9s'):
            return sep.join([fmtOut % (fmt % x) for x in strarr])
        print('number of tracks (NTrack in rows) of type TrackType (in columns) in number of events (elements)')
        print(uline(headline0))
        print(uline(headline1))
        print(uline(headline2))
        print(uline(headline3))
        print(uline(['_' * 5 for i in range(7)]))

        for itrack, (nevents, mevents, unitsevents, events2d, events3d) in enumerate(
                zip(self.storen, self.storem, self.storeunits, self.store2d, self.store3d)):
            entryline = [mystr(itrack), '|',
                         mystr(nevents),
                         mystr(mevents),
                         mystr(unitsevents),
                         mystr(events2d),
                         mystr(events3d)]
            print(uline(entryline))


def addTrgOutput(main, args):
    quickCheck = QuickCheck()
    main.add_module(quickCheck)


def add_matching(path, finderNames):
    for finderName in finderNames:
        path.add_module('CDCTriggerRecoMatcher',
                        TrgTrackCollectionName=finderName)


def mainFunc(args):
    basf2.set_log_level(basf2.LogLevel.WARNING)
    basf2.set_random_seed(args.seed)
    main = basf2.create_path()
    addGen(main, args)
    main.add_module('RootOutput',
                    outputFileName=args.genFileName,
                    updateFileCatalog=False)
    add_cdc_finders(main)
    add_matching(main, ["CDCTrigger3DFinderTracks", "TRGCDC2DFinderTracks"])
    addTrgOutput(main, args)
    main.add_module('RootOutput',
                    outputFileName=args.outputFileName,
                    updateFileCatalog=False)
    basf2.process(main)
    print(basf2.statistics)


def mainFuncRead(args):
    basf2.set_log_level(basf2.LogLevel.WARNING)
    basf2.set_random_seed(args.seed)
    main = basf2.create_path()
    addRead(main, args)
    add_cdc_finders(main)
    add_matching(main, ["CDCTrigger3DFinderTracks", "TRGCDC2DFinderTracks"])
    addTrgOutput(main, args)
    main.add_module('RootOutput',
                    outputFileName=args.outputFileName,
                    updateFileCatalog=False)

    basf2.process(main, max_event=args.nevents)
    print(basf2.statistics)


if __name__ == "__main__":
    import argparse
    parser = argparse.ArgumentParser(description='Test NDFinder in TRG with ISR simulation', epilog='NDFinder Test')
    parser.add_argument('--finalState', default='pi+pi-', help='final state: mu+mu- pi+pi- pi+pi-pi0')
    parser.add_argument('--seed', default=1234567, help='random seed', type=int)
    parser.add_argument('--nevents', default=100, help='number of events to generate', type=int)
    parser.add_argument('--bkgfiles', default=None, help='path to background mixing files')
    parser.add_argument('--outputFileName', default="ndFinderTest.root", help='output trg file name')
    parser.add_argument('--genFileName', default="ndFinderGen.root", help='output gen file name')
    parser.add_argument('--minweight', default=12, type=int, help='NDFinder')
    parser.add_argument('--minhits', default=4, type=int, help='NDFinder')
    parser.add_argument('--minhits_axial', default=2, type=int, help='NDFinder')
    parser.add_argument('--minpts', default=2, type=int, help='NDFinder')
    parser.add_argument('--diagonal', action='store_true', help='NDFinder')
    parser.add_argument('--mincells', default=5, type=int, help='NDFinder')
    parser.add_argument('--minassign', default=0.8, type=float, help='NDFinder')
    parser.add_argument('--thresh', default=0.85, type=float, help='NDFinder')
    parser.add_argument('--readGen', action='store_true', help='skip generation, read from file')
    parser.add_argument('--axialFile',
                        default="data/trg/cdc/ndFinderAxialShallow.txt.gz",
                        # default="data/trg/cdc/ndFinderArrayAxialComp.txt.gz",
                        help='NDFinder File name of the axial hit patterns')
    parser.add_argument('--stereoFile',
                        default="data/trg/cdc/ndFinderStereoShallow.txt.gz",
                        # default="data/trg/cdc/ndFinderArrayStereoComp.txt.gz",
                        help='NDFinder File name of the stereo hit patterns')
    parser.add_argument('--verbose', action='store_true', help='NDFinder')

    args = parser.parse_args()
    if args.readGen:
        mainFuncRead(args)
    else:
        mainFunc(args)
