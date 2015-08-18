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

#include <cmath>
#include <tracking/trackFindingCDC/typedefs/BasicTypes.h>

namespace Belle2 {

  namespace TrackFindingCDC {



    /// Type for the cell states in the cellular automata
    typedef Weight CellState;



    /// Constant to mark that a certain cell is not a continuation to advance on
    extern const CellState NO_CONTINUATION;
    // Implementation note: Uses external linkage because ROOT cint can not handle the inclusion of numeric_limits<> constructs.



  } // namespace TrackFindingCDC

} // namespace Belle2
