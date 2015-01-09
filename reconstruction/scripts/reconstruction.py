#!/usr/bin/env python
# -*- coding: utf-8 -*-

from basf2 import *
from ROOT import Belle2


class PruneGenfitTracks(Module):

    """
    Removes hits from all genfit::Track objects.
    This needs to be done after the dE/dx PID.
    """

    ## See genfit::Track::prune() doc, empty string for no pruning
    m_flags = 'FL'

    def __init__(self):
        """
        Constructor to enable parallel processing for this module.
        """

        # need to call super() _if_ we reimplement the constructor
        super(PruneGenfitTracks, self).__init__()

        self.set_property_flags(ModulePropFlags.PARALLELPROCESSINGCERTIFIED)

    def event(self):
        """
        reimplement Module::event()
        """

        tracks = Belle2.PyStoreArray('GF2Tracks')
        for t in tracks:
            t.prune(self.m_flags)


def add_posttracking_reconstruction(path, components=None):
    """
    This function adds the standard reconstruction modules after tracking
    to a path.
    """

    # track extrapolation
    if components is None or 'CDC' in components:
        ext = register_module('Ext')
        path.add_module(ext)

    # TOP reconstruction
    if components is None or 'TOP' in components:
        top_rec = register_module('TOPReconstructor')
        path.add_module(top_rec)

    # ARICH reconstruction
    if components is None or 'ARICH' in components:
        arich_rec = register_module('ARICHReconstructor')
        path.add_module(arich_rec)

    # ECL reconstruction
    if components is None or 'ECL' in components:

        # shower reconstruction
        ecl_shower_rec = register_module('ECLReconstructor')
        path.add_module(ecl_shower_rec)

        # gamma reconstruction
        gamma_rec = register_module('ECLGammaReconstructor')
        path.add_module(gamma_rec)

        # pi0 reconstruction
        pi0_rec = register_module('ECLPi0Reconstructor')
        path.add_module(pi0_rec)

        # track shower matching
        ecl_track_match = register_module('ECLTrackShowerMatch')
        path.add_module(ecl_track_match)

        # electron ID
        electron_id = register_module('ECLElectronId')
        path.add_module(electron_id)

        # MC matching
        ecl_mc = register_module('ECLMCMatching')
        path.add_module(ecl_mc)

    # EKLM reconstruction
    if components is None or 'EKLM' in components:
        eklm_rec = register_module('EKLMReconstructor')
        path.add_module(eklm_rec)

        # K0L reconstruction
        eklm_k0l_rec = register_module('EKLMK0LReconstructor')
        path.add_module(eklm_k0l_rec)

    # BKLM reconstruction
    if components is None or 'BKLM' in components:
        bklm_rec = register_module('BKLMReconstructor')
        path.add_module(bklm_rec)

        # K0L reconstruction
        bklm_k0l_rec = register_module('BKLMK0LReconstructor')
        path.add_module(bklm_k0l_rec)

    # muon identification
    if components is None or 'BKLM' in components and 'EKLM' in components:
        muid = register_module('Muid')
        path.add_module(muid)

    # charged particle PID
    if components is None or 'SVD' in components or 'CDC' in components:
        mdstPID = register_module('MdstPID')
        path.add_module(mdstPID)


