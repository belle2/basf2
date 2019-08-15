#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
  <contact>simon.kurz@desy.de</contact>
  <input>EvtGenSimNoBkg.root</input>
  <output>toCDCfromEclCKFTrackingValidation.root</output>
  <description>This module validates toCDCfromEclCKF module using Y(4S) runs.</description>
</header>
"""

VALIDATION_OUTPUT_FILE = 'toCDCfromEclCKFTrackingValidation_expert.root'
N_EVENTS = 10000
ACTIVE = True

import basf2
basf2.set_random_seed(1337)

import logging
import tracking
from tracking.validation.run import TrackingValidationRun
import reconstruction


class toCDCfromEclCKF(TrackingValidationRun):
    n_events = N_EVENTS
    #: Generator to be used in the simulation (-so)
    generator_module = 'generic'
    root_input_file = '../EvtGenSimNoBkg.root'

    @staticmethod
    def finder_module(path):
        path.add_module('SetupGenfitExtrapolation',
                        energyLossBrems=False, noiseBrems=False)

        path.add_module("DAFRecoFitter", recoTracksStoreArrayName="RecoTracksSVD")

        reconstruction.add_ecl_modules(path)

        # needed for truth matching
        # tracking.add_mc_track_finding(path)
        # same thing as below
        path.add_module('TrackFinderMCTruthRecoTracks',
                        RecoTracksStoreArrayName="MCRecoTracks",
                        WhichParticles=[],
                        UsePXDHits=False,
                        UseSVDHits=False,
                        UseCDCHits=True)

        path.add_module("TFCDC_WireHitPreparer",
                        wirePosition="aligned",
                        useSecondHits=False,
                        flightTimeEstimation="outwards")

        path.add_module("ToCDCFromEclCKF",
                        inputWireHits="CDCWireHitVector",
                        minimalEnRequirementCluster=0.3,
                        eclSeedRecoTrackStoreArrayName='EclSeedRecoTracks',
                        hitFindingDirection="backward",
                        outputRecoTrackStoreArrayName="CKFCDCRecoTracksFromEcl",
                        outputRelationRecoTrackStoreArrayName="EclSeedRecoTracks",
                        writeOutDirection="forward",
                        stateBasicFilterParameters={"maximalHitDistance": 7.5, "maximalHitDistanceEclSeed": 75.0},
                        stateExtrapolationFilterParameters={"direction": "backward"},
                        pathFilter="arc_length_fromEcl",
                        inputECLshowersStoreArrayName="ECLShowers",
                        trackFindingDirection="backward",
                        seedComponent="ECL"
                        )

        path.add_module("ToCDCCKF",
                        inputWireHits="CDCWireHitVector",
                        inputRecoTrackStoreArrayName="CKFCDCRecoTracksFromEcl",
                        relatedRecoTrackStoreArrayName="RecoTracks",
                        relationCheckForDirection="backward",
                        outputRecoTrackStoreArrayName="RecoTracks",
                        outputRelationRecoTrackStoreArrayName="CKFCDCRecoTracksFromEcl",
                        writeOutDirection="backward",
                        stateBasicFilterParameters={"maximalHitDistance": 0.75},
                        stateExtrapolationFilterParameters={"direction": "forward"},
                        pathFilter="arc_length",
                        seedComponent="ECL"
                        )

        path.add_module("DAFRecoFitter", recoTracksStoreArrayName="RecoTracks")

        path.add_module('TrackCreator', recoTrackColName='RecoTracks')

        path.add_module("MCRecoTracksMatcher",
                        mcRecoTracksStoreArrayName="MCRecoTracks",
                        prRecoTracksStoreArrayName="RecoTracks",
                        UseCDCHits=True,
                        UseSVDHits=False,
                        UsePXDHits=False)

    tracking_coverage = {
        'UsePXDHits': False,
        'UseSVDHits': False,
        'UseCDCHits': True,
        'WhichParticles': [],
    }

    # Already fitted in the finder_module
    fit_tracks = False
    use_fit_information = True
    pulls = True
    resolution = True
    output_file_name = VALIDATION_OUTPUT_FILE
    extended = True


def main():
    validation_run = toCDCfromEclCKF()
    validation_run.configure_and_execute_from_commandline()


if __name__ == '__main__':
    logging.basicConfig(level=logging.INFO)

    if ACTIVE:
        main()
