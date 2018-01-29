"""
A set of common purose translators from complex framework objects to flat dictionaries
"""

import functools

import ROOT
ROOT.gSystem.Load("libtracking")
from ROOT import Belle2
from tracking.validation.tolerate_missing_key_formatter import TolerateMissingKeyFormatter

import math
import numpy as np

formatter = TolerateMissingKeyFormatter()


def format_crop_keys(peel_func):
    @functools.wraps(peel_func)
    def peel_func_formatted_keys(*args, key="{part_name}", **kwargs):
        crops = peel_func(*args, **kwargs, key=key)
        if key and hasattr(crops, 'items'):
            crops_with_formatted_keys = dict()
            for part_name, value in list(crops.items()):
                formatted_key = formatter.format(key, part_name=part_name)
                crops_with_formatted_keys[formatted_key] = value
            return crops_with_formatted_keys

        else:
            return crops

    return peel_func_formatted_keys


@format_crop_keys
def peel_mc_particle(mc_particle, key="{part_name}"):
    if mc_particle:
        helix = get_helix_from_mc_particle(mc_particle)
        momentum = mc_particle.getMomentum()
        vertex = mc_particle.getVertex()
        charge = mc_particle.getCharge()
        pdg_code = mc_particle.getPDG()
        is_primary = bool(mc_particle.hasStatus(Belle2.MCParticle.c_PrimaryParticle))

        decay_vertex = mc_particle.getDecayVertex()
        number_of_daughters = mc_particle.getNDaughters()
        status = mc_particle.getStatus()

        return dict(
            # At origin assuming perfect magnetic field
            d0_truth=helix.getD0(),
            phi0_truth=helix.getPhi0() % (2.0 * math.pi),
            omega_truth=helix.getOmega(),
            z0_truth=helix.getZ0(),
            tan_lambda_truth=helix.getTanLambda(),

            # At the vertex position
            pt_truth=momentum.Perp(),
            px_truth=momentum.X(),
            py_truth=momentum.Y(),
            pz_truth=momentum.Z(),
            x_truth=vertex.X(),
            y_truth=vertex.Y(),
            z_truth=vertex.Z(),

            decay_vertex_radius_truth=decay_vertex.Mag(),
            decay_vertex_x_truth=decay_vertex.X(),
            decay_vertex_y_truth=decay_vertex.Y(),
            decay_vertex_z_truth=decay_vertex.Z(),
            number_of_daughters_truth=number_of_daughters,
            status_truth=status,


            # MC Particle information
            charge_truth=charge,
            pdg_code_truth=pdg_code,
            is_primary=is_primary,
        )

    else:
        nan = float('nan')
        return dict(
            # At origin assuming perfect magnetic field
            omega_truth=nan,
            phi0_truth=nan,
            d0_truth=nan,
            z0_truth=nan,
            tan_lambda_truth=nan,

            # At the vertex position
            pt_truth=nan,
            px_truth=nan,
            py_truth=nan,
            pz_truth=nan,
            x_truth=nan,
            y_truth=nan,
            z_truth=nan,

            # MC Particle information
            charge_truth=nan,
            pdg_code_truth=0,
            is_primary=False,
        )


@format_crop_keys
def peel_reco_track_hit_content(reco_track, key="{part_name}"):
    nan = float('nan')

    first_pxd_layer = nan
    last_pxd_layer = nan
    first_svd_layer = nan
    last_svd_layer = nan
    first_cdc_layer = nan
    last_cdc_layer = nan
    if reco_track:
        n_cdc_hits = reco_track.getNumberOfCDCHits()
        n_svd_hits = reco_track.getNumberOfSVDHits()
        n_pxd_hits = reco_track.getNumberOfPXDHits()
        ndf = 2 * n_pxd_hits + 2 * n_svd_hits + n_cdc_hits

        pxd_hits = [hit.getSensorID().getLayerNumber() for hit in reco_track.getPXDHitList()]
        if pxd_hits:
            first_pxd_layer = min(pxd_hits)
            last_pxd_layer = max(pxd_hits)
        svd_hits = [hit.getSensorID().getLayerNumber() for hit in reco_track.getSVDHitList()]
        if svd_hits:
            first_svd_layer = min(svd_hits)
            last_svd_layer = max(svd_hits)
        cdc_hits = [hit.getICLayer() for hit in reco_track.getCDCHitList()]
        if cdc_hits:
            first_cdc_layer = min(cdc_hits)
            last_cdc_layer = max(cdc_hits)

        return dict(
            n_pxd_hits=n_pxd_hits,
            n_svd_hits=n_svd_hits,
            n_cdc_hits=n_cdc_hits,
            n_hits=n_pxd_hits + n_svd_hits + n_cdc_hits,
            ndf_hits=ndf,
            first_pxd_layer=first_pxd_layer,
            last_pxd_layer=last_pxd_layer,
            first_svd_layer=first_svd_layer,
            last_svd_layer=last_svd_layer,
            first_cdc_layer=first_cdc_layer,
            last_cdc_layer=last_cdc_layer,
        )
    else:
        return dict(
            n_pxd_hits=nan,
            n_svd_hits=nan,
            n_cdc_hits=nan,
            n_hits=nan,
            ndf_hits=nan,
            first_pxd_layer=first_pxd_layer,
            last_pxd_layer=last_pxd_layer,
            first_svd_layer=first_svd_layer,
            last_svd_layer=last_svd_layer,
            first_cdc_layer=first_cdc_layer,
            last_cdc_layer=last_cdc_layer,
        )


