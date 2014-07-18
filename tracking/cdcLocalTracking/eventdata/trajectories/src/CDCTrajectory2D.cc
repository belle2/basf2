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

//const FloatType CDCTrajectory2D::c_bFieldZMagnitude = 1.5;
//const SignType CDCTrajectory2D::c_bFieldZSign = PLUS;
//const FloatType CDCTrajectory2D::c_bFieldZ = c_bFieldZSign * c_bFieldZMagnitude;

namespace {
  // this class should not really be the provider of this information.
  // So use the corresponding functions which should eventually change to the correct
  // information provider.
  // definitions in source file

  /// Constant for the magnetic field strength in z direction ( in Tesla )
  const FloatType c_bFieldZMagnitude = 1.5;

  /// Constant for the sign of the magnetic field in z direction. To be checked.
  const SignType c_bFieldZSign = PLUS; // to be checked

  /// Constant for the signed magnetic field strength in z direction ( in Tesla )
  const FloatType c_bFieldZ = c_bFieldZSign* c_bFieldZMagnitude;


  /// Getter for the absolute magnetic field strength in z direction ( in Tesla )
  inline const FloatType& getBFieldZMagnitude(const Vector2D& pos2D __attribute__((unused)) = Vector2D(0.0, 0.0))
  { return c_bFieldZMagnitude; }

  /// Getter for the sign of the magnetic field in z direction
  inline const SignType& getBFieldZSign()
  { return c_bFieldZSign; }

  /// Getter for the signed of the magnetic field stength in z direction ( in Tesla )
  inline const FloatType& getBFieldZ(const Vector2D& pos2D __attribute__((unused)) = Vector2D(0.0, 0.0))
  { return c_bFieldZ; }



  /// Conversion helper from clockwise or counterclockwise travel to the charge sign.
  /** Return the charge sign based on the travel direction on the fitted circle. \n
   *  With the Lorentz force F = q * v x B \n
   *  For positively charged particles we have \n
   *  Counterclockwise travel <-> Bz < 0 \n
   *  Clockwise travel        <-> Bz > 0 \n
   *  and opposite for negatively charged. \n
   *  Hence the charge sign is -CCWInfo * sign(Bz) */
  SignType ccwInfoToChargeSign(const CCWInfo& ccwInfo)
  { return - ccwInfo * getBFieldZSign(); }



  /// Conversion helper from the charge sign to clockwise or counterclockwise travel
  CCWInfo chargeSignToCCWInfo(const SignType& chargeSign)
  { return - chargeSign * getBFieldZSign(); }



  /// Conversion help for charges to clockwise or counterclockwise travel.
  CCWInfo chargeToCCWInfo(const FloatType& charge)
  { return chargeSignToCCWInfo(sign(charge)); }



  /// Conversion helper for momenta to radii
  FloatType absMom2DToRadius(const FloatType& absMom2D, const FloatType& charge, const Vector2D& pos2D = Vector2D(0.0, 0.0))
  { return - absMom2D / (charge * getBFieldZ(pos2D)  * 0.00299792458); }


  FloatType absMom2DToCurvature(const FloatType& absMom2D, const FloatType& charge, const Vector2D& pos2D = Vector2D(0.0, 0.0))
  { return - charge * getBFieldZ(pos2D) * 0.00299792458 / absMom2D; }


  FloatType curvatureToAbsMom2D(const FloatType& curvature, const Vector2D pos2D = Vector2D(0.0, 0.0))
  { return std::fabs(getBFieldZ(pos2D) * 0.00299792458 / curvature); }

  FloatType curvatureToAbsMom2D(const FloatType& curvature, const FloatType& charge, const Vector2D& pos2D = Vector2D(0.0, 0.0))
  { return  - charge *  getBFieldZ(pos2D) * 0.00299792458 / curvature ; }

}




CDCTrajectory2D::CDCTrajectory2D(const Vector2D& pos2D,
                                 const Vector2D& mom2D,
                                 const FloatType& charge) :
  m_localOrigin(pos2D),
  m_localPerigeeCircle(absMom2DToCurvature(mom2D.norm(), charge, pos2D), mom2D.unit(), 0.0)

{
}



