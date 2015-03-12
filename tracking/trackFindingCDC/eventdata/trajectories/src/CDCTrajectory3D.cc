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
#include <tracking/trackFindingCDC/topology/CDCWireTopology.h>

#include <cmath>
#include <cassert>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

TRACKFINDINGCDC_SwitchableClassImp(CDCTrajectory3D)



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

CDCTrajectory3D::CDCTrajectory3D(const CDCTrajectory2D& trajectory2D) :
  CDCTrajectory3D(trajectory2D, CDCTrajectorySZ::basicAssumption())
{
}


CDCTrajectory3D::CDCTrajectory3D(const genfit::TrackCand& gfTrackCand) :
  CDCTrajectory3D(gfTrackCand.getPosSeed(),
                  gfTrackCand.getMomSeed(),
                  gfTrackCand.getChargeSeed())
{
  // Maybe push these out of this function:
  // Indices of the cartesian coordinates
  const int iX = 0;
  const int iY = 1;
  const int iZ = 2;
  const int iPx = 3;
  const int iPy = 4;
  const int iPz = 5;

  TMatrixDSym cov6 = gfTrackCand.getCovSeed();

  // 1. Rotate to a system where phi0 = 0
  TMatrixD jacobianRot(6, 6);
  jacobianRot.Zero();

  const double px = gfTrackCand.getStateSeed()[iPx];
  const double py = gfTrackCand.getStateSeed()[iPy];
  const double pt = hypot(px, py);

  const double cosPhi0 = px / pt;
  const double sinPhi0 = py / pt;

  // Passive rotation matrix by phi0:
  jacobianRot(iX, iX) = cosPhi0;
  jacobianRot(iX, iY) = sinPhi0;
  jacobianRot(iY, iX) = -sinPhi0;
  jacobianRot(iY, iY) = cosPhi0;
  jacobianRot(iZ, iZ) = 1.0;

  jacobianRot(iPx, iPx) = cosPhi0;
  jacobianRot(iPx, iPy) = sinPhi0;
  jacobianRot(iPy, iPx) = -sinPhi0;
  jacobianRot(iPy, iPy) = cosPhi0;
  jacobianRot(iPz, iPz) = 1.0;

  cov6.Similarity(jacobianRot);

  // 2. Translate to perigee parameters
  TMatrixD jacobianReduce(5, 6);
  jacobianReduce.Zero();

  const double invPt = 1 / pt;
  const double invPtSquared = invPt * invPt;
  const double pz = gfTrackCand.getStateSeed()[iPz];
  const double alpha = getAlphaZ(gfTrackCand.getPosSeed());
  const double charge = gfTrackCand.getChargeSeed();

  jacobianReduce(iCurv, iPx) = charge * invPtSquared / alpha ;
  jacobianReduce(iPhi0, iPy) = invPt;
  jacobianReduce(iI, iY) = 1;
  jacobianReduce(iSZ, iPx) = - pz * invPtSquared;
  jacobianReduce(iSZ, iPz) = invPt;
  jacobianReduce(iZ0, iZ) = 1;
  // Note the column corresponding to iX is completely zero as expectable.

  cov6.Similarity(jacobianReduce);

  // The covariance should now be the correct 5x5 covariance matrix.
  m_localHelix.setHelixCovariance(HelixCovariance(cov6));
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



bool CDCTrajectory3D::fillInto(genfit::TrackCand& gfTrackCand) const
{
  // Set the start parameters
  Vector3D position = getSupport();
  Vector3D momentum = getMom3DAtSupport();
  SignType charge = getChargeSign();

  // Do not propagate invalid fits, signal that the fit is invalid to the caller.
  if (not isValidSign(charge) or momentum.hasNAN() or position.hasNAN()) {
    // B2INFO("Charge " <<  charge);
    // B2INFO("Position " <<  position);
    // B2INFO("Local origin " <<  getLocalOrigin());
    // B2INFO("Momentum " <<  momentum);
    return false;
  }

  gfTrackCand.setPosMomSeed(position, momentum, charge);

  // Now translate and set the covariance matrix.
  const UncertainHelix& localHelix = getLocalHelix();

  const FloatType& impactXY = localHelix.impactXY();
  const Vector2D& tangentialXY = localHelix.tangentialXY();

  const FloatType& cosPhi0 = tangentialXY.x();
  const FloatType& sinPhi0 = tangentialXY.y();

  const FloatType& curvatureXY = localHelix.curvatureXY();
  const FloatType& tanLambda = localHelix.tanLambda();

  // 0. Define indices
  // Maybe push these out of this function:
  // Indices of the cartesian coordinates
  const int iX = 0;
  const int iY = 1;
  const int iZ = 2;
  const int iPx = 3;
  const int iPy = 4;
  const int iPz = 5;

  TMatrixDSym cov5 = localHelix.helixCovariance();

  // 1. Inflat the perigee covariance to a cartesian covariance where phi0 = 0 is assumed
  // Jacobian matrix for the translation
  TMatrixD jacobianInflate(6, 5);
  jacobianInflate.Zero();

  const double alpha = getAlphaZ(position);
  const double chargeAlphaCurv = charge * alpha * curvatureXY;
  const double chargeAlphaCurv2 = charge * alpha * std::pow(curvatureXY, 2);

  const double invChargeAlphaCurv = 1.0 / chargeAlphaCurv;
  const double invChargeAlphaCurv2 = 1.0 / chargeAlphaCurv2;

  // Position
  jacobianInflate(iX, iPhi0) = -impactXY;
  jacobianInflate(iY, iI) = 1.0;
  jacobianInflate(iZ, iZ0) = 1.0;

  // Momentum
  jacobianInflate(iPx, iCurv) = invChargeAlphaCurv2;
  jacobianInflate(iPy, iPhi0) = - invChargeAlphaCurv;
  jacobianInflate(iPz, iCurv) = tanLambda * invChargeAlphaCurv2;
  jacobianInflate(iPz, iSZ) = - invChargeAlphaCurv;

  // Transform
  TMatrixDSym cov6 = cov5; //copy
  cov6.Similarity(jacobianInflate);

  /// 2. Rotate to the right phi0
  TMatrixD jacobianRot(6, 6);
  jacobianRot.Zero();

  // Active rotation matrix by phi0:
  jacobianRot(iX, iX) = cosPhi0;
  jacobianRot(iX, iY) = -sinPhi0;
  jacobianRot(iY, iX) = sinPhi0;
  jacobianRot(iY, iY) = cosPhi0;
  jacobianRot(iZ, iZ) = 1.0;

  jacobianRot(iPx, iPx) = cosPhi0;
  jacobianRot(iPx, iPy) = -sinPhi0;
  jacobianRot(iPy, iPx) = sinPhi0;
  jacobianRot(iPy, iPy) = cosPhi0;
  jacobianRot(iPz, iPz) = 1.0;

  cov6.Similarity(jacobianRot);

  // 3. Forward the covariance matrix.
  gfTrackCand.setCovSeed(cov6);

  return true;
}




SignType CDCTrajectory3D::getChargeSign() const
{
  return ccwInfoToChargeSign(getLocalHelix().circleXY().orientation());
}



FloatType CDCTrajectory3D::getAbsMom3D() const
{
  Vector3D position = getSupport();

  FloatType szSlope = getLocalHelix().szSlope();

  FloatType factor2DTo3D = hypot(1, szSlope);

  FloatType curvatureXY = getLocalHelix().curvatureXY();

  FloatType absMom2D =  curvatureToAbsMom2D(curvatureXY, position);

  return factor2DTo3D * absMom2D;

  FloatType absMomZ = absMom2D * szSlope;

  return hypot(absMom2D, absMomZ);
}




