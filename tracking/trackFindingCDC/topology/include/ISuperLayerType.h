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

#include <iterator>
#include <tracking/trackFindingCDC/utilities/Algorithms.h>

namespace Belle2 {

  namespace TrackFindingCDC {

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
    ISuperLayerType getISuperLayerAtCylindricalR(const double cylindricalR);

    /// Returns the logical superlayer number at the given radius
    ISuperLayerType isAxialISuperLayer(const ISuperLayerType& iSuperLayer);


    /** Returns the common superlayer of hits in a container.
     *  INVALID_ISUPERLAYER if there is no common super layer or the container is empty.
     */
    template<class AHits>
    ISuperLayerType getISuperLayer(const AHits& hits)
    {
      using Hit = ValueType<AHits>;
      auto getISuperLayerOfHit = [](const Hit & hit) {return hit->getISuperLayer();};
      return common(hits, getISuperLayerOfHit, INVALID_ISUPERLAYER);
    }

  } // namespace TrackFindingCDC

} // namespace Belle2
