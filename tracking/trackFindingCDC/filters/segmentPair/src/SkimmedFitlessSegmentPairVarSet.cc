/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include "../include/SkimmedFitlessSegmentPairVarSet.h"

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

bool SkimmedFitlessSegmentPairVarSet::accept(const CDCSegmentPair*)
{
  if (var<named("end_first_to_start_last_hit_pos_phi_difference")>() > 1) return false;
  if (var<named("end_first_to_start_last_hit_phi_difference")>() > 1.4) return false;

  if (var<named("stereo_hits_max_dist_z_forward_wall")>() > 100) {
    return false;
  }

  if (var<named("stereo_hits_min_dist_z_forward_wall")>() < -300) {
    return false;
  }

  if (var<named("stereo_hits_max_dist_z_backward_wall")>() > 300) {
    return false;
  }

  if (var<named("stereo_hits_min_dist_z_backward_wall")>() < -100) {
    return false;
  }

  if (var<named("start_arc_length_front_offset")>() < 0 or
      var<named("end_arc_length_front_offset")>() < 0 or
      var<named("start_arc_length_back_offset")>() < 0 or
      var<named("end_arc_length_back_offset")>() < 0) {
    return false;
  }

  return true;
}
