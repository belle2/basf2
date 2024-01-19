/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <mdst/dataobjects/TrackFitResult.h>
#include <framework/dataobjects/UncertainHelix.h>
#include <mdst/dataobjects/HitPatternCDC.h>
#include <mdst/dataobjects/HitPatternVXD.h>

#include <framework/utilities/HTML.h>

#include <boost/math/special_functions/gamma.hpp>

#include <sstream>
#include <assert.h>

using namespace Belle2;

TrackFitResult::TrackFitResult() :
  m_pdg(0), m_pValue(0),
  m_hitPatternCDCInitializer(0),
  m_hitPatternVXDInitializer(0),
  m_NDF100(c_NDFFlag)
{
  memset(m_tau, 0, sizeof(m_tau));
  memset(m_cov5, 0, sizeof(m_cov5));
}

TrackFitResult::TrackFitResult(const ROOT::Math::XYZVector& position, const ROOT::Math::XYZVector& momentum,
                               const TMatrixDSym& covariance, const short int charge,
                               const Const::ParticleType& particleType, const float pValue,
                               const float bField,
                               const uint64_t hitPatternCDCInitializer,
                               const uint32_t hitPatternVXDInitializer,
                               const float NDF) :
  m_pdg(std::abs(particleType.getPDGCode())), m_pValue(pValue),
  m_hitPatternCDCInitializer(hitPatternCDCInitializer),
  m_hitPatternVXDInitializer(hitPatternVXDInitializer),
  m_NDF100(int(NDF * 100.))
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
                               const uint32_t hitPatternVXDInitializer,
                               const float NDF) :
  m_pdg(std::abs(particleType.getPDGCode())), m_pValue(pValue),
  m_hitPatternCDCInitializer(hitPatternCDCInitializer),
  m_hitPatternVXDInitializer(hitPatternVXDInitializer),
  m_NDF100(int(NDF * 100.))
{
  if (tau.size() != c_NPars
      || cov5.size() != c_NCovEntries)
    B2FATAL("Invalid initializer for TrackFitResult.");

  for (unsigned int i = 0; i < c_NPars; ++i)
    m_tau[i] = tau[i];
  for (unsigned int i = 0; i < c_NCovEntries; ++i)
    m_cov5[i] = cov5[i];
}

void TrackFitResult::updateTrackFitResult(const TrackFitResult& input)
{
  // skip self-assigns
  if (this == &input) return;
  m_pdg = input.m_pdg;
  m_pValue = input.m_pValue;
  m_NDF100 = input.m_NDF100;
  m_hitPatternCDCInitializer = input.m_hitPatternCDCInitializer;
  m_hitPatternVXDInitializer = input.m_hitPatternVXDInitializer;

  std::copy(std::begin(input.m_tau), std::end(input.m_tau), std::begin(this->m_tau));

  std::copy(std::begin(input.m_cov5), std::end(input.m_cov5), std::begin(this->m_cov5));

}

float TrackFitResult::getNDF() const
{
  if (m_NDF100 == c_NDFFlag) {
    return -1;
  }
  return (float)m_NDF100 / 100;
}

double TrackFitResult::getChi2() const
{
  double pValue = getPValue();
  float nDF    = getNDF();
  if (pValue == 0) {
    return std::numeric_limits<double>::infinity();
  }
  if (nDF <= 0) {
    return std::numeric_limits<double>::quiet_NaN();
  }
  return 2 * boost::math::gamma_q_inv(nDF / 2., pValue);
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
std::string TrackFitResult::getInfoHTML() const
{
  std::stringstream out;
  out << "<b>Fit Hypothesis (PDG)</b>: " << m_pdg << "<br>";
  out << "<b>nPXDHits</b>: " << getHitPatternVXD().getNPXDHits() << "<br>";
  out << "<b>nSVDHits</b>: " << getHitPatternVXD().getNSVDHits() << "<br>";
  out << "<b>nCDCHits</b>: " << getHitPatternCDC().getNHits() << "<br>";
  out << "<b>NDF</b>: " << getNDF() << "<br>";
  out << " <br>";
  out << "<b>d0</b>: " << m_tau[iD0] << " cm <br>";
  out << "<b>phi0</b>: " << m_tau[iPhi0] << " rad <br>";
  out << "<b>omega</b>: " << m_tau[iOmega] << " 1/GeV<br>";
  out << "<b>z0</b>: " << m_tau[iZ0] << " cm <br>";
  out << "<b>tanLambda</b>: " << m_tau[iTanLambda] << "<br>";
  out << " <br>";
  out << "<b>Covariance Matrix (d0, phi0, omega, z0, tanLambda)</b>: " << HTML::getString(getCovariance5()) << "<br>";
  out << "<b>pValue</b>: " << m_pValue << "<br>";
  return out.str();
}
