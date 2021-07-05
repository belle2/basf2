/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/track/BasicTrackVarSet.h>

#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>

#include <tracking/trackFindingCDC/eventdata/hits/CDCWireHit.h>
#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectory3D.h>
#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectory2D.h>
#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectorySZ.h>

#include <tracking/trackFindingCDC/numerics/ToFinite.h>

#include <tracking/trackFindingCDC/utilities/Algorithms.h>
#include <tracking/trackFindingCDC/utilities/Functional.h>
#include <numeric>
#include <set>

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
  statistics_accumulator drift_length_acc; /* correlated to actual distance from hit to wire*/
  statistics_accumulator adc_acc; /* integrated charge over the cell*/
  statistics_accumulator empty_s_acc; /* gap within the track?!*/
  statistics_accumulator tot_acc; /* time over threshold */
  /* continuous layer number [from 0 to 55] (not to be distinguished from SuperLayer [from 0 to 8] or Layer (within SuperLayer, so from [0 to 7]) */
  statistics_accumulator  cont_layer_acc;
  statistics_accumulator super_layer_acc;

  unsigned int size = track->size();

  std::vector<unsigned int> cont_layer;
  std::vector<unsigned int> super_layer;
  // Fill accumulators with ADC and drift circle information
  for (const CDCRecoHit3D& recoHit : *track) {
    drift_length_acc(recoHit.getWireHit().getRefDriftLength());
    adc_acc(static_cast<unsigned int>(recoHit.getWireHit().getHit()->getADCCount()));
    tot_acc(static_cast<unsigned int>(recoHit.getWireHit().getHit()->getTOT()));
    cont_layer_acc(static_cast<unsigned int>(recoHit.getWireHit().getHit()->getICLayer()));
    super_layer_acc(static_cast<unsigned int>(recoHit.getWireHit().getISuperLayer()));
    cont_layer.push_back(static_cast<unsigned int>(recoHit.getWireHit().getHit()->getICLayer()));
    super_layer.push_back(static_cast<unsigned int>(recoHit.getWireHit().getISuperLayer()));
  }

  // extract wanted quantities from layers
  unsigned int cont_layer_first = cont_layer.front();
  unsigned int cont_layer_last = cont_layer.back();
  unsigned int cont_layer_min = *std::min_element(cont_layer.begin(), cont_layer.end());
  unsigned int cont_layer_max = *std::max_element(cont_layer.begin(), cont_layer.end());
  unsigned int cont_layer_count = std::set<unsigned int>(cont_layer.begin(), cont_layer.end()).size();
  double cont_layer_occupancy = (double)cont_layer_count / (double)(cont_layer_max - cont_layer_min +
                                1); // +1 b/c the first layer should also be counted
  unsigned int super_layer_first = super_layer.front();
  unsigned int super_layer_last = super_layer.back();
  unsigned int super_layer_min = *std::min_element(super_layer.begin(), super_layer.end());
  unsigned int super_layer_max = *std::max_element(super_layer.begin(), super_layer.end());
  unsigned int super_layer_count = std::set<unsigned int>(super_layer.begin(), super_layer.end()).size();
  double super_layer_occupancy = (double)super_layer_count / (double)(super_layer_max - super_layer_min + 1);

  double hits_per_layer = (double)size / (double)cont_layer_count;

  // Extract empty_s (ArcLength2D gap) information
  double s_range = track->back().getArcLength2D() - track->front().getArcLength2D();
  double avg_hit_dist = s_range / (double)cont_layer_count;

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

    // Wrap a reference to empty_s_acc in a lambda, b/c it is passed by value to the following
    // for_each. If used directly, only a copy would filled in the for_each.
    auto empty_s_acc_ref = [&empty_s_acc](double s) { empty_s_acc(s); };
    // start filling accumulator with hit gaps, but skip first which is not a difference
    std::for_each(next(begin(empty_s_gaps)), end(empty_s_gaps), empty_s_acc_ref);
  }

  unsigned int empty_s_size = bacc::count(empty_s_acc);

  // Overwrite boost-accumulator behavior for containers with 0 or 1 elements
  // Set variances containers with for 0/1 elements to -1 (boost default: nan/0 respectively)
  double drift_length_variance = -1;
  double adc_variance = -1;
  double tot_variance = -1;
  double cont_layer_variance = -1;
  double super_layer_variance = -1;
  if (size > 1) {
    // for more than two elements, calculate variance with bessel correction
    double bessel_corr = (double)size / (size - 1.0);
    drift_length_variance = std::sqrt(bacc::variance(drift_length_acc) * bessel_corr);
    adc_variance = std::sqrt(bacc::variance(adc_acc) * bessel_corr);
    tot_variance = std::sqrt(bacc::variance(tot_acc) * bessel_corr);
    cont_layer_variance = std::sqrt(bacc::variance(cont_layer_acc) * bessel_corr);
    super_layer_variance = std::sqrt(bacc::variance(super_layer_acc) * bessel_corr);
  }
  double empty_s_variance = -1;
  if (empty_s_size > 1) {
    double empty_s_bessel_corr = (double)empty_s_size / (empty_s_size - 1.0);
    empty_s_variance = std::sqrt(bacc::variance(empty_s_acc) * empty_s_bessel_corr);
  }

  const CDCTrajectory3D& trajectory3D = track->getStartTrajectory3D();
  const CDCTrajectory2D& trajectory2D = trajectory3D.getTrajectory2D();
  const CDCTrajectorySZ& trajectorySZ = trajectory3D.getTrajectorySZ();

  var<named("pt")>() = toFinite(trajectory2D.getAbsMom2D(), 0);
  var<named("size")>() = size;
  var<named("hits_per_layer")>() = hits_per_layer;

  var<named("sz_slope")>() = toFinite(trajectorySZ.getTanLambda(), 0);
  var<named("z0")>() = toFinite(trajectorySZ.getZ0(), 0);
  var<named("s_range")>() = toFinite(s_range, 0);
  var<named("avg_hit_dist")>() = toFinite(avg_hit_dist, 0);
  var<named("has_matching_segment")>() = track->getHasMatchingSegment();

  var<named("cont_layer_mean")>() = toFinite(bacc::mean(cont_layer_acc), 0);
  var<named("cont_layer_variance")>() = toFinite(cont_layer_variance, 0);
  var<named("cont_layer_max")>() = toFinite(cont_layer_max, 0);
  var<named("cont_layer_min")>() = toFinite(cont_layer_min, 0);
  var<named("cont_layer_first")>() = toFinite(cont_layer_first, 0);
  var<named("cont_layer_last")>() = toFinite(cont_layer_last, 0);
  var<named("cont_layer_max_vs_last")>() = toFinite(cont_layer_max - cont_layer_last, 0);
  var<named("cont_layer_first_vs_min")>() = toFinite(cont_layer_first - cont_layer_min, 0);
  var<named("cont_layer_count")>() = toFinite(cont_layer_count, 0);
  var<named("cont_layer_occupancy")>() = toFinite(cont_layer_occupancy, 0);

  var<named("super_layer_mean")>() = toFinite(bacc::mean(super_layer_acc), 0);
  var<named("super_layer_variance")>() = toFinite(super_layer_variance, 0);
  var<named("super_layer_max")>() = toFinite(super_layer_max, 0);
  var<named("super_layer_min")>() = toFinite(super_layer_min, 0);
  var<named("super_layer_first")>() = toFinite(super_layer_first, 0);
  var<named("super_layer_last")>() = toFinite(super_layer_last, 0);
  var<named("super_layer_max_vs_last")>() = toFinite(super_layer_max - super_layer_last, 0);
  var<named("super_layer_first_vs_min")>() = toFinite(super_layer_first - super_layer_min, 0);
  var<named("super_layer_count")>() = toFinite(super_layer_count, 0);
  var<named("super_layer_occupancy")>() = toFinite(super_layer_occupancy, 0);

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

  var<named("tot_mean")>() = toFinite(bacc::mean(tot_acc), 0);
  var<named("tot_variance")>() = toFinite(tot_variance, 0);
  var<named("tot_max")>() = toFinite(bacc::max(tot_acc), 0);
  var<named("tot_min")>() = toFinite(bacc::min(tot_acc), 0);
  var<named("tot_sum")>() = toFinite(bacc::sum(tot_acc), 0);

  var<named("empty_s_mean")>() = toFinite(bacc::mean(empty_s_acc), 0);
  var<named("empty_s_sum")>() = toFinite(bacc::sum(empty_s_acc), 0);
  var<named("empty_s_variance")>() = toFinite(empty_s_variance, 0);
  var<named("empty_s_max")>() = toFinite(bacc::max(empty_s_acc), 0);
  var<named("empty_s_min")>() = toFinite(bacc::min(empty_s_acc), 0);

  return true;
}
