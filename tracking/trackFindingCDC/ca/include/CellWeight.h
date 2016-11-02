/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/numerics/Weight.h>
#include <cmath>

namespace Belle2 {

  namespace TrackFindingCDC {

    /// Type for the cell weight in the cellular automata
    using CellWeight = Weight;

    /// Signal that a certain cell should not be constructed. Used by the creator filters
    extern const CellWeight NOT_A_CELL;
    // Implementation note: Uses external linkage because ROOT cint can not handle the inclusion of numeric_limits<> constructs.

    /// Returns if the filter signaled that the cell should not be constructed
    inline bool isNotACell(const CellWeight& cellWeight)
    { return std::isnan(cellWeight); }

  } // namespace TrackFindingCDC

} // namespace Belle2
