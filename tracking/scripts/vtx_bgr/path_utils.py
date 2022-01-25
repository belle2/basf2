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
    """
    Set weightFileIdentifier parameter of first python basf2 module named VTXBgRemover.

    :param path: The path where to search VTXBgRemover module
    :param weightFileIdentifier(str): Identifier for classifier in condition data base
    """

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
    weightFileIdentifier='vxdtf2_vtx_bgr_mva',
):
    """
    This function adds all modules for the VTX background fake track remover to the path.

    :param path: The path to add the fake track remover modules to
    :param inputStoreArrayName: Store array name for Belle2.RecoTracks built from VTX hits
    :param outputStoreArrayName: Store array name for cleaned Belle2.RecoTracks after removing fakes
    :param vtx_bg_cut: Cut value in range [0,1] applied to classifier output
    :param weightFileIdentifier: Identifier for classifier in condition data base
    """

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
    """
    This function adds all modules for the VTX background collector to the path.

    :param path: The path to add the collector modules to
    :param trainingDataOutputName: Name of root file with collected training data
    :param trackCandidatesColumnName: Store array name for Belle2.RecoTracks which needs fake track cleaning
    """

    # Data collection for training
    vtx_collector = VTXTrackCollector(
        trainingDataOutputName=trainingDataOutputName,
        trackCandidatesColumnName=trackCandidatesColumnName
    )
    path.add_module(vtx_collector)