def add_tracking_reconstruction(path, components=None):
    """
    This function adds the standard reconstruction modules for tracking
    to a path.
    """

    if components and not ('SVD' in components or 'CDC' in components):
        return

    use_vxd = components is None or 'SVD' in components
    use_cdc = components is None or 'CDC' in components

    # Material effects for all track extrapolations
    material_effects = register_module('SetupGenfitExtrapolation')
    material_effects.param('whichGeometry', 'TGeo')
    path.add_module(material_effects)

    # CDC track finder: trasan
    if use_cdc:
        cdc_trackcands = ''
        if use_vxd:
            cdc_trackcands = 'CDCGFTrackCands'
        trackcands = cdc_trackcands
        cdc_trackfinder = register_module('Trasan')
        cdc_trackfinder.param('GFTrackCandidatesColName', cdc_trackcands)
        path.add_module(cdc_trackfinder)

    # VXD track finder
    if use_vxd:
        vxd_trackcands = ''
        if use_cdc:
            vxd_trackcands = 'VXDGFTrackCands'
        vxd_trackfinder = register_module('VXDTF')
        vxd_trackfinder.param('GFTrackCandidatesColName', vxd_trackcands)
        if components is not None and 'PXD' not in components:
            vxd_trackfinder.param('sectorSetup',
                                  ['secMapEvtGenAndPGunWithSVDGeo2p2OnR13760Nov2014SVDStd-moreThan500MeV_SVD'
                                  ,
                                  'secMapEvtGenAndPGunWithSVDGeo2p2OnR13760Nov2014SVDStd-125to500MeV_SVD'
                                  ,
                                  'secMapEvtGenAndPGunWithSVDGeo2p2OnR13760Nov2014SVDStd-30to125MeV_SVD'
                                  ])
            vxd_trackfinder.param('tuneCutoffs', 0.06)
        else:
            vxd_trackfinder.param('sectorSetup',
                                  ['secMapEvtGenAndPGunWithSVDGeo2p2OnR13760Nov2014VXDStd-moreThan500MeV_PXDSVD'
                                  ,
                                  'secMapEvtGenAndPGunWithSVDGeo2p2OnR13760Nov2014VXDStd-125to500MeV_PXDSVD'
                                  ,
                                  'secMapEvtGenAndPGunWithSVDGeo2p2OnR13760Nov2014VXDStd-30to125MeV_PXDSVD'
                                  ])
            vxd_trackfinder.param('tuneCutoffs', 0.22)
        path.add_module(vxd_trackfinder)

    # track merging
    if use_vxd and use_cdc:
        track_merger = register_module('MCTrackCandCombiner')
        track_merger.param('CDCTrackCandidatesColName', cdc_trackcands)
        track_merger.param('VXDTrackCandidatesColName', vxd_trackcands)
        path.add_module(track_merger)

    # track fitting
    trackfitter = register_module('GenFitter')
    path.add_module(trackfitter)

    # V0 finding
    v0finder = register_module('V0Finder')
    path.add_module(v0finder)

    # dE/dx PID
    dEdxPID = register_module('DedxPID')
    dEdxPID.param('useSVD', use_vxd)
    dEdxPID.param('useCDC', use_cdc)
    path.add_module(dEdxPID)

    # prune genfit tracks
    path.add_module(PruneGenfitTracks())


def add_mc_tracking_reconstruction(path, components=None):
    """
    This function adds the standard reconstruction modules for MC tracking
    to a path.
    """

    # tracking
    if components and not ('PXD' in components or 'SVD' in components or 'CDC'
                           in components):
        return

    # Material effects for all track extrapolations
    material_effects = register_module('SetupGenfitExtrapolation')
    material_effects.param('whichGeometry', 'TGeo')
    path.add_module(material_effects)

    # find MCTracks in CDC, SVD, and PXD
    mc_trackfinder = register_module('TrackFinderMCTruth')
    # Default setting in the module may be either way, therefore
    # accomodate both cases explicitly.
    if components is None or 'PXD' in components:
        mc_trackfinder.param('UsePXDHits', 1)
    else:
        mc_trackfinder.param('UsePXDHits', 0)
    if components is None or 'SVD' in components:
        mc_trackfinder.param('UseSVDHits', 1)
    else:
        mc_trackfinder.param('UseSVDHits', 0)
    if components is None or 'CDC' in components:
        mc_trackfinder.param('UseCDCHits', 1)
    else:
        mc_trackfinder.param('UseCDCHits', 0)
    path.add_module(mc_trackfinder)

    # track fitting
    trackfitter = register_module('GenFitter')
    path.add_module(trackfitter)

    # V0 finding
    v0finder = register_module('V0Finder')
    path.add_module(v0finder)

    # dE/dx PID
    dEdxPID = register_module('DedxPID')
    if components is not None and 'SVD' not in components:
        dEdxPID.param('useSVD', False)
    if components is not None and 'CDC' not in components:
        dEdxPID.param('useCDC', False)
    path.add_module(dEdxPID)

    # prune genfit tracks
    path.add_module(PruneGenfitTracks())


def add_reconstruction(path, components=None):
    """
    This function adds the standard reconstruction modules to a path.
    """

    # tracking
    add_tracking_reconstruction(path, components)

    # add further reconstruction modules
    add_posttracking_reconstruction(path, components)


def add_mc_reconstruction(path, components=None):
    """
    This function adds the standard reconstruction modules with MC tracking
    to a path.
    """

    # tracking
    add_mc_tracking_reconstruction(path, components)

    # add further reconstruction modules
    add_posttracking_reconstruction(path, components)


def add_mdst_output(
    path,
    mc=True,
    filename='mdst.root',
    additionalBranches=[],
    ):
    """
    This function adds the mdst output modules to a path.
    """

    output = register_module('RootOutput')
    output.param('outputFileName', filename)
    branches = [
        'Tracks',
        'V0s',
        'TrackFitResults',
        'PIDLikelihoods',
        'TracksToPIDLikelihoods',
        'ECLClusters',
        'ECLClustersToTracks',
        'KLMClusters',
        'KLMClustersToTracks',
        'TRGSummary',
        ]
    if mc:
        branches += ['MCParticles', 'TracksToMCParticles',
                     'ECLClustersToMCParticles', 'KLMClustersToMCParticles']
    branches += additionalBranches
    output.param('branchNames', branches)
    path.add_module(output)


