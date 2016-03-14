/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Bastian Kronenbitter                                     *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/trackFindingCDC/legendre/CDCLegendreTrackCandidate.h>
#include <tracking/trackFindingCDC/legendre/TrackHit.h>

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <TMath.h>

#define SQR(x) ((x)*(x)) //we will use it in least squares fit


using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

TrackCandidate::TrackCandidate(
  TrackCandidate& candidate) :
  m_theta(candidate.m_theta), m_r(candidate.m_r), m_xc(candidate.m_xc), m_yc(candidate.m_yc),
  m_ref_x(candidate.m_ref_x), m_ref_y(candidate.m_ref_y), m_charge(candidate.m_charge), m_type(candidate.m_type),
  m_calcedMomentum(candidate.m_calcedMomentum), m_momEstimation(candidate.m_momEstimation)

{
  m_hitPattern = candidate.getHitPattern();
  m_TrackHits = candidate.getTrackHits();
}

TrackCandidate::~TrackCandidate()
{
}

TrackCandidate::TrackCandidate(double theta, double r, int charge,
                               const std::vector<TrackHit*>& trackHitList) :
  m_theta(theta), m_r(r), m_xc(cos(theta) / r), m_yc(sin(theta) / r), m_ref_x(0), m_ref_y(0),
  m_charge(charge), m_axialHits(0), m_stereoHits(0), m_calcedMomentum(false), m_momEstimation(0, 0, 0)
{
  m_TrackHits.reserve(256);

  //only accepts hits, which support the correct curvature

  for (TrackHit* hit : trackHitList) {
    if (/*(m_charge != charge_tracklet) &&*/ ((m_charge == charge_positive || m_charge == charge_negative)
                                              && hit->getCurvatureSignWrt(getXc(), getYc()) != m_charge))
      continue;

    m_TrackHits.push_back(hit);

    hit->setHitUsage(TrackHit::c_usedInTrack);

    m_hitPattern.setLayer(hit->getLayerId());
  }

  makeHitPattern();

  m_type = goodTrack;
}

TrackCandidate::TrackCandidate(const std::vector<QuadTreeLegendre*>& nodeList) :
  m_theta(0), m_r(0), m_xc(0), m_yc(0), m_ref_x(0), m_ref_y(0), m_charge(0), m_axialHits(0), m_stereoHits(0),
  m_calcedMomentum(false), m_momEstimation(0, 0, 0)
{
  m_TrackHits.reserve(256);

  size_t nHitsNodeMax = 0;
  //only accepts hits, which support the correct curvature
  for (QuadTreeLegendre* node : nodeList) {
    for (TrackHit* hit : node->getItemsVector()) {
//            if ((m_charge == charge_positive || m_charge == charge_negative)
//                && hit->getCurvatureSignWrt(getXc(), getYc()) != m_charge)
//              continue;

      m_TrackHits.push_back(hit);

      m_hitPattern.setLayer(hit->getLayerId());
    }
    if (nHitsNodeMax < node->getNItems()) {
      nHitsNodeMax = node->getNItems();
      m_theta = node->getXMean() * boost::math::constants::pi<double>() / TrigonometricalLookupTable::Instance().getNBinsTheta();
      m_r = node->getYMean();
    }
    m_nodes.push_back(node);
  }

  m_xc = cos(m_theta) / m_r;
  m_yc = sin(m_theta) / m_r;

  m_charge = TrackCandidate::getChargeAssumption(m_theta, m_r, m_TrackHits);


  m_TrackHits.erase(std::remove_if(m_TrackHits.begin(), m_TrackHits.end(), [&, this](TrackHit * hit) {
    return ((this->m_charge == charge_positive || this->m_charge == charge_negative)
            && hit->getCurvatureSignWrt(getXc(), getYc()) != this->m_charge);
  })
  , m_TrackHits.end());

  makeHitPattern();

  m_type = goodTrack;

}

void TrackCandidate::determineHitNumbers()
{
  m_axialHits = 0;
  m_stereoHits = 0;
  for (TrackHit* hit : m_TrackHits) {
    if (hit->getIsAxial())
      ++m_axialHits;
    else
      ++m_stereoHits;
  }

}

TVector3 TrackCandidate::getMomentumEstimation(bool force_calculation) const
{
  if (force_calculation || !m_calcedMomentum)
    const_cast<TrackCandidate*>(this)->calculateMomentumEstimation();

  return m_momEstimation;
}

void TrackCandidate::calculateMomentumEstimation()
{
  double pt = TrackCandidate::convertRhoToPt(m_r);

  TVector2 mom2 = (TVector2(m_xc, m_yc).Rotate(TMath::Pi() / 2).Unit()) * pt * getChargeSign();

  double mom_z = getZMomentumEstimation(mom2);

  TVector3 mom3(mom2.X(), mom2.Y(), mom_z);

  m_momEstimation = mom3;
  m_calcedMomentum = true;
}

