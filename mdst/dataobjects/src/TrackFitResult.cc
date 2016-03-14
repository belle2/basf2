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
  m_pdg(0), m_pValue(0),
  m_hitPatternCDCInitializer(0),
  m_hitPatternVXDInitializer(0)
{
  memset(m_tau, 0, sizeof(m_tau));
  memset(m_cov5, 0, sizeof(m_cov5));
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

  m_tau[iD0] = h.getD0();
  m_tau[iPhi0] = h.getPhi0();
  m_tau[iOmega] = h.getOmega();
  m_tau[iZ0] = h.getZ0();
  m_tau[iTanLambda] = h.getTanLambda();

  // Upper half of the covariance matrix goes into m_cov5.
  const TMatrixDSym& cov = h.getCovariance();
  unsigned int counter = 0;
  for (unsigned int i = 0; i < c_NPars; ++i) {
    for (unsigned int j = i; j < c_NPars; ++j) {
      m_cov5[counter++] = cov(i, j);
    }
  }
  assert(counter == c_NCovEntries);
}

TrackFitResult::TrackFitResult(const std::vector<float>& tau, const std::vector<float>& cov5,
                               const Const::ParticleType& particleType, const float pValue,
                               const uint64_t hitPatternCDCInitializer,
                               const uint32_t hitPatternVXDInitializer) :
  m_pdg(std::abs(particleType.getPDGCode())), m_pValue(pValue),
  m_hitPatternCDCInitializer(hitPatternCDCInitializer),
  m_hitPatternVXDInitializer(hitPatternVXDInitializer)
{
  if (tau.size() != c_NPars
      || cov5.size() != c_NCovEntries)
    B2FATAL("Invalid initializer for TrackFitResult.");

  for (unsigned int i = 0; i < c_NPars; ++i)
    m_tau[i] = tau[i];
  for (unsigned int i = 0; i < c_NCovEntries; ++i)
    m_cov5[i] = cov5[i];
}

TMatrixDSym TrackFitResult::getCovariance5() const
{
  TMatrixDSym cov5(c_NPars);
  unsigned int counter = 0;
  for (unsigned int i = 0; i < c_NPars; ++i) {
    for (unsigned int j = i; j < c_NPars; ++j) {
      cov5(i, j) = cov5(j, i) = m_cov5[counter];
      ++counter;
    }
  }
  assert(counter == c_NCovEntries);
  return cov5;
}

HitPatternCDC TrackFitResult::getHitPatternCDC() const
{
  return HitPatternCDC(m_hitPatternCDCInitializer);
}

HitPatternVXD TrackFitResult::getHitPatternVXD() const
{
  return HitPatternVXD(m_hitPatternVXDInitializer);
}
