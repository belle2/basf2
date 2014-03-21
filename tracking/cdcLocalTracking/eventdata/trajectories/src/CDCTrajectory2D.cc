/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "../include/CDCTrajectory2D.h"

#include <framework/logging/Logger.h>
#include <tracking/cdcLocalTracking/topology/CDCWireTopology.h>

#include <cmath>
#include <cassert>

using namespace std;
using namespace Belle2;
using namespace CDCLocalTracking;

ClassImpInCDCLocalTracking(CDCTrajectory2D)



CDCTrajectory2D::CDCTrajectory2D(const Vector2D& startPoint,
                                 const Vector2D& startMomentum,
                                 const FloatType& charge) : m_genCircle(), m_startPos2D(startPoint)
{

  setStartPosMom2D(startPoint, startMomentum, charge);

}


FloatType CDCTrajectory2D::setStartPos2D(const Vector2D& point)
{

  FloatType result = calcPerpS(point);
  m_startPos2D = getClosest(point);
  //cout << "New projTravelDistanceRef " << m_projTravelDistanceRef << endl;
  return result;
}

void
CDCTrajectory2D::setStartPosMom2D(
  const Vector2D& pos2D,
  const Vector2D& mom2D,
  FloatType charge
)
{

  FloatType mom = mom2D.norm();
  FloatType r = momToRadius(mom, charge);

  SignType chargeSign = sign(charge);
  CCWInfo orientation = chargeSignToCCWInfo(chargeSign);

  // For counterclockwise travel the circle center is to the left viewed from the travel direction
  // For clockwise travel to the right
  // Use the correct orthogonal vector to the center
  Vector2D posToCenter = mom2D.orthogonal(orientation);

  // Scale to have the length of the radius to have it point exactly to the circle center
  posToCenter.scale(r / mom);

  //same memory different name
  Vector2D& circleCenter = posToCenter.add(pos2D);

  m_genCircle.setCenterAndRadius(circleCenter, r, orientation);

  setStartPos2D(pos2D);

}

Vector3D CDCTrajectory2D::reconstruct3D(const BoundSkewLine& skewLine) const
{

  const Vector2D& refPos2D = skewLine.refPos2D();
  const GeneralizedCircle genCircle = getGenCircle();

  FloatType firstorder = genCircle.fastDistance(refPos2D);

  //FloatType crossComponent = refPos2D.x() * getGenCircle().n2() -
  //                           refPos2D.y() * getGenCircle().n1();

  FloatType crossComponent = refPos2D.cross(genCircle.n12());

  FloatType quadraticComponent = refPos2D.normSquared() * genCircle.n3();

  const FloatType& a = quadraticComponent;
  const FloatType& b = crossComponent;
  const FloatType& c = firstorder;

  std::pair<FloatType, FloatType> solutionsSkewTimesDeltaZ = solveQuadraticABC(a, b, c);

  //std::pair<FloatType,FloatType> invSolutions = CDCLocalTracking::solveQuadraticPQ(crossComponent, firstorder*quadraticComponent);

  //FloatType deltaZTimesSkew = firstorder/invSolutions.first;

  // Take the solution with the smaller deviation from the reference position
  const FloatType& smallerSolution = solutionsSkewTimesDeltaZ.second;

  return skewLine.pos3DAtDeltaZTimesSkew(smallerSolution);

}


Vector2D CDCTrajectory2D::getInnerExit() const
{

  const CDCWireTopology& topology = CDCWireTopology::getInstance();
  const CDCWireLayer& innerMostLayer = topology.getWireLayers().front();

  FloatType innerPolarR = innerMostLayer.getInnerPolarR();

  return getGenCircle().samePolarRForwardOf(getStartPos2D(), innerPolarR);

}

Vector2D CDCTrajectory2D::getOuterExit() const
{

  const CDCWireTopology& topology = CDCWireTopology::getInstance();
  const CDCWireLayer& outerMostLayer = topology.getWireLayers().back();

  FloatType outerPolarR = outerMostLayer.getOuterPolarR();

  return getGenCircle().samePolarRForwardOf(getStartPos2D(), outerPolarR);

}

Vector2D CDCTrajectory2D::getExit() const
{
  Vector2D outerExit = getOuterExit();
  Vector2D innerExit = getInnerExit();

  return getGenCircle().chooseNextForwardOf(getStartPos2D(), outerExit, innerExit);

}



