/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectory2D.h>

#include <tracking/trackFindingCDC/topology/CDCWireTopology.h>
#include <tracking/trackFindingCDC/eventdata/trajectories/CDCBField.h>

#include <framework/logging/Logger.h>

#include <cmath>
#include <cassert>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;



CDCTrajectory2D::CDCTrajectory2D(const Vector2D& pos2D,
                                 const Vector2D& mom2D,
                                 const double charge,
                                 const double bZ) :
  m_localOrigin(pos2D),
  m_localPerigeeCircle(absMom2DToCurvature(mom2D.norm(), charge, bZ), mom2D.unit(), 0.0)

{
}

CDCTrajectory2D::CDCTrajectory2D(const Vector2D& pos2D,
                                 const Vector2D& mom2D,
                                 const double charge) :
  m_localOrigin(pos2D),
  m_localPerigeeCircle(absMom2DToCurvature(mom2D.norm(), charge, pos2D), mom2D.unit(), 0.0)

{
}



void CDCTrajectory2D::setPosMom2D(const Vector2D& pos2D,
                                  const Vector2D& mom2D,
                                  const double charge)
{
  m_localOrigin = pos2D;
  m_localPerigeeCircle = UncertainPerigeeCircle(absMom2DToCurvature(mom2D.norm(), charge, pos2D), mom2D.unit(), 0.0);
}



ESign CDCTrajectory2D::getChargeSign() const
{
  return ccwInfoToChargeSign(getLocalCircle().orientation());
}


double CDCTrajectory2D::getAbsMom2D(const double bZ) const
{
  return curvatureToAbsMom2D(getLocalCircle().curvature(), bZ);
}

double CDCTrajectory2D::getAbsMom2D() const
{
  Vector2D position = getSupport();
  return curvatureToAbsMom2D(getLocalCircle().curvature(), position);
}



Vector3D CDCTrajectory2D::reconstruct3D(const WireLine& wireLine,
                                        const double distance) const
{
  Vector2D globalRefPos2D = wireLine.refPos2D();
  Vector2D movePerZ = wireLine.movePerZ();

  Vector2D localRefPos2D = globalRefPos2D - getLocalOrigin();
  const PerigeeCircle& localCircle = getLocalCircle();

  double fastDistance = distance != 0.0 ? localCircle.fastDistance(distance) : 0.0;

  double c = localCircle.fastDistance(localRefPos2D) - fastDistance;
  double b = localCircle.gradient(localRefPos2D).dot(movePerZ);
  double a = localCircle.n3() * movePerZ.normSquared();

  std::pair<double, double> solutionsDeltaZ = solveQuadraticABC(a, b, c);

  // Take the solution with the smaller deviation from the reference position
  const double deltaZ = solutionsDeltaZ.second;
  const double z = deltaZ + wireLine.refZ();
  return wireLine.pos3DAtZ(z);
}


Vector2D CDCTrajectory2D::getInnerExit() const
{
  const CDCWireTopology& topology = CDCWireTopology::getInstance();
  const CDCWireLayer& innerMostLayer = topology.getWireLayers().front();
  double innerCylindricalR = innerMostLayer.getInnerCylindricalR();

  const Vector2D support = getSupport();
  const GeneralizedCircle globalCircle = getGlobalCircle();
  if (support.cylindricalR() < innerCylindricalR) {
    // If we start inside of the volume of the CDC we want the trajectory to enter the CDC
    // and not stop at first intersection with the inner wall.
    // Therefore we take the inner exit that comes after the apogee (far point of the circle).
    const Vector2D apogee = globalCircle.apogee();
    return globalCircle.sameCylindricalRForwardOf(apogee, innerCylindricalR);

  } else {
    return globalCircle.sameCylindricalRForwardOf(support, innerCylindricalR);
  }
}

Vector2D CDCTrajectory2D::getOuterExit() const
{

  const CDCWireTopology& topology = CDCWireTopology::getInstance();
  const CDCWireLayer& outerMostLayer = topology.getWireLayers().back();

  double outerCylindricalR = outerMostLayer.getOuterCylindricalR();

  const Vector2D support = getSupport();
  const GeneralizedCircle globalCircle = getGlobalCircle();
  if (support.cylindricalR() > outerCylindricalR) {
    // If we start outside of the volume of the CDC we want the trajectory to enter the CDC
    // and not stop at first intersection with the outer wall.
    // Therefore we take the outer exit that comes after the perigee.
    const Vector2D perigee = globalCircle.perigee();
    return globalCircle.sameCylindricalRForwardOf(perigee, outerCylindricalR);

  } else {
    return getGlobalCircle().sameCylindricalRForwardOf(support, outerCylindricalR);
  }
}

Vector2D CDCTrajectory2D::getExit() const
{
  Vector2D outerExit = getOuterExit();
  const Vector2D innerExit = getInnerExit();
  return getGlobalCircle().chooseNextForwardOf(getLocalOrigin(), outerExit, innerExit);
}



ISuperLayerType CDCTrajectory2D::getISuperLayerAfter(const ISuperLayerType& fromISuperLayer, bool movingOutward,
                                                     const EForwardBackward forwardBackwardInfo) const
{
  if (forwardBackwardInfo != EForwardBackward::c_Forward
      and forwardBackwardInfo != EForwardBackward::c_Backward) return INVALID_ISUPERLAYER;
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

ISuperLayerType CDCTrajectory2D::getISuperLayerAfterStart(const EForwardBackward forwardBackwardInfo) const
{
  bool movingOutward = isMovingOutward();
  ISuperLayerType startISuperLayer = getStartISuperLayer();
  return getISuperLayerAfter(startISuperLayer, movingOutward, forwardBackwardInfo);
}


ISuperLayerType CDCTrajectory2D::getNextISuperLayer() const
{
  return getISuperLayerAfterStart(EForwardBackward::c_Forward);
}



ISuperLayerType CDCTrajectory2D::getPreviousISuperLayer() const
{
  return getISuperLayerAfterStart(EForwardBackward::c_Backward);
}



ISuperLayerType CDCTrajectory2D::getAxialISuperLayerAfterStart(const EForwardBackward forwardBackwardInfo) const
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
  return getAxialISuperLayerAfterStart(EForwardBackward::c_Forward);
}



ISuperLayerType CDCTrajectory2D::getPreviousAxialISuperLayer() const
{
  return getAxialISuperLayerAfterStart(EForwardBackward::c_Backward);
}



ISuperLayerType CDCTrajectory2D::getMaximalISuperLayer() const
{
  double maximalCylindricalR = getMaximalCylindricalR();
  return getISuperLayerAtCylindricalR(maximalCylindricalR);
}


ISuperLayerType CDCTrajectory2D::getStartISuperLayer() const
{
  double startCylindricalR = getLocalOrigin().cylindricalR();
  return getISuperLayerAtCylindricalR(startCylindricalR);
}



ISuperLayerType CDCTrajectory2D::getMinimalISuperLayer() const
{
  double minimalCylindricalR = getMinimalCylindricalR();
  return getISuperLayerAtCylindricalR(minimalCylindricalR);
}
