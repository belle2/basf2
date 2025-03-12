/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

//-----------------------------------------------------------------------------
// Description : A class to relate TRGCDCWireHit and TRGCDCTrack objects.
//-----------------------------------------------------------------------------

#define TRG_SHORT_NAMES
#define TRGCDC_SHORT_NAMES

#include <string>
#include <cstring>
#include <algorithm>
#include "trg/trg/Utilities.h"
#include "trg/cdc/Wire.h"
#include "trg/cdc/WireHit.h"
#include "trg/cdc/WireHitMC.h"
#include "trg/cdc/TRGCDCTrack.h"
#include "trg/cdc/TrackMC.h"
#include "trg/cdc/Segment.h"
#include "trg/cdc/Link.h"

using namespace std;

namespace Belle2 {

  bool TRGCDCLink::ms_smallcell(false);
  bool TRGCDCLink::ms_superb(false);
  unsigned TRGCDCLink::m_nL = 56;
  unsigned TRGCDCLink::m_nSL = 9;
  unsigned TRGCDCLink::m_nSLA = 5;
  unsigned* TRGCDCLink::m_nHitsSL = 0;
  vector<TCLink*> TRGCDCLink::m_all;

  TRGCDCLink::TRGCDCLink(TRGCDCTrack* t,
                         const TRGCDCCellHit* h,
                         const HepGeom::Point3D<double>& p)
    : m_track(t),
      m_hit(h),
      m_position(p),
      m_dPhi(0),
      m_leftRight(0),
      m_pull(0),
      m_link(0),
      m_fit2D(0)
  {
    if (h) {
      m_drift[0] = h->drift(0);
      m_drift[1] = h->drift(1);
      m_dDrift[0] = h->dDrift(0);
      m_dDrift[1] = h->dDrift(1);
    } else {
      m_drift[0] = 0.;
      m_drift[1] = 0.;
      m_dDrift[0] = 0.;
      m_dDrift[1] = 0.;
    }

    for (unsigned i = 0; i < 7; ++i)
      m_neighbor[i] = NULL;

    if (h) {
      m_onTrack = m_onWire = h->xyPosition();
    }
  }

  TRGCDCLink::TRGCDCLink(const TRGCDCLink& l)
    : m_track(l.m_track),
      m_hit(l.m_hit),
      m_onTrack(l.m_onTrack),
      m_onWire(l.m_onWire),
      m_position(l.m_position),
      m_dPhi(l.m_dPhi),
      m_leftRight(l.m_leftRight),
      m_zStatus(l.m_zStatus),
      m_zPair(l.m_zPair),
      m_pull(l.m_pull),
      m_link(l.m_link),
      m_fit2D(l.m_fit2D)
  {
    m_drift[0] = l.m_drift[0];
    m_drift[1] = l.m_drift[1];
    m_dDrift[0] = l.m_dDrift[0];
    m_dDrift[1] = l.m_dDrift[1];
    for (unsigned i = 0; i < 7; ++i)
      m_neighbor[i] = l.m_neighbor[i];
    for (unsigned i = 0; i < 4; ++i)
      m_arcZ[i] = l.m_arcZ[i];
  }

  TRGCDCLink::~TRGCDCLink()
  {
  }

  unsigned
  TRGCDCLink::nLayers(const vector<TRGCDCLink*>& list)
  {
    unsigned l0 = 0;
    unsigned l1 = 0;
    unsigned n = list.size();
    for (unsigned i = 0; i < n; i++) {
      unsigned id = list[i]->cell()->layerId();
      if (id < 32) l0 |= (1u << id);
      else         l1 |= (1 << (id - 32));
    }

    unsigned l = 0;
    for (unsigned i = 0; i < 32; i++) {
      if (l0 & (1u << i)) ++l;
      if (l1 & (1u << i)) ++l;
    }
    return l;
  }

