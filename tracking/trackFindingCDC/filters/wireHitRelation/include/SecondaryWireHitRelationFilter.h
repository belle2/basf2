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

#include <tracking/trackFindingCDC/filters/wireHitRelation/WholeWireHitRelationFilter.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    using SecondaryWireHitRelationFilter = WholeWireHitRelationFilter<true>;

  }
}
