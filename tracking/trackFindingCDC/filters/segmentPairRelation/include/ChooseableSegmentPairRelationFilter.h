/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/filters/segmentPairRelation/BaseSegmentPairRelationFilter.h>
#include <tracking/trackFindingCDC/filters/segmentPairRelation/SegmentPairRelationFilterFactory.h>
#include <tracking/trackFindingCDC/filters/base/ChooseableFilter.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    using ChooseableSegmentPairRelationFilter = ChooseableFilter<SegmentPairRelationFilterFactory>;
  }
}