  void
  TRGCDCLink::nHits(const vector<TRGCDCLink*>& links, unsigned* nHits)
  {
    for (unsigned i = 0; i < m_nL; i++)
      nHits[i] = 0;
    unsigned nLinks = links.size();
    for (unsigned i = 0; i < nLinks; i++)
      ++nHits[links[i]->cell()->layerId()];
  }

  void
  TRGCDCLink::nHitsSuperLayer(const vector<TRGCDCLink*>& links, unsigned* nHits)
  {
    const unsigned nLinks = links.size();
    for (unsigned i = 0; i < nLinks; i++)
      ++nHits[links[i]->cell()->superLayerId()];
  }

  void
  TRGCDCLink::dump_base(const string& msg, const string& pre) const
  {

    //...Basic options...
//  bool track = (msg.find("track") != string::npos);
//  bool mc = (msg.find("mc") != string::npos);
    bool pull = (msg.find("pull") != string::npos);
    bool flag = (msg.find("flag") != string::npos);
    bool stereo = (msg.find("stereo") != string::npos);
    bool pos = (msg.find("position") != string::npos);

    //...Strong options...
    bool breif = (msg.find("breif") != string::npos);
    bool detail = (msg.find("detail") != string::npos);
    if (detail)
//  track = mc = pull = flag = stereo = pos = true;
      pull = flag = stereo = pos = true;
    if (breif)
      pull = flag = true;

    //...Output...
    cout << pre;
    if (m_hit) {
      cout << cell()->name();
    } else {
      cout << "No hit linked";
    }
//     if (mc) {
//  if (_hit) {
//      if (_hit->mc()) {
//    if (_hit->mc()->hep())
//        cout << "(mc" << _hit->mc()->hep()->id() << ")";
//    else
//        cout << "(mc?)";
//      }
//      else {
//    cout << "(mc?)";
//      }
//  }
//     }
    if (pull)
      cout << "[pul=" << this->pull() << "]";
    if (flag) {
      if (m_hit) {
        if (m_hit->state() & CellHitFindingValid)
          cout << "o";
        if (m_hit->state() & CellHitFittingValid)
          cout << "+";
        if (m_hit->state() & CellHitInvalidForFit)
          cout << "x";
      }
    }
    if (stereo) {
      cout << "{" << leftRight() << "," << m_zStatus << "}";
    }
    if (pos) {
      cout << ",pos=" << position();
      cout << ",drift=" << drift(0) << "," << drift(1);
    }
  }

  void
  TRGCDCLink::dump(const string& msg, const string& pre) const
  {
    dump_base(msg, pre);
    cout << endl;
  }

  void
  TRGCDCLink::dump(const vector<TRGCDCLink*>& links,
                   const string& msg,
                   const string& pre)
  {
    vector<const TRGCDCLink*> clinks;
    for (unsigned i = 0; i < links.size(); i++)
      clinks.push_back(links[i]);
    TRGCDCLink::dump(clinks, msg, pre);
  }

