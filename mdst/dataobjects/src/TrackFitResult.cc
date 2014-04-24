/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <mdst/dataobjects/TrackFitResult.h>
#include <TMath.h>

using namespace Belle2;

ClassImp(TrackFitResult);

TrackFitResult::TrackFitResult() : m_pdg(0), m_pValue(0), m_hitPatternCDCInitializer(0), m_hitPatternVXDInitializer(0)
{
}

TrackFitResult::TrackFitResult(const TVector3& position, const TVector3& momentum, const TMatrixDSym& covariance,
                               const short int charge, const Const::ParticleType& particleType, const float pValue,
                               const float bField,
                               const unsigned long hitPatternCDCInitializer, const unsigned short hitPatternVXDInitializer) :
  m_pdg(std::abs(particleType.getPDGCode())), m_pValue(pValue),
  m_hitPatternCDCInitializer(hitPatternCDCInitializer), m_hitPatternVXDInitializer(hitPatternVXDInitializer)
{
  cartesianToPerigee(position, momentum, covariance, charge, bField);
}

TrackFitResult::TrackFitResult(const std::vector<float>& tau, const std::vector<float>& cov5,
                               const Const::ParticleType& particleType, const float pValue,
                               const unsigned long hitPatternCDCInitializer, const unsigned short hitPatternVXDInitializer) :
  m_pdg(std::abs(particleType.getPDGCode())), m_pValue(pValue),
  m_tau(tau), m_cov5(cov5),
  m_hitPatternCDCInitializer(hitPatternCDCInitializer), m_hitPatternVXDInitializer(hitPatternVXDInitializer)
{

}

// This class should be able to give back Helix information either in Perigee Parametrisation
// or as starting position + momentum.
TVector3 TrackFitResult::getPosition() const
{
  return TVector3(calcXFromPerigee(), calcYFromPerigee(), calcZFromPerigee());
}

TVector3 TrackFitResult::getMomentum(const float bField) const
{
  return TVector3(calcPxFromPerigee(bField), calcPyFromPerigee(bField), calcPzFromPerigee(bField));
}

float TrackFitResult::getTransverseMomentum(const float bField) const
{
  float px = calcPxFromPerigee(bField);
  float py = calcPyFromPerigee(bField);
  return std::sqrt(px * px + py * py);
}

TMatrixF TrackFitResult::getCovariance5() const
{
  TMatrixF cov5(5, 5);
  unsigned int counter = 0;
  for (unsigned int i = 0; i < 5; ++i) {
    for (unsigned int j = i; j < 5; ++j) {
      cov5(i, j) = cov5(j, i) = m_cov5[counter];
      ++counter;
    }
  }
  return cov5;
}

TMatrixF TrackFitResult::transformCov5ToCov6(const TMatrixF& cov5, const float bField) const
{
  const double d0 = getD0();
  const double phi = getPhi();
  const double omega = getOmega();
  const double cotTheta = getCotTheta();
  const double alpha = getAlpha(bField);

  TMatrixF B(6, 5); // Matrix is invoked with zeros
  const double alphaOmega = std::fabs(alpha * omega);
  const double alphaOmega2 = std::fabs(alpha * std::pow(omega, 2));

  B(0, 0) = std::sin(phi);
  B(0, 1) = d0 * std::cos(phi);
  B(1, 0) = -std::cos(phi);
  B(1, 1) = d0 * std::sin(phi);
  B(2, 3) = 1;
  B(3, 1) = -std::sin(phi) / alphaOmega;
  B(3, 2) = -std::cos(phi) / alphaOmega2;
  B(4, 1) = std::cos(phi) / alphaOmega;
  B(4, 2) = -std::sin(phi) / alphaOmega2;
  B(5, 2) = -cotTheta / alphaOmega2;
  B(5, 4) = 1.0 / alphaOmega;
  TMatrixF cov6(6, 6);
  TMatrixF BT(5, 6);
  BT.Transpose(B);
  cov6 = B * cov5 * BT;
  return cov6;
}


TMatrixF TrackFitResult::getCovariance6(const float bField) const
{
  TMatrixF cov5(getCovariance5());
  return transformCov5ToCov6(cov5, bField);
}

