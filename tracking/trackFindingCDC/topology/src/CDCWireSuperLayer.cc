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

using namespace Belle2;
using namespace TrackFindingCDC;

CDCWireSuperLayer::CDCWireSuperLayer(const ConstVectorRange<CDCWireLayer>& layerRange)
  : Super(layerRange),
    m_innerRefZ(0.0),
    m_outerRefZ(0.0),
    m_refTanLambda(0.0)
{
  initialize();
}

void CDCWireSuperLayer::initialize()
{
}

EWireNeighborKind CDCWireSuperLayer::getNeighborKind(ILayer iLayer,
                                                     IWire iWire,
                                                     ILayer iOtherLayer,
                                                     IWire iOtherWire) const
{
  if (not isValidILayer(iLayer)) return EWireNeighborKind::c_None;
  if (not isValidILayer(iOtherLayer)) return EWireNeighborKind::c_None;

  const ILayer iLayerDifference = iOtherLayer - iLayer;
  const CDCWireLayer& layer = getWireLayer(iLayer);
  const CDCWireLayer& otherLayer = getWireLayer(iOtherLayer);
  const ERotation deltaShift = otherLayer.getShiftDelta(layer);
  const IWire nWires = layer.size();

  if (iLayerDifference == 0) {
    if (IWireUtil::getNextCW(iWire, nWires) == iOtherWire) return EWireNeighborKind::c_CW;
    else if (IWireUtil::getNextCCW(iWire, nWires) == iOtherWire) return EWireNeighborKind::c_CCW;
    else return EWireNeighborKind::c_None;

  } else if (iLayerDifference == -1) {

    if (iWire == iOtherWire) {
      if (deltaShift == ERotation::c_Clockwise) return EWireNeighborKind::c_CWIn;
      else if (deltaShift == ERotation::c_CounterClockwise) return EWireNeighborKind::c_CCWIn;
      else return EWireNeighborKind::c_None;

    } else if (IWireUtil::getNextCW(iWire, nWires) == iOtherWire) {
      if (deltaShift == ERotation::c_CounterClockwise) return EWireNeighborKind::c_CWIn;
      else return EWireNeighborKind::c_None;

    } else if (IWireUtil::getNextCCW(iWire, nWires) == iOtherWire) {
      if (deltaShift == ERotation::c_Clockwise) return EWireNeighborKind::c_CCWIn;
      else return EWireNeighborKind::c_None;

    } else return EWireNeighborKind::c_None;

  } else if (iLayerDifference == 1) {

    if (iWire == iOtherWire) {
      if (deltaShift == ERotation::c_Clockwise) return EWireNeighborKind::c_CWOut;
      else if (deltaShift == ERotation::c_CounterClockwise) return EWireNeighborKind::c_CCWOut;
      else return EWireNeighborKind::c_None;

    } else if (IWireUtil::getNextCW(iWire, nWires) == iOtherWire) {
      if (deltaShift == ERotation::c_CounterClockwise) return EWireNeighborKind::c_CWOut;
      else return EWireNeighborKind::c_None;

    } else if (IWireUtil::getNextCCW(iWire, nWires) == iOtherWire) {
      if (deltaShift == ERotation::c_Clockwise) return EWireNeighborKind::c_CCWOut;
      else return EWireNeighborKind::c_None;

    } else return EWireNeighborKind::c_None;

  } else return EWireNeighborKind::c_None;

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
