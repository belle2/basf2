/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Bastian Kronenbitter, Nils Braun                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/trackFindingCDC/legendre/CDCLegendreTrackCandidate.h>
#include <tracking/trackFindingCDC/legendre/TrackHit.h>

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <TMath.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

TrackCandidate::TrackCandidate(TrackCandidate& candidate) :
  m_TrackHits(),
  m_theta(candidate.getTheta()), m_r(candidate.getR()), m_xc(candidate.getXc()), m_yc(candidate.getYc()),
  m_ref_x(candidate.getReferencePoint().X()), m_ref_y(candidate.getReferencePoint().Y()), m_charge(candidate.getCharge())
{
  m_TrackHits.reserve(candidate.getNHits());
  for (TrackHit* trackHit : candidate.getTrackHits()) {
    m_TrackHits.push_back(trackHit);
  }
}

TrackCandidate::TrackCandidate(double theta, double r, int charge,
                               const std::vector<TrackHit*>& trackHitList) :
  m_theta(theta), m_r(r), m_xc(cos(theta) / r), m_yc(sin(theta) / r), m_ref_x(0), m_ref_y(0),
  m_charge(charge)
{
  m_TrackHits.reserve(trackHitList.size());

  //only accepts hits, which support the correct curvature
  for (TrackHit* hit : trackHitList) {
    if ((m_charge == charge_positive || m_charge == charge_negative) && hit->getCurvatureSignWrt(getXc(), getYc()) != m_charge)
      continue;

    m_TrackHits.push_back(hit);
    hit->setHitUsage(TrackHit::c_usedInTrack);
  }
}

TrackCandidate::TrackCandidate(const std::vector<QuadTreeLegendre*>& nodeList) :
  m_theta(0), m_r(0), m_xc(0), m_yc(0), m_ref_x(0), m_ref_y(0), m_charge(0)
{
  m_TrackHits.reserve(256);

  size_t nHitsNodeMax = 0;
  //only accepts hits, which support the correct curvature
  for (QuadTreeLegendre* node : nodeList) {
    for (TrackHit* hit : node->getItemsVector()) {
      m_TrackHits.push_back(hit);
    }
    if (nHitsNodeMax < node->getNItems()) {
      nHitsNodeMax = node->getNItems();
      m_theta = node->getXMean() * boost::math::constants::pi<double>() / TrigonometricalLookupTable::Instance().getNBinsTheta();
      m_r = node->getYMean();
    }
  }

  m_xc = cos(m_theta) / m_r;
  m_yc = sin(m_theta) / m_r;

  m_charge = TrackCandidate::getChargeAssumption(m_theta, m_r, m_TrackHits);

  m_TrackHits.erase(std::remove_if(m_TrackHits.begin(), m_TrackHits.end(), [&, this](TrackHit * hit) {
    return ((this->m_charge == charge_positive || this->m_charge == charge_negative)
            && hit->getCurvatureSignWrt(getXc(), getYc()) != this->m_charge);
  })
  , m_TrackHits.end());

  std::sort(m_TrackHits.begin(), m_TrackHits.end(), [](TrackHit * hit1, TrackHit * hit2) {
    return hit1->getWirePosition().Mag2() > hit2->getWirePosition().Mag2();
  });
}


Vector2D TrackCandidate::getMomentumEstimation()
{
  double pt = TrackCandidate::convertRhoToPt(m_r);
  return TVector2(m_xc, m_yc).Rotate(TMath::Pi() / 2).Unit() * pt * getChargeSign();;
}

int TrackCandidate::getChargeAssumption(
  double theta, double r, const std::vector<TrackHit*>& trackHits)
{
  double yc = TMath::Sin(theta) / r;
  double xc = TMath::Cos(theta) / r;
  double rc = fabs(1. / r);

  if (rc < curlerRSplitValue)
    return charge_curler;

  int vote_pos = 0;
  int vote_neg = 0;

  for (TrackHit* Hit : trackHits) {
    int curve_sign = Hit->getCurvatureSignWrt(xc, yc);

    if (curve_sign == TrackCandidate::charge_positive)
      ++vote_pos;
    else if (curve_sign == TrackCandidate::charge_negative)
      ++vote_neg;
    else {
      B2ERROR(
        "Strange behaviour of TrackHit::getCurvatureSignWrt");
      exit(EXIT_FAILURE);
    }
  }

  if ((fabs(vote_pos - vote_neg) / (double)(vote_pos + vote_neg) <= 0.5) && rc > 60.)
    return charge_two_tracks;

  else  if (vote_pos > vote_neg)
    return charge_positive;

  else
    return charge_negative;
}

int TrackCandidate::getChargeSign() const
{
  if (fabs(1. / m_r) > curlerRSplitValue) return m_charge / fabs(m_charge);
  else {
    double trackCharge;
    int vote_pos(0), vote_neg(0);

    for (TrackHit* Hit : m_TrackHits) {
      int curve_sign = Hit->getCurvatureSignWrt(m_xc, m_yc);

      if (curve_sign == TrackCandidate::charge_positive)
        ++vote_pos;
      else if (curve_sign == TrackCandidate::charge_negative)
        ++vote_neg;
      else {
        B2ERROR(
          "Strange behaviour of TrackHit::getCurvatureSignWrt");
        exit(EXIT_FAILURE);
      }
    }

    if (vote_pos > vote_neg)
      trackCharge = TrackCandidate::charge_positive;

    else
      trackCharge = TrackCandidate::charge_negative;


    return trackCharge;
  }
}

void TrackCandidate::addHit(TrackHit* hit)
{
  if ((fabs(1. / m_r) < curlerRSplitValue) || hit->getCurvatureSignWrt(getXc(), getYc()) == m_charge) {
    m_TrackHits.push_back(hit);
  }
}
