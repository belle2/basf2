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
#include <TVectorD.h>


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
                               const float bZ,
                               const TMatrixDSym& cartesianCovariance,
                               const float pValue) :
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

  TMatrixDSym& cov6 = m_covariance;
  // We initialised the m_covariance to the cartesian covariance and
  // reduce it now to the real 5x5 matrix that should be there.

  // 1. Rotate to a system where phi0 = 0
  TMatrixD jacobianRot(6, 6);
  jacobianRot.Zero();

  const double px = momentum.X();
  const double py = momentum.Y();
  const double pz = momentum.Z();

  const double pt = hypot(px, py);
  const double invPt = 1 / pt;
  const double invPtSquared = invPt * invPt;

  const double cosPhi0 = px / pt;
  const double sinPhi0 = py / pt;

  const double alpha = getAlpha(bZ);

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

  jacobianReduce(iD0, iY) = -1;
  jacobianReduce(iPhi0, iPy) = invPt;
  jacobianReduce(iOmega, iPx) = charge * invPtSquared / alpha ;
  jacobianReduce(iTanLambda, iPx) = - pz * invPtSquared;
  jacobianReduce(iTanLambda, iPz) = invPt;
  jacobianReduce(iZ0, iZ) = 1;
  // Note the column corresponding to iX is completely zero as expectable.

  cov6.Similarity(jacobianReduce);

  // The covariance m_covariance is now the correct 5x5 covariance matrix.

  // Final step move to the right position
  passiveMoveBy(TVector3(-position.X(), -position.Y(), 0.0));
}


UncertainHelix::UncertainHelix(const float& d0,
                               const float& phi0,
                               const float& omega,
                               const float& z0,
                               const float& tanLambda,
                               const TMatrixDSym& covariance,
                               const float pValue) :
  Helix(d0, phi0, omega, z0, tanLambda),
  m_covariance(covariance),
  m_pValue(pValue)
{
}



TMatrixDSym UncertainHelix::getCartesianCovariance(const double bZ) const
{
  const double& d0 = getD0();
  // const double& impactXY = localHelix.impactXY();

  const double& cosPhi0 = getCosPhi0();
  const double& sinPhi0 = getSinPhi0();
  const double& omega = getOmega();
  const double& tanLambda = getTanLambda();
  const short& charge = getChargeSign();

  // 0. Define indices
  // Maybe push these out of this function:
  // Indices of the cartesian coordinates
  const int iX = 0;
  const int iY = 1;
  const int iZ = 2;
  const int iPx = 3;
  const int iPy = 4;
  const int iPz = 5;

  // 1. Inflat the perigee covariance to a cartesian covariance where phi0 = 0 is assumed
  // Jacobian matrix for the translation
  TMatrixD jacobianInflate(6, 5);
  jacobianInflate.Zero();

  const double alpha = getAlpha(bZ);
  const double chargeAlphaOmega = charge * alpha * omega;
  const double chargeAlphaOmega2 = charge * alpha * std::pow(omega, 2);

  const double invChargeAlphaOmega = 1.0 / chargeAlphaOmega;
  const double invChargeAlphaOmega2 = 1.0 / chargeAlphaOmega2;

  // Position
  jacobianInflate(iX, iPhi0) = d0;
  jacobianInflate(iY, iD0) = -1.0;
  jacobianInflate(iZ, iZ0) = 1.0;

  // Momentum
  jacobianInflate(iPx, iOmega) = -invChargeAlphaOmega2;
  jacobianInflate(iPy, iPhi0) = invChargeAlphaOmega;
  jacobianInflate(iPz, iOmega) = -tanLambda * invChargeAlphaOmega2;
  jacobianInflate(iPz, iTanLambda) = invChargeAlphaOmega;

  // Transform
  TMatrixDSym cov6 = m_covariance; //copy
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
  return cov6;
}


double UncertainHelix::passiveMoveBy(const TVector3& by)
{
  // Move the covariance matrix first to have access to the original parameters
  TMatrixD jacobian = calcPassiveMoveByJacobian(by);
  m_covariance.Similarity(jacobian);
  return Helix::passiveMoveBy(by);
}
