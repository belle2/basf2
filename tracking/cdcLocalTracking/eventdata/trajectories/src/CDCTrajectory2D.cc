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

const FloatType CDCTrajectory2D::c_bFieldZMagnitude = 1.5;
const SignType CDCTrajectory2D::c_bFieldZSign = PLUS;
const FloatType CDCTrajectory2D::c_bFieldZ = c_bFieldZSign * c_bFieldZMagnitude;