@format_crop_keys
def peel_reco_track_seed(reco_track, key="{part_name}"):
    if reco_track:
        seed_fit_result = get_seed_track_fit_result(reco_track)
        return peel_track_fit_result(seed_fit_result, key="seed_{part_name}")

    else:
        return peel_track_fit_result(None, key="seed_{part_name}")


@format_crop_keys
def peel_event_info(event_meta_data, key="{part_name}"):
    return dict(
        event_number=event_meta_data.getEvent(),
        run_number=event_meta_data.getRun(),
        experiment_number=event_meta_data.getExperiment(),
    )


@format_crop_keys
def peel_store_array_info(item, key="{part_name}"):
    if item:
        return dict(
            store_array_number=item.getArrayIndex()
        )
    else:
        return dict(
            store_array_number=float("nan")
        )


@format_crop_keys
def peel_quality_indicators(reco_track, key="{part_name}"):
    nan = float("nan")

    svd_qi = nan
    space_point_track_cand = reco_track.getRelated('SPTrackCands')

    if not space_point_track_cand:
        space_point_track_cand = reco_track.getRelated('SVDCDCRecoTracks').\
            getRelated('SVDRecoTracks').getRelated('SPTrackCands')

    if space_point_track_cand:
        svd_qi = space_point_track_cand.getQualityIndex()

    crops = dict(
        quality_indicator=reco_track.getQualityIndicator(),
        svd_quality_indicator=svd_qi,
    )

    return crops


@format_crop_keys
def peel_fit_status(reco_track, key="{part_name}"):
    nan = float("nan")

    crops = dict(
        is_fitted=nan,
        fit_pion_ok=nan,
        fit_muon_ok=nan,
        fit_electron_ok=nan,
        fit_proton_ok=nan,
        fit_kaon_ok=nan,
    )

    if reco_track:
        crops["is_fitted"] = reco_track.wasFitSuccessful()

        for rep in reco_track.getRepresentations():
            pdg_code = rep.getPDG()

            for crop in crops.keys():
                if crop.startswith("fit_"):
                    particle_name = crop.split("_")[1]
                    if getattr(Belle2.Const, particle_name).getPDGCode() == pdg_code:
                        crops[crop] = reco_track.wasFitSuccessful(rep)

    return crops


@format_crop_keys
def peel_track_fit_result(track_fit_result, key="{part_name}"):
    nan = float("nan")
    if track_fit_result:
        cov6 = track_fit_result.getCovariance6()
        mom = track_fit_result.getMomentum()
        pos = track_fit_result.getPosition()

        pt_estimate = mom.Perp()

        pt_variance = np.divide(
            mom.X()**2 * cov6(3, 3) + mom.Y()**2 * cov6(4, 4) - 2 * mom.X() * mom.Y() * cov6(3, 4),
            mom.Perp2()
        )

        pt_resolution = np.divide(pt_variance, pt_estimate)

        fit_crops = dict(
            d0_estimate=track_fit_result.getD0(),
            d0_variance=track_fit_result.getCov()[0],
            phi0_estimate=track_fit_result.getPhi() % (2.0 * math.pi),
            phi0_variance=track_fit_result.getCov()[5],
            omega_estimate=track_fit_result.getOmega(),
            omega_variance=track_fit_result.getCov()[9],
            z0_estimate=track_fit_result.getZ0(),
            z0_variance=track_fit_result.getCov()[12],
            tan_lambda_estimate=track_fit_result.getCotTheta(),
            tan_lambda_variance=track_fit_result.getCov()[14],

            x_estimate=pos.X(),
            x_variance=cov6(0, 0),
            y_estimate=pos.Y(),
            y_variance=cov6(1, 1),
            z_estimate=pos.Z(),
            z_variance=cov6(2, 2),

            pt_estimate=pt_estimate,
            pt_variance=pt_variance,
            pt_resolution=pt_resolution,

            b_field=Belle2.BFieldManager.getField(pos).Z(),

            px_estimate=mom.X(),
            px_variance=cov6(3, 3),
            py_estimate=mom.Y(),
            py_variance=cov6(4, 4),
            pz_estimate=mom.Z(),
            pz_variance=cov6(5, 5),

            p_value=track_fit_result.getPValue(),
        )

    else:
        fit_crops = dict(
            d0_estimate=nan,
            d0_variance=nan,
            phi0_estimate=nan,
            phi0_variance=nan,
            omega_estimate=nan,
            omega_variance=nan,
            z0_estimate=nan,
            z0_variance=nan,
            tan_lambda_estimate=nan,
            tan_lambda_variance=nan,

            x_estimate=nan,
            x_variance=nan,
            y_estimate=nan,
            y_variance=nan,
            z_estimate=nan,
            z_variance=nan,

            pt_estimate=nan,
            pt_variance=nan,
            pt_resolution=nan,

            px_estimate=nan,
            px_variance=nan,
            py_estimate=nan,
            py_variance=nan,
            pz_estimate=nan,
            pz_variance=nan,

            p_value=nan,
        )

    return fit_crops