  void
  TRGCDCLink::dump(const vector<const TRGCDCLink*>& links,
                   const string& msg,
                   const string& pre)
  {

    //...Basic options...
    bool mc = (msg.find("mc") != string::npos);
    bool sort = (msg.find("sort") != string::npos);
    bool flag = (msg.find("flag") != string::npos);

    //...Strong options...
    bool detail = (msg.find("detail") != string::npos);
    if (detail)
      mc = flag = true;

    vector<const TRGCDCLink*> tmp = links;
    if (sort)
      std::sort(tmp.begin(), tmp.end(), TRGCDCLink::sortById);
//  sort(tmp.begin(), tmp.end(), TRGCDCLink::sortById);
    unsigned n = tmp.size();
    unsigned nForFit = 0;
#define MCC_MAX 1000
    unsigned MCC0[MCC_MAX];
    unsigned MCC1[MCC_MAX];
    memset((char*) MCC0, 0, sizeof(unsigned) * MCC_MAX);
    memset((char*) MCC1, 0, sizeof(unsigned) * MCC_MAX);
    bool MCCOverFlow = false;

    cout << pre;
    for (unsigned i = 0; i < n; i++) {
      const TRGCDCLink& l = * tmp[i];

//  if (mc) {
//      unsigned mcId = 999;
//      if (l.hit()) {
//    if (l.hit()->mc())
//        if (l.hit()->mc()->hep())
//      mcId = l.hit()->mc()->hep()->id();
//    if (mcId < MCC_MAX) {
//        ++MCC0[mcId];
//        if (l.hit()->state() & WireHitFittingValid) {
//      if (! (l.hit()->state() & WireHitInvalidForFit))
//          ++MCC1[mcId];
//        }
//    }
//    else {
//        MCCOverFlow = true;
//    }
//      }
//  }
      if (flag) {
        if (l.hit()) {
          if (l.hit()->state() & CellHitFittingValid) {
            if (!(l.hit()->state() & CellHitInvalidForFit))
              ++nForFit;
          }
        }
      }
      if (i)
        cout << ",";
      l.dump_base(msg);
    }
    if (n)
      cout << ",Total " << n << " links";
    else
      cout << "no link";
    if (flag) cout << ",fv " << nForFit << " l(s)";
    if (mc) {
      unsigned nMC = 0;
      cout << ", mc";
      for (unsigned i = 0; i < MCC_MAX; i++) {
        if (MCC0[i] > 0) {
          ++nMC;
          cout << i << ":" << MCC0[i] << ",";
        }
      }
      cout << "total " << nMC << " mc contributions";
      if (flag) {
        nMC = 0;
        cout << ", fv mc";
        for (unsigned i = 0; i < MCC_MAX; i++) {
          if (MCC1[i] > 0) {
            ++nMC;
            cout << i << ":" << MCC1[i] << ",";
          }
        }
        cout << " total " << nMC << " mc fit valid contribution(s)";
      }

      // cppcheck-suppress knownConditionTrueFalse
      if (MCCOverFlow)
        cout << "(counter overflow)";
    }
    cout << endl;

    //...Parent...
    if (mc) {
      vector<const Belle2::TRGCDCTrackMC*> list = Belle2::TRGCDCTrackMC::list();
      if (! list.size()) return;
      cout << pre;
//cnv unsigned nMC = 0;
      for (unsigned i = 0; i < MCC_MAX; i++) {
        if (MCC0[i] > 0) {
          const Belle2::TRGCDCTrackMC* h = list[i];
//    cout << ", mc" << i << "(" << h->pType() << ")";
          cout << ", mc" << i << "(";
          if (h)
            cout << h->pType() << ")";
          else
            cout << "?)";
          while (h) {
            const Belle2::TRGCDCTrackMC* m = h->mother();
            if (m) {
              cout << "<-mc" << m->id();
              h = m;
            } else {
              break;
            }
          }
        }
      }
      if (MCCOverFlow)
        cout << "(counter overflow)";
      cout << endl;
    }
  }

  void
  TRGCDCLink::dump(const TRGCDCLink& link,
                   const string& msg,
                   const string& pre)
  {
    vector<const TRGCDCLink*> tmp;
    tmp.push_back(& link);
    dump(tmp, msg, pre);
  }

  unsigned
  TRGCDCLink::nStereoHits(const vector<TRGCDCLink*>& links)
  {
    unsigned nLinks = links.size();
    unsigned n = 0;
    for (unsigned i = 0; i < nLinks; i++)
      if (links[i]->cell()->stereo())
        ++n;
    return n;
  }

  unsigned
  TRGCDCLink::nAxialHits(const vector<TRGCDCLink*>& links)
  {
    unsigned nLinks = links.size();
    unsigned n = 0;
    for (unsigned i = 0; i < nLinks; i++)
      if (links[i]->cell()->axial())
        ++n;
    return n;
  }

