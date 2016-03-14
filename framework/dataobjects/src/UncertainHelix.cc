/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <framework/dataobjects/UncertainHelix.h>
#include <framework/logging/Logger.h>

#include <TVector3.h>

#include <boost/math/special_functions/sign.hpp>
#include <assert.h>

using namespace Belle2;
using namespace HelixParameterIndex;

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
  const double alpha = getAlpha(bZ);

  TMatrixD jacobianToHelixParameters(5, 6);
  jacobianToHelixParameters.Zero();

  jacobianToHelixParameters(iD0, iY) = charge;
  jacobianToHelixParameters(iPhi0, iX) = charge * invPt / alpha;
  jacobianToHelixParameters(iPhi0, iPy) = invPt;
  jacobianToHelixParameters(iOmega, iPx) = charge * invPtSquared / alpha;
  jacobianToHelixParameters(iTanLambda, iPx) = - pz * invPtSquared;
  jacobianToHelixParameters(iTanLambda, iPz) = invPt;
  jacobianToHelixParameters(iZ0, iZ) = 1;
  m_covariance.Similarity(jacobianToHelixParameters);

  // The covariance m_covariance is now the correct 5x5 covariance matrix.
  assert(m_covariance.GetNrows() == 5);

  // 3. Extrapolate to the origin.
  /*const double arcLength2D = */ passiveMoveBy(-position.X(), -position.Y(), 0.0);
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



TMatrixDSym UncertainHelix::getCartesianCovariance(const double bZ_tesla) const
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
  TMatrixD jacobianPassiveMove(5, 5);
  calcPassiveMoveByJacobian(getPerigeeX(), getPerigeeY(), jacobianPassiveMove);
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

  const double alpha = getAlpha(bZ_tesla);
  const double absAlphaOmega = alpha * std::fabs(omega);
  const double signedAlphaOmega2 =  absAlphaOmega  * omega;

  const double invAbsAlphaOmega = 1.0 / absAlphaOmega;
  const double invSignedAlphaOmega2 = 1.0 / signedAlphaOmega2;

  // Position after the move.
  jacobianInflate(iX, iPhi0) = 0.0; //charge * d0 == 0 after moving
  jacobianInflate(iY, iD0) = boost::math::sign(omega);
  jacobianInflate(iZ, iZ0) = 1.0;

  // Momentum
  jacobianInflate(iPx, iOmega) = invSignedAlphaOmega2;
  jacobianInflate(iPy, iPhi0) = invAbsAlphaOmega;
  jacobianInflate(iPz, iOmega) = tanLambda * invSignedAlphaOmega2;
  jacobianInflate(iPz, iTanLambda) = invAbsAlphaOmega;
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

void UncertainHelix::reverse()
{
  // All except z0 have to be taken to their opposites
  TMatrixD jacobianReverse(5, 5);
  jacobianReverse.UnitMatrix();
  jacobianReverse(iD0, iD0) = -1;
  jacobianReverse(iOmega, iOmega) = -1;
  jacobianReverse(iTanLambda, iTanLambda) = -1;

  m_covariance.Similarity(jacobianReverse);

}

double UncertainHelix::passiveMoveBy(const double& byX,
                                     const double& byY,
                                     const double& byZ)
{
  // Move the covariance matrix first to have access to the original parameters
  TMatrixD jacobianPassiveMove(5, 5);
  calcPassiveMoveByJacobian(byX, byY, jacobianPassiveMove);
  m_covariance.Similarity(jacobianPassiveMove);
  return Helix::passiveMoveBy(byX, byY, byZ);
}
