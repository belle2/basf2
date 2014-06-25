/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2012 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Oliver Frost                                             *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/
#ifndef BASICCONSTANTS_H
#define BASICCONSTANTS_H

#include <limits>
#include <tracking/cdcLocalTracking/typedefs/BasicTypes.h>
#include <tracking/cdcLocalTracking/numerics/numerics.h>

namespace Belle2 {

  namespace CDCLocalTracking {

    // ROOT cint can not deal with std::numeric_limits so we have to put it in an extra file to include it in the .cc files only


    /**@name Additional constant
     * Additional constants ROOT cint cannot deal with. Only use them in .cc files */
    /**@{*/
    /*
    ///Constant for the highest possible weight
    const Weight HIGHEST_WEIGHT =  std::numeric_limits<Weight>::infinity();

    ///Constant for the lowest possible weight
    const Weight LOWEST_WEIGHT  =  -std::numeric_limits<Weight>::infinity();

    /// Constant to mark that a certain cell is not a continuation to advance on
    const CellWeight NO_CONTINUATION = -std::numeric_limits<CellWeight>::infinity();

    ///Signal that a certain cell should not be constructed. Used by the creator filters
    const CellWeight NOT_A_CELL = -std::numeric_limits<CellWeight>::quiet_NaN();

    ///Returns if the filter signaled that the cell should not be constructed
    inline bool isNotACell(const CellWeight& cellWeight) { return isNAN(cellWeight); }

    ///Indicates that a connection in a neighborhood shall not be made. Used by neighorhood_choosers.
    const NeighborWeight NOT_A_NEIGHBOR =  std::numeric_limits<NeighborWeight>::quiet_NaN();

    ///Returns if the weight indicates that a certain connection in a neighborhood shall not be made.
    inline bool isNotANeighbor(const NeighborWeight& weight) { return isNAN(weight); }
    */
    /**@}*/

  } // CDCLocalTracking
} // namespace Belle2
#endif // BASICCONSTANTS
