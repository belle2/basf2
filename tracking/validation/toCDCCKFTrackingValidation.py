#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
  <contact>glazov@mail.desy.de</contact>
  <input>EvtGenSimNoBkg.root</input>
  <output>toCDCCKFTrackingValidation.root</output>
  <description>This module validates ToCDCCKF module using Y(4S) runs.</description>
</header>
"""

VALIDATION_OUTPUT_FILE = 'toCDCCKFTrackingValidation.root'
N_EVENTS = 1000
ACTIVE = True

import basf2
basf2.set_random_seed(1337)

import logging
import tracking
from tracking.validation.run import TrackingValidationRun


class toCDCCKF(TrackingValidationRun):
    n_events = N_EVENTS
    #: Generator to be used in the simulation (-so)
    generator_module = 'generic'
    root_input_file = '../EvtGenSimNoBkg.root'

    @staticmethod
    def finder_module(path):
        path.add_module('SetupGenfitExtrapolation',
                        energyLossBrems=False, noiseBrems=False)

        tracking.add_svd_reconstruction(path)
        tracking.add_vxd_track_finding_vxdtf2(path, reco_tracks="RecoTracksSVD", components=["SVD"])
        path.add_module("DAFRecoFitter", recoTracksStoreArrayName="RecoTracksSVD")

        path.add_module("TFCDC_WireHitPreparer",
                        wirePosition="aligned",
                        useSecondHits=False,
                        flightTimeEstimation="outwards")
        path.add_module("ToCDCCKF",
                        inputWireHits="CDCWireHitVector",
                        inputRecoTrackStoreArrayName="RecoTracksSVD",
                        relatedRecoTrackStoreArrayName="CKFCDCRecoTracks",
                        relationCheckForDirection="backward",
                        outputRecoTrackStoreArrayName="CKFCDCRecoTracks",
                        outputRelationRecoTrackStoreArrayName="RecoTracksSVD",
                        writeOutDirection="backward",
                        stateBasicFilterParameters={"maximalHitDistance": 0.75},
                        stateExtrapolationFilterParameters={"extrapolationDirection": "forward"},
                        pathFilter="arc_length")

        path.add_module("RelatedTracksCombiner",
                        CDCRecoTracksStoreArrayName="CKFCDCRecoTracks",
                        VXDRecoTracksStoreArrayName="RecoTracksSVD",
                        recoTracksStoreArrayName="RecoTracks")

        path.add_module("DAFRecoFitter", recoTracksStoreArrayName="RecoTracks")

        path.add_module('TrackCreator', recoTrackColName='RecoTracks')

    tracking_coverage = {
        'UsePXDHits': False,
        'UseSVDHits': True,
        'UseCDCHits': True,
        'WhichParticles': [],
    }

    # Already fitted in the finder_module
    fit_tracks = False
    use_fit_information = True
    pulls = True
    resolution = True
    output_file_name = VALIDATION_OUTPUT_FILE


def main():
    validation_run = toCDCCKF()
    validation_run.configure_and_execute_from_commandline()


if __name__ == '__main__':
    logging.basicConfig(level=logging.INFO)
    if ACTIVE:
        main()
