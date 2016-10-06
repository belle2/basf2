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
#include <tracking/trackFindingCDC/numerics/Quadratic.h>

#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>

#include <cmath>
#include <cassert>

using namespace Belle2;
using namespace TrackFindingCDC;


CDCTrajectory2D::CDCTrajectory2D(const Vector2D& pos2D,
                                 const double time,
                                 const Vector2D& mom2D,
                                 const double charge,
                                 const double bZ)
  : m_localOrigin(pos2D),
    m_localPerigeeCircle(CDCBFieldUtil::absMom2DToCurvature(mom2D.norm(), charge, bZ),
                         mom2D.unit(),
                         0.0),
    m_flightTime(time)
{
}

CDCTrajectory2D::CDCTrajectory2D(const Vector2D& pos2D,
                                 const double time,
                                 const Vector2D& mom2D,
                                 const double charge)
  : m_localOrigin(pos2D),
    m_localPerigeeCircle(CDCBFieldUtil::absMom2DToCurvature(mom2D.norm(), charge, pos2D),
                         mom2D.unit(),
                         0.0),
    m_flightTime(time)
{
}

Vector3D CDCTrajectory2D::reconstruct3D(const WireLine& wireLine,
                                        const double distance) const
{
  Vector2D globalRefPos2D = wireLine.refPos2D();
  Vector2D movePerZ = wireLine.nominalMovePerZ();

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

  Vector3D recoWirePos2D = wireLine.nominalPos3DAtZ(z);
  return Vector3D(getClosest(recoWirePos2D.xy()), z);
}

ISuperLayer CDCTrajectory2D::getISuperLayerAfter(ISuperLayer iSuperLayer, bool movingOutward) const
{
  if (ISuperLayerUtil::isInvalid(iSuperLayer)) return ISuperLayerUtil::c_Invalid;

  ISuperLayer minimalISuperLayer = getMinimalISuperLayer();
  ISuperLayer maximalISuperLayer = getMaximalISuperLayer();
  if (minimalISuperLayer == maximalISuperLayer) return ISuperLayerUtil::c_Invalid; // No next super layer to go to
  if (iSuperLayer == minimalISuperLayer) return ISuperLayerUtil::getNextOutwards(iSuperLayer);
  if (iSuperLayer == maximalISuperLayer) return ISuperLayerUtil::getNextInwards(iSuperLayer);

  if (movingOutward) return ISuperLayerUtil::getNextOutwards(iSuperLayer);
  else return ISuperLayerUtil::getNextInwards(iSuperLayer);
}

ISuperLayer CDCTrajectory2D::getISuperLayerAfterStart(bool movingOutward) const
{
  ISuperLayer iSuperLayer = getStartISuperLayer();
  return getISuperLayerAfter(iSuperLayer, movingOutward);
}

ISuperLayer CDCTrajectory2D::getISuperLayerAfterStart(const EForwardBackward forwardBackwardInfo) const
{
  bool movingOutward = isMovingOutward();
  if (forwardBackwardInfo == EForwardBackward::c_Backward) {
    movingOutward = not movingOutward;
  }
  return getISuperLayerAfterStart(movingOutward);
}

ISuperLayer CDCTrajectory2D::getNextISuperLayer() const
{
  return getISuperLayerAfterStart(EForwardBackward::c_Forward);
}

ISuperLayer CDCTrajectory2D::getPreviousISuperLayer() const
{
  return getISuperLayerAfterStart(EForwardBackward::c_Backward);
}

ISuperLayer CDCTrajectory2D::getAxialISuperLayerAfterStart(const EForwardBackward forwardBackwardInfo) const
{
  bool movingOutward = isMovingOutward();
  if (forwardBackwardInfo == EForwardBackward::c_Backward) {
    movingOutward = not movingOutward;
  }
  ISuperLayer startISuperLayer = getStartISuperLayer();
  if (ISuperLayerUtil::isInvalid(startISuperLayer)) return ISuperLayerUtil::c_Invalid;

  ISuperLayer nextISuperLayer = getISuperLayerAfter(startISuperLayer, movingOutward);
  if (ISuperLayerUtil::isInvalid(nextISuperLayer)) return ISuperLayerUtil::c_Invalid;
  if (ISuperLayerUtil::isAxial(nextISuperLayer)) return nextISuperLayer;

  ISuperLayer iSuperLayerStep = nextISuperLayer - startISuperLayer;
  assert(std::abs(iSuperLayerStep) == 1);
  bool nextMovingOutward = iSuperLayerStep > 0;
  return getISuperLayerAfter(nextISuperLayer, nextMovingOutward);
}

ISuperLayer CDCTrajectory2D::getNextAxialISuperLayer() const
{
  return getAxialISuperLayerAfterStart(EForwardBackward::c_Forward);
}

