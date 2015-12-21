/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/trackFindingCDC/topology/CDCWireTopology.h>

#include <cdc/geometry/CDCGeometryPar.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;
using namespace CDC;


CDCWireTopology& CDCWireTopology::getInstance()
{
  // Definition of the singleton instance
  static CDCWireTopology instance;
  return instance;
}



void CDCWireTopology::initialize()
{

  m_wires.clear();
  m_wireLayers.clear();
  m_wireSuperLayers.clear();

  m_wires.reserve(N_WIRES);
  m_wireLayers.reserve(N_LAYERS);
  m_wireSuperLayers.reserve(N_SUPERLAYERS);

  //create all wires
  CDCGeometryPar& cdcGeo = CDCGeometryPar::Instance();
  for (size_t iCLayer = 0; iCLayer < cdcGeo.nWireLayers() ; ++iCLayer) {
    for (size_t iWire = 0; iWire < cdcGeo.nWiresInLayer(iCLayer); ++iWire) {
      m_wires.push_back(CDCWire(WireID(iCLayer, iWire)));
    }
  }
  iterator beginOfLayer = m_wires.begin();

  //create all wire layers
  for (iterator itWire = m_wires.begin(); itWire != m_wires.end(); ++itWire) {
    if (itWire->getILayer() != beginOfLayer->getILayer()) {
      m_wireLayers.push_back(CDCWireLayer(beginOfLayer , itWire));
      beginOfLayer = itWire;
    }
  }
  m_wireLayers.push_back(CDCWireLayer(beginOfLayer , m_wires.end()));

  //create all superlayers
  CDCWireSuperLayer::const_iterator beginOfSuperlayer = m_wireLayers.begin();

  for (CDCWireSuperLayer::const_iterator itLayer = m_wireLayers.begin();
       itLayer != m_wireLayers.end(); ++itLayer) {

    if (itLayer->getISuperLayer() != beginOfSuperlayer->getISuperLayer()) {
      m_wireSuperLayers.push_back(CDCWireSuperLayer(beginOfSuperlayer , itLayer));
      beginOfSuperlayer = itLayer;
    }
  }
  m_wireSuperLayers.push_back(CDCWireSuperLayer(beginOfSuperlayer , m_wireLayers.end()));
}

ISuperLayer CDCWireTopology::getISuperLayerAtCylindricalR(const double cylindricalR)
{
  const CDCWireTopology& cdcWireTopology = CDCWireTopology::getInstance();
  const std::vector<CDCWireSuperLayer>& wireSuperLayers = cdcWireTopology.getWireSuperLayers();

  if (std::isnan(cylindricalR) or cylindricalR < 0) {
    return ISuperLayerUtil::c_Invalid;
  }

  if (cylindricalR < cdcWireTopology.getWireSuperLayer(0).getInnerCylindricalR()) {
    return ISuperLayerUtil::c_InnerVolume;
  }

  for (const CDCWireSuperLayer& wireSuperLayer : wireSuperLayers) {
    if (cylindricalR <= wireSuperLayer.getOuterCylindricalR()) {
      return wireSuperLayer.getISuperLayer();
    }
  }

  return ISuperLayerUtil::c_OuterVolume;
}


EWireNeighborKind CDCWireTopology::getNeighborKind(const WireID& wireID, const WireID& otherID) const
{
  if (wireID.getISuperLayer() !=  otherID.getISuperLayer() and
      isValidISuperLayer(wireID.getISuperLayer())) {
    return EWireNeighborKind::c_None;
  } else {
    const CDCWireSuperLayer& superlayer = getWireSuperLayer(wireID.getISuperLayer());
    return superlayer.getNeighborKind(wireID.getILayer(), wireID.getIWire(), otherID.getILayer(), otherID.getIWire());
  }
}
