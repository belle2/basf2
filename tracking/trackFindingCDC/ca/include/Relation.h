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

#include <utility>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Type for two related objects of the same type.
    template<class AObject>
    using Relation = std::pair<AObject*, AObject*>;

  } //end namespace TrackFindingCDC
} //end namespace Belle2
