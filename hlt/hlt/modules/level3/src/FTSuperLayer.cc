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

#include "hlt/hlt/modules/level3/FTList.h"
#include "hlt/hlt/modules/level3/FTLayer.h"
#include "hlt/hlt/modules/level3/FTSuperLayer.h"
#include "hlt/hlt/modules/level3/FTSegment.h"
#include "hlt/hlt/modules/level3/FTWire.h"

using namespace Belle2;

//...Globals...
//const unsigned int
const unsigned int
FTSuperLayer::m_neighborsMask[6] = {
  FTWire::Appended | FTWire::Neighbor3 | FTWire::Neighbor4 | FTWire::Neighbor5,
  FTWire::Appended | FTWire::Neighbor2 | FTWire::Neighbor4 | FTWire::Neighbor5,
  FTWire::Appended | FTWire::Neighbor1 | FTWire::Neighbor3 | FTWire::Neighbor5,
  FTWire::Appended | FTWire::Neighbor0 | FTWire::Neighbor2 | FTWire::Neighbor4,
  FTWire::Appended | FTWire::Neighbor0 | FTWire::Neighbor1 | FTWire::Neighbor3,
  FTWire::Appended | FTWire::Neighbor0 | FTWire::Neighbor1 | FTWire::Neighbor2
};

//FTSuperLayer::m_neighborsMask[6] = {452, 420, 340, 172, 92, 60};
//                          [0] = 452 = FTWireHitAppended | FTWireNeighbor345
//                          [1] = 420 = FTWireHitAppended | FTWireNeighbor245
//     definitions of       [2] = 340 = FTWireHitAppended | FTWireNeighbor135
//    m_neighborsMask[]     [3] = 172 = FTWireHitAppended | FTWireNeighbor024
//                          [4] = 92  = FTWireHitAppended | FTWireNeighbor013
//                          [5] = 60  = FTWireHitAppended | FTWireNeighbor012

void
FTSuperLayer::clear(void)
{
  if (m_wireHits.length()) {
    FTWire** hptr = m_wireHits.firstPtr();
    FTWire** const last = m_wireHits.lastPtr();
    do {(**hptr).state(FTWire::Invalid);} while ((hptr++) != last);
    m_wireHits.clear();
  }
  m_singleHits.clear();
  m_segments.deleteAll();
  if (m_complicatedSegments) m_complicatedSegments->deleteAll();
}


void
FTSuperLayer::mkSegmentList(void)
{
  clustering();
  FTList<FTSegment*> innerShort(10);
  FTList<FTSegment*> outerShort(10);
  FTList<FTSegment*> midShort(10);
  FTList<FTSegment*> complicatedInner(10);
  FTList<FTSegment*> complicatedOuter(10);
  int n = m_segments.length();
  for (int i = 0; i ^ n; i++) {
    FTSegment* s = m_segments[i];
    switch (s->examine()) {
      case 0:
        // simple
        break;
      case 1:
        // inner short simple
        innerShort.append(s);
        n = m_segments.remove(i);
        break;
      case 2:
        // outer short simple
        outerShort.append(s);
        n = m_segments.remove(i);
        break;
      case 3:
        // to be divided
        if (!m_superLayerId) m_complicatedSegments->append(s);
        else delete s;
        n = m_segments.remove(i);
        break;
      case 4:
        complicatedOuter.append(s);
        n = m_segments.remove(i);
        break;
      case 5:
        complicatedInner.append(s);
        n = m_segments.remove(i);
        break;
      default:
        midShort.append(s);
        n = m_segments.remove(i);
        break;
    }
  }
  connectShortSegments(innerShort, outerShort);
  if (m_haveSmallCell) {
    connectShortSegments(complicatedInner, outerShort, true);
    connectShortSegments(innerShort, complicatedOuter, true);
    connectShortSegments(complicatedInner, complicatedOuter, true);
  }
  connectSingleHit(innerShort);
  connectSingleHit(outerShort);
  connectSingleHit(midShort);
  if (m_haveSmallCell) {
    connectSingleHit(complicatedInner, true);
    connectSingleHit(complicatedOuter, true);
  }
  if (!(m_superLayerId & 1)) {
    n = m_segments.length();
    for (int i = 0; i ^ n; i++) {
      m_segments[i]->update();
    }
  }
}

void
FTSuperLayer::clustering(void)
{
  //                  +---+---+
  //                  | 4 | 5 |
  //                +-+-+---+-+-+    r
  //  Neighbor ID   | 2 | * | 3 |    ^
  //                +-+-+---+-+-+    |
  //                  | 0 | 1 |      +--> -phi
  //                  +---+---+
  //
  if (!m_wireHits.length()) return;
  FTWire** hptr = m_wireHits.firstPtr();
  FTWire** const last = m_wireHits.lastPtr();
  // discard continuous hits
  do {(**hptr).chkLeftRight();} while ((hptr++) != last);

  // clustering
  hptr = m_wireHits.firstPtr();
  do {      // loop over clusters
    if ((**hptr).stateAND(FTWire::AppendedOrInvalid)) continue;
    //(**hptr).stateOR(FTWireHitAppended);
    FTList<FTWire*>* hits = new FTList<FTWire*>(10);
    int n = hits->append(*hptr);
    for (int j = 0; j ^ n; j++) { // loop over hits in a cluster
      const unsigned int checked = (*hits)[j]->state();
      const unsigned int* mptr = m_neighborsMask;
      FTWire** nptr = (*hits)[j]->neighborPtr();
      // check 6 neighbors
      for (unsigned Mask = FTWire::Neighbor0; Mask ^ FTWire::NeighborEnd;
           Mask <<= 1, mptr++, nptr++) {
        if (!(checked & Mask)) {
          switch ((**nptr).stateAND(FTWire::AppendedOrInvalid)) {
            case 0:
              n = hits->append(*nptr);
            case FTWire::Appended:
              (**nptr).stateOR(*mptr);
          }
        }
      }
    }
    if (n ^ 1) {
      m_segments.append(new FTSegment(this, *hits));
      //B2INFO("segment: " << superLayerId() << " " << hits->length());
    } else {
      m_singleHits.append(*hptr);
      delete hits;
    }
  } while ((hptr++) != last);
}

