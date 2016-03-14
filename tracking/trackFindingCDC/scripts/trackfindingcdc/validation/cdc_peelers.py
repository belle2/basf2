import functools

import tracking.validation.peelers as peelers
from tracking.validation.utilities import prob


@peelers.format_crop_keys
def peel_segment2d(segment, key="{part_name}"):
    superlayer_id = segment.getISuperLayer()
    size = segment.size()
    supercluster_id = segment.getISuperCluster()

    trajectory2d = segment.getTrajectory2D()
    trajectory2d_crops = peel_trajectory2d(trajectory2d, key="{part_name}")

    crops = dict(
        size=size,
        superlayer_id=superlayer_id,
        supercluster_id=supercluster_id,
        stereo_type=segment.getStereoType(),
    )

    crops.update(trajectory2d_crops)
    return crops


@peelers.format_crop_keys
def peel_trajectory2d(trajectory2d, key="{part_name}"):
    chi2 = trajectory2d.getChi2()
    ndf = trajectory2d.getNDF()

    i_curv = 0
    curvature_estimate = trajectory2d.getCurvature()
    curvature_variance = trajectory2d.getLocalVariance(i_curv)

    return dict(
        chi2=chi2,
        ndf=ndf,
        p_value=prob(chi2, ndf),
        curvature_estimate=curvature_estimate,
        curvature_variance=curvature_variance,
    )


@peelers.format_crop_keys
def peel_trajectory3d(trajectory3d, key="{part_name}"):
    i_curv = 0
    i_tan_lambda = 3

    curvature_estimate = trajectory3d.getCurvatureXY()
    curvature_variance = trajectory3d.getLocalVariance(i_curv)

    tan_lambda_estimate = trajectory3d.getTanLambda()
    tan_lambda_variance = trajectory3d.getLocalVariance(i_tan_lambda)

    chi2 = trajectory3d.getChi2()
    ndf = trajectory3d.getNDF()

    return dict(
        chi2=chi2,
        ndf=ndf,
        p_value=prob(chi2, ndf),
        curvature_estimate=curvature_estimate,
        curvature_variance=curvature_variance,
        tan_lambda_estimate=tan_lambda_estimate,
        tan_lambda_variance=tan_lambda_variance,
    )


@peelers.format_crop_keys
def peel_axial_stereo_segment_pair(axial_stereo_segment_pair, key="{part_name}"):
    start_segment = axial_stereo_segment_pair.getStartSegment()
    end_segment = axial_stereo_segment_pair.getEndSegment()

    start_segment_crops = peel_segment2d(start_segment, key="start_{part_name}")
    end_segment_crops = peel_segment2d(end_segment, key="end_{part_name}")

    sorted_superlayer_ids = sorted([start_segment_crops["start_superlayer_id"],
                                    end_segment_crops["end_superlayer_id"]])

    superlayer_id_pair = 10.0 * sorted_superlayer_ids[1] + sorted_superlayer_ids[0]

    trajectory3d = axial_stereo_segment_pair.getTrajectory3D()
    trajectory3d_crops = peel_trajectory3d(trajectory3d, key="{part_name}")

    crops = dict(
        superlayer_id_pair=superlayer_id_pair
    )

    crops.update(start_segment_crops)
    crops.update(end_segment_crops)
    crops.update(trajectory3d_crops)

    return crops