  vector<TRGCDCLink*>
  TRGCDCLink::axialHits(const vector<TRGCDCLink*>& links)
  {
    vector<TRGCDCLink*> a;
    unsigned n = links.size();
    for (unsigned i = 0; i < n; i++) {
      if (links[i]->cell()->axial())
        a.push_back(links[i]);
    }
    return a;
  }

  vector<TRGCDCLink*>
  TRGCDCLink::stereoHits(const vector<TRGCDCLink*>& links)
  {
    vector<TRGCDCLink*> a;
    unsigned n = links.size();
    for (unsigned i = 0; i < n; i++) {
      if (! links[i]->cell()->axial())
        a.push_back(links[i]);
    }
    return a;
  }

  TRGCDCLink*
  TRGCDCLink::innerMost(const vector<TRGCDCLink*>& a)
  {
    unsigned n = a.size();
    unsigned minId = 19999;
    TRGCDCLink* t = 0;
    for (unsigned i = 0; i < n; i++) {
      unsigned id = a[i]->cell()->id();
      if (id < minId) {
        minId = id;
        t = a[i];
      }
    }
    return t;
  }

  TRGCDCLink*
  TRGCDCLink::outerMost(const vector<TRGCDCLink*>& a)
  {
    unsigned n = a.size();
    unsigned maxId = 0;
    TRGCDCLink* t = 0;
    for (unsigned i = 0; i < n; i++) {
      unsigned id = a[i]->cell()->id();
      if (id >= maxId) {
        maxId = id;
        t = a[i];
      }
    }
    return t;
  }

  void
  TRGCDCLink::separateCores(const vector<TRGCDCLink*>& input,
                            vector<TRGCDCLink*>& cores,
                            vector<TRGCDCLink*>& nonCores)
  {
    unsigned n = input.size();
    for (unsigned i = 0; i < n; i++) {
      TRGCDCLink& t = * input[i];
      const Belle2::TCCHit& h = * t.hit();
      if (h.state() & CellHitFittingValid)
        cores.push_back(& t);
      else
        nonCores.push_back(& t);
    }
  }

  vector<TRGCDCLink*>
  TRGCDCLink::cores(const vector<TRGCDCLink*>& input)
  {
    vector<TRGCDCLink*> a;
    unsigned n = input.size();
    for (unsigned i = 0; i < n; i++) {
      TRGCDCLink& t = * input[i];
      const Belle2::TCCHit& h = * t.hit();
      if (h.state() & CellHitFittingValid)
        a.push_back(& t);
    }
    return a;
  }

  bool
  TRGCDCLink::sortById(const TRGCDCLink* a, const TRGCDCLink* b)
  {
    return a->cell()->id() < b->cell()->id();
  }

  int
  TRGCDCLink::sortByX(const TRGCDCLink* a, const TRGCDCLink* b)
  {
    return a->position().x() < b->position().x();
  }

  unsigned
  TRGCDCLink::width(const vector<TRGCDCLink*>& list)
  {
    const unsigned n = list.size();
    if (n < 2) return n;

    const TCCell* const w0 = list[0]->cell();
//cnv    const unsigned sId = w0->superLayerId();
    unsigned nWires = w0->layer().nCells();
    unsigned center = w0->localId();

    if (ms_smallcell && w0->layerId() < 3) {
      nWires /= 2;
      center /= 2;
    }

    unsigned left = 0;
    unsigned right = 0;
    for (unsigned i = 1; i < n; i++) {
      const TCCell* const w = list[i]->cell();
      unsigned id = w->localId();

      if (ms_smallcell && w->layerId() < 3)
        id /= 2;

      unsigned distance0, distance1;
      if (id > center) {
        distance0 = id - center;
        distance1 = nWires - distance0;
      } else {
        distance1 = center - id;
        distance0 = nWires - distance1;
      }

      if (distance0 < distance1) {
        if (distance0 > right) right = distance0;
      } else {
        if (distance1 > left) left = distance1;
      }
    }

    return right + left + 1;
  }

