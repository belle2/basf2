"""
A set of common purose translators from complex framework objects to flat dictionaries
"""

import functools

import math
import numpy as np

import ROOT

from ROOT import Belle2

import basf2
from tracking.validation.tolerate_missing_key_formatter import TolerateMissingKeyFormatter

Belle2.RecoTrack.getRightLeftInformation["Belle2::CDCHit"]
ROOT.gSystem.Load("libtracking")
#: string formatter that handles missing keys gracefully
formatter = TolerateMissingKeyFormatter()

#: create a formatted-key dictionary from specified peeler


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

#: create a dictionary from MCParticle information


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
            d0_truth=nan,
            phi0_truth=nan,
            omega_truth=nan,
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

            decay_vertex_radius_truth=nan,
            decay_vertex_x_truth=nan,
            decay_vertex_y_truth=nan,
            decay_vertex_z_truth=nan,
            number_of_daughters_truth=nan,
            status_truth=nan,


            # MC Particle information
            charge_truth=nan,
            pdg_code_truth=0,
            is_primary=False,
        )

#: create a dictionary from RecoTrack's hit content


@format_crop_keys
def peel_reco_track_hit_content(reco_track, key="{part_name}"):
    nan = float('nan')

    first_pxd_layer = nan
    last_pxd_layer = nan
    first_svd_layer = nan
    last_svd_layer = nan
    first_cdc_layer = nan
    last_cdc_layer = nan
    sum_of_weights = nan
    last_fit_layer = nan

    if reco_track:
        sum_of_weights = 0
        last_fit_layer = 0

        n_cdc_hits = reco_track.getNumberOfCDCHits()
        n_svd_hits = reco_track.getNumberOfSVDHits()
        n_pxd_hits = reco_track.getNumberOfPXDHits()
        ndf = 2 * n_pxd_hits + n_svd_hits + n_cdc_hits

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
        for hit_info in reco_track.getRelationsWith("RecoHitInformations"):
            track_point = reco_track.getCreatedTrackPoint(hit_info)
            if track_point:
                fitted_state = track_point.getFitterInfo()
                if fitted_state:
                    W = max(fitted_state.getWeights())
                    sum_of_weights += W
                    if W > 0.5 and hit_info.getTrackingDetector() == Belle2.RecoHitInformation.c_CDC:
                        layer = hit_info.getRelated("CDCHits").getICLayer()
                        if layer > last_fit_layer:
                            last_fit_layer = layer
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
            sum_of_weights=sum_of_weights,
            last_fit_layer=last_fit_layer
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
            sum_of_weights=sum_of_weights,
            last_fit_layer=last_fit_layer
        )

#: create a dictionary from RecoTrack's seed


@format_crop_keys
def peel_reco_track_seed(reco_track, key="{part_name}"):
    if reco_track:
        seed_fit_result = get_seed_track_fit_result(reco_track)
        return peel_track_fit_result(seed_fit_result, key="seed_{part_name}")

    else:
        return peel_track_fit_result(None, key="seed_{part_name}")

#: create a dictionary from EventMetaData


@format_crop_keys
def peel_event_info(event_meta_data, key="{part_name}"):
    return dict(
        event_number=event_meta_data.getEvent(),
        run_number=event_meta_data.getRun(),
        experiment_number=event_meta_data.getExperiment(),
    )

#: create a dictionary from a StoreArray


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

#: create a dictionary from the size of a StoreArray


@format_crop_keys
def peel_store_array_size(array_name, key="{part_name}"):
    array = Belle2.PyStoreArray(array_name)
    return {str(array_name) + "_size": array.getEntries() if array else 0}

#: create a dictionary from the event-level Track info


@format_crop_keys
def peel_event_level_tracking_info(event_level_tracking_info, key="{part_name}"):
    if not event_level_tracking_info:
        return dict(
            has_vxdtf2_failure_flag=False,
            has_svdckf_failure_flag=False,
            has_pxdckf_failure_flag=False,
            has_unspecified_trackfinding_failure=False,
        )
    return dict(has_vxdtf2_failure_flag=event_level_tracking_info.hasVXDTF2AbortionFlag(),
                has_svdckf_failure_flag=event_level_tracking_info.hasSVDCKFAbortionFlag(),
                has_pxdckf_failure_flag=event_level_tracking_info.hasPXDCKFAbortionFlag(),
                has_unspecified_trackfinding_failure=event_level_tracking_info.hasUnspecifiedTrackFindingFailure(),
                )


