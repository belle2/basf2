#!/usr/bin/env python
# -*- coding: utf-8 -*-

from basf2 import *


def add_reconstruction(path, components=None):
    """
    This function adds the standard reconstruction modules to a path.
    """

    # tracking
    if components == None or 'PXD' in components or 'SVD' in components \
        or 'CDC' in components:

        # find MCTracks in CDC, SVD, and PXD
        mc_trackfinder = register_module('MCTrackFinder')
        if components == None or 'PXD' in components:
            mc_trackfinder.param('UsePXDHits', 1)
        if components == None or 'SVD' in components:
            mc_trackfinder.param('UseSVDHits', 1)
        if components == None or 'CDC' in components:
            mc_trackfinder.param('UseCDCHits', 1)
        path.add_module(mc_trackfinder)

        # track fitting
        trackfitter = register_module('GenFitter')
        path.add_module(trackfitter)

        # track extrapolation
        ext = register_module('Ext')
        ext.param('GFTracksColName', 'GFTracks')  # input to ext
        ext.param('ExtTrackCandsColName', 'ExtTrackCands')  # output from ext
        ext.param('ExtRecoHitsColName', 'ExtRecoHits')  # output from ext
        path.add_module(ext)

    # TOP reconstruction
    if components == None or 'TOP' in components:
        top_rec = register_module('TOPReco')
        path.add_module(top_rec)

    # ARICH reconstruction
    if components == None or 'ARICH' in components:
        arich_rec = register_module('ARICHRec')
        path.add_module(arich_rec)

    # ECL reconstruction
    if components == None or 'ECL' in components:

        # shower reconstruction
        ecl_shower_rec = register_module('ECLRecCR')
        path.add_module(ecl_shower_rec)

        # gamma reconstruction
        gamma_rec = register_module('ECLRecGamma')
        param_gamma_rec = {
            'gammaEnergyCut': 0.02,
            'gammaE9o25Cut': 0.75,
            'gammaWidthCut': 6.0,
            'gammaNhitsCut': 0,
            }
        gamma_rec.param(param_gamma_rec)
        path.add_module(gamma_rec)

        # pi0 reconstruction
        pi0_rec = register_module('ECLRecPi0')
        path.add_module(pi0_rec)

    # EKLM reconstruction
    if components == None or 'EKLM' in components:
        eklm_rec = register_module('EKLMReconstruction')
        path.add_module(eklm_rec)


