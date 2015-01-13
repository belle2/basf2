/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef ISUPERLAYERTYPE_H
#define ISUPERLAYERTYPE_H

#include <tracking/cdcLocalTracking/typedefs/BasicTypes.h>

namespace Belle2 {

  namespace CDCLocalTracking {

    /// The type of the layer and superlayer ids
    typedef signed short ISuperLayerType;

    /// Constant representing the total number of cdc superlayers
    const ISuperLayerType NSUPERLAYERS = 9;

    /// Constant marking the subdetectors closer to the IP than the CDC.
    const ISuperLayerType INNER_ISUPERLAYER = -1;

    /// Constant marking the subdetectors further away from the IP than the CDC.
    const ISuperLayerType OUTER_ISUPERLAYER = NSUPERLAYERS;

    /// Constant making an invalid superlayer id
    const ISuperLayerType INVALID_ISUPERLAYER = 15;

    /// Indicates if the given number corresponds to a true cdc superlayer - excludes the logic ids for inner and outer volumn
    bool isValidISuperLayer(const ISuperLayerType& iSuperLayer);

    /// Indicates if the given number corresponds to a logical superlayer - includes the logic ids for inner and outer volumn
    bool isLogicISuperLayer(const ISuperLayerType& iSuperLayer);

    /// Returns the logical superlayer number at the given radius
    ISuperLayerType getISuperLayerAtPolarR(const FloatType& polarR);

    /// Returns the logical superlayer number at the given radius
    ISuperLayerType isAxialISuperLayer(const ISuperLayerType& iSuperLayer);


  } // namespace CDCLocalTracking

} // namespace Belle2
#endif // ISUPERLAYERTYPE_H