ISuperLayer CDCTrajectory2D::getPreviousAxialISuperLayer() const
{
  return getAxialISuperLayerAfterStart(EForwardBackward::c_Backward);
}

ISuperLayer CDCTrajectory2D::getMaximalISuperLayer() const
{
  double maximalCylindricalR = getMaximalCylindricalR();
  return CDCWireTopology::getInstance().getISuperLayerAtCylindricalR(maximalCylindricalR);
}

ISuperLayer CDCTrajectory2D::getStartISuperLayer() const
{
  double startCylindricalR = getLocalOrigin().cylindricalR();
  return CDCWireTopology::getInstance().getISuperLayerAtCylindricalR(startCylindricalR);
}

ISuperLayer CDCTrajectory2D::getMinimalISuperLayer() const
{
  double minimalCylindricalR = getMinimalCylindricalR();
  return CDCWireTopology::getInstance().getISuperLayerAtCylindricalR(minimalCylindricalR);
}

bool CDCTrajectory2D::isCurler(double factor) const
{
  const CDCWireTopology& topology = CDCWireTopology::getInstance();
  return getMaximalCylindricalR() < factor * topology.getOuterCylindricalR();
}

ESign CDCTrajectory2D::getChargeSign() const
{
  return CDCBFieldUtil::ccwInfoToChargeSign(getLocalCircle()->orientation());
}

double CDCTrajectory2D::getAbsMom2D(const double bZ) const
{
  return CDCBFieldUtil::curvatureToAbsMom2D(getLocalCircle()->curvature(), bZ);
}

double CDCTrajectory2D::getAbsMom2D() const
{
  Vector2D position = getSupport();
  return CDCBFieldUtil::curvatureToAbsMom2D(getLocalCircle()->curvature(), position);
}

Vector2D CDCTrajectory2D::getInnerExit() const
{
  const CDCWireTopology& topology = CDCWireTopology::getInstance();
  const CDCWireLayer& innerMostLayer = topology.getWireLayers().front();
  double innerCylindricalR = innerMostLayer.getInnerCylindricalR();

  const Vector2D support = getSupport();
  const PerigeeCircle globalCircle = getGlobalCircle();
  if (support.cylindricalR() < innerCylindricalR) {
    // If we start within the inner volumn of the CDC we want the trajectory to enter the CDC
    // and not stop at first intersection with the inner wall.
    // Therefore we take the inner exit that comes after the apogee (far point of the circle).
    const Vector2D apogee = globalCircle.apogee();
    return globalCircle.atCylindricalRForwardOf(apogee, innerCylindricalR);

  } else {
    return globalCircle.atCylindricalRForwardOf(support, innerCylindricalR);
  }
}

Vector2D CDCTrajectory2D::getOuterExit(double factor) const
{
  const CDCWireTopology& topology = CDCWireTopology::getInstance();
  const CDCWireLayer& outerMostLayer = topology.getWireLayers().back();
  double outerCylindricalR = outerMostLayer.getOuterCylindricalR() * factor;

  const Vector2D support = getSupport();
  const PerigeeCircle globalCircle = getGlobalCircle();
  if (support.cylindricalR() > outerCylindricalR) {
    // If we start outside of the volume of the CDC we want the trajectory to enter the CDC
    // and not stop at first intersection with the outer wall.
    // Therefore we take the outer exit that comes after the perigee.
    const Vector2D perigee = globalCircle.perigee();
    return globalCircle.atCylindricalRForwardOf(perigee, outerCylindricalR);

  } else {
    return getGlobalCircle().atCylindricalRForwardOf(support, outerCylindricalR);
  }
}

Vector2D CDCTrajectory2D::getExit() const
{
  const Vector2D outerExit = getOuterExit();
  const Vector2D innerExit = getInnerExit();
  const Vector2D localExit =  getLocalCircle()->chooseNextForwardOf(Vector2D(0, 0),
                              outerExit - getLocalOrigin(),
                              innerExit - getLocalOrigin());
  return localExit + getLocalOrigin();
}

void CDCTrajectory2D::setPosMom2D(const Vector2D& pos2D,
                                  const Vector2D& mom2D,
                                  const double charge)
{
  m_localOrigin = pos2D;
  double curvature = CDCBFieldUtil::absMom2DToCurvature(mom2D.norm(), charge, pos2D);
  Vector2D phiVec = mom2D.unit();
  double impact = 0.0;
  m_localPerigeeCircle = UncertainPerigeeCircle(curvature, phiVec, impact);
}

double CDCTrajectory2D::setLocalOrigin(const Vector2D& localOrigin)
{
  double arcLength2D = calcArcLength2D(localOrigin);
  m_flightTime += arcLength2D / Const::speedOfLight;
  m_localPerigeeCircle.passiveMoveBy(localOrigin - m_localOrigin);
  m_localOrigin = localOrigin;
  return arcLength2D;
}
