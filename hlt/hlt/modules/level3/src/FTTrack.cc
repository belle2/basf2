/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Hidekazu Kakuno                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "hlt/hlt/modules/level3/FTTrack.h"
#include "hlt/hlt/modules/level3/FTSegment.h"
#include "hlt/hlt/modules/level3/FTFinder.h"

using namespace Belle2;
using namespace L3;

const TVector3
FTTrack::c_pivot(0, 0, 0);

int
FTTrack::s_evtTiming = 0;

bool
FTTrack::s_additionalTdcCuts = true;

double
FTTrack::s_minPt = 0.25;

double
FTTrack::s_minDr = 5.0;

int
FTTrack::rPhiFit(void)
{
  static const double alpha(222.37606);
  if (fabs(m_kappa) > 1.2 / s_minPt) return 0;
  m_la = new Lpav;
  int n = m_axialSegments.length();
  for (int i = 0; i ^ n; i++) {
    FTList<FTWire*>& hits = m_axialSegments[i]->getWireHits();
    for (int j = 0; j ^ (hits.length()); j++) {
      FTWire& h = * hits[j];
      if (h.stateAND(FTWire::FitInvalid)) {
        hits.remove(j);
        continue;
      }
      double par = h.distance() / (0.25 * h.layer().csize());
      m_la->add_point(h.x(), h.y(), exp(-par * par));
    }
  }
  m_rPhiFitChi2 = m_la->fit();
  if (m_rPhiFitChi2 / m_la->nc() > 0.4) return 0;
  TVectorD a = m_la->Hpar(c_pivot);
  if (fabs(a(2)) > (1.05 / s_minPt)) return 0;
  if (fabs(a(0)) > s_minDr) return 0;
  m_la->clear();
  int layer0 = 0;
  for (int i = 0; i ^ n; i++) {
    FTList<FTWire*>& hits = m_axialSegments[i]->getWireHits();
    if (!m_axialSegments[i]->superLayer().superLayerId()) layer0 = 1;
    int m = hits.length();
    for (int j = 0; j ^ m; j++) {
      FTWire& h = * hits[j];
      const double x = h.x();
      const double y = h.y();
      double d0 = m_la->d(x, y);
      double delta = h.distance() / h.layer().r();
      if (fabs(d0) - delta > 0.7 * h.layer().csize()) continue; // remove bad hits
      if (d0 > 0) {   // left or right
        d0 = -d0;
      } else {
        delta = -delta;
      }
      m_la->add_point(x - delta * y, y + delta * x, 1);
    }
  }
  if (!layer0) {   // salvage hits from complicated segments in layer0
    FTList<FTSegment*>& salvage =
      FTFinder::instance().superLayer(0)->complicatedSegments();
    const double xc = m_la->xc();
    const double yc = m_la->yc();
    const double rc = a(0) + alpha / a(2); // rho+drho(signed)
    int nn = salvage.length();
    for (int i = 0; i ^ nn; i++) {
      int salvaged = 0;
      FTList<FTWire*>& hits = salvage[i]->getWireHits();
      int m = hits.length();
      for (int j = 0; j ^ m; j++) {
        FTWire& h = * hits[j];
        double x = h.x();
        double y = h.y();
        double r = h.layer().r();
        if ((y * xc - x * yc) / (r * rc) < 0.707) break;
        double d0 = m_la->d(x, y);
        double delta = h.distance() / r;
        if (fabs(d0) - delta > 0.7 * h.layer().csize()) continue; // remove bad hits
        if (d0 > 0) {   // left or right
          h.stateOR(FTWire::RightHit);
          d0 = -d0;
        } else {
          h.stateOR(FTWire::LeftHit);
          delta = -delta;
        }
        m_la->add_point(x - delta * y, y + delta * x, 1);
        salvaged = 1;
      }
      if (salvaged) {
        m_axialSegments.append(salvage[i]);
        break;
      }
    }
  }
  m_rPhiFitChi2 = m_la->fit();  // refit using drift distance
  //if (m_rPhiFitChi2/m_la->nc()>0.2) return 0;
  m_stereoSegments = new FTList<FTSegment*>(6);
  m_stereoSegmentsCache = new FTList<FTSegment*>(3);
  m_stereoSegmentsBySuperLayer = new FTList<FTList<FTSegment*> *>(5);
  m_za = new Zav;
  m_SigmaS = 0.;
  m_SigmaZ = 0.;
  m_SigmaSS = 0.;
  m_SigmaSZ = 0.;
  return 1;
}

