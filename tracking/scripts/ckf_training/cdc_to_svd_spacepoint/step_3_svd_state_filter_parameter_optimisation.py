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


def create_path(
    bkg, inputfile, layer,
    records1_fname, records2_fname, records3_fname,
):
    path = basf2.create_path()
    path.add_module("RootInput",
                    inputFileNames=["/path/to/the/output/file.root"])

    path.add_module("Gearbox")
    path.add_module("Geometry")
    path.add_module("SetupGenfitExtrapolation")

    add_track_finding(path, reco_tracks="CDCRecoTracks", components=["CDC"], use_vxdtf2=True)

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
                    hitFilter="sensor",
                    seedFilter="distance",

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

    return path, {"max_event": 1000}


class ReconstructionTask(Basf2PathTask):
    inputfile = b2luigi.Parameter()
    layer = b2luigi.IntParameter

    def output(self):
        for record_fname in ["records1.root", "records2.root", "records3.root"]:
            yield self.add_to_output(record_fname)

    def create_path(self):
        return create_path(
            inputfile=self.inputfile,
            bkg=self.bkg,
            layer=self.layer,
            tracking=self.tracking,
            records1_fname=self.get_output_fname("records1.root"),
            records2_fname=self.get_output_fname("records2.root"),
            records3_fname=self.get_output_fname("records3.root"),
        )


class MainTask(b2luigi.WrapperTask):
    def requires(self):
        for layer in [3, 4, 5, 6, 7]:
            yield self.clone(
                ReconstructionTask,
                inputfile="EvtGen",
                layer=layer,
            )


if __name__ == "__main__":
    b2luigi.set_setting("result_dir", "/portal/ekpbms2/home/nbraun/results/studies/vxdCDCExtrapolator/layerwise/")
    b2luigi.set_setting("batch_system", "htcondor")
    b2luigi.set_setting("env_script", "~/path/to/basf2/setup/script.sh")
    b2luigi.process(MainTask, workers=5)
