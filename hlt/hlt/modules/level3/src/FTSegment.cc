/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Hidekazu Kakuno                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <cmath>
#include "skim/hlt/modules/level3/FTSegment.h"
#include "skim/hlt/modules/level3/FTTrack.h"
#include "skim/hlt/modules/level3/FTWire.h"
#include "skim/hlt/modules/level3/FTSuperLayer.h"

using namespace Belle2;
using namespace L3;

int
FTSegment::examine()
{
  int stat = -1;
  const int minLength = (m_superLayer.superLayerId() & 1) ? 0 : 2;
  FTList<FTWire*> wireHitsByLocalLayer[8] = {
    FTList<FTWire*>(3), FTList<FTWire*>(3), FTList<FTWire*>(3),
    FTList<FTWire*>(3), FTList<FTWire*>(3), FTList<FTWire*>(3),
    FTList<FTWire*>(3), FTList<FTWire*>(3)
  };
  FTWire** hptr = m_wireHits.firstPtr();
  FTWire** const last = m_wireHits.lastPtr();
  do {
    wireHitsByLocalLayer[(**hptr).layer().localLayerId()].append(*hptr);
  } while ((hptr++) != last);
  int lengthPhi = 0;
  int innerBoundId = 0;
  int outerBoundId = 0;
  int AlreadyFound = 0;
  const int LocalLayerLength = m_wireHits.first()->layer().nWire();
  const int HalfLocalLayerLength = LocalLayerLength >> 1;
  const int nLayer = m_superLayer.nLayer();
  for (int i = 0; i ^ nLayer; i++) {
    FTList<FTWire*>& hits = wireHitsByLocalLayer[i];
    int N_wireHits = hits.length();
    if (N_wireHits) {
      unsigned localIdMax = 0;
      unsigned localIdMin = 9999;
      for (int j = 0; j ^ N_wireHits; j++) {
        unsigned localId = hits[j]->localId();
        if (localId > localIdMax) localIdMax = localId;
        if (localId < localIdMin) localIdMin = localId;
      }
      int localLengthPhi = (int)localIdMax - (int)localIdMin + 1;
      if (localLengthPhi > (int)HalfLocalLayerLength)
        localLengthPhi = LocalLayerLength - localLengthPhi + 2;
      if (localLengthPhi > lengthPhi) lengthPhi = localLengthPhi;
      outerBoundId = i;
      AlreadyFound = 1;
    } else if (!AlreadyFound) {
      innerBoundId++;
    }
  }
  if (lengthPhi > 2) { // to be divided
    if (m_superLayer.haveSmallCell() && innerBoundId > 1) {
      stat = 4;
    } else if (m_superLayer.haveSmallCell() && outerBoundId < 2) {
      stat = 5;
    } else {
      stat = 3;
    }
  } else {
    if (innerBoundId > 2 || (m_superLayer.haveSmallCell() && innerBoundId > 1)) {
      stat = 2;     // outer short
    } else {
      if (outerBoundId < m_superLayer.nLayer() - 3 || (m_superLayer.haveSmallCell() && outerBoundId < 2)) {
        stat = 1;   // inner short
      } else {
        if (outerBoundId - innerBoundId > minLength) {
          stat = 0;   // long simple
        }
      }
    }
  }
  m_innerBoundHits.append(wireHitsByLocalLayer[innerBoundId]);
  m_outerBoundHits.append(wireHitsByLocalLayer[outerBoundId]);
  return stat;
}