#: create a dictionary from RecoTrack's and its related SPTrackCand's quality indicator

@format_crop_keys
def peel_quality_indicators(reco_track, key="{part_name}"):
    nan = float("nan")

    svd_qi = nan
    cdc_qi = nan
    qi = nan

    if reco_track:
        qi = reco_track.getQualityIndicator()
        space_point_track_cand = reco_track.getRelated('SPTrackCands')

        # adjust relations if SVD->CDC CKF enabled
        if not space_point_track_cand:
            svd_track_cand = reco_track.getRelated('SVDRecoTracks')
            if not svd_track_cand:
                svd_cdc_track_cand = reco_track.getRelated('SVDCDCRecoTracks')
                if svd_cdc_track_cand:
                    svd_track_cand = svd_cdc_track_cand.getRelated('SVDRecoTracks')
                    if not svd_track_cand:
                        temp_svd_track_cand = svd_cdc_track_cand.getRelated('SVDPlusCDCStandaloneRecoTracks')
                        if temp_svd_track_cand:
                            svd_track_cand = temp_svd_track_cand.getRelated('SVDRecoTracks')
            if svd_track_cand:
                space_point_track_cand = svd_track_cand.getRelated('SPTrackCands')

        if space_point_track_cand:
            svd_qi = space_point_track_cand.getQualityIndicator()

        cdc_track_cand = reco_track.getRelated('CDCRecoTracks')
        if not cdc_track_cand:
            svd_cdc_track_cand = reco_track.getRelated('SVDCDCRecoTracks')
            if svd_cdc_track_cand:
                cdc_track_cand = svd_cdc_track_cand.getRelated('CDCRecoTracks')
                if not cdc_track_cand:
                    cdc_track_cand = svd_cdc_track_cand.getRelated('CKFCDCRecoTracks')
                if not cdc_track_cand:
                    temp_cdc_track_cand = svd_cdc_track_cand.getRelated('SVDPlusCDCStandaloneRecoTracks')
                    if temp_cdc_track_cand:
                        cdc_track_cand = temp_cdc_track_cand.getRelated('CDCRecoTracks')

        if cdc_track_cand:
            cdc_qi = cdc_track_cand.getQualityIndicator()

    crops = dict(
        quality_indicator=qi,
        svd_quality_indicator=svd_qi,
        cdc_qualityindicator=cdc_qi,
    )

    return crops


#: create a dictionary that shows used trackfinders

@format_crop_keys
def peel_trackfinder(reco_track, key="{part_name}"):
    used_CDCTrackFinder = False
    used_VXDTrackFinder = False
    used_SVDtoCDCCKF = False
    used_ECLtoCDCCKF = False
    used_CDCtoSVDCKF = False

    if reco_track:
        if reco_track.getNumberOfSVDHits() > 0:
            info = get_reco_hit_information(reco_track, reco_track.getSVDHitList()[0])
            svd_tf = info.getFoundByTrackFinder()
            used_VXDTrackFinder = svd_tf == Belle2.RecoHitInformation.c_VXDTrackFinder
            used_CDCtoSVDCKF = svd_tf == Belle2.RecoHitInformation.c_CDCtoSVDCKF

        if reco_track.getNumberOfCDCHits() > 0:
            info = get_reco_hit_information(reco_track, reco_track.getCDCHitList()[0])
            cdc_tf = info.getFoundByTrackFinder()
            used_CDCTrackFinder = cdc_tf == Belle2.RecoHitInformation.c_CDCTrackFinder
            used_SVDtoCDCCKF = cdc_tf == Belle2.RecoHitInformation.c_SVDtoCDCCKF
            used_ECLtoCDCCKF = cdc_tf == Belle2.RecoHitInformation.c_ECLtoCDCCKF

    crops = dict(
        foundby_CDCTrackFinder=used_CDCTrackFinder,
        foundby_VXDTrackFinder=used_VXDTrackFinder,
        foundby_SVDtoCDCCKF=used_SVDtoCDCCKF,
        foundby_CDCtoSVDCKF=used_CDCtoSVDCKF,
        foundby_ECLtoCDCCKF=used_ECLtoCDCCKF,
    )

    return crops


