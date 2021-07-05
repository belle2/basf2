##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import basf2
from iov_conditional import make_conditional_at


def add_ckf_based_merger(path, cdc_reco_tracks, svd_reco_tracks, use_mc_truth=False, direction="backward"):
    """
    Convenience function to add the SVD track finding using VXDTF2 and the merger based on the CKF to the path.
    :param path: The path to add the module to
    :param cdc_reco_tracks: The name of the already created CDC Reco Tracks
    :param svd_reco_tracks: The name of the already created CDC Reco Tracks
    :param use_mc_truth: Use the MC information in the CKF
    :param direction: where to extrapolate to. Valid options are forward and backward
    """
    # The CDC tracks need to be fitted
    path.add_module("DAFRecoFitter", recoTracksStoreArrayName=cdc_reco_tracks)

    if use_mc_truth:
        # MC CKF needs MC matching information
        path.add_module("MCRecoTracksMatcher", UsePXDHits=False, UseSVDHits=True, UseCDCHits=False,
                        mcRecoTracksStoreArrayName="MCRecoTracks",
                        prRecoTracksStoreArrayName=svd_reco_tracks)

        result_filter = "truth_svd_cdc_relation"
        result_filter_parameters = {}
    else:
        result_filter = "mva_with_relations"
        result_filter_parameters = {"cut": 0.6}

    if direction == "forward":
        reverse_seed = True
    else:
        reverse_seed = False

    path.add_module("CDCToSVDSeedCKF",
                    advanceHighFilterParameters={"direction": direction},

                    fromRelationStoreArrayName=cdc_reco_tracks,
                    toRelationStoreArrayName=svd_reco_tracks,

                    writeOutDirection=direction,

                    inputRecoTrackStoreArrayName=cdc_reco_tracks,
                    relatedRecoTrackStoreArrayName=svd_reco_tracks,
                    relationCheckForDirection=direction,
                    cdcTracksStoreArrayName=cdc_reco_tracks,
                    vxdTracksStoreArrayName=svd_reco_tracks,

                    firstHighFilterParameters={"direction": direction},
                    reverseSeed=reverse_seed,

                    filter=result_filter,
                    filterParameters=result_filter_parameters
                    ).set_name(f"CDCToSVDSeedCKF_{direction}")


def add_pxd_ckf(path, svd_cdc_reco_tracks, pxd_reco_tracks, use_mc_truth=False, filter_cut=0.03,
                overlap_cut=0.2, use_best_seeds=10, use_best_results=2, direction="backward"):
    """
    Convenience function to add the PXD ckf to the path.
    :param path: The path to add the module to
    :param svd_cdc_reco_tracks: The name of the already created SVD+CDC reco tracks
    :param pxd_reco_tracks: The name to output the PXD reco tracks to
    :param use_mc_truth: Use the MC information in the CKF
    :param filter_cut: CKF parameter for MVA state filter
    :param overlap_cut: CKF parameter for MVA overlap filter.
    :param use_best_results: CKF parameter for useBestNInSeed
    :param use_best_seeds: CKF parameter for UseNStates
    :param direction: where to extrapolate to. Valid options are forward and backward
    """
    if "PXDSpacePointCreator" not in [m.name() for m in path.modules()]:
        path.add_module("PXDSpacePointCreator")

    path.add_module("DAFRecoFitter", recoTracksStoreArrayName=svd_cdc_reco_tracks)

    if direction == "forward":
        reverse_seed = True
    else:
        reverse_seed = False

    if use_mc_truth:
        path.add_module("MCRecoTracksMatcher", UsePXDHits=False, UseSVDHits=True, UseCDCHits=True,
                        mcRecoTracksStoreArrayName="MCRecoTracks",
                        prRecoTracksStoreArrayName=svd_cdc_reco_tracks)

        module_parameters = dict(
            firstHighFilter="truth",
            secondHighFilter="all",
            thirdHighFilter="all",

            filter="truth",
            useBestNInSeed=1
        )
    else:
        module_parameters = dict(
            firstHighFilterParameters={"cut": filter_cut, "identifier": "ckf_ToPXDStateFilter_1",
                                       "direction": direction},
            firstHighUseNStates=use_best_seeds,

            secondHighFilterParameters={"cut": filter_cut, "identifier": "ckf_ToPXDStateFilter_2"},
            secondHighUseNStates=use_best_seeds,

            thirdHighFilterParameters={"cut": filter_cut, "identifier": "ckf_ToPXDStateFilter_3"},
            thirdHighUseNStates=use_best_seeds,

            filterParameters={"cut": overlap_cut, "identifier": "ckf_PXDTrackCombination"},
            useBestNInSeed=use_best_results,
        )

    module_parameters["seedHitJumping"] = -1  # get from payload
    module_parameters["hitHitJumping"] = 0

    path.add_module("ToPXDCKF",
                    advanceHighFilterParameters={"direction": direction},

                    writeOutDirection=direction,

                    inputRecoTrackStoreArrayName=svd_cdc_reco_tracks,
                    relatedRecoTrackStoreArrayName=pxd_reco_tracks,
                    relationCheckForDirection=direction,

                    outputRecoTrackStoreArrayName=pxd_reco_tracks,
                    outputRelationRecoTrackStoreArrayName=svd_cdc_reco_tracks,

                    reverseSeed=reverse_seed,
                    reverseSeedState=reverse_seed,  # Parameter cannot be read twice within a module
                    **module_parameters).set_name(f"ToPXDCKF_{direction}")


