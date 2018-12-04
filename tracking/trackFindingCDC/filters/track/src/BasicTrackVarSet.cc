/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun, Michael Eliachevitch                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/track/BasicTrackVarSet.h>

#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>
#include <tracking/trackFindingCDC/mclookup/CDCMCHitLookUp.h>
#include <tracking/trackFindingCDC/mclookup/CDCMCTrackLookUp.h>

#include <tracking/trackFindingCDC/eventdata/hits/CDCWireHit.h>
#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectory3D.h>
#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectory2D.h>
#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectorySZ.h>

#include <tracking/trackFindingCDC/numerics/ToFinite.h>

#include <tracking/trackFindingCDC/utilities/Algorithms.h>
#include <tracking/trackFindingCDC/utilities/Functional.h>
#include <numeric>

#include <cdc/dataobjects/CDCHit.h>

using namespace Belle2;
using namespace TrackFindingCDC;

bool BasicTrackVarSet::extract(const CDCTrack* track)
{
  if (not track) return false;

  if (track->empty()) {
    return false;
  }

  // use boost accumulators, which lazily provide different statistics (mean, variance, ...) for the
  // data that they accumulate (i.e. are "filled" with).
  statistics_accumulator drift_length_acc;
  statistics_accumulator adc_acc;
  statistics_accumulator empty_s_acc;

  unsigned int size = track->size();

  // Fill accumulators with ADC and drift circle information
  for (const CDCRecoHit3D& recoHit : *track) {
    drift_length_acc(recoHit.getWireHit().getRefDriftLength());
    adc_acc(recoHit.getWireHit().getHit()->getADCCount());
  }

  // Extract empty_s (ArcLength2D gap) information
  double s_range = track->back().getArcLength2D() - track->front().getArcLength2D();

  // fill a vector with all 2D arc lengths in track
  std::vector<double> arc_lengths;
  auto get_arc_length = [](const CDCRecoHit3D & recoHit) { return recoHit.getArcLength2D(); };
  std::transform(begin(*track), end(*track), back_inserter(arc_lengths), get_arc_length);
  // Remove all NAN elements. For some reason, last hit in track is sometimes NAN
  erase_remove_if(arc_lengths, IsNaN());

  // calculate gaps in arc length s between adjacent hits
  // beware: first element not a difference but mapped onto itself, empty_s_gaps[0] = arc_lengths[0]
  if (arc_lengths.size() > 1) {
    std::vector<double> empty_s_gaps;
    std::adjacent_difference(begin(arc_lengths), end(arc_lengths), back_inserter(empty_s_gaps));

    // start filling accumulator with hit gaps, but skip first which is not a difference
    std::for_each(next(begin(empty_s_gaps)), end(empty_s_gaps), [&empty_s_acc](double empty_s) {
      empty_s_acc(empty_s);
    });
  }

  unsigned int empty_s_size = bacc::count(empty_s_acc);

  // Overwrite boost-accumulator behavior for containers with 0 or 1 elements
  // Set variances containers with for 0/1 elements to -1 (boost default: nan/0 respectively)
  double drift_length_variance = -1;
  double adc_variance = -1;
  if (size > 1) {
    // for more than two elements, calculate variance with bessel correction
    double bessel_corr = size / (size - 1);
    drift_length_variance = std::sqrt(bacc::variance(drift_length_acc) * bessel_corr);
    adc_variance = std::sqrt(bacc::variance(adc_acc) * bessel_corr);
  }
  double empty_s_variance = -1;
  if (empty_s_size > 1) {
    double empty_s_bessel_corr = empty_s_size / (empty_s_size - 1);
    empty_s_variance = std::sqrt(bacc::variance(empty_s_acc) * empty_s_bessel_corr);
  }

  const CDCTrajectory3D& trajectory3D = track->getStartTrajectory3D();
  const CDCTrajectory2D& trajectory2D = trajectory3D.getTrajectory2D();
  const CDCTrajectorySZ& trajectorySZ = trajectory3D.getTrajectorySZ();

  var<named("size")>() = size;
  var<named("pt")>() = toFinite(trajectory2D.getAbsMom2D(), 0);

  var<named("sz_slope")>() = toFinite(trajectorySZ.getTanLambda(), 0);
  var<named("drift_length_mean")>() = toFinite(bacc::mean(drift_length_acc), 0);
  var<named("drift_length_variance")>() = toFinite(drift_length_variance, 0);
  var<named("drift_length_max")>() = toFinite(bacc::max(drift_length_acc), 0);
  var<named("drift_length_min")>() = toFinite(bacc::min(drift_length_acc), 0);
  var<named("drift_length_sum")>() = toFinite(bacc::sum(drift_length_acc), 0);

  var<named("adc_mean")>() = toFinite(bacc::mean(adc_acc), 0);
  var<named("adc_variance")>() = toFinite(adc_variance, 0);
  var<named("adc_max")>() = toFinite(bacc::max(adc_acc), 0);
  var<named("adc_min")>() = toFinite(bacc::min(adc_acc), 0);
  var<named("adc_sum")>() = toFinite(bacc::sum(adc_acc), 0);

  var<named("has_matching_segment")>() = track->getHasMatchingSegment();

  var<named("empty_s_mean")>() = toFinite(bacc::mean(empty_s_acc), 0);
  var<named("empty_s_sum")>() = toFinite(bacc::sum(empty_s_acc), 0);
  var<named("empty_s_variance")>() = toFinite(empty_s_variance, 0);
  // bacc::min/max returns max/min double for no entries which happens for empty_s
  // toFinite only works here because of implicit conversion to float, turning max doubles to inf
  // TODO: This is future-unsafe, write tests to ensure that this works
  var<named("empty_s_max")>() = toFinite(bacc::max(empty_s_acc), 0);
  var<named("empty_s_min")>() = toFinite(bacc::min(empty_s_acc), 0);
  var<named("s_range")>() = toFinite(s_range, 0);

  return true;
}
