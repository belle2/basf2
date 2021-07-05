/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/topology/CDCWireSuperLayer.h>

#include <tracking/trackFindingCDC/numerics/Modulo.h>

using namespace Belle2;
using namespace TrackFindingCDC;

CDCWireSuperLayer::CDCWireSuperLayer(const ConstVectorRange<CDCWireLayer>& wireLayers)
  : Super(wireLayers)
  , m_innerRefZ(0.0)
  , m_outerRefZ(0.0)
  , m_refTanLambda(0.0)
{
  initialize();
}

void CDCWireSuperLayer::initialize()
{
}

WireNeighborKind CDCWireSuperLayer::getNeighborKind(ILayer iLayer,
                                                    IWire iWire,
                                                    ILayer iOtherLayer,
                                                    IWire iOtherWire) const
{
  if (not isValidILayer(iLayer)) return WireNeighborKind();
  if (not isValidILayer(iOtherLayer)) return WireNeighborKind();

  const ILayer iLayerDifference = iOtherLayer - iLayer;
  const ILayer absILayerDifference = abs(iLayerDifference);
  const CDCWireLayer& layer = getWireLayer(iLayer);
  const CDCWireLayer& otherLayer = getWireLayer(iOtherLayer);
  const ERotation deltaShift = otherLayer.getShiftDelta(layer);
  const IWire nWires = layer.size();

  int iRow = 2 * iWire;
  int iOtherRow = 2 * iOtherWire + deltaShift;
  int iRowDelta = symmetricModuloFast(iOtherRow - iRow, 2 * nWires);
  int absIRowDelta = abs(iRowDelta);

  if ((absILayerDifference + absIRowDelta) > 4 or absILayerDifference > 2) return WireNeighborKind(); // Invalid case
  if (absILayerDifference + absIRowDelta == 0) return WireNeighborKind(0, 0);

  int cellDistance = absILayerDifference == 2 ? 2 : (absILayerDifference + absIRowDelta) / 2;

  int slope = 0;
  if (iRowDelta == 0) {
    slope = 3;
  } else if (iLayerDifference == 0) {
    slope = 0;
  } else if (absIRowDelta == 3) {
    slope = 1;
  } else {
    slope = 2;
  }

  if (iRowDelta > 0) {
    slope = 6 - slope;
  }

  if (iLayerDifference > 0) {
    slope = -slope;
  }

  int oClockDirection = moduloFast(slope + 3 , 12);
  return WireNeighborKind(cellDistance, oClockDirection);
}

WireNeighborPair CDCWireSuperLayer::getNeighborsInwards(ILayer iLayer, IWire iWire) const
{
  if (not isValidILayer(iLayer - 1)) return WireNeighborPair(nullptr, nullptr);

  const CDCWireLayer& layer = getWireLayer(iLayer);
  const CDCWireLayer& neighborLayer = getWireLayer(iLayer - 1);
  const ERotation deltaShift = neighborLayer.getShiftDelta(layer);

  if (deltaShift == ERotation::c_CounterClockwise) {
    return WireNeighborPair(&(neighborLayer.getWireWrappedAround(iWire)),
                            &(neighborLayer.getWireWrappedAround(iWire - 1)));
  } else if (deltaShift == ERotation::c_Clockwise) {
    return WireNeighborPair(&(neighborLayer.getWireWrappedAround(iWire + 1)),
                            &(neighborLayer.getWireWrappedAround(iWire)));
  } else {
    B2WARNING("Wire numbering shift bigger than one in magnitude. Adjust getNeighbor functions " << static_cast<int>(deltaShift));
    B2WARNING("From iLayer " << iLayer << " and wire " << iWire << " to iLayer " << iLayer - 1);
    return WireNeighborPair(nullptr , nullptr);
  }

}

WireNeighborPair CDCWireSuperLayer::getNeighborsOutwards(ILayer iLayer, IWire iWire) const
{
  if (not isValidILayer(iLayer + 1)) return WireNeighborPair(nullptr, nullptr);

  const CDCWireLayer& layer = getWireLayer(iLayer);
  const CDCWireLayer& neighborLayer = getWireLayer(iLayer + 1);
  const ERotation deltaShift = neighborLayer.getShiftDelta(layer);

  if (deltaShift == ERotation::c_CounterClockwise) {
    return WireNeighborPair(&(neighborLayer.getWireWrappedAround(iWire)),
                            &(neighborLayer.getWireWrappedAround(iWire - 1)));
  } else if (deltaShift == ERotation::c_Clockwise) {
    return WireNeighborPair(&(neighborLayer.getWireWrappedAround(iWire + 1)),
                            &(neighborLayer.getWireWrappedAround(iWire)));
  } else {
    B2WARNING("Wire numbering shift bigger than one in magnitude. Adjust getNeighbor functions " << static_cast<int>(deltaShift));
    B2WARNING("From iLayer " << iLayer << " and wire " << iWire << " to iLayer " << iLayer + 1);
    return WireNeighborPair(nullptr , nullptr);
  }

}