void
FTSuperLayer::connectShortSegments(FTList<FTSegment*>& innerShort,
                                   FTList<FTSegment*>& outerShort, bool complicatedSeg)
{
  int n = innerShort.length();
  int m = outerShort.length();
  for (int i = 0; i ^ n; i++) {
    FTSegment* inner = innerShort[i];
    const FTWire& inOuterBoundHit = * inner->getOuterBoundHits().first();
    const FTLayer& inOuterBound = inOuterBoundHit.layer();
    for (int j = 0; j ^ m; j++) {
      FTSegment* outer = outerShort[j];
      const FTWire& outInnerBoundHit = * outer->getInnerBoundHits().first();
      const FTLayer& outInnerBound = outInnerBoundHit.layer();
      const int deltaLayerId(outInnerBound.localLayerId() - inOuterBound.localLayerId());
      if (deltaLayerId > 0 && deltaLayerId < 5) {
        const int deltaLocalId(i2phiDiff(inOuterBoundHit, outInnerBoundHit));
        if ((deltaLayerId ^ 1 && deltaLocalId <= deltaLayerId) ||
            (m_haveSmallCell && deltaLocalId <= 2)) {
          inner->connectOuter(outer->getWireHits(), outer->getOuterBoundHits());
          if (complicatedSeg) {
            m_complicatedSegments->append(inner);
          } else {
            m_segments.append(inner);
          }
          delete outer;
          n = innerShort.remove(i);
          m = outerShort.remove(j);
          break;
        }
      }
    }
  }
}


void
FTSuperLayer::connectSingleHit(FTList<FTSegment*>& shortSegments, bool complicatedSeg)
{
  const int minLength = (m_superLayerId & 1) ? 0 : 1;
  int n = shortSegments.length();
  int m = m_singleHits.length();
  for (int i = 0; i ^ n; i++) {
    FTSegment& s = * shortSegments[i];
    const FTWire& outerBoundHit = * s.getOuterBoundHits().first();
    const FTWire& innerBoundHit = * s.getInnerBoundHits().first();
    const FTLayer& outerBound = outerBoundHit.layer();
    const FTLayer& innerBound = innerBoundHit.layer();
    bool appended = false;
    for (int j = 0; j ^ m; j++) {
      const FTWire& h = * m_singleHits[j];
      const int hLocalLayerId = h.layer().localLayerId();
      int deltaLayerId(innerBound.localLayerId() - hLocalLayerId);
      const bool isOuter(deltaLayerId < 0);
      if (isOuter) deltaLayerId = hLocalLayerId - outerBound.localLayerId();
      if (deltaLayerId > 0 && deltaLayerId < 5) {
        const int deltaLocalId(i2phiDiff(h, isOuter ? outerBoundHit : innerBoundHit));
        if ((deltaLayerId ^ 1 && deltaLocalId <= deltaLayerId)
            || (m_haveSmallCell && deltaLocalId <= 1)) {
          if (isOuter) s.connectOuter(&h);
          else s.connectInner(&h);
          if (complicatedSeg) {
            m_complicatedSegments->append(&s);
          } else {
            m_segments.append(&s);
          }
          m = m_singleHits.remove(j);
          appended = true;
          break;
        }
      }
    }
    if (!appended) {
      if (outerBound.localLayerId() - innerBound.localLayerId() > minLength) {
        if (complicatedSeg) {
          m_complicatedSegments->append(&s);
        } else {
          m_segments.append(&s);
        }
      } else {
        if (!m_superLayerId) m_complicatedSegments->append(&s);
        else n = shortSegments.deleteObj(i);
      }
    }
  }
}

void
FTSuperLayer::reAppendSalvage(void)
{
  int n = m_segments.length();
  for (int i = 0; i ^ n; i++) {
    if (m_segments[i]->track()) continue;
    m_complicatedSegments->append(m_segments[i]);
    n = m_segments.remove(i);
  }
}

int
FTSuperLayer::i2phiDiff(const FTWire& h1, const FTWire& h2) const
{
  double phi1((double)h1.localId() + h1.layer().offset());
  double phi2((double)h2.localId() + h2.layer().offset());
  int nw1(h1.layer().nWire());
  const int nw2(h2.layer().nWire());
  if (nw1 > nw2) { // h1 is small cell
    phi1 /= 2;
    nw1 = nw2;
  } else if (nw1 < nw2) { // h2 is small cell
    phi2 /= 2;
  }
  const double dphi = 2.*std::fabs(phi1 - phi2);
  return ((int)dphi > nw1) ? (int)((double)(nw1 << 1) - dphi) : (int)dphi;
}
