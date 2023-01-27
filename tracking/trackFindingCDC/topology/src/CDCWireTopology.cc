/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <tracking/trackFindingCDC/topology/CDCWireTopology.h>

#include <tracking/trackFindingCDC/utilities/Algorithms.h>

#include <tracking/trackFindingCDC/utilities/Common.h>

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
  m_FirstLayerOffset = cdcGeo.getOffsetOfFirstLayer();

  for (size_t iCLayer = 0; iCLayer < cdcGeo.nWireLayers() ; ++iCLayer) {
    for (size_t iWire = 0; iWire < cdcGeo.nWiresInLayer(iCLayer); ++iWire) {
      m_wires.push_back(CDCWire(WireID(iCLayer, iWire)));
    }
  }

  // create all wire layers
  std::vector<VectorRange<CDCWire>> wiresByILayer =
                                   adjacent_groupby(m_wires.begin(), m_wires.end(), GetILayer());

  for (VectorRange<CDCWire> wiresForILayer : wiresByILayer) {
    m_wireLayers.push_back(CDCWireLayer(wiresForILayer));
  }

  // create all superlayers
  std::vector<VectorRange<CDCWireLayer> > layersByISuperLayer =
    adjacent_groupby(m_wireLayers.begin(), m_wireLayers.end(), Common<GetISuperLayer>());

  if (cdcGeo.getOffsetOfFirstSuperLayer() > 0) {
    for (uint superLayer = 0; superLayer < cdcGeo.getOffsetOfFirstSuperLayer(); ++superLayer) {
      m_wireSuperLayers.push_back(CDCWireSuperLayer());
    }
  }

  for (VectorRange<CDCWireLayer> layersForISuperLayer : layersByISuperLayer) {
    m_wireSuperLayers.push_back(CDCWireSuperLayer(layersForISuperLayer));
  }
}

void CDCWireTopology::reinitialize(EWirePosition wirePosition, bool ignoreWireSag)
{
  for (CDCWire& wire : m_wires) {
    wire.initialize(wirePosition, ignoreWireSag);
  }

  for (CDCWireLayer& wireLayer : m_wireLayers) {
    wireLayer.initialize();
  }

  for (CDCWireSuperLayer& wireSuperLayer : m_wireSuperLayers) {
    wireSuperLayer.initialize();
  }
}

ISuperLayer CDCWireTopology::getISuperLayerAtCylindricalR(double cylindricalR)
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

WireNeighborKind CDCWireTopology::getNeighborKind(const WireID& wireID, const WireID& otherWireID) const
{
  if (wireID.getISuperLayer() !=  otherWireID.getISuperLayer() and
      isValidISuperLayer(wireID.getISuperLayer())) {
    return WireNeighborKind();
  } else {
    const CDCWireSuperLayer& superlayer = getWireSuperLayer(wireID.getISuperLayer());
    return superlayer.getNeighborKind(wireID.getILayer(),
                                      wireID.getIWire(),
                                      otherWireID.getILayer(),
                                      otherWireID.getIWire());
  }
}