def add_svd_ckf(path, cdc_reco_tracks, svd_reco_tracks, use_mc_truth=False, filter_cut=0.1,
                overlap_cut=0.2, use_best_results=5, use_best_seeds=10, direction="backward"):
    """
    Convenience function to add the SVD ckf to the path.
    :param path: The path to add the module to
    :param cdc_reco_tracks: The name of the already created CDC reco tracks
    :param svd_reco_tracks: The name to output the SVD reco tracks to
    :param use_mc_truth: Use the MC information in the CKF
    :param filter_cut: CKF parameter for MVA filter
    :param overlap_cut: CKF parameter for MVA overlap filter
    :param use_best_results: CKF parameter for useNResults
    :param use_best_seeds: CKF parameter for useBestNInSeed
    :param direction: where to extrapolate to. Valid options are forward and backward
    """
    if direction == "forward":
        reverse_seed = True
    else:
        reverse_seed = False

    if use_mc_truth:
        module_parameters = dict(
            firstHighFilter="truth",
            secondHighFilter="all",
            thirdHighFilter="all",

            filter="truth",
            useBestNInSeed=1
        )
    else:
        module_parameters = dict(
            firstHighFilterParameters={"identifier": "ckf_CDCSVDStateFilter_1", "cut": filter_cut,
                                       "direction": direction},
            firstHighUseNStates=use_best_seeds,

            secondHighFilterParameters={"identifier": "ckf_CDCSVDStateFilter_2", "cut": filter_cut},
            secondHighUseNStates=use_best_seeds,

            thirdHighFilterParameters={"identifier": "ckf_CDCSVDStateFilter_3", "cut": filter_cut},
            thirdHighUseNStates=use_best_seeds,

            filterParameters={"cut": overlap_cut, "identifier": "ckf_CDCToSVDResult"},
            useBestNInSeed=use_best_results,
        )

    path.add_module("CDCToSVDSpacePointCKF",
                    inputRecoTrackStoreArrayName=cdc_reco_tracks,
                    outputRecoTrackStoreArrayName=svd_reco_tracks,
                    outputRelationRecoTrackStoreArrayName=cdc_reco_tracks,
                    relatedRecoTrackStoreArrayName=svd_reco_tracks,

                    advanceHighFilterParameters={"direction": direction},
                    reverseSeed=reverse_seed,

                    writeOutDirection=direction,
                    relationCheckForDirection=direction,

                    seedHitJumping=1,
                    hitHitJumping=1,

                    **module_parameters).set_name(f"CDCToSVDSpacePointCKF_{direction}")


