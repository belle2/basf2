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

#include <cdc/geometry/CDCGeometryPar.h>

#include <tracking/trackFindingCDC/fitting/CDCObservations2D.h>
#include <tracking/trackFindingCDC/fitting/CDCSZFitter.h>

using namespace std;
using namespace Belle2;
using namespace CDC;

using namespace TrackFindingCDC;


CDCWireSuperLayer::CDCWireSuperLayer() {}

CDCWireSuperLayer::CDCWireSuperLayer(const const_iterator& begin, const const_iterator& end):
  m_begin(begin),
  m_end(end),
  m_innerRefZ(0.0),
  m_outerRefZ(0.0),
  m_refTanLambda(0.0)
{
  initialize();
}

void CDCWireSuperLayer::initialize()
{

  // set the clockwise shift info of the wire layers relativ the first layer in the superlayer
  // this section could get hard coded since the wire numbering is fixed in a certain manner
  // we just try to calculate it here not make any wrong guesses
  const CDCWireLayer& layerZero = first();
  const Vector3D& layerZeroRef3D = layerZero.first().getRefPos3D();
  const Vector2D& layerZeroRef2D = layerZeroRef3D.xy();

  // Prepare a fit to z versus the cylindricalR slope at the reference coordinates
  CDCObservations2D observations2D;
  CDCSZFitter szFitter;

  observations2D.clear();
  observations2D.reserve(size());

  for (const CDCWireLayer& layer : *this) {
    // Set the numbering shift of each layer within this superlayer
    if (isEven(layer.getILayer())) {
      layer.setShift(ERotation::c_Unknown);
    } else {
      const Vector3D& layerRefPos3D = layer.first().getRefPos3D();
      const Vector2D& layerRefPos2D = layerRefPos3D.xy();

      layer.setShift(layerRefPos2D.isCCWOrCWOf(layerZeroRef2D));
    }

    observations2D.append(layer.getRefCylindricalR(), layer.getRefZ());

  }

  CDCTrajectorySZ zVersusArcLength2D;
  szFitter.update(zVersusArcLength2D, observations2D);

  const UncertainSZLine& arcLength2DZLine = zVersusArcLength2D.getSZLine();

  m_refTanLambda = arcLength2DZLine.slope();
  m_innerRefZ = arcLength2DZLine.map(getInnerCylindricalR());
  m_outerRefZ = arcLength2DZLine.map(getOuterCylindricalR());


  /*
  CDCGeometryPar& cdcgp = CDCGeometryPar::Instance();

  //declared but not defined function
  //int momBound = cdcgp.momBound();
  int momBound = 7;
  //Check what momZ and momRmin contain
  for (int iBound = 0; iBound < momBound; ++iBound) {

    double momZ = cdcgp.momZ(iBound);
    double momRmin = cdcgp.momRmin(iBound);


    B2INFO("iBound : " << iBound );
    B2INFO("momZ : " << momZ );
    B2INFO("momRmin : " << momRmin);


  }
  //cin >> momBound;
  */

}

const CDCWireLayer* CDCWireSuperLayer::nextWireLayer(const CDCWireLayer* layer) const
{
  if (layer == nullptr) {
    if (begin() == end()) {
      return nullptr;
    } else {
      const CDCWireLayer& nextlayer = *begin();
      return &nextlayer;
    }
  }
  IWireType iLayer = layer->getILayer();
  if (isValidILayer(iLayer + 1)) {
    const CDCWireLayer& nextlayer = getWireLayer(iLayer + 1);
    return &nextlayer;
  } else {
    return nullptr;
  }
  return nullptr;
}

