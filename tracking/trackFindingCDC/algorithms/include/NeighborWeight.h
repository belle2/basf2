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
#ifndef NEIGHBORWEIGHT_H
#define NEIGHBORWEIGHT_H

#include <cmath>
#include <tracking/trackFindingCDC/typedefs/BasicTypes.h>

namespace Belle2 {

  namespace TrackFindingCDC {

    /// Type for the cell weight in the cellular automata
    typedef Weight NeighborWeight;

    /**
    Indicates that a connection in a neighborhood shall not be made.
    Used by relation filters.
    */
    extern const NeighborWeight NOT_A_NEIGHBOR;
    // Implementation note: Uses external linkage,
    // because ROOT cint can not handle the inclusion of numeric_limits<> constructs.

    /// Weight indicates that a certain connection in a neighborhood shall not be made.
    inline bool isNotANeighbor(const NeighborWeight& weight)
    { return std::isnan(weight); }

  } // namespace TrackFindingCDC

} // namespace Belle2
#endif // NEIGHBORWEIGHT_H