def add_cosmics_svd_ckf(path, cdc_reco_tracks, svd_reco_tracks, use_mc_truth=False, use_best_results=5,
                        use_best_seeds=10, direction="backward"):
    """
    Convenience function to add the SVD ckf to the path with cosmics settings valid for phase2 and 3.
    :param path: The path to add the module to
    :param cdc_reco_tracks: The name of the already created CDC reco tracks
    :param svd_reco_tracks: The name to output the SVD reco tracks to
    :param use_mc_truth: Use the MC information in the CKF
    :param use_best_results: CKF parameter for useNResults
    :param use_best_seeds: CKF parameter for useBestNInSeed
    :param direction: where to extrapolate to. Valid options are forward and backward
    """
    if direction == "forward":
        reverse_seed = True
    else:
        reverse_seed = False

    if use_mc_truth:
        module_parameters = dict(
            firstHighFilter="truth",
            secondHighFilter="all",
            thirdHighFilter="all",

            filter="truth",
            useBestNInSeed=1
        )
    else:
        module_parameters = dict(
            hitFilter="all",
            seedFilter="all",

            firstHighFilter="non_ip_crossing",
            firstHighFilterParameters={"direction": direction},
            firstHighUseNStates=0,

            secondHighFilter="residual",
            secondHighFilterParameters={},
            secondHighUseNStates=use_best_seeds,

            thirdHighFilter="residual",
            thirdHighFilterParameters={},
            thirdHighUseNStates=use_best_seeds,

            filter="weight",
            filterParameters={},
            useBestNInSeed=use_best_results,
        )

    path.add_module("CDCToSVDSpacePointCKF",
                    inputRecoTrackStoreArrayName=cdc_reco_tracks,
                    outputRecoTrackStoreArrayName=svd_reco_tracks,
                    outputRelationRecoTrackStoreArrayName=cdc_reco_tracks,
                    relatedRecoTrackStoreArrayName=svd_reco_tracks,

                    advanceHighFilterParameters={"direction": direction},
                    reverseSeed=reverse_seed,

                    writeOutDirection=direction,
                    relationCheckForDirection=direction,

                    seedHitJumping=3,
                    hitHitJumping=1,
                    **module_parameters).set_name(f"CDCToSVDSpacePointCKF_{direction}")


def add_cosmics_pxd_ckf(path, svd_cdc_reco_tracks, pxd_reco_tracks, use_mc_truth=False, use_best_results=5,
                        filter_cut=0.03, overlap_cut=0.2, use_best_seeds=10, direction="backward"):
    """
    Convenience function to add the PXD ckf to the path with cosmics settings valid for phase2 and 3.
    :param path: The path to add the module to
    :param svd_cdc_reco_tracks: The name of the already created CDC reco tracks
    :param pxd_reco_tracks: The name to output the SVD reco tracks to
    :param use_mc_truth: Use the MC information in the CKF
    :param use_best_results: CKF parameter for useNResults
    :param filter_cut: CKF parameter for MVA filter
    :param overlap_cut: CKF parameter for MVA overlap filter
    :param use_best_seeds: CKF parameter for useBestNInSeed
    :param direction: where to extrapolate to. Valid options are forward and backward
    """
    if "PXDSpacePointCreator" not in [m.name() for m in path.modules()]:
        path.add_module("PXDSpacePointCreator")

    path.add_module("DAFRecoFitter", recoTracksStoreArrayName=svd_cdc_reco_tracks)

    if direction == "forward":
        reverse_seed = True
    else:
        reverse_seed = False

    if use_mc_truth:
        path.add_module("MCRecoTracksMatcher", UsePXDHits=False, UseSVDHits=True, UseCDCHits=True,
                        mcRecoTracksStoreArrayName="MCRecoTracks",
                        prRecoTracksStoreArrayName=svd_cdc_reco_tracks)

        module_parameters = dict(
            firstHighFilter="truth",
            secondHighFilter="all",
            thirdHighFilter="all",

            filter="truth",
            useBestNInSeed=1
        )
    else:
        module_parameters = dict(
            hitFilter="all",
            seedFilter="all",
            preHitFilter="all",
            preSeedFilter="all",

            firstHighFilterParameters={"cut": filter_cut, "identifier": "ckf_ToPXDStateFilter_1",
                                       "direction": direction},
            firstHighUseNStates=use_best_seeds,

            secondHighFilterParameters={"cut": filter_cut, "identifier": "ckf_ToPXDStateFilter_2"},
            secondHighUseNStates=use_best_seeds,

            thirdHighFilterParameters={"cut": filter_cut, "identifier": "ckf_ToPXDStateFilter_3"},
            thirdHighUseNStates=use_best_seeds,

            filterParameters={"cut": overlap_cut, "identifier": "ckf_PXDTrackCombination"},
            useBestNInSeed=use_best_results,

            seedHitJumping=1,
            hitHitJumping=0,
        )

    path.add_module("ToPXDCKF",
                    advanceHighFilterParameters={"direction": direction},

                    writeOutDirection=direction,

                    inputRecoTrackStoreArrayName=svd_cdc_reco_tracks,
                    relatedRecoTrackStoreArrayName=pxd_reco_tracks,
                    relationCheckForDirection=direction,

                    outputRecoTrackStoreArrayName=pxd_reco_tracks,
                    outputRelationRecoTrackStoreArrayName=svd_cdc_reco_tracks,

                    reverseSeed=reverse_seed,
                    reverseSeedState=reverse_seed,
                    **module_parameters).set_name(f"ToPXDCKF_{direction}")
