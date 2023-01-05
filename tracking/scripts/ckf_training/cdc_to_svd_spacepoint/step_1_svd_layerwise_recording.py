#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################
import basf2

from tracking import add_track_finding
import b2luigi
from b2luigi.basf2_helper import Basf2PathTask


def create_path(layer, records1_fname, records2_fname, records3_fname):
    path = basf2.create_path()
    path.add_module("RootInput",
                    inputFileName="/path/to/your/Trainingsample.root")

    path.add_module("Gearbox")
    path.add_module("Geometry")
    path.add_module("SetupGenfitExtrapolation")

    add_track_finding(path, reco_tracks="CDCRecoTracks", components=["CDC"])

    path.add_module('TrackFinderMCTruthRecoTracks',
                    RecoTracksStoreArrayName="MCRecoTracks",
                    WhichParticles=[],
                    UsePXDHits=True,
                    UseSVDHits=True,
                    UseCDCHits=True)

    path.add_module("MCRecoTracksMatcher", UsePXDHits=False, UseSVDHits=False, UseCDCHits=True,
                    mcRecoTracksStoreArrayName="MCRecoTracks",
                    prRecoTracksStoreArrayName="CDCRecoTracks")
    path.add_module("DAFRecoFitter", recoTracksStoreArrayName="CDCRecoTracks")

    path.add_module("SVDSpacePointCreator")

    path.add_module("CDCToSVDSpacePointCKF",
                    inputRecoTrackStoreArrayName="CDCRecoTracks",
                    outputRecoTrackStoreArrayName="VXDRecoTracks",
                    outputRelationRecoTrackStoreArrayName="CDCRecoTracks",
                    hitFilter="sensor",
                    seedFilter="distance",

                    relationCheckForDirection="backward",
                    reverseSeed=False,
                    writeOutDirection="backward",

                    firstHighFilter="truth",
                    firstEqualFilter="recording",
                    firstEqualFilterParameters={"treeName": "records1", "rootFileName":
                                                records1_fname, "returnWeight": 1.0},
                    firstLowFilter="none",
                    firstHighUseNStates=0,
                    firstToggleOnLayer=layer,

                    advanceHighFilter="advance",

                    secondHighFilter="truth",
                    secondEqualFilter="recording",
                    secondEqualFilterParameters={"treeName": "records2", "rootFileName":
                                                 records2_fname, "returnWeight": 1.0},
                    secondLowFilter="none",
                    secondHighUseNStates=0,
                    secondToggleOnLayer=layer,

                    updateHighFilter="fit",

                    thirdHighFilter="truth",
                    thirdEqualFilter="recording",
                    thirdEqualFilterParameters={"treeName": "records3", "rootFileName": records3_fname},
                    thirdLowFilter="none",
                    thirdHighUseNStates=0,
                    thirdToggleOnLayer=layer,

                    filter="none",
                    exportTracks=False,

                    enableOverlapResolving=False)

    return path  # , {"max_event": 1000}


class ReconstructionTask(Basf2PathTask):
    layer = b2luigi.IntParameter()

    def output(self):
        for record_fname in ["records1.root", "records2.root", "records3.root"]:
            yield self.add_to_output(record_fname)

    def create_path(self):
        return create_path(
            layer=self.layer,
            records1_fname=self.get_output_file_name("records1.root"),
            records2_fname=self.get_output_file_name("records2.root"),
            records3_fname=self.get_output_file_name("records3.root"),
        )


class MainTask(b2luigi.WrapperTask):
    def requires(self):
        for layer in [3, 4, 5, 6, 7]:
            yield self.clone(
                ReconstructionTask,
                layer=layer,
            )


if __name__ == "__main__":
    b2luigi.set_setting("result_dir", "/path/to/your/training/results")
    # b2luigi.set_setting("batch_system", "htcondor")
    b2luigi.set_setting("env_script", "/path/to/setup_basf2.sh")
    # b2luigi.set_setting("executable", ["python3"])
    b2luigi.process(MainTask(), workers=5, batch=False)
