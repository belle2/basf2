/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/track/TrackQualityVarSet.h>

#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>
#include <tracking/trackFindingCDC/mclookup/CDCMCHitLookUp.h>
#include <tracking/trackFindingCDC/mclookup/CDCMCTrackLookUp.h>

#include <tracking/trackFindingCDC/eventdata/hits/CDCWireHit.h>
#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectory3D.h>
#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectory2D.h>
#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectorySZ.h>

#include <tracking/trackFindingCDC/numerics/ToFinite.h>
#include <tracking/trackFindingCDC/utilities/Algorithms.h>
#include <numeric>

#include <cdc/dataobjects/CDCHit.h>

using namespace Belle2;
using namespace TrackFindingCDC;

/// TODO: add eventwise features
/// TODO: add class for feature extraction, to reduce redundancy
bool TrackQualityVarSet::extract(const CDCTrack* track)
{
  if ((not track) or track->empty()) {
    return false;
  }

  unsigned int size = track->size();

  std::vector<double> drift_lengths;
  std::transform(std::begin(*track),
                 std::end(*track),
                 std::back_inserter(drift_lengths),
  [](const CDCRecoHit3D & recoHit) {
    return recoHit.getWireHit().getRefDriftLength();
  });
  double drift_length_sum = std::accumulate(std::begin(drift_lengths), std::end(drift_lengths), 0.);
  double drift_length_mean = drift_length_sum / size;
  double drift_length_min = *std::min_element(std::begin(drift_lengths), std::end(drift_lengths));
  double drift_length_max = *std::max_element(std::begin(drift_lengths), std::end(drift_lengths));

  std::vector<double> adc_counts;
  std::transform(std::begin(*track),
                 std::end(*track),
                 std::back_inserter(adc_counts),
  [](const CDCRecoHit3D & recoHit) {
    return static_cast<double>(recoHit.getWireHit().getHit()->getADCCount());
  });

  double adc_sum = std::accumulate(std::begin(adc_counts), std::end(adc_counts), 0.);
  double adc_mean = adc_sum / size;
  double adc_min = *std::min_element(std::begin(adc_counts), std::end(adc_counts));
  double adc_max = *std::max_element(std::begin(adc_counts), std::end(adc_counts));

  double s_range = track->back().getArcLength2D() - track->front().getArcLength2D();

  // variables of variances and hit gaps (empty_s) require at least 2 hits in the tracks
  // if only one hit available and they are thus not defined, initialize them to -1 (or better 0?)
  double empty_s_sum = -1;
  double empty_s_mean = -1;
  double empty_s_min = -1;
  double empty_s_max = -1;

  double adc_std = -1;
  double empty_s_std = -1;
  double drift_length_std = -1;

  if (size > 1) {
    std::vector<double> arc_lengths;
    std::transform(std::begin(*track),
                   std::end(*track),
                   std::back_inserter(arc_lengths),
    [](const CDCRecoHit3D & recoHit) { return recoHit.getArcLength2D(); });
    // vector of gaps in arc length s between adjacent hits
    std::vector<double> empty_s_gaps;
    // calculate difference between subsequent arc lengths, accept first element, which stays the same
    std::adjacent_difference(std::begin(arc_lengths),
                             std::end(arc_lengths),
                             std::back_inserter(empty_s_gaps));
    // remove first element from empty s gaps, which is not a difference
    std::vector<double>(std::next(std::begin(empty_s_gaps)), std::end(empty_s_gaps))
    .swap(empty_s_gaps);

    double empty_s_size = empty_s_gaps.size();

    empty_s_sum = std::accumulate(std::begin(empty_s_gaps), std::end(empty_s_gaps), 0.);
    empty_s_mean = empty_s_sum / empty_s_size;
    empty_s_min = *std::min_element(std::begin(empty_s_gaps), std::end(empty_s_gaps));
    empty_s_max = *std::max_element(std::begin(empty_s_gaps), std::end(empty_s_gaps));

    std::vector<double> drift_residuals;
    std::transform(std::begin(drift_lengths),
                   std::end(drift_lengths),
                   std::back_inserter(drift_residuals),
    [&](double x) { return x - drift_length_mean; });

    double drift_length_variance_squared = std::inner_product(drift_residuals.begin(),
                                                              drift_residuals.end(),
                                                              drift_residuals.begin(),
                                                              0.0)
                                           / (size - 1);
    drift_length_std = std::sqrt(drift_length_variance_squared);

    std::vector<double> adc_residuals;
    std::transform(std::begin(adc_counts),
                   std::end(adc_counts),
                   std::back_inserter(adc_residuals),
    [&](double x) { return x - adc_mean; });

    double adc_variance_squared =
      std::inner_product(adc_residuals.begin(), adc_residuals.end(), adc_residuals.begin(), 0.0)
      / (size - 1);
    adc_std = std::sqrt(adc_variance_squared);

    // equivalent to size > 2, need at least two gaps between hits to calculate their variance
    if (empty_s_size > 1) {
      std::vector<double> empty_s_residuals;
      std::transform(std::begin(empty_s_gaps),
                     std::end(empty_s_gaps),
                     std::back_inserter(empty_s_residuals),
      [&](double x) { return x - empty_s_mean; });

      double empty_s_variance_squared = std::inner_product(empty_s_residuals.begin(),
                                                           empty_s_residuals.end(),
                                                           empty_s_residuals.begin(),
                                                           0.0)
                                        / (empty_s_size - 1);

      empty_s_std = std::sqrt(empty_s_variance_squared);
    }
  }

  const CDCTrajectory3D& trajectory3D = track->getStartTrajectory3D();
  const CDCTrajectory2D& trajectory2D = trajectory3D.getTrajectory2D();
  const CDCTrajectorySZ& trajectorySZ = trajectory3D.getTrajectorySZ();

  var<named("size")>() = size;
  var<named("pt")>() = toFinite(trajectory2D.getAbsMom2D(), 0);

  var<named("sz_slope")>() = toFinite(trajectorySZ.getTanLambda(), 0);
  var<named("drift_length_mean")>() = toFinite(drift_length_mean, 0);
  var<named("drift_length_variance")>() = toFinite(drift_length_std, 0);
  var<named("drift_length_max")>() = toFinite(drift_length_max, 0);
  var<named("drift_length_min")>() = toFinite(drift_length_min, 0);
  var<named("drift_length_sum")>() = toFinite(drift_length_sum, 0);

  var<named("adc_mean")>() = toFinite(adc_sum / size, 0);
  var<named("adc_variance")>() = toFinite(adc_std, 0);
  var<named("adc_max")>() = toFinite(adc_max, 0);
  var<named("adc_min")>() = toFinite(adc_min, 0);
  var<named("adc_sum")>() = toFinite(adc_sum, 0);

  var<named("empty_s_mean")>() = toFinite(empty_s_mean, 0);
  var<named("empty_s_sum")>() = toFinite(empty_s_sum, 0);
  var<named("empty_s_variance")>() = toFinite(empty_s_std, 0);
  var<named("empty_s_max")>() = toFinite(empty_s_max, 0);
  var<named("empty_s_min")>() = toFinite(empty_s_min, 0);
  var<named("s_range")>() = toFinite(s_range, 0);

  return true;
}