  vector<TRGCDCLink*>
  TRGCDCLink::edges(const vector<TRGCDCLink*>& list)
  {
    vector<TRGCDCLink*> a;

    unsigned n = list.size();
    if (n < 2) return a;
    else if (n == 2) return list;

    const TCCell* w = list[0]->cell();
    unsigned nWires = w->layer().nCells();
    unsigned center = w->localId();

    unsigned left = 0;
    unsigned right = 0;
    TRGCDCLink* leftL = list[0];
    TRGCDCLink* rightL = list[0];
    for (unsigned i = 1; i < n; i++) {
      w = list[i]->cell();
      unsigned id = w->localId();

      unsigned distance0, distance1;
      if (id > center) {
        distance0 = id - center;
        distance1 = nWires - distance0;
      } else {
        distance1 = center - id;
        distance0 = nWires - distance1;
      }

      if (distance0 < distance1) {
        if (distance0 > right) {
          right = distance0;
          rightL = list[i];
        }
      } else {
        if (distance1 > left) {
          left = distance1;
          leftL = list[i];
        }
      }
    }

    a.push_back(leftL);
    a.push_back(rightL);
    return a;
  }

  vector<TRGCDCLink*>
  TRGCDCLink::sameLayer(const vector<TRGCDCLink*>& list, const TRGCDCLink& a)
  {
    vector<TRGCDCLink*> same;
    unsigned id = a.cell()->layerId();
    unsigned n = list.size();
    for (unsigned i = 0; i < n; i++) {
      if (list[i]->cell()->layerId() == id) same.push_back(list[i]);
    }
    return same;
  }

  vector<TRGCDCLink*>
  TRGCDCLink::sameSuperLayer(const vector<TRGCDCLink*>& list, const TRGCDCLink& a)
  {
    vector<TRGCDCLink*> same;
    unsigned id = a.cell()->superLayerId();
    unsigned n = list.size();
    for (unsigned i = 0; i < n; i++) {
      if (list[i]->cell()->superLayerId() == id) same.push_back(list[i]);
    }
    return same;
  }

  vector<TRGCDCLink*>
  TRGCDCLink::sameLayer(const vector<TRGCDCLink*>& list, unsigned id)
  {
    vector<TRGCDCLink*> same;
    unsigned n = list.size();
    for (unsigned i = 0; i < n; i++) {
      if (list[i]->cell()->layerId() == id) same.push_back(list[i]);
    }
    return same;
  }

  vector<TRGCDCLink*>
  TRGCDCLink::sameSuperLayer(const vector<TRGCDCLink*>& list, unsigned id)
  {
    vector<TRGCDCLink*> same;
    unsigned n = list.size();
    for (unsigned i = 0; i < n; i++) {
      if (list[i]->cell()->superLayerId() == id) same.push_back(list[i]);
    }
    return same;
  }

  vector<TRGCDCLink*>
  TRGCDCLink::inOut(const vector<TRGCDCLink*>& list)
  {
    vector<TRGCDCLink*> inners;
    vector<TRGCDCLink*> outers;
    unsigned n = list.size();
    unsigned innerMostLayer = 999;
    unsigned outerMostLayer = 0;
    for (unsigned i = 0; i < n; i++) {
      unsigned id = list[i]->cell()->layerId();
      if (id < innerMostLayer) innerMostLayer = id;
      else if (id > outerMostLayer) outerMostLayer = id;
    }
    for (unsigned i = 0; i < n; i++) {
      unsigned id = list[i]->cell()->layerId();
      if (id == innerMostLayer) inners.push_back(list[i]);
      else if (id == outerMostLayer) outers.push_back(list[i]);
    }
//  inners.push_back(outers);
    inners.insert(inners.end(), outers.begin(), outers.end());
    return inners;
  }

  unsigned
  TRGCDCLink::superLayer(const vector<TRGCDCLink*>& list)
  {
    unsigned sl = 0;
    unsigned n = list.size();
    for (unsigned i = 0; i < n; i++)
      sl |= (1 << (list[i]->cell()->superLayerId()));
    return sl;
  }

