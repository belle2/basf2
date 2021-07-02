/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <framework/dataobjects/UncertainHelix.h>

#include <TVector3.h>

#include <cassert>

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

  // 2. Translate to perigee parameters on the position
  const double pz = momentum.Z();
  const double invPt = 1 / pt;
  const double invPtSquared = invPt * invPt;
  const double alpha = getAlpha(bZ);

  TMatrixD jacobianToHelixParameters(5, 6);
  jacobianToHelixParameters.Zero();

  jacobianToHelixParameters(iD0, iY) = -1;
  jacobianToHelixParameters(iPhi0, iX) = charge * invPt / alpha;
  jacobianToHelixParameters(iPhi0, iPy) = invPt;
  jacobianToHelixParameters(iOmega, iPx) = -charge * invPtSquared / alpha;
  jacobianToHelixParameters(iTanLambda, iPx) = - pz * invPtSquared;
  jacobianToHelixParameters(iTanLambda, iPz) = invPt;
  jacobianToHelixParameters(iZ0, iX) = - pz * invPt;
  jacobianToHelixParameters(iZ0, iZ) = 1;
  m_covariance.Similarity(jacobianToHelixParameters);

  // The covariance m_covariance is now the correct 5x5 covariance matrix.
  assert(m_covariance.GetNrows() == 5);

  // 3. Extrapolate to the origin.
  /* const double arcLength2D = */ passiveMoveBy(-position.X(), -position.Y(), 0.0);
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
  TMatrixD jacobianInflate(6, 5);
  jacobianInflate.Zero();

  // 1. Inflate the perigee covariance to a cartesian covariance where phi0 == 0 is assumed
  // The real phi0 is a simple rotation which can be handled in the next step.
  // Jacobian matrix for the translation

  const double& d0 = getD0();
  const double& omega = getOmega();
  const double& tanLambda = getTanLambda();

  const double alpha = getAlpha(bZ_tesla);
  const double absAlphaOmega = alpha * std::fabs(omega);
  const double signedAlphaOmega2 = absAlphaOmega * omega;

  const double invAbsAlphaOmega = 1.0 / absAlphaOmega;
  const double invSignedAlphaOmega2 = 1.0 / signedAlphaOmega2;

  // Position after the move.
  jacobianInflate(iX, iPhi0) = d0;
  jacobianInflate(iY, iD0) = -1.0;
  jacobianInflate(iZ, iZ0) = 1.0;

  // Momentum
  jacobianInflate(iPx, iOmega) = -invSignedAlphaOmega2;
  jacobianInflate(iPy, iPhi0) = invAbsAlphaOmega;
  jacobianInflate(iPz, iOmega) = -tanLambda * invSignedAlphaOmega2;
  jacobianInflate(iPz, iTanLambda) = invAbsAlphaOmega;

  TMatrixDSym cov6 = m_covariance; //copy
  cov6.Similarity(jacobianInflate);

  /// 2. Rotate to the right phi0
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
  Helix::reverse();

  // D0, omega and tan lambda have to be taken to their opposites
  // Phi0 is augmented by pi which does not change its covariances
  // Z0 stays the same
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
