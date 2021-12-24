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


def set_weightFileIdentifier(path, weightFileIdentifier):
    """Set weightFileIdentifier parameter of first python basf2 module named VTXBgRemover."""

    for e in path.modules():
        if e.name() == "VTXBgRemover":
            e.m_identifier = weightFileIdentifier
            return True

    return False


def add_vtx_bg_remover(
    path,
    inputStoreArrayName,
    outputStoreArrayName,
    vtx_bg_cut=0.0,
    weightFileIdentifier='vxdtf2_mva_bgr',
):

    bg_remover = VTXBgRemover(
        trackCandidatesColumnName=inputStoreArrayName,
        weightFileIdentifier=weightFileIdentifier,
    )
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
    trainingDataOutputName="train.root",
    trackCandidatesColumnName="VTXRecoTracks",
):

    # Data collection for training
    vtx_collector = VTXTrackCollector(
        trainingDataOutputName=trainingDataOutputName,
        trackCandidatesColumnName=trackCandidatesColumnName
    )
    path.add_module(vtx_collector)
