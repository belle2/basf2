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

#include <tracking/trackFindingCDC/numerics/Weight.h>

#include <tracking/trackFindingCDC/utilities/Functional.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Operator to sort for lowest according to the weight of the object.
    using LessWeight = LessOf<MayIndirectTo<GetWeight>>;

    /// Operator to sort for highest according to the weight of the object.
    using GreaterWeight = GreaterOf<MayIndirectTo<GetWeight>>;

    /// Operator testing for Nan weight
    using HasNaNWeight = Composition<IsNaN, GetWeight>;
  }
}
