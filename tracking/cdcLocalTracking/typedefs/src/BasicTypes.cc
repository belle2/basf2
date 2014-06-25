/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include "../include/BasicTypes.h"

#include <tracking/cdcLocalTracking/topology/CDCWireTopology.h>

#include <limits>



using namespace std;
using namespace Belle2;
using namespace CDCLocalTracking;





const Weight CDCLocalTracking::HIGHEST_WEIGHT = std::numeric_limits<Weight>::infinity();

const Weight CDCLocalTracking::LOWEST_WEIGHT = -std::numeric_limits<Weight>::infinity();





bool CDCLocalTracking::isValidISuperLayer(const ISuperLayerType& iSuperLayer)
{
  return 0 <= iSuperLayer and iSuperLayer < NSUPERLAYERS;
}





bool CDCLocalTracking::isLogicISuperLayer(const ISuperLayerType& iSuperLayer)
{
  return INNER_ISUPERLAYER <= iSuperLayer and iSuperLayer <= OUTER_ISUPERLAYER;
}





ISuperLayerType CDCLocalTracking::getISuperLayerAtPolarR(const FloatType& polarR)
{
  const CDCWireTopology& cdcWireTopology = CDCWireTopology::getInstance();
  const std::vector<CDCWireSuperLayer>& wireSuperLayers = cdcWireTopology.getWireSuperLayers();

  if (isnan(polarR) or polarR < 0) return INVALID_ISUPERLAYER;

  if (polarR < cdcWireTopology.getWireSuperLayer(0).getInnerPolarR()) return INNER_ISUPERLAYER;

  for (const CDCWireSuperLayer & wireSuperLayer : wireSuperLayers) {
    if (polarR <= wireSuperLayer.getOuterPolarR()) {
      return wireSuperLayer.getISuperLayer();
    }
  }

  return OUTER_ISUPERLAYER;

}





ISuperLayerType CDCLocalTracking::isAxialISuperLayer(const ISuperLayerType& iSuperLayer)
{
  return isValidISuperLayer(iSuperLayer) and isEven(iSuperLayer);
}
