/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <framework/dataobjects/UncertainHelix.h>

#include <assert.h>

using namespace Belle2;
using namespace HelixParameterIndex;

ClassImp(UncertainHelix);


UncertainHelix::UncertainHelix() :
  Helix(),
  m_covariance(5),
  m_pValue(0)
{
}

UncertainHelix::UncertainHelix(const TVector3& position,
                               const TVector3& momentum,
                               const short int charge,
                               const double bZ,
                               const TMatrixDSym& cartesianCovariance,
                               const double pValue) :
  Helix(TVector3(0.0, 0.0, position.Z()), momentum, charge, bZ),
  m_covariance(cartesianCovariance), // Initialize the covariance matrix to the 6x6 covariance and reduce it afterwards
  m_pValue(pValue)
{
  // Maybe push these out of this function:
  // Indices of the cartesian coordinates
  const int iX = 0;
  const int iY = 1;
  const int iZ = 2;
  const int iPx = 3;
  const int iPy = 4;
  const int iPz = 5;

  // We initialised the m_covariance to the cartesian covariance and
  // reduce it now to the real 5x5 matrix that should be there.

  // 1. Rotate to a system where phi0 = 0
  TMatrixD jacobianRot(6, 6);
  jacobianRot.Zero();

  const double px = momentum.X();
  const double py = momentum.Y();
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

  m_covariance.Similarity(jacobianRot);

  // 2. Translate to perigee parameters
  const double pz = momentum.Z();
  const double invPt = 1 / pt;
  const double invPtSquared = invPt * invPt;
  const double omega = getOmega();
  const double alpha = getAlpha(bZ);

  TMatrixD jacobianToHelixParameters(5, 6);
  jacobianToHelixParameters.Zero();

  jacobianToHelixParameters(iD0, iY) = charge;
  jacobianToHelixParameters(iPhi0, iX) = omega;
  jacobianToHelixParameters(iPhi0, iPy) = invPt;
  jacobianToHelixParameters(iOmega, iPx) = charge * invPtSquared / alpha ;
  jacobianToHelixParameters(iTanLambda, iPx) = - pz * invPtSquared;
  jacobianToHelixParameters(iTanLambda, iPz) = invPt;
  jacobianToHelixParameters(iZ0, iZ) = 1;
  m_covariance.Similarity(jacobianToHelixParameters);

  // 3. Extrapolate to the origin.
  TVector3 by(-position.X(), -position.Y(), 0.0);
  TMatrixD jacobianPassiveMove(5, 5);
  calcPassiveMoveByJacobian(by, jacobianPassiveMove);

  m_covariance.Similarity(jacobianPassiveMove);
  /*const double arcLength2D = */ Helix::passiveMoveBy(TVector3(-position.X(), -position.Y(), 0.0));

  // The covariance m_covariance is now the correct 5x5 covariance matrix.
  assert(m_covariance.GetNrows() == 5);
}


UncertainHelix::UncertainHelix(const double& d0,
                               const double& phi0,
                               const double& omega,
                               const double& z0,
                               const double& tanLambda,
                               const TMatrixDSym& covariance,
                               const double pValue) :
  Helix(d0, phi0, omega, z0, tanLambda),
  m_covariance(covariance),
  m_pValue(pValue)
{
}



TMatrixDSym UncertainHelix::getCartesianCovariance(const double bZ) const
{
  // 0. Define indices
  // Maybe push these out of this function:
  // Indices of the cartesian coordinates
  const int iX = 0;
  const int iY = 1;
  const int iZ = 2;
  const int iPx = 3;
  const int iPy = 4;
  const int iPz = 5;

  // Transform covariance matrix
  TMatrixDSym cov6 = m_covariance; //copy

  // 1. Move the reference point to the perigee
  // In this way we are making sure that all covariance entries related to component parallel to phi are zero.
  TVector3 perigee = getPerigee();
  TVector3 by(perigee);
  TMatrixD jacobianPassiveMove(5, 5);
  calcPassiveMoveByJacobian(by, jacobianPassiveMove);
  cov6.Similarity(jacobianPassiveMove);

  TMatrixD jacobianInflate(6, 5);
  jacobianInflate.Zero();

  // 2. Inflate the perigee covariance to a cartesian covariance where phi0 == 0 and d0 == 0 is assumed
  // d0 == 0 because we moved the reference point into the perigee point first.
  // The real phi0 is a simple rotation which can be handled in the next step.
  // Jacobian matrix for the translation

  //const double& d0 = getD0(); // == 0 since we moved to the perigee point first.
  const double& omega = getOmega();
  const double& tanLambda = getTanLambda();
  const short& charge = getChargeSign();

  const double alpha = getAlpha(bZ);
  const double chargeAlphaOmega = charge * alpha * omega;
  const double chargeAlphaOmega2 = charge * alpha * std::pow(omega, 2);

  const double invChargeAlphaOmega = 1.0 / chargeAlphaOmega;
  const double invChargeAlphaOmega2 = 1.0 / chargeAlphaOmega2;

  // Position after the move.
  jacobianInflate(iX, iPhi0) = 0.0; //charge * d0 == 0 after moving
  jacobianInflate(iY, iD0) = charge;
  jacobianInflate(iZ, iZ0) = 1.0;

  // Momentum
  jacobianInflate(iPx, iOmega) = invChargeAlphaOmega2;
  jacobianInflate(iPy, iPhi0) = invChargeAlphaOmega;
  jacobianInflate(iPz, iOmega) = tanLambda * invChargeAlphaOmega2;
  jacobianInflate(iPz, iTanLambda) = invChargeAlphaOmega;
  cov6.Similarity(jacobianInflate);

  /// 3. Rotate to the right phi0
  const double& cosPhi0 = getCosPhi0();
  const double& sinPhi0 = getSinPhi0();

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
  return cov6;
}


double UncertainHelix::passiveMoveBy(const TVector3& by)
{
  // Move the covariance matrix first to have access to the original parameters
  TMatrixD jacobian = calcPassiveMoveByJacobian(by);
  m_covariance.Similarity(jacobian);
  return Helix::passiveMoveBy(by);
}