double TrackCandidate::getZMomentumEstimation(TVector2 mom2) const
{
  std::vector<double> median_vector;
  median_vector.reserve(m_TrackHits.size() / 2);

  for (std::vector<TrackHit*>::const_iterator it = m_TrackHits.begin(); it != m_TrackHits.end(); ++it) {

    TrackHit* trackHit = *it;

    if (trackHit->getIsAxial())
      continue;

    if (trackHit->getSuperlayerId() > 5)
      continue;

    double x = trackHit->getWirePosition().X();
    double y = trackHit->getWirePosition().Y();
    double z = trackHit->getWirePosition().Z();
    double r = sqrt(x * x + y * y);

    median_vector.push_back(atan2(z, r));
  }

  if (median_vector.size() == 0)
    return (0.0001);

  sort(median_vector.begin(), median_vector.end());

  double medianTheta = median_vector.at(median_vector.size() / 2);

  double zmom = tan(medianTheta) * sqrt(mom2.X() * mom2.X() + mom2.Y() * mom2.Y());

  double __attribute__((unused)) theta_mean = 0;
  int nStereo = 0;

  for (TrackHit* hit : m_TrackHits) {
    if (hit->getIsAxial())
      continue;

    double alpha = acos(1. - (SQR(hit->getOriginalWirePosition().X() - m_ref_x) + SQR(hit->getOriginalWirePosition().Y() - m_ref_y)) /
                        (2.*SQR(1. / m_r)));

    double theta = atan(alpha * fabs(1. / m_r) / hit->getWirePosition().Z()) + TMath::Pi();
    nStereo++;
    theta_mean += theta;
  }

  if (nStereo != 0) theta_mean = theta_mean / nStereo;

  zmom = tan(medianTheta) * sqrt(mom2.X() * mom2.X() + mom2.Y() * mom2.Y());

  return zmom;
}

double TrackCandidate::DistanceTo(
  const TrackHit& tHit) const
{
  return (DistanceTo(getXc(), getYc(), tHit, false));
}

double TrackCandidate::OriginalDistanceTo(
  const TrackHit& tHit) const
{
  return (DistanceTo(getXc(), getYc(), tHit, true));
}

double TrackCandidate::DistanceTo(double xc, double yc,
                                  const TrackHit& tHit, bool orig)
{
  double rc = sqrt(xc * xc + yc * yc);

  double xw;
  double yw;
  double rw = tHit.getDriftLength();

  if (orig) {
    xw = tHit.getOriginalWirePosition().X();
    yw = tHit.getOriginalWirePosition().Y();
  } else {
    xw = tHit.getWirePosition().X();
    yw = tHit.getWirePosition().Y();
  }

  //distance of the centers of the drift circle and the track
  double d = sqrt((xc - xw) * (xc - xw) + (yc - yw) * (yc - yw));

  //take into account the two radii correctly
  double sum_r = (rc + rw);
  double dif_r = fabs(rc - rw);

  double distance;
  if (d > max(rw, rc))
    distance = fabs(d - sum_r);
  else
    distance = fabs(d - dif_r);

  return distance;
}