void CDCTrajectory2D::setPosMom2D(const Vector2D& pos2D,
                                  const Vector2D& mom2D,
                                  const FloatType& charge)
{
  m_localOrigin = pos2D;
  m_localPerigeeCircle = UncertainPerigeeCircle(absMom2DToCurvature(mom2D.norm(), charge, pos2D), mom2D.unit(), 0.0);

  // FloatType mom = mom2D.norm();
  // FloatType r = momToRadius(mom, charge);

  // SignType chargeSign = sign(charge);
  // CCWInfo orientation = chargeSignToCCWInfo(chargeSign);

  // // For counterclockwise travel the circle center is to the left viewed from the travel direction
  // // For clockwise travel to the right
  // // Use the correct orthogonal vector to the center
  // Vector2D posToCenter = mom2D.orthogonal(orientation);

  // // Scale to have the length of the radius to have it point exactly to the circle center
  // posToCenter.scale(r / mom);

  // //same memory different name
  // Vector2D& circleCenter = posToCenter.add(pos2D);

  // m_perigeeCircle = PerigeeCircle::fromCenterAndRadius(circleCenter, r, orientation);

  // setStartPos2D(pos2D);

}



SignType CDCTrajectory2D::getChargeSign() const
{
  return ccwInfoToChargeSign(getLocalCircle().orientation());
}



FloatType CDCTrajectory2D::getAbsMom2D() const
{
  return curvatureToAbsMom2D(getLocalCircle().curvature());
}



Vector3D CDCTrajectory2D::reconstruct3D(const BoundSkewLine& globalSkewLine) const
{
  Vector2D globalRefPos2D = globalSkewLine.refPos2D();
  PerigeeCircle globalCircle = getGlobalCircle();

  FloatType firstorder = globalCircle.fastDistance(globalRefPos2D);

  FloatType crossComponent = globalRefPos2D.cross(globalCircle.n12());
  FloatType quadraticComponent = globalRefPos2D.normSquared() * globalCircle.n3();

  const FloatType& a = quadraticComponent;
  const FloatType& b = crossComponent;
  const FloatType& c = firstorder;

  std::pair<FloatType, FloatType> solutionsSkewTimesDeltaZ = solveQuadraticABC(a, b, c);

  //std::pair<FloatType,FloatType> invSolutions = CDCLocalTracking::solveQuadraticPQ(crossComponent, firstorder*quadraticComponent);
  //FloatType deltaZTimesSkew = firstorder / invSolutions.first;

  // Take the solution with the smaller deviation from the reference position
  const FloatType& smallerSolution = solutionsSkewTimesDeltaZ.second;

  return globalSkewLine.pos3DAtDeltaZTimesSkew(smallerSolution);

}


Vector2D CDCTrajectory2D::getInnerExit() const
{

  const CDCWireTopology& topology = CDCWireTopology::getInstance();
  const CDCWireLayer& innerMostLayer = topology.getWireLayers().front();

  FloatType innerPolarR = innerMostLayer.getInnerPolarR();

  return getGlobalCircle().samePolarRForwardOf(getLocalOrigin(), innerPolarR);

}

Vector2D CDCTrajectory2D::getOuterExit() const
{

  const CDCWireTopology& topology = CDCWireTopology::getInstance();
  const CDCWireLayer& outerMostLayer = topology.getWireLayers().back();

  FloatType outerPolarR = outerMostLayer.getOuterPolarR();

  return getGlobalCircle().samePolarRForwardOf(getLocalOrigin(), outerPolarR);

}

Vector2D CDCTrajectory2D::getExit() const
{
  Vector2D outerExit = getOuterExit();
  Vector2D innerExit = getInnerExit();

  return getGlobalCircle().chooseNextForwardOf(getLocalOrigin(), outerExit, innerExit);

}