#: create a dictionary from RecoTrack fit status

@format_crop_keys
def peel_fit_status(reco_track, key="{part_name}"):
    nan = float("nan")

    crops = dict(
        is_fitted=nan,
        fit_pion_ok=nan,
        ndf_pion=nan,
        fit_muon_ok=nan,
        ndf_muon=nan,
        fit_electron_ok=nan,
        ndf_electron=nan,
        fit_proton_ok=nan,
        ndf_proton=nan,
        fit_kaon_ok=nan,
        ndf_kaon=nan,
    )

    if reco_track:
        crops["is_fitted"] = reco_track.wasFitSuccessful()

        for rep in reco_track.getRepresentations():
            was_successful = reco_track.wasFitSuccessful(rep)
            pdg_code = rep.getPDG()

            for crop in crops.keys():
                if crop.startswith("fit_"):
                    particle_name = crop.split("_")[1]
                    if abs(getattr(Belle2.Const, particle_name).getPDGCode()) == abs(pdg_code):
                        crops[crop] = was_successful

                        if was_successful:
                            crops[f"ndf_{particle_name}"] = reco_track.getTrackFitStatus(rep).getNdf()

    return crops

#: create a dictionary from the TrackFit information


@format_crop_keys
def peel_track_fit_result(track_fit_result, key="{part_name}"):
    nan = float("nan")
    if track_fit_result:
        cov6 = track_fit_result.getCovariance6()
        mom = track_fit_result.getMomentum()
        pos = track_fit_result.getPosition()

        pt_estimate = mom.Perp()

        pt_variance = np.divide(
            mom.X() ** 2 * cov6(3, 3) + mom.Y() ** 2 * cov6(4, 4) - 2 * mom.X() * mom.Y() * cov6(3, 4),
            mom.Perp2()
        )

        pt_resolution = np.divide(pt_variance, pt_estimate)

        fit_crops = dict(
            has_trackFitResult=True,
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

            track_charge=track_fit_result.getChargeSign(),
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
            has_trackFitResult=False,
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

            track_charge=nan,
            b_field=nan,

            px_estimate=nan,
            px_variance=nan,
            py_estimate=nan,
            py_variance=nan,
            pz_estimate=nan,
            pz_variance=nan,

            p_value=nan,
        )

    return fit_crops

#: create a dictionary from RecoTrack's hit information


def get_reco_hit_information(reco_track, hit):
    """Helper function for getting the correct reco hit info"""
    for info in hit.getRelationsFrom("RecoHitInformations"):
        if info.getRelatedFrom(reco_track.getArrayName()) == reco_track:
            return info


#: Custom peel function to get the sub detector hit efficiencies
@format_crop_keys
def peel_subdetector_hit_efficiency(mc_reco_track, reco_track, key="{part_name}"):
    def get_efficiency(detector_string):
        if not reco_track or not mc_reco_track:
            hit_efficiency = float("nan")
        else:
            mc_reco_hits = getattr(mc_reco_track, "get{}HitList".format(detector_string.upper()))()
            if mc_reco_hits.size() == 0:
                hit_efficiency = float('nan')
            else:
                mc_reco_hit_size = 0
                hit_efficiency = 0.
                for mc_reco_hit in mc_reco_hits:
                    info = get_reco_hit_information(mc_reco_track, mc_reco_hit)

                    if info.getFoundByTrackFinder() == Belle2.RecoHitInformation.c_MCTrackFinderAuxiliaryHit:
                        continue

                    mc_reco_hit_size += 1

                    for reco_hit in getattr(reco_track, "get{}HitList".format(detector_string.upper()))():
                        if mc_reco_hit.getArrayIndex() == reco_hit.getArrayIndex():
                            hit_efficiency += 1
                            break

                if not mc_reco_hit_size:
                    hit_efficiency = float('nan')
                else:
                    hit_efficiency /= mc_reco_hit_size

        return {"{}_hit_efficiency".format(detector_string.lower()): hit_efficiency}

    return dict(**get_efficiency("CDC"), **get_efficiency("SVD"), **get_efficiency("PXD"))


