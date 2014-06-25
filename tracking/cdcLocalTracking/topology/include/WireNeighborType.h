/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef WIRENEIGHBORTYPE_H
#define WIRENEIGHBORTYPE_H

namespace Belle2 {

  namespace CDCLocalTracking {
    /// Type for the neighbor relationship from in wire to an other, imagined in the clock
    typedef signed short WireNeighborType;


    /// Constant for clockwise outwards
    const WireNeighborType CW_OUT_NEIGHBOR = 1;

    /// Constant for clockwise
    const WireNeighborType CW_NEIGHBOR = 3;

    /// Constant for clockwise inwards
    const WireNeighborType CW_IN_NEIGHBOR = 5;

    /// Constant for counterclockwise inwards
    const WireNeighborType CCW_IN_NEIGHBOR = 7;

    /// Constant for counterclockwise
    const WireNeighborType CCW_NEIGHBOR = 9;

    /// Constant for counterclockwise outwards
    const WireNeighborType CCW_OUT_NEIGHBOR = 11;


  } // namespace CDCLocalTracking

} // namespace Belle2

#endif // WIRENEIGHBORTYPE_H
