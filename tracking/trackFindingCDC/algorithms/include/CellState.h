/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef CELLSTATE_H
#define CELLSTATE_H

#include <cmath>
#include <tracking/cdcLocalTracking/typedefs/BasicTypes.h>

namespace Belle2 {

  namespace CDCLocalTracking {



    /// Type for the cell states in the cellular automata
    typedef Weight CellState;



    /// Constant to mark that a certain cell is not a continuation to advance on
    extern const CellState NO_CONTINUATION;
    // Implementation note: Uses external linkage because ROOT cint can not handle the inclusion of numeric_limits<> constructs.



  } // namespace CDCLocalTracking

} // namespace Belle2
#endif // CELLSTATE_H
