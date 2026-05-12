/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/filters/segmentPairRelation/BaseSegmentPairRelationFilter.h>
#include <tracking/trackFindingCDC/filters/segmentPairRelation/SegmentPairRelationFilterFactory.h>

#include <tracking/trackingUtilities/filters/base/ChooseableFilter.dcl.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    // Guard to prevent repeated instantiations
    // extern template class TrackingUtilities::Chooseable<BaseSegmentPairRelationFilter>;
    // extern template class TrackingUtilities::ChooseableFilter<SegmentPairRelationFilterFactory>;
    using ChooseableSegmentPairRelationFilter = TrackingUtilities::ChooseableFilter<SegmentPairRelationFilterFactory>;
  }
}
