#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import sys
from basf2 import *
from ROOT import Belle2

# set_random_seed(101)
set_log_level(LogLevel.INFO)

if len(sys.argv) < 5:
    print('Usage: basf2 1_generate.py experiment_number run_number num_events cosmics_run=0|1')
    sys.exit(1)

experiment = int(sys.argv[1])
run = int(sys.argv[2])
nevents = int(sys.argv[3])
cosmics_run = bool(int(sys.argv[4]))


class TrackFitCheck(Module):
    """
    Python module to discard events
    where there are no fully fitted tracks
    or where a track does not fullfill some
    criteria. However works only
    for single track per event in generator.
    """

    def __init__(self):
        """ init """
        super(TrackFitCheck, self).__init__()

    def isOK(self, track):
        """ Check criteria for a genfit::Track """

        # VXD only (12 layers (cosmics!) x 2 for overlaps)
        if track.getNumPointsWithMeasurement() > 24:
            print('Num points > 24 : ', str(track.getNumPointsWithMeasurement()))
            return False
        if track.getNumPointsWithMeasurement() < 5:
            print('Num points < 5 : ', str(track.getNumPointsWithMeasurement()))
            return False
        if track.getFitStatus().getPVal() < 0.001:
            print('P-value < 0.001 : ', str(track.getFitStatus().getPVal()))
            return False

        return True

        return True

    def event(self):
        """ Return True if event is fine, False otherwise """
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
            print('Event has no good tracks. It will not be stored')
        super(TrackFitCheck, self).return_value(ok)

main = create_path()
main.add_module('EventInfoSetter', expList=[experiment], runList=[run], evtNumList=[nevents])

if cosmics_run:
    main.add_module('Cosmics')
else:
    main.add_module('ParticleGun')

main.add_module('Gearbox')

if cosmics_run:
    main.add_module('Geometry', components=['BeamPipe', 'PXD', 'SVD'])
else:
    main.add_module('Geometry', components=['BeamPipe', 'MagneticFieldConstant4LimitedRCDC', 'PXD', 'SVD'])

main.add_module('MagnetConfiguration')

main.add_module('FullSim')
# main.add_module('CDCDigitizer')
main.add_module('TrackFinderMCTruth', UseClusters=False)


main.add_module('MagnetSwitcher')
main.add_module('GBLfit', UseClusters=False)

store = create_path()
store.add_module(
    'RootOutput',
    outputFileName='DST_exp{:d}_run{:d}.root'.format(
        experiment,
        run),
    branchNames=[
        'EventMetaData',
        'PXDTrueHits',
        'SVDTrueHits',
        'CDCHits',
        'TrackCands',
        'TrackFitResult',
        'MagnetOffEvents'])
# main.add_module('Display')
trackFitCheck = TrackFitCheck()
trackFitCheck.if_true(store, AfterConditionPath.CONTINUE)
main.add_module(trackFitCheck)


main.add_module('Progress')
process(main)
print(statistics)
