/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013, 2014, 2015 - Belle II Collaboration                 *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck, Tobias Schlüter                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <mdst/dataobjects/TrackFitResult.h>
#include <framework/dataobjects/UncertainHelix.h>

using namespace Belle2;

ClassImp(TrackFitResult);

TrackFitResult::TrackFitResult() :
  m_pdg(0), m_pValue(0), m_tau(), m_cov5(),
  m_hitPatternCDCInitializer(0),
  m_hitPatternVXDInitializer(0)
{
}

TrackFitResult::TrackFitResult(const TVector3& position, const TVector3& momentum,
                               const TMatrixDSym& covariance, const short int charge,
                               const Const::ParticleType& particleType, const float pValue,
                               const float bField,
                               const uint64_t hitPatternCDCInitializer,
                               const uint32_t hitPatternVXDInitializer) :
  m_pdg(std::abs(particleType.getPDGCode())), m_pValue(pValue),
  m_hitPatternCDCInitializer(hitPatternCDCInitializer),
  m_hitPatternVXDInitializer(hitPatternVXDInitializer)
{
  UncertainHelix h(position, momentum, charge, bField, covariance, pValue);

  m_tau.reserve(5);
  m_tau.push_back(h.getD0());
  m_tau.push_back(h.getPhi0());
  m_tau.push_back(h.getOmega());
  m_tau.push_back(h.getZ0());
  m_tau.push_back(h.getTanLambda());

  // Upper half of the covariance matrix goes into m_cov5.
  m_cov5.reserve(5 * 6 / 2);
  const TMatrixDSym& cov = h.getCovariance();
  for (unsigned int i = 0; i < 5; ++i) {
    for (unsigned int j = i; j < 5; ++j) {
      m_cov5.push_back(cov(i, j));
    }
  }
}

TrackFitResult::TrackFitResult(const std::vector<float>& tau, const std::vector<float>& cov5,
                               const Const::ParticleType& particleType, const float pValue,
                               const uint64_t hitPatternCDCInitializer,
                               const uint32_t hitPatternVXDInitializer) :
  m_pdg(std::abs(particleType.getPDGCode())), m_pValue(pValue),
  m_tau(tau), m_cov5(cov5),
  m_hitPatternCDCInitializer(hitPatternCDCInitializer),
  m_hitPatternVXDInitializer(hitPatternVXDInitializer)
{
}

TMatrixDSym TrackFitResult::getCovariance5() const
{
  TMatrixDSym cov5(5);
  unsigned int counter = 0;
  for (unsigned int i = 0; i < 5; ++i) {
    for (unsigned int j = i; j < 5; ++j) {
      cov5(i, j) = cov5(j, i) = m_cov5[counter];
      ++counter;
    }
  }
  return cov5;
}
