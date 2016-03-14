#!/usr/bin/env python
# -*- coding: utf-8 -*-
import os
import sys
from basf2 import *
from ROOT import Belle2

# set_random_seed(101)
set_log_level(LogLevel.INFO)

if len(sys.argv) < 4:
    print 'Usage: basf2 GenDST.py experiment run num_events'
    sys.exit(1)

experiment = int(sys.argv[1])
run = int(sys.argv[2])
nevents = int(sys.argv[3])


class TrackFitCheck(Module):

    def __init__(self):
        super(TrackFitCheck, self).__init__()

    def isOK(self, track):
        # VXD only (12 layers (cosmics!) x 2 for overlaps)
        if track.getNumPointsWithMeasurement() > 24:
            print 'Num points > 24 : ', str(track.getNumPointsWithMeasurement())
            return False
        if track.getNumPointsWithMeasurement() < 5:
            print 'Num points < 6 : ', str(track.getNumPointsWithMeasurement())
            return False
        if track.getFitStatus().getPVal() < 0.001:
            print 'P-value < 0.001 : ', str(track.getFitStatus().getPVal())
            return False

        return True

    def event(self):
        tracks = Belle2.PyStoreArray('GF2Tracks')
        ok = True
        if tracks.getEntries():
            for track in tracks:
                if not self.isOK(track):
                    ok = False
                    break
        else:
            ok = False
        if not ok:
            print 'Event has no good tracks. It will not be stored'
        super(TrackFitCheck, self).return_value(ok)

main = create_path()
main.add_module('EventInfoSetter', expList=[experiment], runList=[run], evtNumList=[nevents])
# main.add_module('Cosmics', ipRequirement=0)
main.add_module('ParticleGun')

main.add_module('Gearbox')
main.add_module('Geometry', components=['MagneticFieldConstant4LimitedRCDC', 'PXD', 'SVD'])
# main.add_module('Geometry', components=['PXD', 'SVD'])

main.add_module('FullSim')
main.add_module('TrackFinderMCTruth', UseClusters=False)
main.add_module('GBLfit', UseClusters=False)

store = create_path()
store.add_module(
    'RootOutput',
    outputFileName='DST_Exp{:d}_Run{:d}.root'.format(
        experiment,
        run),
    branchNames=[
        'EventMetaData',
        'PXDTrueHits',
        'SVDTrueHits',
        'TrackCands'])

trackFitCheck = TrackFitCheck()
trackFitCheck.if_true(store, AfterConditionPath.CONTINUE)
main.add_module(trackFitCheck)


main.add_module('Progress')
process(main)
print statistics
