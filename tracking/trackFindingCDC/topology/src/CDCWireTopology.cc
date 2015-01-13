/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "../include/CDCWireTopology.h"

#include <cdc/geometry/CDCGeometryPar.h>

using namespace std;
using namespace Belle2;
using namespace CDCLocalTracking;
using namespace CDC;

CDCLOCALTRACKING_SwitchableClassImp(CDCWireTopology)


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

/*
const CDCWire * CDCWireTopology::nextWire(const CDCWire * wire) const{

  if (wire == nullptr){
    if ( begin() == end() ){
      return nullptr;
    } else {
      const CDCWire & nextwire = *begin();
      return &nextwire;
    }
  }
  IWireType iWire = wire->getIWire();

  if ( isValidIWire(iWire + 1) ){
    const CDCWire & nextwire = getWire(iWire + 1);
    return &nextwire;
  } else {
    return nullptr;
  }
  return nullptr;

}
*/


WireNeighborType CDCWireTopology::areNeighbors(const WireID& wireID, const WireID& otherID) const
{

  if (wireID.getISuperLayer() !=  otherID.getISuperLayer() and
      isValidISuperLayer(wireID.getISuperLayer())) {
    return 0;
  } else {
    const CDCWireSuperLayer& superlayer = getWireSuperLayer(wireID.getISuperLayer());
    return superlayer.areNeighbors(wireID.getILayer(), wireID.getIWire(), otherID.getILayer(), otherID.getIWire());
  }
}

// Definition of the singleton instance
CDCWireTopology* CDCWireTopology::s_instance = nullptr;




