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

#include <tracking/trackFindingCDC/filters/base/Filter.dcl.h>

#include <tracking/trackFindingCDC/utilities/WeightedRelation.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    class CDCRLWireHit;
    class CDCTrack;

    // Guard to prevent repeated instantiations
    extern template class Filter<WeightedRelation<CDCTrack, const CDCRLWireHit>>;

    /// Base filter for stereo hit - track relations.
    using BaseStereoHitFilter = Filter<WeightedRelation<CDCTrack, const CDCRLWireHit>>;
  }
}