void
FTSegment::update(void)
{
  static const double alpha(222.37606);
  int n = m_innerBoundHits.length();
  int m = m_outerBoundHits.length();
  m_incoming_x = (n ^ 1)
                 ? 0.5 * (m_innerBoundHits[0]->x() + m_innerBoundHits[1]->x())
                 : m_innerBoundHits[0]->x();
  m_incoming_y = (n ^ 1)
                 ? 0.5 * (m_innerBoundHits[0]->y() + m_innerBoundHits[1]->y())
                 : m_innerBoundHits[0]->y();
  m_outgoing_x = (m ^ 1)
                 ? 0.5 * (m_outerBoundHits[0]->x() + m_outerBoundHits[1]->x())
                 : m_outerBoundHits[0]->x();
  m_outgoing_y = (m ^ 1)
                 ? 0.5 * (m_outerBoundHits[0]->y() + m_outerBoundHits[1]->y())
                 : m_outerBoundHits[0]->y();
  double in_r = m_innerBoundHits.first()->layer().r();
  double out_r = m_outerBoundHits.first()->layer().r();
  double sbX = m_incoming_x - m_outgoing_x;
  double sbY = m_incoming_y - m_outgoing_y;
  // kappa = -2. * alpha * ((Vin X Vout)_z / |Vin|*|Vout|) / |Vin-Vout|
  m_kappa = 2.*alpha * (m_outgoing_x * m_incoming_y - m_outgoing_y * m_incoming_x) /
            (in_r * out_r * sqrt(sbX * sbX + sbY * sbY));
  m_r = 0.5 * (in_r + out_r);
}


int
FTSegment::update3D(const FTTrack* track)
{
  if (m_superLayer.superLayerId() >
      track->getAxialSegments().first()->superLayer().superLayerId() + 1) return 0;
  if (!m_sList) {
    int n = m_innerBoundHits.length();
    int m = m_outerBoundHits.length();
    m_incoming_x = (n ^ 1)
                   ? 0.5 * (m_innerBoundHits[0]->x() + m_innerBoundHits[1]->x())
                   : m_innerBoundHits[0]->x();
    m_incoming_y = (n ^ 1)
                   ? 0.5 * (m_innerBoundHits[0]->y() + m_innerBoundHits[1]->y())
                   : m_innerBoundHits[0]->y();
    m_outgoing_x = (m ^ 1)
                   ? 0.5 * (m_outerBoundHits[0]->x() + m_outerBoundHits[1]->x())
                   : m_outerBoundHits[0]->x();
    m_outgoing_y = (m ^ 1)
                   ? 0.5 * (m_outerBoundHits[0]->y() + m_outerBoundHits[1]->y())
                   : m_outerBoundHits[0]->y();
    m_sList = new FTList<double>(5);
    m_zList = new FTList<double>(5);
    m_trackList = new FTList<FTTrack*>(5);
  }
  const Lpav& la = track->lpav();
  double d;
  double inS;
  const FTLayer* bound = &m_innerBoundHits.first()->layer();
  if (!(la.sd(bound->r(), m_incoming_x,
              m_incoming_y, bound->limit(), inS, d))) return 0;
  double inZ = bound->z(d);
  double outS;
  bound = &m_outerBoundHits.first()->layer();
  if (!(la.sd(bound->r(), m_outgoing_x,
              m_outgoing_y, bound->limit(), outS, d))) return 0;
  double outZ = bound->z(d);
  m_sList->append(inS + outS);
  m_zList->append(inZ + outZ);
  m_trackList->append(const_cast<FTTrack*>(track));
  return 1;
}

void
FTSegment::linkStereoSegments(void)
{
  double minDeltaZ = 9999.;
  double S = 0.;
  double Z = 0.;
  FTTrack* selected = NULL;
  int m = m_trackList->length();
  for (int j = 0; j ^ m; j++) {
    FTTrack* t = (*m_trackList)[j];
    double zTmp = (*m_zList)[j];
    double sTmp = (*m_sList)[j];
    double DeltaZ = fabs(t->deltaZ(sTmp, zTmp));
    if (DeltaZ < minDeltaZ) {
      selected = t;
      minDeltaZ = DeltaZ;
      S = sTmp;
      Z = zTmp;
    }
  }
  if (selected) {
    selected->appendStereo(this, S, Z);
  }
}
