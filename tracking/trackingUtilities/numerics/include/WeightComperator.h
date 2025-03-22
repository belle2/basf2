/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
