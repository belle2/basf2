/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/topology/ISuperLayerType.h>

#include <tracking/trackFindingCDC/topology/CDCWireTopology.h>

#include <limits>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;





bool TrackFindingCDC::isValidISuperLayer(const ISuperLayerType& iSuperLayer)
{
  return 0 <= iSuperLayer and iSuperLayer < NSUPERLAYERS;
}





bool TrackFindingCDC::isLogicISuperLayer(const ISuperLayerType& iSuperLayer)
{
  return INNER_ISUPERLAYER <= iSuperLayer and iSuperLayer <= OUTER_ISUPERLAYER;
}





ISuperLayerType TrackFindingCDC::getISuperLayerAtCylindricalR(const double cylindricalR)
{
  const CDCWireTopology& cdcWireTopology = CDCWireTopology::getInstance();
  const std::vector<CDCWireSuperLayer>& wireSuperLayers = cdcWireTopology.getWireSuperLayers();

  if (std::isnan(cylindricalR) or cylindricalR < 0) return INVALID_ISUPERLAYER;

  if (cylindricalR < cdcWireTopology.getWireSuperLayer(0).getInnerCylindricalR()) return INNER_ISUPERLAYER;

  for (const CDCWireSuperLayer& wireSuperLayer : wireSuperLayers) {
    if (cylindricalR <= wireSuperLayer.getOuterCylindricalR()) {
      return wireSuperLayer.getISuperLayer();
    }
  }

  return OUTER_ISUPERLAYER;

}





ISuperLayerType TrackFindingCDC::isAxialISuperLayer(const ISuperLayerType& iSuperLayer)
{
  return isValidISuperLayer(iSuperLayer) and isEven(iSuperLayer);
}