EWireNeighborKind CDCWireSuperLayer::getNeighborKind(const ILayerType& iLayer,
                                                     const IWireType& iWire,
                                                     const ILayerType& iOtherLayer,
                                                     const IWireType& iOtherWire) const
{
  ILayerType iLayerDifference = iOtherLayer - iLayer;

  if (iLayerDifference == 0 and
      isValidILayer(iLayer)) {

    const CDCWireLayer& layer = getWireLayer(iLayer);
    if (iWire == (iOtherWire + 1) % layer.size()) return EWireNeighborKind::c_CW;
    else if ((iWire + 1) % layer.size()  ==  iOtherWire) return EWireNeighborKind::c_CCW;
    else return EWireNeighborKind::c_None;

  } else if (iLayerDifference == -1 and
             isValidILayer(iLayer)  and
             isValidILayer(iOtherLayer)) {

    const CDCWireLayer& layer = getWireLayer(iLayer);
    const CDCWireLayer& otherLayer = getWireLayer(iOtherLayer);
    const ERotation deltaShift = getShiftDelta(otherLayer.getShift(), layer.getShift());

    if (iWire == iOtherWire) {
      if (deltaShift == ERotation::c_Clockwise) return EWireNeighborKind::c_CWIn;
      else if (deltaShift == ERotation::c_CounterClockwise) return EWireNeighborKind::c_CCWIn;
      else return EWireNeighborKind::c_None;

    } else if (iWire == (iOtherWire + 1) % otherLayer.size()) {
      if (deltaShift == ERotation::c_CounterClockwise) return EWireNeighborKind::c_CWIn;
      else return EWireNeighborKind::c_None;

    } else if ((iWire + 1) % layer.size()  ==  iOtherWire) {
      if (deltaShift == ERotation::c_Clockwise) return EWireNeighborKind::c_CCWIn;
      else return EWireNeighborKind::c_None;

    } else return EWireNeighborKind::c_None;

  } else if (iLayerDifference == 1 and
             isValidILayer(iLayer) and
             isValidILayer(iOtherLayer)) {

    const CDCWireLayer& layer = getWireLayer(iLayer);
    const CDCWireLayer& otherLayer = getWireLayer(iOtherLayer);
    const ERotation deltaShift = getShiftDelta(otherLayer.getShift(), layer.getShift());

    if (iWire == iOtherWire) {
      if (deltaShift == ERotation::c_Clockwise) return EWireNeighborKind::c_CWOut;
      else if (deltaShift == ERotation::c_CounterClockwise) return EWireNeighborKind::c_CCWOut;
      else return EWireNeighborKind::c_None;

    } else if (iWire == (iOtherWire + 1) % otherLayer.size()) {
      if (deltaShift == ERotation::c_CounterClockwise) return EWireNeighborKind::c_CWOut;
      else return EWireNeighborKind::c_None;

    } else if ((iWire + 1) % layer.size()  ==  iOtherWire) {
      if (deltaShift == ERotation::c_Clockwise) return EWireNeighborKind::c_CCWOut;
      else return EWireNeighborKind::c_None;

    } else return EWireNeighborKind::c_None;

  } else return EWireNeighborKind::c_None;

}

CDCWireSuperLayer::NeighborPair CDCWireSuperLayer::getNeighborsInwards(const ILayerType& iLayer, const IWireType& iWire) const
{

  if (not isValidILayer(iLayer - 1)) return NeighborPair(nullptr , nullptr);

  const CDCWireLayer& layer = getWireLayer(iLayer);
  const CDCWireLayer& neighborLayer = getWireLayer(iLayer - 1);
  const ERotation deltaShift = getShiftDelta(neighborLayer.getShift(), layer.getShift());

  if (deltaShift == ERotation::c_CounterClockwise) {
    return NeighborPair(&(neighborLayer.getWireSafe(iWire)) , &(neighborLayer.getWireSafe(iWire - 1)));
  } else if (deltaShift == ERotation::c_Clockwise) {
    return NeighborPair(&(neighborLayer.getWireSafe(iWire + 1)) , &(neighborLayer.getWireSafe(iWire)));
  } else {
    B2WARNING("Wire numbering shift bigger than one in magnitude. Adjust getNeighbor functions " << static_cast<int>(deltaShift));
    B2WARNING("From iLayer " << iLayer << " and wire " << iWire << " to iLayer " << iLayer - 1);
    return NeighborPair(nullptr , nullptr);
  }

}

CDCWireSuperLayer::NeighborPair CDCWireSuperLayer::getNeighborsOutwards(const ILayerType& iLayer, const IWireType& iWire) const
{

  if (not isValidILayer(iLayer + 1)) return NeighborPair(nullptr, nullptr);

  const CDCWireLayer& layer = getWireLayer(iLayer);
  const CDCWireLayer& neighborLayer = getWireLayer(iLayer + 1);
  const ERotation deltaShift = getShiftDelta(neighborLayer.getShift(), layer.getShift());

  if (deltaShift == ERotation::c_CounterClockwise) {
    return NeighborPair(&(neighborLayer.getWireSafe(iWire)), &(neighborLayer.getWireSafe(iWire - 1)));
  } else if (deltaShift == ERotation::c_Clockwise) {
    return NeighborPair(&(neighborLayer.getWireSafe(iWire + 1)), &(neighborLayer.getWireSafe(iWire)));
  } else {
    B2WARNING("Wire numbering shift bigger than one in magnitude. Adjust getNeighbor functions " << static_cast<int>(deltaShift));
    B2WARNING("From iLayer " << iLayer << " and wire " << iWire << " to iLayer " << iLayer + 1);
    return NeighborPair(nullptr , nullptr);
  }

}