#: Custom peel function to get the sub detector hit purity
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


#: Get hit level information information
@format_crop_keys
def peel_hit_information(hit_info, reco_track, key="{part_name}"):
    nan = np.float("nan")

    crops = dict(residual=nan,
                 residual_x=nan,
                 residual_y=nan,
                 residuals=nan,
                 weight=nan,
                 tracking_detector=hit_info.getTrackingDetector(),
                 use_in_fit=hit_info.useInFit(),
                 hit_time=nan,
                 layer_number=nan,
                 )

    if hit_info.useInFit() and reco_track.hasTrackFitStatus():
        track_point = reco_track.getCreatedTrackPoint(hit_info)
        fitted_state = track_point.getFitterInfo()
        if fitted_state:
            try:
                res_state = fitted_state.getResidual().getState()
                crops["residual"] = np.sqrt(res_state.Norm2Sqr())
                if res_state.GetNoElements() == 2:
                    crops["residual_x"] = res_state[0]
                    crops["residual_y"] = res_state[1]
                weights = fitted_state.getWeights()
                crops['weight'] = max(weights)
            except BaseException:
                pass

    if hit_info.getTrackingDetector() == Belle2.RecoHitInformation.c_SVD:
        hit = hit_info.getRelated("SVDClusters")
        crops["hit_time"] = hit.getClsTime()
        crops["layer_number"] = hit.getSensorID().getLayerNumber()
    if hit_info.getTrackingDetector() == Belle2.RecoHitInformation.c_PXD:
        hit = hit_info.getRelated("PXDClusters")
        crops["layer_number"] = hit.getSensorID().getLayerNumber()
    if hit_info.getTrackingDetector() == Belle2.RecoHitInformation.c_CDC:
        hit = hit_info.getRelated("CDCHits")
        crops["layer_number"] = hit.getICLayer()

    return crops


#: Peeler for module statistics
@format_crop_keys
def peel_module_statistics(modules=None, key="{part_name}"):
    if modules is None:
        modules = []
    module_stats = dict()

    for module in basf2.statistics.modules:
        if module.name in modules:
            module_stats[str(module.name) + "_mem"] = module.memory_sum(basf2.statistics.EVENT)
            module_stats[str(module.name) + "_time"] = module.time_sum(basf2.statistics.EVENT)
            module_stats[str(module.name) + "_calls"] = module.calls(basf2.statistics.EVENT)

    return module_stats

#: create a dictionary for MCParticle information


def get_helix_from_mc_particle(mc_particle):
    position = mc_particle.getVertex()
    momentum = mc_particle.getMomentum()
    charge_sign = (-1 if mc_particle.getCharge() < 0 else 1)
    b_field = Belle2.BFieldManager.getField(position).Z() / Belle2.Unit.T

    seed_helix = Belle2.Helix(position, momentum, charge_sign, b_field)
    return seed_helix

#: create a dictionary for RecoTrack's seed values


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
    # the value 0xFFFF will cause the TrackFitResult::getNDF() to return -1
    ndf = 0xFFFF

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
        ndf,
    )

    return track_fit_result

#: create a dictionary for RecoTrack CDCHit's right-left information


def is_correct_rl_information(cdc_hit, reco_track, hit_lookup):
    rl_info = reco_track.getRightLeftInformation["const Belle2::CDCHit"](cdc_hit)
    truth_rl_info = hit_lookup.getRLInfo(cdc_hit)

    if rl_info == Belle2.RecoHitInformation.c_right and truth_rl_info == 1:
        return True
    if rl_info == Belle2.RecoHitInformation.c_left and truth_rl_info == 65535:  # -1 as short
        return True

    return False
