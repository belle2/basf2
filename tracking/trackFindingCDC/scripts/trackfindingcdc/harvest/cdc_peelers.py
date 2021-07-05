
import tracking.harvest.peelers as peelers
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
        stereo_kind=segment.getStereoKind(),
    )

    crops.update(trajectory2d_crops)
    return crops


@peelers.format_crop_keys
def peel_trajectory2d(trajectory2d, key="{part_name}"):
    chi2 = trajectory2d.getChi2()
    ndf = trajectory2d.getNDF()
    is_fitted = trajectory2d.isFitted()

    i_curv = 0
    curvature_estimate = trajectory2d.getCurvature()
    curvature_variance = trajectory2d.getLocalVariance(i_curv)

    return dict(
        chi2=chi2,
        ndf=ndf,
        is_fitted=is_fitted,
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
def peel_segment_pair(segment_pair, key="{part_name}"):
    from_segment = segment_pair.getFromSegment()
    to_segment = segment_pair.getToSegment()

    from_segment_crops = peel_segment2d(from_segment, key="from_{part_name}")
    to_segment_crops = peel_segment2d(to_segment, key="to_{part_name}")

    sorted_superlayer_ids = sorted([from_segment_crops["from_superlayer_id"],
                                    to_segment_crops["to_superlayer_id"]])

    superlayer_id_pair = 10.0 * sorted_superlayer_ids[1] + sorted_superlayer_ids[0]

    trajectory3d = segment_pair.getTrajectory3D()
    trajectory3d_crops = peel_trajectory3d(trajectory3d, key="{part_name}")

    crops = dict(
        superlayer_id_pair=superlayer_id_pair
    )

    crops.update(from_segment_crops)
    crops.update(to_segment_crops)
    crops.update(trajectory3d_crops)

    return crops
