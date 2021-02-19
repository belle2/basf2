/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2021 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Wessel                                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/filters/base/Filter.dcl.h>
#include <tracking/datcon/optimizedDATCON/entities/HitData.h>

namespace Belle2 {
  /// Base filter for hits stored in the HitData
  using BaseRelationFilter = TrackFindingCDC::Filter<std::pair<const HitData*, const HitData*>>;
}
