/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Bastian Kronenbitter                                     *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/cdcLegendreTracking/CDCLegendreTrackCandidate.h>

#include <framework/datastore/StoreArray.h>
#include <cmath>
#include "TMath.h"
#include "boost/foreach.hpp"

using namespace std;
using namespace Belle2;

ClassImp(CDCLegendreTrackCandidate)

CDCLegendreTrackCandidate::CDCLegendreTrackCandidate() :
  m_theta(-999), m_r(-999), m_y0(-999), m_slope(-999), m_charge(-999), m_ID(
    -999), m_assignHits(true)
{
}

CDCLegendreTrackCandidate::CDCLegendreTrackCandidate(
  CDCLegendreTrackCandidate& candidate) :
  TObject(), m_theta(candidate.m_theta), m_r(candidate.m_r), m_y0(
    candidate.m_y0), m_slope(candidate.m_slope), m_charge(
      candidate.m_charge), m_ID(candidate.m_ID), m_assignHits(
        candidate.m_assignHits)
{
  m_TrackHits = candidate.getTrackHits();
}

CDCLegendreTrackCandidate::~CDCLegendreTrackCandidate()
{
}

CDCLegendreTrackCandidate::CDCLegendreTrackCandidate(int ID, double theta,
                                                     double r, int charge, std::string cdcHitsDatastoreName,
                                                     double resolutionAxial, double resolutionStereo, bool assignHits) :
  m_theta(theta), m_r(r), m_y0(m_r / sin(m_theta)), m_slope(
    -1 / tan(m_theta)), m_charge(charge), m_ID(ID), m_assignHits(
      assignHits)
{
  AddAxialHits(cdcHitsDatastoreName, resolutionAxial);

  AddStereoHits(cdcHitsDatastoreName, resolutionStereo);

  m_uniqueHits = getNUniqueHits();
}

TVector3 CDCLegendreTrackCandidate::getMomentumEstimation() const
{
  double yc = getYc();
  double xc = getXc();

  double R = sqrt(xc * xc + yc * yc);
  double pt = R * 1.5 * 0.00299792458;

  TVector2 mom2 = (TVector2(xc, yc).Rotate(TMath::Pi() / 2).Unit()) * pt * getChargeSign();

  double mom_z = getZMomentumEstimation(mom2);

  TVector3 mom3(mom2.X(), mom2.Y(), mom_z);

  return mom3 ;
}

double CDCLegendreTrackCandidate::getZMomentumEstimation(TVector2 mom2) const
{
  std::vector<double> median_vector;

  BOOST_FOREACH(CDCLegendreTrackHit * trackHit, m_TrackHits) {
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

  sort(median_vector.begin(), median_vector.end());

  double medianTheta = median_vector.at(median_vector.size() / 2);

  return (tan(medianTheta) * sqrt(mom2.X() * mom2.X() + mom2.Y() * mom2.Y()));
}

void CDCLegendreTrackCandidate::AddAxialHits(std::string cdcHitsDatastoreName,
                                             double resolution)
{
  if (m_charge != charge_positive && m_charge != charge_negative
      && m_charge != charge_curler) {
    B2ERROR(
      "Undefined charge given to CDCLegendreTrackCandidate::AddAxialHits");
    exit(EXIT_FAILURE);
  }

  StoreArray<CDCLegendreTrackHit> cdcLegendreTrackHits(cdcHitsDatastoreName);

  for (int i = 0; i < cdcLegendreTrackHits.getEntries(); i++) {
    //add only axial hits
    if (not cdcLegendreTrackHits[i]->getIsAxial())
      continue;

    //add only hits within the resolution range
    if (DistanceTo(*cdcLegendreTrackHits[i]) > resolution)
      continue;

    if ((m_charge == charge_positive || m_charge == charge_negative)
        && cdcLegendreTrackHits[i]->getCurvatureSignWrt(getXc(),
                                                        getYc()) != m_charge)
      continue;

    m_TrackHits.push_back(cdcLegendreTrackHits[i]);
    if (m_assignHits)
      cdcLegendreTrackHits[i]->assignToTrack(m_ID);
  }
}

void CDCLegendreTrackCandidate::AddStereoHits(std::string cdcHitsDatastoreName,
                                              double resolution)
{
  if (m_charge != charge_positive && m_charge != charge_negative
      && m_charge != charge_curler) {
    B2ERROR(
      "Undefined charge given to CDCLegendreTrackCandidate::AddStereoHits");
    exit(EXIT_FAILURE);
  }

  StoreArray<CDCLegendreTrackHit> cdcLegendreTrackHits(cdcHitsDatastoreName);

  for (int i = 0; i < cdcLegendreTrackHits.getEntries(); i++) {
    if (cdcLegendreTrackHits[i]->getIsAxial())
      continue;

    if ((m_charge == charge_positive || m_charge == charge_negative)
        && cdcLegendreTrackHits[i]->getCurvatureSignWrt(getXc(), getYc()) != getChargeSign())
      continue;

    if (OriginalDistanceTo(*cdcLegendreTrackHits[i]) > 2.)
      continue;

    cdcLegendreTrackHits[i]->shiftAlongZ(*this);

    double d_new = DistanceTo(*cdcLegendreTrackHits[i]);

    if (d_new < resolution) {
      m_TrackHits.push_back(cdcLegendreTrackHits[i]);
      if (m_assignHits)
        cdcLegendreTrackHits[i]->assignToTrack(m_ID);
    }
  }
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

  double d = sqrt((xc - xw) * (xc - xw) + (yc - yw) * (yc - yw));

  if (d > max(rw, rc))
    d = fabs(d - (rc + rw));
  else
    d = fabs(max(rw, rc) - (d + min(rw, rc)));

  return d;
}

int CDCLegendreTrackCandidate::getChargeAssumption(
  std::string cdcHitsDatastoreName, double theta, double r,
  double resolution)
{
  StoreArray<CDCLegendreTrackHit> cdcLegendreTrackHits(
    cdcHitsDatastoreName.c_str());

  double y0 = r / sin(theta);
  double slope = -1 / tan(theta);
  double yc = 1 / y0;
  double xc = 1 / (-1 * y0 / slope);
  double rc = sqrt(xc * xc + yc * yc);

  if (rc < 56.5)
    return charge_curler;

  int vote_pos = 0;
  int vote_neg = 0;

  for (int i = 0; i < cdcLegendreTrackHits.getEntries(); i++) {
    if (DistanceTo(xc, yc, *cdcLegendreTrackHits[i]) >= resolution)
      continue;

    int curve_sign = cdcLegendreTrackHits[i]->getCurvatureSignWrt(xc, yc);

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

int CDCLegendreTrackCandidate::getNUniqueHits() const
{
  int nUnique = 0;

  BOOST_FOREACH(CDCLegendreTrackHit * trackHit, m_TrackHits) {
    if (trackHit->getAssignedTracks().size() == 1)
      ++nUnique;
  }

  return nUnique;
}