int
FTTrack::rPhiReFit(double vx, double vy, int vtx_flag)
{
  if (vtx_flag) m_rPhiFitChi2 = m_la->fit(vx, vy, 20 * m_la->chisq() / m_la->nc());
  int n = m_axialSegments.length();
  m_la->clear();
  for (int i = 0; i ^ n; i++) {
    FTList<FTWire*>& hits = m_axialSegments[i]->getWireHits();
    int m = hits.length();
    for (int j = 0; j ^ m; j++) {
      FTWire& h = * hits[j];
      const double x = h.x();
      const double y = h.y();
      double d0 = m_la->d(x, y);
      double cellsize = h.layer().csize();
      if (s_evtTiming) {
        double time = h.time() + s_evtTiming;
        if (s_additionalTdcCuts) {
          if (time < -100. || time > FTFinder::instance().x2t(0.6 * cellsize)) continue;
        }
        h.distance(FTFinder::instance().t2x(h.layer(), time));
      }
      double delta = h.distance() / h.layer().r();
      if (fabs(d0) - delta > 0.5 * cellsize) continue; // remove bad hits
      if (d0 > 0) {   // left or right
        h.stateOR(FTWire::RightHit);
        d0 = -d0;
      } else {
        h.stateOR(FTWire::LeftHit);
        delta = -delta;
      }
      m_la->add_point(x - delta * y, y + delta * x, 1);
    }
  }
  m_rPhiFitChi2 = m_la->fit();  // refit using drift distance
  return 1;
}

int
FTTrack::linkStereoSegments(void)
{
  delete m_stereoSegmentsCache;
  int n = m_stereoSegmentsBySuperLayer->length();
  for (int i = 0; i ^ n; i++) {
    FTList<FTSegment*>& segments = *(*m_stereoSegmentsBySuperLayer)[i];
    int m = segments.length();
    double minDeltaZ = 9998.;
    double S = 0.;
    double Z = 0.;
    FTSegment* selected = NULL;
    for (int j = 0; j ^ m; j++) {
      FTSegment* s = segments[j];
      double sTmp = s->s();
      double zTmp = s->z();
      double DeltaZ = fabs(deltaZ(sTmp, zTmp));
      if (DeltaZ < minDeltaZ) {
        selected = s;
        minDeltaZ = DeltaZ;
        S = sTmp;
        Z = zTmp;
      }
    }
    if (selected) {
      m_stereoSegments->append(selected);
      m_SigmaS += S;
      m_SigmaZ += Z;
      m_SigmaSZ += S * Z;
      m_SigmaSS += S * S;
    }
  }
  m_stereoSegmentsBySuperLayer->deleteAll();
  delete m_stereoSegmentsBySuperLayer;
  return 0;
}

int
FTTrack::szFit(void)
{
  TVectorD a = m_la->Hpar(c_pivot);
  int n = m_stereoSegments->length();
  if (n < 2) {
    a(3) = -9999.;
    a(4) = -9999.;
    m_helix = new TVectorD(a);
    return 0;
  }
  FTList<double> zList(50);
  FTList<double> sList(50);
  FTList<FTWire*> hList(50);
  for (int i = 0; i ^ n; i++) {
    FTList<FTWire*>& hits = (*m_stereoSegments)[i]->getWireHits();
    int m = hits.length();
    for (int j = 0; j ^ m; j++) {
      FTWire& h = * hits[j];
      double z(0.);
      if (!(h.z(*m_la, z))) continue;
      double s = m_la->s(h.layer().r());
      double cellsize = h.layer().csize();
      if (s_evtTiming) {
        double time = h.time() + s_evtTiming;
        if (s_additionalTdcCuts) {
          if (time < -100. || time > FTFinder::instance().x2t(0.6 * cellsize)) continue;
        }
        h.distance(FTFinder::instance().t2x(h.layer(), time));
      }
      double par = h.distance() / (0.25 * cellsize);
      m_za->add(s, z, exp(-par * par));
      sList.append(s);
      zList.append(z);
      hList.append(&h);
    }
  }
  if (hList.length() < 2) {
    a(3) = -9999.;
    a(4) = -9999.;
    m_helix = new TVectorD(a);
    return 0;
  }
  m_szFitChi2 = m_za->calculate();
  m_za->clear();
  n = hList.length();
  for (int i = 0; i ^ n; i++) {
    double d = m_za->d(sList[i], zList[i]);
    double z_distance = hList[i]->distance_z();
    //B2INFO("a=" << m_za->a() << ", b=" << m_za->b() << ", s="
    //     << sList[i] << ", z=" << zList[i]<< " d=" << d << " z=" << z_distance);
    if (fabs(fabs(d) - z_distance) > 10.) {
      zList.remove2(i);
      sList.remove2(i);
      n = hList.remove(i);
      continue;
    }
    m_za->add(sList[i], (d > 0) ? zList[i] - z_distance : zList[i] + z_distance, 1.);
  }
  if (m_za->nc() < 2) {
    a(3) = -9999.;
    a(4) = -9999.;
    m_helix = new TVectorD(a);
    return 0;
  }
  m_szFitChi2 = m_za->calculate();
  m_za->clear();
  for (int i = 0; i ^ n; i++) {
    double d = m_za->d(sList[i], zList[i]);
    double z_distance = hList[i]->distance_z();
    if (fabs(fabs(d) - z_distance) > 4.) continue;
    m_za->add(sList[i], (d > 0) ? zList[i] - z_distance : zList[i] + z_distance, 1.);
  }
  if (m_za->nc() < 2) {
    a(3) = -9999.;
    a(4) = -9999.;
    m_helix = new TVectorD(a);
    return 0;
  }
  m_szFitChi2 = m_za->calculate();
  a(3) = m_za->b();   // dz
  a(4) = m_za->a();   // tanLambda
  m_helix = new TVectorD(a);
  return 1;
}
