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

#include <tracking/datcon/optimizedDATCON/filters/relations/LayerRelationFilter.dcl.h>
#include <tracking/datcon/optimizedDATCON/filters/relations/RelationFilterFactory.h>
#include <tracking/trackFindingCDC/filters/base/ChooseableFilter.dcl.h>

namespace Belle2 {
  /// A chooseable filter for picking out the relations between states
  extern template class LayerRelationFilter<TrackFindingCDC::ChooseableFilter<RelationFilterFactory>>;
  /// A chooseable filter for picking out the relations between states
  using ChooseableRelationFilter = LayerRelationFilter<TrackFindingCDC::ChooseableFilter<RelationFilterFactory>>;
}
