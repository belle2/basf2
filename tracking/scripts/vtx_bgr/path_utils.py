##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import basf2 as b2

from vtx_bgr.modules.vtx_bg_remover import VTXBgRemover
from vtx_bgr.modules.collector import VTXTrackCollector


def add_vtx_bg_remover(
    path,
    inputStoreArrayName,
    outputStoreArrayName,
    vtx_bg_cut=0.0,
):

    bg_remover = VTXBgRemover(trackCandidatesColumnName=inputStoreArrayName)
    path.add_module(bg_remover)

    tracks_cutter = b2.register_module('RecoTracksCutter')
    tracks_cutter.param("inputStoreArrayName", inputStoreArrayName)
    tracks_cutter.param("outputStoreArrayName", outputStoreArrayName)
    tracks_cutter.param("onlyFittedTracks", False)
    tracks_cutter.param("onlyGoodQITracks", True)
    tracks_cutter.param("minRequiredQuality", vtx_bg_cut)
    path.add_module(tracks_cutter)


def add_vtx_bg_collector(
    path,
    output_file_name="train.root",
    trackCandidatesColumnName="VTXRecoTracks",
):

    # Data collection for training
    vtx_collector = VTXTrackCollector(
        output_file_name=output_file_name,
        trackCandidatesColumnName=trackCandidatesColumnName
    )
    path.add_module(vtx_collector)