  unsigned
  TRGCDCLink::superLayer(const vector<TRGCDCLink*>& links, unsigned minN)
  {
    clearBufferSL();
    unsigned n = links.size();
    for (unsigned i = 0; i < n; i++)
      ++m_nHitsSL[links[i]->cell()->superLayerId()];
    unsigned sl = 0;
    for (unsigned i = 0; i < m_nSL; i++)
      if (m_nHitsSL[i] >= minN)
        sl |= (1 << i);
    return sl;
  }

  unsigned
  TRGCDCLink::nSuperLayers(const vector<TRGCDCLink*>& list)
  {
    unsigned l0 = 0;
    unsigned n = list.size();
    for (unsigned i = 0; i < n; i++) {
      unsigned id = list[i]->cell()->superLayerId();
      l0 |= (1 << id);
    }

    unsigned l = 0;
    for (unsigned i = 0; i < m_nSL; i++) {
      if (l0 & (1 << i)) ++l;
    }
    return l;
  }

  unsigned
  TRGCDCLink::nSuperLayers(const vector<TRGCDCLink*>& links, unsigned minN)
  {
    clearBufferSL();
    unsigned n = links.size();
    for (unsigned i = 0; i < n; i++)
      ++m_nHitsSL[links[i]->cell()->superLayerId()];
    unsigned sl = 0;
    for (unsigned i = 0; i < m_nSL; i++)
      if (m_nHitsSL[i] >= minN)
        ++sl;
    return sl;
  }

  unsigned
  TRGCDCLink::nMissingAxialSuperLayers(const vector<TRGCDCLink*>& links)
  {
    clearBufferSL();
    const unsigned n = links.size();
//  unsigned nHits[6] = {0, 0, 0, 0, 0, 0};
    for (unsigned i = 0; i < n; i++)
      if (links[i]->cell()->axial())
        ++m_nHitsSL[links[i]->cell()->axialStereoSuperLayerId()];
//      ++nHits[links[i]->cell()->superLayerId() / 2];
    unsigned j = 0;
    while (m_nHitsSL[j] == 0) ++j;
    unsigned nMissing = 0;
    unsigned nMax = 0;
    for (unsigned i = j; i < m_nSLA; i++) {
      if (+m_nHitsSL[i] == 0) ++nMissing;
      else {
        if (nMax < nMissing) nMax = nMissing;
        nMissing = 0;
      }
    }
    return nMax;
  }

  const Belle2::TRGCDCTrackMC&
  TRGCDCLink::links2HEP(const vector<TRGCDCLink*>&)
  {
    const Belle2::TRGCDCTrackMC* best = TRGCDCTrackMC::_undefined;
    const vector<const Belle2::TRGCDCTrackMC*> list =
      Belle2::TRGCDCTrackMC::list();
    unsigned nHep = list.size();

    if (! nHep) return * best;

    unsigned* N;
    if (NULL == (N = (unsigned*) malloc(nHep * sizeof(unsigned)))) {
//      perror("$Id: TRGCDCLink.cc 11153 2010-04-28 03:36:53Z yiwasaki $:N:malloc");
      exit(1);
    }
    for (unsigned i = 0; i < nHep; i++) N[i] = 0;

//     for (unsigned i = 0; i < (unsigned) links.size(); i++) {
//  const TRGCDCLink & l = * links[i];
//  const Belle2::TRGCDCTrackMC & hep = * l.hit()->mc()->hep();
//  for (unsigned j = 0; j < nHep; j++)
//      if (list[j] == & hep)
//    ++N[j];
//     }

    unsigned nMax = 0;
    for (unsigned i = 0; i < nHep; i++) {
      if (N[i] > nMax) {
        best = list[i];
        nMax = N[i];
      }
    }

    free(N);

    return * best;
  }

