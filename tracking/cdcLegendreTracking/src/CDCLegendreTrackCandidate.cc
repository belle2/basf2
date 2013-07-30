/**************************************************************************
09 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Bastian Kronenbitter                                     *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/cdcLegendreTracking/CDCLegendreTrackCandidate.h>

#include <tracking/cdcLegendreTracking/CDCLegendreTrackHit.h>

#include <framework/datastore/StoreArray.h>
#include <cmath>
#include <cstdlib>
#include "TMath.h"
#include "boost/foreach.hpp"

using namespace std;
using namespace Belle2;

CDCLegendreTrackCandidate::CDCLegendreTrackCandidate(
  CDCLegendreTrackCandidate& candidate) :
  m_theta(candidate.m_theta), m_r(candidate.m_r), m_xc(candidate.m_xc), m_yc(
    candidate.m_yc), m_charge(
      candidate.m_charge), m_calcedMomentum(candidate.m_calcedMomentum), m_momEstimation(candidate.m_momEstimation)
{
  m_TrackHits = candidate.getTrackHits();
}

CDCLegendreTrackCandidate::~CDCLegendreTrackCandidate()
{
}

CDCLegendreTrackCandidate::CDCLegendreTrackCandidate(double theta, double r, int charge,
                                                     const std::vector<CDCLegendreTrackHit*>& trackHitList) :
  m_theta(theta), m_r(r), m_xc(cos(theta) / r), m_yc(sin(theta) / r), m_charge(charge), m_axialHits(0), m_stereoHits(0), m_calcedMomentum(false), m_momEstimation(0, 0, 0)
{
  m_TrackHits.reserve(256);

  //only accepts hits, which support the correct curvature
  BOOST_FOREACH(CDCLegendreTrackHit * hit, trackHitList) {
    if ((m_charge == charge_positive || m_charge == charge_negative)
        && hit->getCurvatureSignWrt(getXc(), getYc()) != m_charge)
      continue;

    m_TrackHits.push_back(hit);
  }

  DetermineHitNumbers();
}

void CDCLegendreTrackCandidate::DetermineHitNumbers()
{
  BOOST_FOREACH(CDCLegendreTrackHit * hit, m_TrackHits) {
    if (hit->getIsAxial())
      ++m_axialHits;
    else
      ++m_stereoHits;
  }

}

TVector3 CDCLegendreTrackCandidate::getMomentumEstimation(bool force_calculation) const
{
  if (force_calculation || !m_calcedMomentum)
    const_cast<CDCLegendreTrackCandidate*>(this)->calculateMomentumEstimation();

  return m_momEstimation;
}

void CDCLegendreTrackCandidate::calculateMomentumEstimation()
{
  double R = fabs(1 / m_r);
  double pt = R * 1.5 * 0.00299792458;

  TVector2 mom2 = (TVector2(m_xc, m_yc).Rotate(TMath::Pi() / 2).Unit()) * pt * getChargeSign();

  double mom_z = getZMomentumEstimation(mom2);

  TVector3 mom3(mom2.X(), mom2.Y(), mom_z);

  m_momEstimation = mom3;
  m_calcedMomentum = true;
}

double CDCLegendreTrackCandidate::getZMomentumEstimation(TVector2 mom2) const
{
  std::vector<double> median_vector;
  median_vector.reserve(m_TrackHits.size() / 2);

  for (std::vector<CDCLegendreTrackHit*>::const_iterator it = m_TrackHits.begin(); it != m_TrackHits.end(); ++it) {

    CDCLegendreTrackHit* trackHit = *it;

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

  return zmom;
}

double CDCLegendreTrackCandidate::DistanceTo(
  const CDCLegendreTrackHit& tHit) const
{
  return (DistanceTo(getXc(), getYc(), tHit, false));
}

double CDCLegendreTrackCandidate::OriginalDistanceTo(
  const CDCLegendreTrackHit& tHit) const
{
  return (DistanceTo(getXc(), getYc(), tHit, true));
}

double CDCLegendreTrackCandidate::DistanceTo(double xc, double yc,
                                             const CDCLegendreTrackHit& tHit, bool orig)
{
  double rc = sqrt(xc * xc + yc * yc);

  double xw;
  double yw;
  double rw = tHit.getDriftTime();

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

int CDCLegendreTrackCandidate::getChargeAssumption(
  double theta, double r, const std::vector<CDCLegendreTrackHit*>& trackHits)
{
  double yc = TMath::Sin(theta) / r;
  double xc = TMath::Cos(theta) / r;
  double rc = fabs(1 / r);

  if (rc < 56.5)
    return charge_curler;

  int vote_pos = 0;
  int vote_neg = 0;

  BOOST_FOREACH(CDCLegendreTrackHit * Hit, trackHits) {
    int curve_sign = Hit->getCurvatureSignWrt(xc, yc);

    if (curve_sign == CDCLegendreTrackCandidate::charge_positive)
      ++vote_pos;
    else if (curve_sign == CDCLegendreTrackCandidate::charge_negative)
      ++vote_neg;
    else {
      B2ERROR(
        "Strange behaviour of CDCLegendreTrackHit::getCurvatureSignWrt");
      exit(EXIT_FAILURE);
    }
  }

  if (fabs(vote_pos - vote_neg) / (double)(vote_pos + vote_neg) <= 0.5)
    return charge_two_tracks;

  else if (vote_pos > vote_neg)
    return charge_positive;

  else
    return charge_negative;
}

int CDCLegendreTrackCandidate::getChargeSign() const
{
  return m_charge / abs(m_charge);
}

void CDCLegendreTrackCandidate::setR(double r)
{
  m_r = r;
  m_xc = cos(m_theta) / r;
  m_yc = sin(m_theta) / r;
}

void CDCLegendreTrackCandidate::setTheta(double theta)
{
  m_theta = theta;
  m_xc = cos(theta) / m_r;
  m_yc = sin(theta) / m_r;
}

void CDCLegendreTrackCandidate::addHit(CDCLegendreTrackHit* hit)
{
  if ((m_charge == charge_curler) || hit->getCurvatureSignWrt(getXc(), getYc()) == m_charge)
    m_TrackHits.push_back(hit);

  if (hit->getIsAxial())
    ++m_axialHits;
  else
    ++m_stereoHits;
}

int CDCLegendreTrackCandidate::getInnermostAxialLayer()
{
  int minLayer = 999;

  BOOST_FOREACH(CDCLegendreTrackHit * hit, m_TrackHits) {
    if (hit->getIsAxial()) {
      if (hit->getLayerId() < minLayer)
        minLayer = hit->getLayerId();
    }
  }

  return minLayer;
}

double CDCLegendreTrackCandidate::getLayerWaight()
{
  int maxLayer = -999;
  BOOST_FOREACH(CDCLegendreTrackHit * hit, m_TrackHits) {
    if (hit->getIsAxial()) {
      if (hit->getLayerId() > maxLayer)
        maxLayer = hit->getLayerId();
    }
  }

  double layerWaight = 0;
  BOOST_FOREACH(CDCLegendreTrackHit * hit, m_TrackHits) {
    if (hit->getIsAxial()) {
      layerWaight += 1. / (hit->getLayerId() + 1);
    }
  }

  return layerWaight;
}

#include <iostream>

void CDCLegendreTrackCandidate::CheckStereoHits()
{
  std::vector<CDCLegendreTrackHit*> replace_vector;
  replace_vector.reserve(m_TrackHits.size());

  TVector3 mom = getMomentumEstimation();

  double mom_r = sqrt(mom.X() * mom.X() + mom.Y() * mom.Y());
  double phi_track = atan2(mom.Z(), mom_r);
  m_stereoHits = 0;

  BOOST_FOREACH(CDCLegendreTrackHit * hit, m_TrackHits) {
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
