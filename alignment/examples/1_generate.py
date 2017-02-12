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

'''
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
        if track.getNumPointsWithMeasurement() < 3:
            print('Num points < 3 : ', str(track.getNumPointsWithMeasurement()))
            return False
        if track.getFitStatus().getPVal() < 0.001:
            print('P-value < 0.001 : ', str(track.getFitStatus().getPVal()))
            return False
        if not track.getFitStatus().isFitConvergedFully():
            print('Fit not completely sucessfull')
            return False

        return True

        return True

    def event(self):
        """ Return True if event is fine, False otherwise """
        tracks = Belle2.PyStoreArray('GF2Tracks')
        someOK = False
        if tracks.getEntries():
            for itrack, track in enumerate(tracks):
                if self.isOK(track):
                    someOK = True
                else:
                    relations = Belle2.PyStoreObj('TrackCandsToGF2Tracks').obj()
                    for irelation in range(0, relations.getEntries()):
                        relation = relations.getElement(irelation)
                        if relation.getToIndex() == itrack:
                            cands = Belle2.PyStoreArray('TrackCands')
                            print('Re-setting cand')
                            cands[relation.getFromIndex()].reset()

        if not someOK:
            print('Event has no good tracks. It will not be stored')
        super(TrackFitCheck, self).return_value(someOK)
'''

main = create_path()
main.add_module('EventInfoSetter', expList=[experiment], runList=[run], evtNumList=[nevents])

if cosmics_run:
    main.add_module('Cosmics')
else:
    main.add_module('EvtGenInput')

main.add_module('Gearbox')

if cosmics_run:
    main.add_module('Geometry', components=['BeamPipe', 'PXD', 'SVD', 'CDC'])
else:
    main.add_module('Geometry', components=['BeamPipe', 'MagneticFieldConstant4LimitedRCDC', 'PXD', 'SVD', 'CDC'])

main.add_module('FullSim')

main.add_module('PXDDigitizer')
main.add_module('SVDDigitizer')
main.add_module('PXDClusterizer')
main.add_module('SVDClusterizer')

main.add_module('CDCDigitizer')

main.add_module('TrackFinderMCTruthRecoTracks', WhichParticles='SVD')

main.add_module('RootOutput', outputFileName='DST_exp{:d}_run{:d}.root'.format(experiment, run))

# main.add_module('Display')
# trackFitCheck = TrackFitCheck()
# trackFitCheck.if_true(store, AfterConditionPath.CONTINUE)
# main.add_module(trackFitCheck)


main.add_module('Progress')
process(main)
print(statistics)
