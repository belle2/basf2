#!/usr/bin/env python
# -*- coding: utf-8 -*-

from basf2 import Module

from ROOT import gSystem
gSystem.Load('libframework')  # for PyStoreArray
gSystem.Load('libcdc')  # for CDCSimHit
gSystem.Load('libtracking')  # for CDCHit and so on

from ROOT import Belle2  # make Belle2 namespace available
from ROOT import std

import subprocess
from datetime import datetime

import basf2tools

import os
import os.path
import math
from itertools import cycle


class CDCEvaluationModule(Module):

    def __init__(self, output_folder):
        super(CDCEvaluationModule, self).__init__()
                 # call constructor of base class, required!

        self.output_folder = output_folder
        self.write_sz_tracks = True and False

    def initialize(self):
        print 'initialize()'

    def beginRun(self):
        print 'beginRun()'

    def event(self):
        print '#################### EVALUATE EVENT ###########################'

    # ##### Short evaluation of the tracking performance

    # Write sz coordinate of the reconstructed tracks
        if self.write_sz_tracks:
            print 'Write sz coordinate of the reconstructed tracks'
            track_storearray = Belle2.PyStoreArray('CDCTracks')
            if track_storearray:
                print '#Tracks', track_storearray.getEntries()
                lineprototype = '%s,%s,%s\n'
                output_filename = os.path.join(self.output_folder, 'sz.txt')
                output_file = open('output/sz/sz.txt', 'w')
                for (iTrack, track) in enumerate(track_storearray):
                    for recohit in \
                        basf2tools.CDCGenericHitCollectionIterator(track):
                        output_file.write(lineprototype % (recohit.getPerpS(),
                                recohit.getPos3D().z(), iTrack))
                output_file.close()

    # Print efficiencies of the segments
        if True:
            mcLookUp = Belle2.CDCLocalTracking.CDCMCLookUp.Instance()
            print 'Printing efficiencoes of the selected RecoHit2DSegments'
            segment_collection = \
                Belle2.PyStoreArray('CDCRecoHit2DSegmentsSelected')
            if segment_collection:
                for segment in segment_collection:
          # print dir(mcLookUp.getHighestEfficieny(segment))
                    print mcLookUp.getHighestEfficieny(segment).first, \
                        mcLookUp.getHighestEfficieny(segment).second

    # Print efficiencies of the tracks
        if True:
            mcLookUp = Belle2.CDCLocalTracking.CDCMCLookUp.Instance()
            print 'Printing efficiencies of the tracks'
            track_collection = Belle2.PyStoreArray('CDCTracks')
            if track_collection:
                for track in track_collection:
                    print mcLookUp.getHighestEfficieny(track).first, \
                        mcLookUp.getHighestEfficieny(track).second
          # print mcLookUp.getEfficiencies(track)
          # print dir(mcLookUp.getEfficienies(track))

    # Print confusion matrix
        if True:
            print 'Confusion matrix of the tracks'

            mcLookUp = Belle2.CDCLocalTracking.CDCMCLookUp.Instance()
            track_collection = Belle2.PyStoreArray('CDCTracks')
            wirehit_collection = Belle2.PyStoreObj('CDCAllWireHitCollection')

            sparse_confusion_matrix = []

            if track_collection and wirehit_collection:
                allTrackIds = set()
                for wirehit in \
                    basf2tools.CDCGenericHitCollectionIterator(wirehit_collection.obj()):
                    trackId = int(mcLookUp.getMajorMCTrackId(wirehit))
                    allTrackIds.add(trackId)

                all_row = dict((trackId, 0) for trackId in allTrackIds)
                for wirehit in \
                    basf2tools.CDCGenericHitCollectionIterator(wirehit_collection.obj()):
                    trackId = int(mcLookUp.getMajorMCTrackId(wirehit))

                    hitCountForTrackId = all_row[trackId]
                    hitCountForTrackId = hitCountForTrackId + 1
                    all_row[trackId] = hitCountForTrackId

        # get ride of the minor track ids for better overview
                minorTrackIds = set()
        # for trackId, count in all_row.iteritems():
        #  if count < 3:
        #    minorTrackIds.add(trackId)

                majorTrackIds = allTrackIds - minorTrackIds

                showTrackIds = sorted(majorTrackIds)
                outputRow = lambda row: '|'.join('%(trackCount)5d'
                        % {'trackCount': row[iTrack]} for iTrack in
                        showTrackIds)
        # print all_row
                head_row = dict((trackId, trackId) for trackId in showTrackIds)
                print outputRow(head_row)
                print outputRow(all_row)
                print

                for (iTrack, track) in enumerate(track_collection):
                    confusion_row = dict((trackId, 0) for trackId in
                            allTrackIds)
                    for hit in \
                        basf2tools.CDCGenericHitCollectionIterator(track):
                        wirehit = hit.getWireHit()
                        mcITrack = mcLookUp.getMajorMCTrackId(wirehit)
                        if mcITrack not in confusion_row:
                            confusion_row[mcITrack] = 0

                        hitCountForTrackId = confusion_row[mcITrack]
                        hitCountForTrackId = hitCountForTrackId + 1
                        confusion_row[mcITrack] = hitCountForTrackId

                    sparse_confusion_matrix.append(confusion_row)
                    print outputRow(confusion_row)

    def endRun(self):
        print 'endRun()'

    def terminate(self):
        print 'terminate()'


