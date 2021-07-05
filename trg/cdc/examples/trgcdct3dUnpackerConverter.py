#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
# -----------------------------------------------------------------------------------
#
#                        CDCTRGT3D Unpacker and Converter
#
#    usage : %> basf2 trgcdct3dUnpackerConverter -i [input sroot file name] -o [output root file name]
#
# -----------------------------------------------------------------------------------

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################
import basf2
import ROOT.Belle2


class is2DSkim(basf2.Module):
    """check the 2D skim with Tracks StoreArray"""

    def event(self):
        """event function of the is2DSkim class"""
        Tracks = ROOT.Belle2.PyStoreArray("Tracks")
        TRGGRLT3DUnpackerStores = ROOT.Belle2.PyStoreArray("TRGCDCT3DUnpackerStores")

        isTwoTrack = False
        if len(Tracks) == 2:
            isTwoTrack = True

        areTracksClean = True
        for track in Tracks:
            trackResult = track.getTrackFitResultWithClosestMass(ROOT.Belle2.Const.pion)
            if trackResult.getPValue() > 0.1:
                areTracksClean = False

        enoughHits = True
        for track in Tracks:
            trackResult = track.getTrackFitResultWithClosestMass(ROOT.Belle2.Const.pion)
            nHitStSl = 0
            nHitAxSl = 0
            for iAx in range(5):
                if (trackResult.getHitPatternCDC().getSLayerNHits(2 * iAx) > 3):
                    nHitAxSl += 1
            for iSt in range(4):
                if (trackResult.getHitPatternCDC().getSLayerNHits(2 * iSt + 1) > 3):
                    nHitStSl += 1
            if nHitStSl < 3:
                enoughHits = False
            if nHitAxSl < 4:
                enoughHits = False

        is2D = False
        for iClk, data in enumerate(TRGGRLT3DUnpackerStores):
            if (data.m_t2d_fnf != 0):
                is2D = True

        self.return_value(isTwoTrack and is2D and areTracksClean and enoughHits)


if __name__ == '__main__':

    basf2.use_central_database("development")
    # Hot fix for CDCFudgeFactorFromSigma
    basf2.use_local_database(ROOT.Belle2.FileSystem.findFile("data/trg/cdc/db_CDCFudgeFactorsForSigma.txt"), "localdb")

    empty_path = basf2.create_path()
    skim = basf2.register_module(is2DSkim())
    skim.if_value('=0', empty_path, basf2.AfterConditionPath.END)

    # Create main path
    main = basf2.create_path()
    # Add modules to main path
    main.add_module('RootInput')
    main.add_module('TRGCDCT3DUnpacker')
    main.add_module(skim)
    main.add_module('Gearbox')
    main.add_module('TRGCDCT3DConverter',
                    hitCollectionName='FirmCDCTriggerSegmentHits',
                    addTSToDatastore=True,
                    EventTimeName='FirmBinnedEventT0',
                    addEventTimeToDatastore=True,
                    inputCollectionName='FirmTRGCDC2DFinderTracks',
                    add2DFinderToDatastore=True,
                    outputCollectionName='FirmTRGCDC3DFitterTracks',
                    add3DToDatastore=True,
                    fit3DWithTSIM=0,
                    firmwareResultCollectionName='TRGCDCT3DUnpackerStores',
                    isVerbose=0)
    main.add_module(
        'RootOutput',
        branchNames=[
            'TRGCDCT3DUnpackerStores',
            'FirmCDCTriggerSegmentHits',
            'FirmBinnedEventT0',
            'FirmTRGCDC2DFinderTracks',
            'FirmTRGCDC3DFitterTracks'])
    # Process all events
    basf2.process(main)
    print(basf2.statistics)
