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

using namespace Belle2;
using namespace TrackFindingCDC;

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

  m_wires.reserve(c_NWires);
  m_wireLayers.reserve(c_NLayers);
  m_wireSuperLayers.reserve(c_NSuperLayers);

  // create all wires
  CDC::CDCGeometryPar& cdcGeo = CDC::CDCGeometryPar::Instance();
  for (size_t iCLayer = 0; iCLayer < cdcGeo.nWireLayers() ; ++iCLayer) {
    for (size_t iWire = 0; iWire < cdcGeo.nWiresInLayer(iCLayer); ++iWire) {
      m_wires.push_back(CDCWire(WireID(iCLayer, iWire)));
    }
  }

  // create all wire layers
  std::vector<VectorRange<CDCWire> > wiresByILayer =
    adjacent_groupby(m_wires.begin(), m_wires.end(),
                     ILayerUtil::getFrom<CDCWire>);

  for (VectorRange<CDCWire> wiresForILayer : wiresByILayer) {
    m_wireLayers.push_back(CDCWireLayer(wiresForILayer));
  }

  // create all superlayers
  std::vector<VectorRange<CDCWireLayer> > layersByISuperLayer =
    adjacent_groupby(m_wireLayers.begin(), m_wireLayers.end(),
                     ISuperLayerUtil::getFrom<CDCWireLayer>);

  for (VectorRange<CDCWireLayer> layersForISuperLayer : layersByISuperLayer) {
    m_wireSuperLayers.push_back(CDCWireSuperLayer(layersForISuperLayer));
  }
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
    return superlayer.getNeighborKind(wireID.getILayer(), wireID.getIWire(),
                                      otherID.getILayer(), otherID.getIWire());
  }
}
