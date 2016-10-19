/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/track/CDCTrackVarSet.h>

#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>
#include <tracking/trackFindingCDC/mclookup/CDCMCHitLookUp.h>
#include <tracking/trackFindingCDC/mclookup/CDCMCTrackLookUp.h>
#include <tracking/trackFindingCDC/eventtopology/CDCWireHitTopology.h>

using namespace Belle2;
using namespace TrackFindingCDC;

bool CDCTrackVarSet::extract(const CDCTrack* track)
{
  bool extracted = extractNested(track);
  if (not extracted or not track) return false;

  if (track->empty()) {
    return false;
  }

  unsigned int size = track->size();
  double drift_length_sum = 0;
  double drift_length_sum_squared = 0;
  double drift_length_variance = 0;
  double drift_length_max = -1000;
  double drift_length_min = 1000;
  double adc_sum = 0;
  double adc_sum_squared = 0;
  double adc_variance = 0;
  double adc_max = -1000;
  double adc_min = 1000;
  double s_range = track->back().getArcLength2D() - track->front().getArcLength2D();
  double empty_s_sum = 0;
  double empty_s_sum_squared = 0;
  double empty_s_variance = 0;
  double empty_s_max = 0;
  double empty_s_min = 0;

  double last_perp_s = std::nan("");

  for (const CDCRecoHit3D& recoHit : *track) {
    // Drift circle information
    double driftLength = recoHit.getWireHit().getRefDriftLength();
    drift_length_sum += driftLength;
    drift_length_sum_squared += driftLength * driftLength;

    if (driftLength < drift_length_min) {
      drift_length_min = driftLength;
    }

    if (driftLength > drift_length_max) {
      drift_length_max = driftLength;
    }

    // ADC information
    double adc = static_cast<double>(recoHit.getWireHit()->getHit()->getADCCount());
    adc_sum += adc;
    adc_sum_squared += adc * adc;

    if (adc < adc_min) {
      adc_min = adc;
    }

    if (adc > adc_max) {
      adc_max = adc;
    }

    // perpS Information
    double currentPerpS = recoHit.getArcLength2D();
    if (not std::isnan(last_perp_s)) {
      double perp_s_difference = currentPerpS - last_perp_s;
      empty_s_sum += perp_s_difference;
      empty_s_sum_squared += perp_s_difference * perp_s_difference;

      if (perp_s_difference < empty_s_min) {
        empty_s_min = perp_s_difference;
      }

      if (perp_s_difference > empty_s_max) {
        empty_s_max = perp_s_difference;
      }
    }
    last_perp_s = currentPerpS;
  }

  if (size > 1) {
    double driftLengthVarianceSquared = (drift_length_sum_squared - drift_length_sum * drift_length_sum / size)  / (size - 1.0) ;
    double adcVarianceSquared = (adc_sum_squared - adc_sum * adc_sum / size)  / (size - 1.0) ;
    double emptySVarianceSquared = (empty_s_sum_squared - empty_s_sum * empty_s_sum / size)  / (size - 1.0) ;

    if (driftLengthVarianceSquared > 0) {
      drift_length_variance = std::sqrt(driftLengthVarianceSquared);
    } else {
      drift_length_variance = 0;
    }

    if (adcVarianceSquared > 0) {
      adc_variance = std::sqrt(adcVarianceSquared);
    } else {
      adc_variance = 0;
    }

    if (emptySVarianceSquared > 0) {
      empty_s_variance = std::sqrt(emptySVarianceSquared);
    } else {
      empty_s_variance = 0;
    }

  } else {
    drift_length_variance = -1;
    adc_variance = -1;
    empty_s_variance = -1;
  }

  const CDCTrajectory3D& trajectory3D = track->getStartTrajectory3D();
  const CDCTrajectory2D& trajectory2D = trajectory3D.getTrajectory2D();
  const CDCTrajectorySZ& trajectorySZ = trajectory3D.getTrajectorySZ();

  var<named("size")>() = size;
  setVariableIfNotNaN<named("pt")>(trajectory2D.getAbsMom2D());
  //var<named("fit_prob_3d")>() = trajectory3D.getPValue();
  //var<named("fit_prob_2d")>() = trajectory2D.getPValue();
  //var<named("fit_prob_sz")>() = trajectorySZ.getPValue();

  setVariableIfNotNaN<named("sz_slope")>(trajectorySZ.getTanLambda());
  setVariableIfNotNaN<named("drift_length_mean")>(drift_length_sum / size);
  setVariableIfNotNaN<named("drift_length_variance")>(drift_length_variance);
  setVariableIfNotNaN<named("drift_length_max")>(drift_length_max);
  setVariableIfNotNaN<named("drift_length_min")>(drift_length_min);
  setVariableIfNotNaN<named("drift_length_sum")>(drift_length_sum);

  setVariableIfNotNaN<named("adc_mean")>(adc_sum / size);
  setVariableIfNotNaN<named("adc_variance")>(adc_variance);
  setVariableIfNotNaN<named("adc_max")>(adc_max);
  setVariableIfNotNaN<named("adc_min")>(adc_min);
  setVariableIfNotNaN<named("adc_sum")>(adc_sum);

  var<named("has_matching_segment")>() = track->getHasMatchingSegment();

  setVariableIfNotNaN<named("empty_s_mean")>(empty_s_sum / size);
  setVariableIfNotNaN<named("empty_s_sum")>(empty_s_sum);
  setVariableIfNotNaN<named("empty_s_variance")>(empty_s_variance);
  setVariableIfNotNaN<named("empty_s_max")>(empty_s_max);
  setVariableIfNotNaN<named("empty_s_min")>(empty_s_min);
  setVariableIfNotNaN<named("s_range")>(s_range);

  return true;
}
