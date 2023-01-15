##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################


import numpy as np
import pandas as pd
import math
from ROOT import Belle2
from ROOT.Math import XYZVector

scales_vxd_hits = {
    'track_id': 1.0,
    'layer': 5,
    'r': 14.0,
    'phi': math.pi,
    'z': 50,
    'charge': 50,
    'seedCharge': 40,
    'sizeU': 3,
    'sizeV': 3}
scales_cdc_hits = {'track_id': 1.0, 'layer': 55, 'r': 100.0, 'phi': math.pi, 'z': 50, 'tdc': 500}
scales_cdc_tracks = {
    'nhits': 56.0,
    'seed_pT': 0.3,
    'seed_tanLambda': 1.0,
    'seed_pMag': 0.3,
    'q': 1.0,
    'first_layer': 56,
    'last_layer': 56}
scales_vxd_tracks = {
    'nhits': 5.0,
    'seed_pT': 0.3,
    'seed_tanLambda': 1.0,
    'seed_pMag': 0.3,
    'q': 1.0,
    'first_layer': 5,
    'last_layer': 5}


def get_array(df, scales, use_scales=True):
    """Returns array with scaled features."""
    feature_names = list(scales.keys())
    feature_scales = list(scales.values())

    if use_scales:
        X = (df[feature_names].values / feature_scales).astype(np.float32)
    else:
        X = df[feature_names].values

    return X


def empty_event_data():
    """Returns empty event data tables."""

    # Initialze variables to be returned
    cdc_hits = {'track_id': [], 'hit_id': [], 'layer': [], 'r': [], 'phi': [], 'z': [], 'tdc': []}
    vxd_hits = {
        'track_id': [],
        'hit_id': [],
        'layer': [],
        'r': [],
        'phi': [],
        'z': [],
        'charge': [],
        'seedCharge': [],
        'sizeU': [],
        'sizeV': []}

    cdc_tracks = {
        'track_id': [],
        'nhits': [],
        'seed_pT': [],
        'seed_tanLambda': [],
        'seed_pMag': [],
        'q': [],
        'first_layer': [],
        'last_layer': []}
    vxd_tracks = {
        'track_id': [],
        'nhits': [],
        'seed_pT': [],
        'seed_tanLambda': [],
        'seed_pMag': [],
        'q': [],
        'first_layer': [],
        'last_layer': []}

    return cdc_hits, cdc_tracks, vxd_hits, vxd_tracks


def extract_event_data(cdcTracks, vxdTracks):
    """
    Returns event data for vtx to cdc merging and cleaning
    """

    # Create an empty event structure
    cdc_hits, cdc_tracks, vxd_hits, vxd_tracks = empty_event_data()

    for vxdTrack in vxdTracks:

        track_id = vxdTrack.getArrayIndex()

        momentum = vxdTrack.getMomentumSeed()
        seed_pT = momentum.Rho()
        seed_pmag = math.sqrt(momentum.X()**2 + momentum.Y()**2 + momentum.Z()**2)
        seed_tanLambda = np.divide(1.0, math.tan(momentum.Theta()))
        layers = [hit.getSensorID().getLayerNumber() for hit in vxdTrack.getSortedVTXHitList()]

        vxd_tracks['track_id'].append(track_id)
        vxd_tracks['nhits'].append(len(vxdTrack.getSortedVTXHitList()))
        vxd_tracks['seed_pT'].append(seed_pT)
        vxd_tracks['seed_pMag'].append(seed_pmag)
        vxd_tracks['seed_tanLambda'].append(seed_tanLambda)
        vxd_tracks['q'].append(vxdTrack.getChargeSeed())
        vxd_tracks['first_layer'].append(layers[0])
        vxd_tracks['last_layer'].append(layers[-1])

        for hit in vxdTrack.getSortedVTXHitList():

            hit_id = hit.getArrayIndex()
            layer = hit.getSensorID().getLayerNumber()
            sensor_info = Belle2.VXD.GeoCache.get(hit.getSensorID())
            position = sensor_info.pointToGlobal(XYZVector(hit.getU(), hit.getV(), 0), True)

            # Calculate derived hits variables
            r = np.sqrt(position.X()**2 + position.Y()**2)
            phi = np.arctan2(position.Y(), position.X())

            vxd_hits['r'].append(r)
            vxd_hits['phi'].append(phi)
            vxd_hits['z'].append(position.Z())
            vxd_hits['layer'].append(layer)
            vxd_hits['hit_id'].append(hit_id)
            vxd_hits['track_id'].append(track_id)

            vxd_hits['charge'].append(hit.getCharge())
            vxd_hits['seedCharge'].append(hit.getSeedCharge())
            vxd_hits['sizeU'].append(hit.getUSize())
            vxd_hits['sizeV'].append(hit.getVSize())

    for cdcTrack in cdcTracks:

        track_id = cdcTrack.getArrayIndex()

        momentum = cdcTrack.getMomentumSeed()
        seed_pT = momentum.Rho()
        seed_pmag = math.sqrt(momentum.X()**2 + momentum.Y()**2 + momentum.Z()**2)
        seed_tanLambda = np.divide(1.0, math.tan(momentum.Theta()))
        layers = [hit.getICLayer() for hit in cdcTrack.getSortedCDCHitList()]

        cdc_tracks['track_id'].append(track_id)
        cdc_tracks['nhits'].append(len(cdcTrack.getSortedCDCHitList()))
        cdc_tracks['seed_pT'].append(seed_pT)
        cdc_tracks['seed_pMag'].append(seed_pmag)
        cdc_tracks['seed_tanLambda'].append(seed_tanLambda)
        cdc_tracks['q'].append(cdcTrack.getChargeSeed())
        cdc_tracks['first_layer'].append(layers[0])
        cdc_tracks['last_layer'].append(layers[-1])

        for hit in cdcTrack.getSortedCDCHitList():

            hit_id = hit.getArrayIndex()
            layer = hit.getICLayer()

            WireHit = Belle2.TrackFindingCDC.CDCWireHit(hit)
            wirePos = WireHit.getRefPos3D()

            wireX = wirePos.x()
            wireY = wirePos.y()
            wireZ = wirePos.z()
            wireT = hit.getTDCCount() - 5000
            r = np.sqrt(wireX**2 + wireY**2)
            phi = np.arctan2(wireY, wireX)

            cdc_hits['layer'].append(layer)
            cdc_hits['hit_id'].append(hit_id)
            cdc_hits['track_id'].append(track_id)
            cdc_hits['r'].append(r)
            cdc_hits['phi'].append(phi)
            cdc_hits['z'].append(wireZ)
            cdc_hits['tdc'].append(wireT)

    cdc_hits = pd.DataFrame(cdc_hits)
    cdc_tracks = pd.DataFrame(cdc_tracks)
    vxd_hits = pd.DataFrame(vxd_hits)
    vxd_tracks = pd.DataFrame(vxd_tracks)

    return cdc_hits, cdc_tracks, vxd_hits, vxd_tracks