ISuperLayerType CDCTrajectory2D::getISuperLayerAfter(const ISuperLayerType& fromISuperLayer, bool movingOutward, const ForwardBackwardInfo& forwardBackwardInfo) const
{
  if (forwardBackwardInfo != FORWARD and forwardBackwardInfo != BACKWARD) return INVALID_ISUPERLAYER;
  if (fromISuperLayer == INVALID_ISUPERLAYER) return INVALID_ISUPERLAYER;

  if (fromISuperLayer == INNER_ISUPERLAYER or fromISuperLayer == OUTER_ISUPERLAYER) {
    // no extrapolation from the outside in yet
    return fromISuperLayer;
  } else {
    // Start position is inside the CDC
    ISuperLayerType minimalISuperLayer = getMinimalISuperLayer();
    ISuperLayerType maximalISuperLayer = getMaximalISuperLayer();

    if (maximalISuperLayer == minimalISuperLayer) {
      // Trajectory is limited to a single superlayer.
      // There is no next superlayer.
      return fromISuperLayer;

    } else {
      // Trajectory traverses several superlayers (including the logical INNER_ISUPERLAYER and OUTER_ISUPERLAYER)
      if (fromISuperLayer == maximalISuperLayer) {
        return fromISuperLayer - 1;
      } else if (fromISuperLayer == minimalISuperLayer) {
        return fromISuperLayer + 1;
      } else {
        ISuperLayerType iSuperLayerStep = movingOutward ? forwardBackwardInfo : -forwardBackwardInfo;
        return fromISuperLayer + iSuperLayerStep;
      }

    }
  }
}

ISuperLayerType CDCTrajectory2D::getISuperLayerAfterStart(const ForwardBackwardInfo& forwardBackwardInfo) const
{
  bool movingOutward = isMovingOutward();
  ISuperLayerType startISuperLayer = getStartISuperLayer();
  return getISuperLayerAfter(startISuperLayer, movingOutward, forwardBackwardInfo);
}


ISuperLayerType CDCTrajectory2D::getNextISuperLayer() const
{
  return getISuperLayerAfterStart(FORWARD);
}



ISuperLayerType CDCTrajectory2D::getPreviousISuperLayer() const
{
  return getISuperLayerAfterStart(BACKWARD);
}



ISuperLayerType CDCTrajectory2D::getAxialISuperLayerAfterStart(const ForwardBackwardInfo& forwardBackwardInfo) const
{

  ISuperLayerType startISuperLayer = getStartISuperLayer();
  if (startISuperLayer == INVALID_ISUPERLAYER) return INVALID_ISUPERLAYER;

  ISuperLayerType nextISuperLayer =  getISuperLayerAfterStart(forwardBackwardInfo);
  if (nextISuperLayer == INVALID_ISUPERLAYER) return INVALID_ISUPERLAYER;

  ISuperLayerType iSuperLayerStep = nextISuperLayer - startISuperLayer;

  assert(std::abs(iSuperLayerStep) <= 1);

  if (isAxialISuperLayer(nextISuperLayer)) return nextISuperLayer;
  // do not try to attempt to come back from the outside
  else if (nextISuperLayer == OUTER_ISUPERLAYER) return INVALID_ISUPERLAYER;
  else if (nextISuperLayer == INNER_ISUPERLAYER) return 0;
  else if (nextISuperLayer == startISuperLayer) {
    // If the next superlayer is already the same as the current one and it is not an axial layer
    // the trajectory is fully contained in a true axial layer which implies that there is no next axial layer.
    return INVALID_ISUPERLAYER;
  } else {
    // the next layer is a true stereo layer go to the next layer which is always an axial layer.
    bool movingOutward = iSuperLayerStep * forwardBackwardInfo > 0;
    return getISuperLayerAfter(nextISuperLayer, movingOutward, forwardBackwardInfo);
  }

}


ISuperLayerType CDCTrajectory2D::getNextAxialISuperLayer() const
{
  return getAxialISuperLayerAfterStart(FORWARD);
}



ISuperLayerType CDCTrajectory2D::getPreviousAxialISuperLayer() const
{
  return getAxialISuperLayerAfterStart(BACKWARD);
}



ISuperLayerType CDCTrajectory2D::getMaximalISuperLayer() const
{
  FloatType maximalPolarR = getMaximalPolarR();
  return getISuperLayerAtPolarR(maximalPolarR);
}


ISuperLayerType CDCTrajectory2D::getStartISuperLayer() const
{
  FloatType startPolarR = getLocalOrigin().polarR();
  return getISuperLayerAtPolarR(startPolarR);
}



ISuperLayerType CDCTrajectory2D::getMinimalISuperLayer() const
{
  FloatType minimalPolarR = getMinimalPolarR();
  return getISuperLayerAtPolarR(minimalPolarR);
}