int TrackCandidate::getChargeAssumption(
  double theta, double r, const std::vector<TrackHit*>& trackHits)
{
  double yc = TMath::Sin(theta) / r;
  double xc = TMath::Cos(theta) / r;
  double rc = fabs(1. / r);

  if (rc < 56.5)
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
  if (fabs(1. / m_r) > 56.5) return m_charge / fabs(m_charge);
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

void TrackCandidate::setR(double r)
{
  m_r = r;
  m_xc = cos(m_theta) / r + m_ref_x;
  m_yc = sin(m_theta) / r + m_ref_y;
}

void TrackCandidate::setTheta(double theta)
{
  m_theta = theta;
  m_xc = cos(theta) / m_r + m_ref_x;
  m_yc = sin(theta) / m_r + m_ref_y;
}

void TrackCandidate::addHit(TrackHit* hit)
{
  if (/*(m_charge == charge_curler)*/(fabs(1. / m_r) < 56.5) || hit->getCurvatureSignWrt(getXc(), getYc()) == m_charge) {
    m_TrackHits.push_back(hit);

    m_hitPattern.setLayer(hit->getLayerId());

    if (hit->getIsAxial())
      ++m_axialHits;
    else
      ++m_stereoHits;
  }
}

void TrackCandidate::removeHit(TrackHit* hit)
{
  /*  m_TrackHits.erase(std::remove_if(m_TrackHits.begin(), m_TrackHits.end(),
  [&, &hit](TrackHit * trackHit) {
    return trackHit == hit;
  }), m_TrackHits.end());
  */
  m_TrackHits.erase(std::remove(m_TrackHits.begin(), m_TrackHits.end(), hit), m_TrackHits.end());


  makeHitPattern();

  if (hit->getIsAxial())
    --m_axialHits;
  else
    --m_stereoHits;

  hit->setHitUsage(TrackHit::c_notUsed);
}

int TrackCandidate::getInnermostSLayer(bool forced, int minNHits)
{

  if ((m_innermostAxialSLayer == -1) || (forced)) {
    unsigned nHits = m_TrackHits.size();
    int minSLayer = 0;
    do {
      if (m_hitPattern.hasSLayer(minSLayer)) {
        if (m_hitPattern.getSLayerNHits(minSLayer) < minNHits) {
          //using lambda functions for erase-remove idiom
          m_TrackHits.erase(std::remove_if(m_TrackHits.begin(), m_TrackHits.end(), [&minSLayer](TrackHit * hit) {return hit->getSuperlayerId() == minSLayer;}),
          m_TrackHits.end());
        } else {
          m_innermostAxialSLayer = minSLayer;
          break;
        }
      }
      ++minSLayer;
    } while (minSLayer <= 8);
    m_innermostAxialSLayer = minSLayer - 1;
    if (m_TrackHits.size() != nHits) makeHitPattern();
  }

  return m_innermostAxialSLayer;
}

int TrackCandidate::getOutermostSLayer(bool forced, int minNHits)
{
  if ((m_outermostAxialSLayer == -1) || (forced)) {
    unsigned nHits = m_TrackHits.size();
    int maxSLayer = 8;
    do {
      if (m_hitPattern.hasSLayer(maxSLayer)) {
        if (m_hitPattern.getSLayerNHits(maxSLayer) < minNHits) {
          //using lambda functions for erase-remove idiom
          m_TrackHits.erase(std::remove_if(m_TrackHits.begin(), m_TrackHits.end(), [&maxSLayer](TrackHit * hit) {return hit->getSuperlayerId() == maxSLayer;}),
          m_TrackHits.end());
        } else {
          m_outermostAxialSLayer = maxSLayer;
          break;
        }
      }
      --maxSLayer;
    } while (maxSLayer >= 0);
    m_outermostAxialSLayer = maxSLayer + 1;
    if (m_TrackHits.size() != nHits) makeHitPattern();
  }

  return m_outermostAxialSLayer;
}


void TrackCandidate::makeHitPattern()
{
  m_hitPattern.resetPattern();

  for (TrackHit* hit : m_TrackHits) {
    m_hitPattern.setLayer(hit->getLayerId());
  }

  determineHitNumbers();

//  B2DEBUG(100, "pattern:" << m_hitPatternAxial.getHitPattern());

}



double TrackCandidate::getLayerWaight()
{
  int maxLayer = -999;
  BOOST_FOREACH(TrackHit * hit, m_TrackHits) {
    if (hit->getIsAxial()) {
      if (hit->getLayerId() > maxLayer)
        maxLayer = hit->getLayerId();
    }
  }

  double layerWaight = 0;
  BOOST_FOREACH(TrackHit * hit, m_TrackHits) {
    if (hit->getIsAxial()) {
      layerWaight += 1. / (hit->getLayerId() + 1);
    }
  }

  return layerWaight;
}

#include <iostream>

void TrackCandidate::CheckStereoHits()
{
  std::vector<TrackHit*> replace_vector;
  replace_vector.reserve(m_TrackHits.size());

  TVector3 mom = getMomentumEstimation();

  double mom_r = sqrt(mom.X() * mom.X() + mom.Y() * mom.Y());
  double phi_track = atan2(mom.Z(), mom_r);
  m_stereoHits = 0;

  for (TrackHit* hit : m_TrackHits) {
    if (hit->getIsAxial())
      replace_vector.push_back(hit);
    else {
      double hit_x = hit->getWirePosition().X();
      double hit_y = hit->getWirePosition().Y();
      double hit_z = hit->getWirePosition().Z();
      double hit_r = sqrt(hit_x * hit_x + hit_y * hit_y);
      double hit_d = sqrt(hit_x * hit_x + hit_y * hit_y + hit_z * hit_z);

      double phi_hit = atan2(hit_z, hit_r);

      if (fabs(phi_hit - phi_track) / hit_d < 0.0025) {
        replace_vector.push_back(hit);
        ++m_stereoHits;
      }
    }
  }

  m_TrackHits = replace_vector;
}

void TrackCandidate::setReferencePoint(double x0, double y0)
{
  m_ref_x = x0;
  m_ref_y = y0;
  m_xc = cos(m_theta) / m_r + m_ref_x;
  m_yc = sin(m_theta) / m_r + m_ref_y;
}

void TrackCandidate::clearBadHits()
{
  double R = fabs(1. / this->m_r);
  double x0_track = cos(this->m_theta) / this->m_r + m_ref_x;
  double y0_track = sin(this->m_theta) / this->m_r + m_ref_y;
  m_TrackHits.erase(std::remove_if(m_TrackHits.begin(), m_TrackHits.end(),
  [&R, &x0_track, &y0_track](TrackHit * hit) {
    double x0_hit = hit->getOriginalWirePosition().X();
    double y0_hit = hit->getOriginalWirePosition().Y();
    double dist = fabs(R - sqrt(SQR(x0_track - x0_hit) + SQR(y0_track - y0_hit))) - hit->getDriftLength();
    if (dist > hit->getSigmaDriftLength() * 2.) {
      hit->setHitUsage(TrackHit::c_bad);
      return true;
    } else {
      return false;
    }
  }), m_TrackHits.end());


}