  void
  TRGCDCLink::nHitsSuperLayer(const vector<TRGCDCLink*>& links, vector<TRGCDCLink*>* list)
  {
    const unsigned nLinks = links.size();
    for (unsigned i = 0; i < nLinks; i++)
      list[links[i]->cell()->superLayerId()].push_back(links[i]);
  }

  string
  TRGCDCLink::layerUsage(const vector<TRGCDCLink*>& links)
  {
//  unsigned n[11];
    static unsigned* n = new unsigned[Belle2::TRGCDC::getTRGCDC()->nSuperLayers()];
    for (unsigned i = 0; i < m_nSL; i++) {
      n[i] = 0;
    }
    nHitsSuperLayer(links, n);
    string nh = "";
    for (unsigned i = 0; i < m_nSL; i++) {
      nh += TRGUtil::itostring(n[i]);
      if (i % 2) nh += ",";
      else if (i < 10) nh += "-";
    }
    return nh;
  }

  void
  TRGCDCLink::remove(vector<TRGCDCLink*>& list,
                     const vector<TRGCDCLink*>& links)
  {
    const unsigned n = list.size();
    const unsigned m = links.size();
    // vector<TRGCDCLink *> toBeRemoved;

    for (unsigned i = 0; i < n; i++) {
      for (unsigned j = 0; j < m; j++) {
        if (list[i]->cell()->id() == links[j]->cell()->id())
//    toBeRemoved.push_back(list[i]);

          cout << "TCLink::remove !!! not implemented yet" << endl;
      }
    }

//  list.remove(toBeRemoved);
  }

  void
  TRGCDCLink::initializeBuffers(void)
  {
    static bool first = true;
    if (first) {
      const Belle2::TRGCDC& cdc = * Belle2::TRGCDC::getTRGCDC();
      m_nL = cdc.nLayers();
      m_nSL = cdc.nSuperLayers();
      m_nSLA = cdc.nAxialSuperLayers();
      m_nHitsSL = new unsigned[m_nSL];
      first = false;
    }
  }

  void
  TRGCDCLink::separate(const vector<TRGCDCLink*>& links,
                       unsigned nLayers,
                       vector<TRGCDCLink*>* layers)
  {
    for (unsigned i = 0; i < links.size(); i++) {
      const TCCell* c = links[i]->cell();
      if (c) {
        unsigned lid = c->layer().id();
        if (lid < nLayers)
          layers[lid].push_back(links[i]);
      }
    }
  }

  const TRGCDCWire*
  TRGCDCLink::wire(void) const
  {
    if (m_hit)
      return dynamic_cast<const TRGCDCWire*>(& m_hit->cell());
    return 0;
  }

// inline
// const TRGCDCSegment *
// TRGCDCLink::segment(void) const {
//     if (_hit)
//  return dynamic_cast<const TRGCDCSegment *>(& _hit->cell());
//     return 0;
// }

  void
  TRGCDCLink::removeAll(void)
  {
    while (m_all.size())
      delete m_all.back();
  }

  void*
  TRGCDCLink::operator new (size_t size)
  {
    void* p = malloc(size);
    m_all.push_back(static_cast<TRGCDCLink*>(p));

//     cout << ">---------------------" << endl;
//     for (unsigned i = 0; i < _all.size(); i++)
//  cout << "> " << i << " " << _all[i] << endl;

    return p;
  }

  void
  TRGCDCLink::operator delete (void* t)
  {
    for (vector<TRGCDCLink*>::iterator it = m_all.begin();
         it != m_all.end();
         ++it) {
      if ((* it) == static_cast<TRGCDCLink*>(t)) {
        m_all.erase(it);
        break;
      }
    }
    free(t);

//     cout << "<---------------------" << endl;
//     cout << "==> " << t << " erased" << endl;
//     for (unsigned i = 0; i < _all.size(); i++)
//  cout << "< " << i << " " << _all[i] << endl;
  }

} // namespace Belle
