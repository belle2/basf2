#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""
<header>
  <contact>christian.wessel@desy.de</contact>
  <input>VTXEvtGenSimNoBkg.root</input>
  <output>VTXToCDCCKFTrackingValidation.root</output>
  <description>This module validates VTXToCDCCKF module using Y(4S) runs.</description>
</header>
"""

from tracking.validation.run import TrackingValidationRun
from tracking.path_utils import add_hit_preparation_modules, add_vtx_track_finding_vxdtf2
import logging
from basf2 import set_log_level, LogLevel
import basf2
VALIDATION_OUTPUT_FILE = 'vtxToCDCCKFTrackingValidation.root'
N_EVENTS = 1000
ACTIVE = True

basf2.set_random_seed(1337)
basf2.set_log_level(LogLevel.ERROR)


class vtxToCDCCKF(TrackingValidationRun):
    """Validate the ToCDCCKF Kalman finder/filter algorithm with Y(4S) events"""
    #: number of events to generate
    n_events = N_EVENTS
    #: Generator to be used in the simulation (-so)
    generator_module = 'generic'
    #: no background overlay
    root_input_file = '../VTXEvtGenSimNoBkg.root'

    @staticmethod
    def finder_module(path):
        """Add the VTXToCDCCKF module and related modules to the basf2 path"""

        path.add_module('SetupGenfitExtrapolation',
                        energyLossBrems=False, noiseBrems=False)

        add_hit_preparation_modules(path, components=["VTX"], useVTX=True)
        add_vtx_track_finding_vxdtf2(path, reco_tracks="RecoTracksVTX", components=["VTX"])
        path.add_module("DAFRecoFitter", recoTracksStoreArrayName="RecoTracksVTX")

        path.add_module("TFCDC_WireHitPreparer",
                        wirePosition="aligned",
                        useSecondHits=False,
                        flightTimeEstimation="outwards")
        path.add_module("ToCDCCKF",
                        inputWireHits="CDCWireHitVector",
                        inputRecoTrackStoreArrayName="RecoTracksVTX",
                        relatedRecoTrackStoreArrayName="CKFCDCRecoTracks",
                        relationCheckForDirection="backward",
                        outputRecoTrackStoreArrayName="CKFCDCRecoTracks",
                        outputRelationRecoTrackStoreArrayName="RecoTracksVTX",
                        writeOutDirection="backward",
                        stateBasicFilterParameters={"maximalHitDistance": 0.2},
                        stateExtrapolationFilterParameters={"direction": "forward"},
                        pathFilter="arc_length",
                        seedComponent="VTX")

        path.add_module("RelatedTracksCombiner",
                        CDCRecoTracksStoreArrayName="CKFCDCRecoTracks",
                        VXDRecoTracksStoreArrayName="RecoTracksVTX",
                        recoTracksStoreArrayName="RecoTracks")

        path.add_module("DAFRecoFitter", recoTracksStoreArrayName="RecoTracks")

        path.add_module('TrackCreator', recoTrackColName='RecoTracks')

    #: Define the user parameters for the track-finding module
    tracking_coverage = {
        'UsePXDHits': False,
        'UseSVDHits': False,
        'UseCDCHits': True,
        'UseVTXHits': True,
        'WhichParticles': [],
        "UseReassignedHits": True,
        'UseOnlyBeforeTOP': True,
        'UseNLoops': 1
    }

    #: Already fitted in the finder_module
    fit_tracks = False
    #: But we need to tell the validation module to use the fit information
    use_fit_information = True
    #: Include pulls in the validation output
    pulls = True
    #: Include resolution information in the validation output
    resolution = True
    #: name of the output ROOT file
    output_file_name = VALIDATION_OUTPUT_FILE


def main():
    validation_run = vtxToCDCCKF()
    validation_run.configure_and_execute_from_commandline()


if __name__ == '__main__':
    logging.basicConfig(level=logging.INFO)
    if ACTIVE:
        main()