# Custom peel function to get the sub detector hit efficiencies
@format_crop_keys
def peel_subdetector_hit_efficiency(mc_reco_track, reco_track, key="{part_name}"):
    def get_efficiency(detector_string):
        if not reco_track or not mc_reco_track:
            hit_efficiency = float("nan")
        else:
            mc_reco_hits = getattr(mc_reco_track, "get{}HitList".format(detector_string.upper()))()
            mc_reco_hit_size = mc_reco_hits.size()

            if mc_reco_hit_size == 0:
                hit_efficiency = float('nan')
            else:
                hit_efficiency = 0.
                for mc_reco_hit in mc_reco_hits:
                    for reco_hit in getattr(reco_track, "get{}HitList".format(detector_string.upper()))():
                        if mc_reco_hit.getArrayIndex() == reco_hit.getArrayIndex():
                            hit_efficiency += 1
                            break

                hit_efficiency /= mc_reco_hit_size

        return {"{}_hit_efficiency".format(detector_string.lower()): hit_efficiency}

    return dict(**get_efficiency("CDC"), **get_efficiency("SVD"), **get_efficiency("PXD"))


# Custom peel function to get the sub detector hit purity
@format_crop_keys
def peel_subdetector_hit_purity(reco_track, mc_reco_track, key="{part_name}"):
    def get_efficiency(detector_string):
        if not reco_track or not mc_reco_track:
            hit_purity = float("nan")
        else:
            reco_hits = getattr(reco_track, "get{}HitList".format(detector_string.upper()))()
            reco_hit_size = reco_hits.size()

            if reco_hit_size == 0:
                hit_purity = float('nan')
            else:
                hit_purity = 0.
                for reco_hit in reco_hits:
                    for mc_reco_hit in getattr(mc_reco_track, "get{}HitList".format(detector_string.upper()))():
                        if mc_reco_hit.getArrayIndex() == reco_hit.getArrayIndex():
                            hit_purity += 1
                            break

                hit_purity /= reco_hit_size

        return {"{}_hit_purity".format(detector_string.lower()): hit_purity}

    return dict(**get_efficiency("CDC"), **get_efficiency("SVD"), **get_efficiency("PXD"))


def get_helix_from_mc_particle(mc_particle):
    position = mc_particle.getVertex()
    momentum = mc_particle.getMomentum()
    charge_sign = (-1 if mc_particle.getCharge() < 0 else 1)
    b_field = Belle2.BFieldManager.getField(position).Z() / Belle2.Unit.T

    seed_helix = Belle2.Helix(position, momentum, charge_sign, b_field)
    return seed_helix


def get_seed_track_fit_result(reco_track):
    position = reco_track.getPositionSeed()
    momentum = reco_track.getMomentumSeed()
    cartesian_covariance = reco_track.getSeedCovariance()
    charge_sign = (-1 if reco_track.getChargeSeed() < 0 else 1)
    # It does not matter, which particle we put in here, so we just use a pion
    particle_type = Belle2.Const.pion
    p_value = float('nan')
    b_field = Belle2.BFieldManager.getField(position).Z() / Belle2.Unit.T
    cdc_hit_pattern = 0
    svd_hit_pattern = 0

    track_fit_result = Belle2.TrackFitResult(
        position,
        momentum,
        cartesian_covariance,
        charge_sign,
        particle_type,
        p_value,
        b_field,
        cdc_hit_pattern,
        svd_hit_pattern,
    )

    return track_fit_result
