#!/usr/bin/env python3

import basf2
from generators import add_phokhara_generator
from simulation import add_simulation
from reconstruction import add_reconstruction
from ROOT import Belle2
import numpy as np


def addGen(main, args):
    main.add_module('EventInfoSetter', runList=[1], evtNumList=args.nevents)
    main.add_module('Progress')
    main.add_module('Gearbox')
    main.add_module('Geometry')
    add_phokhara_generator(main, finalstate=args.finalState)

    if args.bkgfiles is not None:
        print('args.bkgfiles', args.bkgfiles)
        def strip(x): return x.split('/')[-1].split('.')[0]
        bkgraws = args.bkgfiles.strip("'")
        bkgfiles = glob.glob(bkgraws)
        add_simulation(main, bkgfiles=bkgfiles, bkgOverlay=False)
    else:
        add_simulation(main)
    add_reconstruction(main)


def add_cdc_trigger(path):
    """
    This function adds the CDC trigger modules to a path.
    @path              modules are added to this path
    """
    # TSF
    path.add_module('CDCTriggerTSF',
                    InnerTSLUTFile=Belle2.FileSystem.findFile("data/trg/cdc/innerLUT_Bkg_p0.70_b0.80.coe"),
                    OuterTSLUTFile=Belle2.FileSystem.findFile("data/trg/cdc/outerLUT_Bkg_p0.70_b0.80.coe"),
                    TSHitCollectionName="CDCTriggerSegmentHits")
    # 2D finder
    path.add_module('CDCTrigger2DFinder')
    # 3D finder
    path.add_module("CDCTriggerNDFinder", logLevel=basf2.LogLevel.DEBUG, debugLevel=1000,
                    diagonal=True, minhits=5, minweight=26, minpts=2, mincells=0)


class QuickCheck(basf2.Module):
    def initialize(self):
        self.storen = np.zeros(20, dtype='int')
        self.storem = np.zeros(20, dtype='int')
        self.storets = np.zeros(20, dtype='int')
        self.store2d = np.zeros(20, dtype='int')
        self.store3d = np.zeros(20, dtype='int')

    def event(self):
        recos = Belle2.PyStoreArray("RecoTracks")
        nrecos = len(recos)
        mrecos = 0
        tsrecos = 0
        for rec in recos:
            ncdc = len(rec.getCDCHitList())
            if ncdc > 15:
                mrecos += 1
            nts = len(rec.getRelationsTo('CDCTriggerSegmentHits'))
            if nts >= 4:
                tsrecos += 1
        if nrecos >= 20:
            nrecos = 19
        if mrecos >= 20:
            mrecos = 10
        if tsrecos >= 20:
            tsrecos = 10
        self.storen[nrecos] += 1
        self.storem[mrecos] += 1
        self.storets[tsrecos] += 1
        find2ds = Belle2.PyStoreArray("TRGCDC2DFinderTracks")
        n2ds = len(find2ds)
        if n2ds >= 20:
            n2ds = 19
        self.store2d[n2ds] += 1
        find3ds = Belle2.PyStoreArray("CDCTrigger3DFinderTracks")
        n3ds = len(find3ds)
        if n3ds >= 20:
            n3ds = 19
        self.store3d[n3ds] += 1

    def terminate(self):
        def mystr(val):
            return '%3s' % ('%d' % val)
        for itrack, (nevents, mevents, tsevents, events2d, events3d) in enumerate(
                zip(self.storen, self.storem, self.storets, self.store2d, self.store3d)):
            print(mystr(itrack), ' RecoTracks in ',
                  mystr(nevents), 'nevents; ',
                  mystr(mevents), ' reco with >15 CDCHits; ',
                  mystr(tsevents), ' reco with >4 TSHits; ',
                  mystr(events2d), ' 2d tracks; ',
                  mystr(events3d), ' 3d tracks')


def addTrgOutput(main, args):
    quickCheck = QuickCheck()
    main.add_module(quickCheck)


def mainFunc(args):
    basf2.set_log_level(basf2.LogLevel.WARNING)
    basf2.set_random_seed(args.seed)
    main = basf2.create_path()
    addGen(main, args)
    add_cdc_trigger(main)
    addTrgOutput(main, args)
    main.add_module('RootOutput',
                    outputFileName=args.outputFileName,
                    updateFileCatalog=False)

    basf2.process(main)
    print(basf2.statistics)


if __name__ == "__main__":
    import argparse
    parser = argparse.ArgumentParser(description='Test NDFinder in TRG with ISR simulation', epilog='NDFinder Test')
    parser.add_argument('--finalState', default='pi+pi-', help='final state: mu+mu- pi+pi- pi+pi-pi0')
    parser.add_argument('--seed', default=1234567, help='random seed', type=int)
    parser.add_argument('--nevents', default=100, help='number of events to generate', type=int)
    parser.add_argument('--bkgfiles', default=None, help='path to background mixing files')
    parser.add_argument('--outputFileName', default="ndFinderTest.root", help='output trg file name')

    args = parser.parse_args()
    mainFunc(args)