/// Indicates which superlayer the trajectory traverses after the current one
ISuperLayerType CDCTrajectory2D::getNextISuperLayer() const
{
  bool movingOutward = isMovingOutward();

  ISuperLayerType startISuperLayer = getStartISuperLayer();
  if (startISuperLayer == INVALID_ISUPERLAYER) return INVALID_ISUPERLAYER;

  if (startISuperLayer == INNER_ISUPERLAYER or startISuperLayer == OUTER_ISUPERLAYER) {
    // no extrapolation from the outside in yet
    return INVALID_ISUPERLAYER;
  } else {
    // Start position is inside the CDC
    ISuperLayerType minimalISuperLayer = getMinimalISuperLayer();
    ISuperLayerType maximalISuperLayer = getMaximalISuperLayer();

    if (maximalISuperLayer == minimalISuperLayer) {
      // Trajectory is limited to a single superlayer.
      // There is no next superlayer.
      return INVALID_ISUPERLAYER;

    } else {
      // Trajectory traverses several superlayers (including the logical INNER_ISUPERLAYER and OUTER_ISUPERLAYER
      if (startISuperLayer == maximalISuperLayer) {
        return startISuperLayer - 1;
      } else if (startISuperLayer == minimalISuperLayer) {
        return startISuperLayer + 1;
      } else {
        return movingOutward ? startISuperLayer + 1 : startISuperLayer - 1;
      }

    }
  }
}



/// Indicates which axial superlayer the trajectory traverses after the current one
ISuperLayerType CDCTrajectory2D::getNextAxialISuperLayer() const
{
  ISuperLayerType startISuperLayer = getStartISuperLayer();
  if (startISuperLayer == INVALID_ISUPERLAYER) return INVALID_ISUPERLAYER;

  ISuperLayerType nextISuperLayer =  getNextISuperLayer();
  if (nextISuperLayer == INVALID_ISUPERLAYER) return INVALID_ISUPERLAYER;

  ISuperLayerType iSuperLayerStep = nextISuperLayer - startISuperLayer;

  assert(std::abs(iSuperLayerStep) == 1);

  if (isAxialISuperLayer(nextISuperLayer)) return nextISuperLayer;
  // do not try to attempt to come back from the outside
  else if (nextISuperLayer == OUTER_ISUPERLAYER) return INVALID_ISUPERLAYER;
  else if (nextISuperLayer == INNER_ISUPERLAYER) return 0;
  else {
    // nextISuperLayer is not axial nor does it refer to logical inner or outer layer
    // Hence it is true stereo layer
    // Evaluated if the trajectory is curling back in this superlayer
    ISuperLayerType maximalISuperLayer = getMaximalISuperLayer();
    if (maximalISuperLayer == INVALID_ISUPERLAYER) return INVALID_ISUPERLAYER;

    ISuperLayerType minimalISuperLayer = getMinimalISuperLayer();
    if (minimalISuperLayer == INVALID_ISUPERLAYER) return INVALID_ISUPERLAYER;

    if (nextISuperLayer == maximalISuperLayer) return nextISuperLayer - 1;   // Curling back to inwards
    else if (nextISuperLayer == minimalISuperLayer) return nextISuperLayer + 1;   // Curling back to outwards
    else return nextISuperLayer + iSuperLayerStep ;
  }
}



ISuperLayerType CDCTrajectory2D::getMaximalISuperLayer() const
{
  FloatType maximalPolarR = getMaximalPolarR();
  return getISuperLayerAtPolarR(maximalPolarR);
}


ISuperLayerType CDCTrajectory2D::getStartISuperLayer() const
{
  FloatType startPolarR = getStartPos2D().polarR();
  return getISuperLayerAtPolarR(startPolarR);
}



ISuperLayerType CDCTrajectory2D::getMinimalISuperLayer() const
{
  FloatType minimalPolarR = getMinimalPolarR();
  return getISuperLayerAtPolarR(minimalPolarR);
}



const FloatType CDCTrajectory2D::c_bFieldZMagnitude = 1.5;
const SignType CDCTrajectory2D::c_bFieldZSign = PLUS;
const FloatType CDCTrajectory2D::c_bFieldZ = c_bFieldZSign * c_bFieldZMagnitude;




