/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/filters/base/Filter.h>
#include <tracking/trackFindingCDC/utilities/WeightedRelation.h>
#include <tracking/dataobjects/RecoTrack.h>

namespace Belle2 {
  /**
   * Base filter for detector track combination filters. Please note that for implementation reasons
   * the base object includes poitners to pointers.
   *
   * TODO: We should remove this...
   */
  using BaseDetectorTrackCombinationFilter = TrackFindingCDC::Filter<TrackFindingCDC::WeightedRelation<RecoTrack*, RecoTrack* const>>;
}
