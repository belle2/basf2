"""
A set of common purose translators from complex framework objects to flat dictionaries
"""

import functools

import ROOT
ROOT.gSystem.Load("libtracking")
from ROOT import Belle2
from tracking.validation.tolerate_missing_key_formatter import TolerateMissingKeyFormatter

import math


formatter = TolerateMissingKeyFormatter()


def format_crop_keys(peel_func):
    @functools.wraps(peel_func)
    def peel_func_formatted_keys(obj, key="{part_name}"):
        crops = peel_func(obj, key=key)
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
def peel_track_cand_hit_content(track_cand, key="{part_name}"):
    n_pxd_hits = track_cand.getHitIDs(Belle2.Const.PXD).size()
    n_svd_hits = track_cand.getHitIDs(Belle2.Const.SVD).size()
    n_cdc_hits = track_cand.getHitIDs(Belle2.Const.CDC).size()
    ndf = 2 * n_pxd_hits + 2 * n_svd_hits + n_cdc_hits

    return dict(
        n_pxd_hits=n_pxd_hits,
        n_svd_hits=n_svd_hits,
        n_cdc_hits=n_cdc_hits,
        ndf_hits=ndf,
    )


@format_crop_keys
def peel_track_cand_seed(track_cand, key="{part_name}"):
    if track_cand:
        # Need those? Make congruent with the mc_particle variables above
        # seed_position = track_cand.getPosSeed()
        # seed_momentum = track_cand.getMomSeed()

        # crops = dict(
        #     seed_phi=seed_position.Phi(),
        #     seed_theta=seed_position.Theta(),
        #     )

        seed_fit_result = get_seed_track_fit_result(track_cand)
        return peel_fit_result(seed_fit_result, key="seed_{part_name}")

    else:
        return peel_fit_result(None, key="seed_{part_name}")


@format_crop_keys
def peel_fit_result(fit_result, key="{part_name}"):
    nan = float("nan")
    if fit_result:
        fit_crops = dict(
            d0_estimate=fit_result.getD0(),
            d0_variance=fit_result.getCov()[0],
            phi0_estimate=fit_result.getPhi() % (2.0 * math.pi),
            phi0_variance=fit_result.getCov()[5],
            omega_estimate=fit_result.getOmega(),
            omega_variance=fit_result.getCov()[9],
            z0_estimate=fit_result.getZ0(),
            z0_variance=fit_result.getCov()[12],
            tan_lambda_estimate=fit_result.getCotTheta(),
            tan_lambda_variance=fit_result.getCov()[14],
            is_fitted=True,  # FIXME
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
            is_fitted=False,
        )

    return fit_crops


# @format_crop_keys
def get_helix_from_mc_particle(mc_particle, key="{part_name}"):
    position = mc_particle.getVertex()
    momentum = mc_particle.getMomentum()
    charge_sign = (-1 if mc_particle.getCharge() < 0 else 1)
    b_field = 1.5

    seed_helix = Belle2.Helix(position, momentum, charge_sign, b_field)
    return seed_helix


# @format_crop_keys
def get_seed_track_fit_result(track_cand, key="{part_name}"):
    position = track_cand.getPosSeed()
    momentum = track_cand.getMomSeed()
    cartesian_covariance = track_cand.getCovSeed()
    charge_sign = (-1 if track_cand.getChargeSeed() < 0 else 1)
    particle_type = Belle2.Const.ParticleType(track_cand.getPdgCode())
    p_value = float('nan')
    b_field = 1.5
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
