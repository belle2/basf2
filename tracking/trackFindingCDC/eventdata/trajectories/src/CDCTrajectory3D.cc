/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "../include/CDCTrajectory3D.h"
#include "../include/CDCBField.h"


#include <framework/logging/Logger.h>
#include <tracking/cdcLocalTracking/topology/CDCWireTopology.h>

#include <cmath>
#include <cassert>

using namespace std;
using namespace Belle2;
using namespace CDCLocalTracking;

CDCLOCALTRACKING_SwitchableClassImp(CDCTrajectory3D)


CDCTrajectory3D::CDCTrajectory3D(const Vector3D& pos3D,
                                 const Vector3D& mom3D,
                                 const FloatType& charge) :
  m_localOrigin(pos3D),
  m_localHelix(absMom2DToCurvature(mom3D.xy().norm(), charge, pos3D),
               mom3D.xy().unit(),
               0.0,
               mom3D.cotTheta(),
               0.0)
{
}



CDCTrajectory3D::CDCTrajectory3D(const MCParticle& mcParticle) :
  CDCTrajectory3D(mcParticle.getProductionVertex(),
                  mcParticle.getMomentum(),
                  mcParticle.getCharge())
{




}



CDCTrajectory3D::CDCTrajectory3D(const CDCTrajectory2D& trajectory2D,
                                 const CDCTrajectorySZ& trajectorySZ) :
  m_localOrigin(trajectory2D.getLocalOrigin()),
  m_localHelix(trajectory2D.getLocalCircle(), trajectorySZ.getSZLine())
{
}



void CDCTrajectory3D::setPosMom3D(const Vector3D& pos3D,
                                  const Vector3D& mom3D,
                                  const FloatType& charge)
{
  m_localOrigin = pos3D;
  m_localHelix = UncertainHelix(absMom2DToCurvature(mom3D.xy().norm(), charge, pos3D),
                                mom3D.xy().unit(),
                                0.0,
                                mom3D.cotTheta(),
                                0.0);
}



void CDCTrajectory3D::fillInto(genfit::TrackCand& trackCand) const
{
  Vector3D position = getSupport();
  Vector3D momentum = getMom3DAtSupport();

  // The initial covariance matrix is calculated from these errors and it is important (!!) that it is not completely wrong - took them from Oksana
  // Currently unused
  TVector3 posError;
  posError.SetXYZ(2.0, 2.0, 2.0);
  TVector3 momError;
  momError.SetXYZ(0.1, 0.1, 0.5);

  // Set the start parameters
  SignType q = getChargeSign();
  trackCand.setPosMomSeed(position, momentum, q);

}




SignType CDCTrajectory3D::getChargeSign() const
{
  return ccwInfoToChargeSign(getLocalHelix().circleXY().orientation());
}



FloatType CDCTrajectory3D::getAbsMom3D() const
{
  FloatType szSlope = getLocalHelix().szSlope();

  FloatType factor2DTo3D = hypot(1, szSlope);

  FloatType curvatureXY = getLocalHelix().curvatureXY();

  FloatType absMom2D =  curvatureToAbsMom2D(curvatureXY);

  return factor2DTo3D * absMom2D;

  FloatType absMomZ = absMom2D * szSlope;

  return hypot(absMom2D, absMomZ);
}