void TrackFitResult::cartesianToPerigee(const TVector3& position, const TVector3& momentum, const TMatrixDSym& covariance,
                                        const short int charge, const float bField)
{
  const double alpha = getAlpha(bField);

  m_tau.push_back(calcD0FromCartesian(position, momentum));
  m_tau.push_back(calcPhiFromCartesian(momentum));
  m_tau.push_back(calcOmegaFromCartesian(momentum, charge, bField));
  m_tau.push_back(calcZ0FromCartesian(position));
  m_tau.push_back(calcCotThetaFromCartesian(momentum));

  TMatrixF A(5, 6); // Matrix is invoked with zeros
  const double positionPerp = position.Perp();
  const double momentumPerp2 = momentum.Perp2();
  const double momentumPerp3 = std::pow(momentum.Perp(), 3);
  const double signD0 = m_tau[0] > 0 ? 1 : -1;

  A(0, 0) = signD0 * position.X() / positionPerp;
  A(0, 1) = signD0 * position.Y() / positionPerp;
  A(1, 3) = -momentum.Py() / momentumPerp2;
  A(1, 4) = momentum.Px() / momentumPerp2;
  A(2, 3) = -momentum.Px() / (alpha * momentumPerp3);
  A(2, 4) = -momentum.Py() / (alpha * momentumPerp3);
  A(3, 2) = 1.0;
  A(4, 3) = -momentum.Px() * momentum.Pz() / momentumPerp3;
  A(4, 4) = -momentum.Py() * momentum.Pz() / momentumPerp3;
  A(4, 5) = 1.0 / momentum.Perp();
  TMatrixF cov5(5, 5);
  TMatrixF AT(6, 5);
  AT.Transpose(A);
  cov5 = A * TMatrixF(covariance) * AT;

  for (unsigned int i = 0; i < 5; ++i) {
    for (unsigned int j = i; j < 5; ++j) {
      m_cov5.push_back(cov5(i, j));
    }
  }

}

double TrackFitResult::getAlpha(const float bField) const
{
  return 1.0 / (bField * TMath::C()) * 10E10;
}

float TrackFitResult::calcD0FromCartesian(const TVector3& position, const TVector3& momentum) const
{
  // see: Fast vertex fitting with a local parametrization of tracks - Billoir, Pierre et al. Nucl.Instrum.Meth. A311 (1992) 139-150
  double positivePocaAngle(position.Phi() > 0 ? position.Phi() : 2 * TMath::Pi() + position.Phi());
  double positivePtAngle(momentum.Phi() > 0 ? momentum.Phi() : 2 * TMath::Pi() + momentum.Phi());
  double phiDiff(positivePtAngle - positivePocaAngle);
  if (phiDiff < 0.0) phiDiff += 2 * TMath::Pi();
  short int d0sign = phiDiff > TMath::Pi() ? -1 : 1;
  return d0sign * std::sqrt(position.X() * position.X() + position.Y() * position.Y());
}

float TrackFitResult::calcPhiFromCartesian(const TVector3& momentum) const
{
  return momentum.Phi();
}

float TrackFitResult::calcOmegaFromCartesian(const TVector3& momentum, const short int charge, const float bField) const
{
  return charge / (getAlpha(bField) * momentum.Perp());
}

float TrackFitResult::calcZ0FromCartesian(const TVector3& position) const
{
  return position.Z();
}

float TrackFitResult::calcCotThetaFromCartesian(const TVector3& momentum) const
{
  return momentum.Pz() / momentum.Perp();
}

float TrackFitResult::calcXFromPerigee() const
{
  return m_tau.at(0) * std::sin(m_tau.at(1));
}

float TrackFitResult::calcYFromPerigee() const
{
  return -m_tau.at(0) * std::cos(m_tau.at(1));
}

float TrackFitResult::calcZFromPerigee() const
{
  return m_tau.at(3);
}

float TrackFitResult::calcPxFromPerigee(const float bField) const
{
  return std::cos(m_tau.at(1)) / (std::fabs(m_tau.at(2) * getAlpha(bField)));
}

float TrackFitResult::calcPyFromPerigee(const float bField) const
{
  return std::sin(m_tau.at(1)) / (std::fabs(m_tau.at(2) * getAlpha(bField)));
}

float TrackFitResult::calcPzFromPerigee(const float bField) const
{
  return m_tau.at(4) / (std::fabs(m_tau.at(2) * getAlpha(bField)));
}
