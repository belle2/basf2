#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
  <contact>oliver.frost@desy.de</contact>
  <input>EvtGenSimNoBkg.root</input>
  <output>CDCLocalTrackingValidation.root</output>
  <description>This module validates that local track finding is capable of reconstructing tracks in Y(4S) runs.</description>
</header>
"""

VALIDATION_OUTPUT_FILE = 'CDCLocalTrackingValidation.root'
CONTACT = 'oliver.frost@desy.de'
N_EVENTS = 1000
ACTIVE = True

import basf2
basf2.set_random_seed(1337)

import logging

from tracking.validation.run import TrackingValidationRun


class CDCLocal(TrackingValidationRun):
    n_events = N_EVENTS
    root_input_file = '../EvtGenSimNoBkg.root'

    def finder_module(self, path):
        # path.add_module("WireHitTopologyPreparer",
        #                 flightTimeEstimation="outwards"
        #                 )
        # path.add_module("SegmentCreatorMCTruth")
        # path.add_module("SegmentFitter",
        #                 inputSegments="CDCRecoSegment2DVector",
        #                 )
        # path.add_module("TrackCreatorSingleSegments",
        #                 MinimalHitsForSingleSegmentTrackBySuperLayerId={ i : 1 for i in range(9) }
        #                 )
        # path.add_module("TrackExporter")

        # path.add_module("WireHitTopologyPreparer",
        #                 flightTimeEstimation="outwards")

        # path.add_module("SegmentCreatorMCTruth")
        # path.add_module("SegmentFitter",
        #                 inputSegments="CDCRecoSegment2DVector",
        #                 )
        # path.add_module("TrackFinderSegmentPairAutomaton",
        #                 # SegmentPairFilter="truth",
        #                 # SegmentPairRelationFilter="truth",
        #                 # TrackRelationFilter="truth",
        #                 SegmentPairFilter="realistic",
        #                 SegmentPairRelationFilter="simple",
        #                 )

        # path.add_module("TrackExporter")
        # return

        path.add_module('TrackFinderCDCAutomaton',
                        # WriteTracks=True,
                        # FacetFilter="truth",
                        # FacetRelationFilter="chi2",
                        # FacetRelationFilterParameters=dict(chi2Cut=130.0,     # 350.0
                        #                                   penaltyWidth=200.0  # 800.0
                        #                                   ),
                        # FacetRelationOnlyBest=1,
                        # SegmentRelationFilter="truth",
                        # SegmentRelationFilter="realistic",
                        # SegmentOrientation="curling",

                        # SegmentPairFilter="truth",
                        # SegmentPairRelationFilter="truth",
                        # TrackRelationFilter="truth",
                        # TrackOrientation="outwards",
                        # TrackOrientation="curling",
                        # SegmentOrientation="outwards",
                        # SegmentPairFilter="realistic",
                        # SegmentPairRelationFilter="realistic",
                        # SegmentPairRelationFilterParameters=dict(cut=0.08),
                        # SegmentPairRelationFilter="all",
                        # SegmentPairRelationOnlyBest=1,
                        # TrackRelationFilter="realistic",
                        # TrackRelationFilterParameters=dict(cut=0.80),
                        )

    tracking_coverage = {
        'UsePXDHits': False,
        'UseSVDHits': False,
        'UseCDCHits': True,
        'UseOnlyAxialCDCHits': False,
        # 'WhichParticles': ['CDC'],
        # 'EnergyCut': 0.1,
    }

    fit_geometry = None
    pulls = True
    contact = CONTACT
    output_file_name = VALIDATION_OUTPUT_FILE


def main():
    validation_run = CDCLocal()
    validation_run.configure_and_execute_from_commandline()

if __name__ == '__main__':
    logging.basicConfig(level=logging.INFO)
    if ACTIVE:
        main()
