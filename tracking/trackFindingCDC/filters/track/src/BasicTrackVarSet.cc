/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
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

// use BOOST for  accumulators
#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/count.hpp>
#include <boost/accumulators/statistics/min.hpp>
#include <boost/accumulators/statistics/max.hpp>
#include <boost/accumulators/statistics/sum.hpp>
#include <boost/accumulators/statistics/mean.hpp>
#include <boost/accumulators/statistics/variance.hpp>

#include <cdc/dataobjects/CDCHit.h>

using namespace Belle2;
using namespace TrackFindingCDC;

namespace bacc = boost::accumulators;

bool BasicTrackVarSet::extract(const CDCTrack* track)
{
  if (not track) return false;

  if (track->empty()) {
    return false;
  }

  double s_range = track->back().getArcLength2D() - track->front().getArcLength2D();
  double last_perp_s = NAN;

  // use boost accumulators, which lazily provide different statistics (mean, variance, ...) for the
  // data that they accumulate (i.e. are "filled" with).
  // TODO Maybe wrap the accumulators code in an interface and put them in some utilityfile
  using features_set = bacc::features<bacc::tag::count,
        bacc::tag::sum,
        bacc::tag::min,
        bacc::tag::max,
        bacc::tag::mean,
        bacc::tag::lazy_variance>;
  using feature_accumulator = bacc::accumulator_set<double, features_set>;

  feature_accumulator drift_length_acc;
  feature_accumulator adc_acc;
  feature_accumulator empty_s_acc;

  for (const CDCRecoHit3D& recoHit : *track) {
    // Drift circle information
    double driftLength = recoHit.getWireHit().getRefDriftLength();
    drift_length_acc(driftLength);

    // ADC information
    double adc = static_cast<double>(recoHit.getWireHit().getHit()->getADCCount());
    adc_acc(adc);

    // perpS Information
    double current_perp_s = recoHit.getArcLength2D();
    if (not(std::isnan(last_perp_s) or std::isnan(current_perp_s))) {
      double perp_s_difference = current_perp_s - last_perp_s;
      empty_s_acc(perp_s_difference);
    }
    if (not std::isnan(current_perp_s)) { last_perp_s = current_perp_s; }
  }

  // if track has insufficient hits (size) for some variables to be computable, set them to -1
  // variance calculations and empty_s (hit gaps) information need at least 2 hits (1 hit gap)
  unsigned int size = track->size();
  unsigned int empty_s_size = bacc::count(empty_s_acc); // smaller than `size`, usually by 1

  double drift_length_variance = -1;
  double adc_variance = -1;
  double empty_s_variance = -1;
  double empty_s_sum = -1;
  double empty_s_min = -1;
  double empty_s_max = -1;
  double empty_s_mean = -1;

  if (size > 1) {
    drift_length_variance = std::sqrt(bacc::variance(drift_length_acc) * size / (size - 1));
    adc_variance = std::sqrt(bacc::variance(adc_acc) * size / (size - 1));
  }

  if (empty_s_size > 0) {
    empty_s_sum = bacc::sum(empty_s_acc);
    empty_s_min = bacc::min(empty_s_acc);
    empty_s_max = bacc::max(empty_s_acc);
    empty_s_mean = bacc::mean(empty_s_acc);
    if (empty_s_size > 1) {
      empty_s_variance = std::sqrt(bacc::variance(empty_s_acc) * empty_s_size / (empty_s_size - 1));
    }
  }

  const CDCTrajectory3D& trajectory3D = track->getStartTrajectory3D();
  const CDCTrajectory2D& trajectory2D = trajectory3D.getTrajectory2D();
  const CDCTrajectorySZ& trajectorySZ = trajectory3D.getTrajectorySZ();

  var<named("size")>() = size;
  var<named("pt")>() = toFinite(trajectory2D.getAbsMom2D(), 0);
  // var<named("fit_prob_3d")>() = trajectory3D.getPValue();
  // var<named("fit_prob_2d")>() = trajectory2D.getPValue();
  // var<named("fit_prob_sz")>() = trajectorySZ.getPValue();

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

  var<named("empty_s_mean")>() = toFinite(empty_s_mean, 0);
  var<named("empty_s_sum")>() = toFinite(empty_s_sum, 0);
  var<named("empty_s_variance")>() = toFinite(empty_s_variance, 0);
  var<named("empty_s_max")>() = toFinite(empty_s_max, 0);
  var<named("empty_s_min")>() = toFinite(empty_s_min, 0);
  var<named("s_range")>() = toFinite(s_range, 0);

  return true;
}
