//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : TCDC.cc
// Section  : CDC tracking trasan
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to represent CDC.
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#include <cstdlib>
#include <iostream>

#include "tracking/modules/trasan/TCDC.h"
#include "tracking/modules/trasan/TWire.h"
#include "tracking/modules/trasan/TLayer.h"
#include "tracking/modules/trasan/TWireHit.h"
#include "tracking/modules/trasan/TWireHitMC.h"
#include "tracking/modules/trasan/TDebug.h"
#include "tracking/modules/trasan/TUtilities.h"

#include "framework/datastore/StoreArray.h"
#include "framework/datastore/RelationArray.h"
#include "mdst/dataobjects/MCParticle.h"
#include "cdc/dataobjects/CDCHit.h"
#include "cdc/dataobjects/CDCSimHit.h"
#include "cdc/geometry/CDCGeometryPar.h"

#define P3D HepGeom::Point3D<double>

using namespace std;

namespace Belle {

  string
  TCDC::name(void) const
  {
    return "TCDC";
  }

  string
  TCDC::version(void) const
  {
    return string("TCDC 5.39");
  }

  TCDC*
  TCDC::_cdc = 0;

  TCDC*
  TCDC::getTCDC(const string& configFile)
  {
    if (_cdc) {
      //delete _cdc;
      _cdc = 0;
    }

    if (configFile != "good-bye") {
      _cdc = new TCDC(configFile);
    } else {
      cout << "TCDC::getTCDC ... good-bye" << endl;
      //        delete _cdc;
      _cdc = 0;
    }

    return _cdc;
  }

  TCDC*
  TCDC::getTCDC(void)
  {
    if (! _cdc)
      cout << "TCDC::getTCDC !!! TCDC is not created yet" << endl;
    return _cdc;
  }

  TCDC::TCDC(const string&)
    : _debugLevel(0),
      _fudgeFactor(1.),
      _width(0),
      _r(0),
      _r2(0)
  {
    initialize();
  }

  void
  TCDC::initialize(void)
  {

    //...CDC...
    Belle2::CDC::CDCGeometryPar& cdc2 =
      Belle2::CDC::CDCGeometryPar::Instance();
    const unsigned nLayers = cdc2.nWireLayers();

    //...Loop over layers...
    int superLayerId = -1;
    vector<TLayer*>* superLayer;
    unsigned lastNWires = 0;
    int lastShifts = -1000;
    int ia = -1;
    int is = -1;
    int ias = -1;
    int iss = -1;
    unsigned nWires = 0;
    float fwrLast = 0;
    unsigned axialStereoSuperLayerId = 0;
    for (unsigned i = 0; i < nLayers; i++) {
      const unsigned nWiresInLayer = cdc2.nWiresInLayer(i);

      //...Axial or stereo?...
      int nShifts = cdc2.nShifts(i);
      bool axial = true;
      if (nShifts != 0)
        axial = false;

      unsigned axialStereoLayerId = 0;
      if (axial) {
        ++ia;
        axialStereoLayerId = ia;
      } else {
        ++is;
        axialStereoLayerId = is;
      }

      //...Is this in a new super layer?...
      if ((lastNWires != nWiresInLayer) || (lastShifts != nShifts)) {
        ++superLayerId;
        superLayer = new vector<TLayer*>;
        _superLayers.push_back(superLayer);
        if (axial) {
          ++ias;
          axialStereoSuperLayerId = ias;
          _axialSuperLayers.push_back(superLayer);
        } else {
          ++iss;
          axialStereoSuperLayerId = iss;
          _stereoSuperLayers.push_back(superLayer);
        }
        lastNWires = nWiresInLayer;
        lastShifts = nShifts;
      }

      //...Calculate radius...
      const float swr = cdc2.senseWireR(i);
      float fwr = 0.0;
      if (i < nLayers - 1) {
        // number of field wire layers is nLayers - 1
        fwr = cdc2.fieldWireR(i);
        if (i == nLayers - 2)
          fwrLast = fwr;
      } else {
        fwr = swr + (swr - fwrLast);
      }
      const float innerRadius = swr - (fwr - swr);
      const float outerRadius = swr + (fwr - swr);

      if (TDebug::level() > 9)
        cout << "lyr " << i << ", in=" << innerRadius << ", out="
             << outerRadius << ", swr=" << swr << ", fwr" << fwr << endl;

      //...New layer...
      TLayer* layer = new TLayer(i,
                                 superLayerId,
                                 _superLayers[superLayerId]->size(),
                                 axialStereoLayerId,
                                 axialStereoSuperLayerId,
                                 cdc2.zOffsetWireLayer(i),
                                 nShifts,
                                 M_PI * cdc2.senseWireR(i)
                                 * cdc2.senseWireR(i)
                                 / double(nWiresInLayer),
                                 nWiresInLayer,
                                 innerRadius,
                                 outerRadius);
      _layers.push_back(layer);
      superLayer->push_back(layer);
      if (axial)
        _axialLayers.push_back(layer);
      else
        _stereoLayers.push_back(layer);

      //...Loop over all wires in a layer...
      for (unsigned j = 0; j < nWiresInLayer; j++) {
        const P3D fp = P3D(cdc2.wireForwardPosition(i, j).x(),
                           cdc2.wireForwardPosition(i, j).y(),
                           cdc2.wireForwardPosition(i, j).z());
        const P3D bp = P3D(cdc2.wireBackwardPosition(i, j).x(),
                           cdc2.wireBackwardPosition(i, j).y(),
                           cdc2.wireBackwardPosition(i, j).z());
        TWire* tw = new TWire(nWires++, j, * layer, fp, bp);
        _wires.push_back(tw);
        layer->push_back(tw);
      }
    }

    //...Fill caches...
    if (_width) delete [] _width;
    if (_r) delete [] _r;
    if (_r2) delete [] _r2;
    _width = new float[nSuperLayers()];
    _r = new float[nSuperLayers() + 1];
    _r2 = new float[nSuperLayers() + 1];
    for (unsigned i = 0; i < nSuperLayers(); i++) {
      const vector<TLayer*>& slayer = * _superLayers[i];
      _width[i] = M_PI * 2 / float(slayer.back()->nCells());
      _r[i] = slayer[0]->innerRadius();
      _r2[i] = _r[i] * _r[i];
      if (i == (nSuperLayers() - 1)) {
        _r[i + 1] = slayer.back()->outerRadius();
        _r2[i + 1] = _r[i + 1] * _r[i + 1];
      }
    }
  }


  void
  TCDC::terminate(void)
  {
  }

  void
  TCDC::dump(const string& msg) const
  {
    if (msg.find("name")    != string::npos ||
        msg.find("version") != string::npos ||
        msg.find("detail")  != string::npos ||
        msg == "") {
      cout << name() << "(CDC version=" << versionCDC() << ", "
           << version() << ") ";
    }
    if (msg.find("detail") != string::npos ||
        msg.find("state") != string::npos) {
      cout << "Debug Level=" << _debugLevel;
    }
    cout << endl;

    string tab("        ");

    if (msg == "" || msg.find("geometry") != string::npos) {

      //...Get information..."
      unsigned nLayer = _layers.size();
      cout << "    version    : " << version() << endl;
      cout << "    cdc version: " << versionCDC() << endl;
      cout << "    # of wires : " << _wires.size() << endl;
      cout << "    # of layers: " << nLayer << endl;
      cout << "    super layer information" << endl;
      cout << "        # of super layers() = "
           << nSuperLayers() << endl;
      cout << "        # of Axial super layers = "
           << nAxialSuperLayers() << endl;
      cout << "        # of Stereo super layers = "
           << nStereoSuperLayers() << endl;

      if (msg.find("superLayers") != string::npos) {
        cout << "        super layer detail" << endl;
        cout << "            id #layers (stereo type)" << endl;
        for (unsigned i = 0; i < nSuperLayers(); ++i) {
          const unsigned n = _superLayers[i]->size();
          cout << "            " << i << "  " << n << " (";
          for (unsigned j = 0; j < n; j++) {
            cout << (* _superLayers[i])[0]->stereoType();
          }
          cout << ")" << endl;
        }
      }

      cout << "    layer information" << endl;
      cout << "        # of Axial layers = "
           << nAxialLayers() << endl;
      cout << "        # of Stereo layers = "
           << nStereoLayers() << endl;

      if (msg.find("layers") != string::npos) {
        cout << "        layer detail" << endl;
        cout << "            id type sId #wires lId asId assId"
             << endl;
        for (unsigned int i = 0; i < nLayers(); ++i) {
          const TLayer& l = * _layers[i];
          cout << "            " << i
               << " " << l.stereoType()
               << " " << l.superLayerId()
               << " " << l.nCells()
               << " " << l.localLayerId()
               << " " << l.axialStereoLayerId()
               << " " << l.axialStereoSuperLayerId()
               << endl;
        }
      }

      if (msg.find("wires") != string::npos) {
        cout << "    wire information" << endl;
        for (unsigned i = 0; i < nWires(); i++)
          (_wires[i])->dump("neighbor", tab);
      }

      return;
    }
    if (msg.find("hits") != string::npos) {
      cout << "    hits : " << _hits.size() << endl;
      for (unsigned i = 0; i < (unsigned) _hits.size(); i++)
        _hits[i]->dump("mc drift", tab);
    }
    if (msg.find("axialHits") != string::npos) {
      cout << "    hits : " << _axialHits.size() << endl;
      for (unsigned i = 0; i < (unsigned) _axialHits.size(); i++)
        _axialHits[i]->dump("mc drift", tab);
    }
    if (msg.find("stereoHits") != string::npos) {
      cout << "    hits : " << _stereoHits.size() << endl;
      for (unsigned i = 0; i < (unsigned) _stereoHits.size(); i++)
        _stereoHits[i]->dump("mc drift", tab);
    }
  }

  const TWire*
  TCDC::wire(unsigned id) const
  {
    if (id < nWires())
      return _wires[id];
    return 0;
  }

  const TWire*
  TCDC::wire(unsigned layerId, int localId) const
  {
    if (layerId < nLayers())
      return (TWire*) & _layers[layerId]->cell(localId);
    return 0;
  }

// const TWire *
// TCDC::wire(const HepGeom::Point3D<double> & p) const {
//     float r = p.mag();
//     float phi = p.phi();
//     return wire(r, phi);
// }

  const TWire*
  TCDC::wire(float , float) const
  {

    //...Not implemented yet...
    return _wires[0];

    // //     cout << "r,phi = " << r << "," << p << endl;

    // //     unsigned id = 25;
    // //     bool ok = false;
    // //     const TLayer * l;
    // //     while (! ok) {
    // //         l = layer(id);
    // //         if (! l) return 0;

    // //         const geocdc_layer * geo = l->geocdc();
    // //         if (geo->m_r + geo->m_rcsiz2 < r) ++id;
    // //         else if (geo->m_r - geo->m_rcsiz1 > r) --id;
    // //         else ok = true;
    // //     }
    // //     float dPhi = 2. * M_PI / float(l->nCells());
    // //     if (l->geocdc()->m_offset > 0.) p -= dPhi / 2.;
    // //     unsigned localId = unsigned(phi(p) / dPhi);
    // //     return l->wire(localId);
    // }
  }

  void
  TCDC::clear(void)
  {
    TWireHit::removeAll();

    //     for (unsigned i = 0; i < _hits.size(); i++)
    //         delete _hits[i];
    for (unsigned i = 0; i < _hitsMC.size(); i++)
      delete _hitsMC[i];
    //     for (unsigned i = 0; i < _badHits.size(); i++)
    //         delete _badHits[i];
    //     for (unsigned i = 0; i < _segmentHits.size(); i++)
    //         delete _segmentHits[i];

    for (unsigned i = 0; i < _wires.size(); i++) {
      TWire* w = _wires[i];
      w->clear();
    }
    _hitWires.clear();
    _hits.clear();
    _axialHits.clear();
    _stereoHits.clear();
    _badHits.clear();
    _hitsMC.clear();
  }

  void
  TCDC::fastClear(void)
  {
  }

  void
  TCDC::update(bool)
  {

    TDebug::enterStage("TCDC update");

    //...Clear old information...
    //  fastClear();
    clear();

    //...CDCSimHit...
    Belle2::StoreArray<Belle2::CDCSimHit> SimHits;
    if (! SimHits) {
      cout << "TCDC !!! can not access to CDCSimHits" << endl;
      TDebug::leaveStage("TCDC update");
      return;
    }
    const unsigned n = SimHits.getEntries();

    //...CDCHit...
    Belle2::StoreArray<Belle2::CDCHit> CDCHits("CDCHits");
    if (! CDCHits) {
      cout << "TCDC !!! can not access to CDCHits" << endl;
      TDebug::leaveStage("TCDC update");
      return;
    }
    const unsigned nHits = CDCHits.getEntries();

    //...MCParticle...
    Belle2::StoreArray<Belle2::MCParticle> mcParticles;
    if (! mcParticles) {
      cout << "TCDC !!! can not access to MCParticles" << endl;
      TDebug::leaveStage("TCDC update");
      return;
    }

    //...Relations...
    Belle2::RelationArray rels(SimHits, CDCHits);
    const unsigned nRels = rels.getEntries();
    Belle2::RelationArray relsMC(mcParticles, CDCHits);
    const unsigned nRelsMC = relsMC.getEntries();

    //...Loop over CDCHits...
    for (unsigned i = 0; i < nHits; i++) {
      const Belle2::CDCHit& h = * CDCHits[i];

      //      //...Check validity (skip broken channel)...
      //      if (! (h->m_stat & CellHitFindingValid)) continue;

      //...Get CDCSimHit... This is expensive. Should be moved outside.
      unsigned iSimHit = 0;
      for (unsigned j = 0; j < nRels; j++) {
        const unsigned k = rels[j].getToIndices().size();
        for (unsigned l = 0; l < k; l++) {
          if (rels[j].getToIndex(l) == i)
            iSimHit = rels[j].getFromIndex();
        }

        if (TDebug::level())
          if (k > 1)
            cout << "TCDC::update !!! CDCSimHit[" << iSimHit
                 << "] has multiple CDCHit(" << k << " hits)" << endl;
      }

      //...Get MCParticle... This is expensive, again.
      //   (Getting the first MCParticle only)
      unsigned iMCPart = 0;
      for (unsigned j = 0; j < nRelsMC; j++) {
        const unsigned k = relsMC[j].getToIndices().size();
        for (unsigned l = 0; l < k; l++) {
          if (relsMC[j].getToIndex(l) == i) {
            iMCPart = relsMC[j].getFromIndex();
            break;
          }
        }

        if (TDebug::level())
          if (k > 1)
            cout << "TCDC::update !!! MCParticle[" << iMCPart
                 << "] has multiple CDCHit(" << k << " hits)" << endl;
      }

      //...Wire...
      int t_layerId;
      if (h.getISuperLayer() == 0) t_layerId = h.getILayer();
      else t_layerId = h.getILayer() + 6 * h.getISuperLayer() + 2;
      const unsigned layerId = t_layerId;
      const unsigned wireId = h.getIWire();
      TWire& w = * (TWire*) wire(layerId, wireId);

      //...TDC count...
      const int tdcCount = h.getTDCCount();

      //...Drift length from TDC...
      const float driftLength = tdcCount * (40. / 10000.);
      const float driftLengthError = 0.013;

      //...Left/right...
      const int LRflag = SimHits[iSimHit]->getPosFlag();

      //...TWireHit...
      TWireHit* hit = new TWireHit(w,
                                   i,
                                   iSimHit,
                                   iMCPart,
                                   driftLength,
                                   driftLengthError,
                                   driftLength,
                                   driftLengthError,
                                   LRflag,
                                   1);
      hit->state(CellHitFindingValid | CellHitFittingValid);

      //...Store a hit...
      ((TWire*)(* _layers[layerId])[wireId])->hit(hit);
      _hits.push_back(hit);
      if (w.axial()) _axialHits.push_back(hit);
      else           _stereoHits.push_back(hit);

      //...Debug...
      if (TDebug::level() > 2) {
        std::cout << TDebug::tab(4) << "CDCHit TDC count="
                  << h.getTDCCount() << std::endl;
      }
    }

    //...Hit classification...
    //  _hits.sort(TCWHit::sortByWireId);
    classification();

    if (TDebug::level() > 2) {
      Belle2::StoreArray<Belle2::CDCSimHit> simHits("CDCSimHits");

      if (TDebug::level()) {
        cout << TDebug::tab() << "#CDCSimHit=" << n << ",#CDCHit="
             << nHits << endl;
      }

      if (TDebug::level() > 10) {
        for (unsigned i = 0; i < _hits.size(); i++) {
          const TWireHit& h = * _hits[i];
          h.dump("detail", TDebug::tab(4));
        }
      } else {
        const unsigned n = 10;
        cout << TDebug::tab() << "Dump of the first " << n
             << " hits of a wire" << endl;
        for (unsigned i = 0; i < n; i++) {
          const TWireHit& h = * _hits[i];
          h.dump("detail", TDebug::tab(4));
        }
      }
    }

    TDebug::leaveStage("TCDC update");
  }

  void
  TCDC::classification(void)
  {
    unsigned n = _hits.size();

    for (unsigned i = 0; i < n; i++) {
      TWireHit* h = _hits[i];
      const TWire& w = h->wire();
      unsigned state = h->state();

      //...Cache pointers to a neighbor...
      const TWire* neighbor[7];
      for (unsigned j = 0; j < 7; j++) neighbor[j] = w.neighbor(j);

      //...Decide hit pattern...
      unsigned pattern = 0;
      for (unsigned j = 0; j < 7; j++) {
        if (neighbor[j])
          if (neighbor[j]->hit())
            pattern += (1 << j);
      }
      state |= (pattern << CellHitNeighborHit);

      //...Check isolation...
      const TWireHit* hr1 = neighbor[2]->hit();
      const TWireHit* hl1 = neighbor[3]->hit();
      if ((hr1 == 0) && (hl1 == 0)) {
        state |= CellHitIsolated;
      } else {
        const TWireHit* hr2 = neighbor[2]->neighbor(2)->hit();
        const TWireHit* hl2 = neighbor[3]->neighbor(3)->hit();
        if (((hr2 == 0) && (hr1 != 0) && (hl1 == 0)) ||
            ((hl2 == 0) && (hl1 != 0) && (hr1 == 0)))
          state |= CellHitIsolated;
      }

      //...Check continuation...
      //        unsigned superLayer = w.superLayerId();
      bool previous = false;
      bool next = false;
      if (neighbor[0] == 0) previous = true;
      else {
        if ((neighbor[0]->hit()) || neighbor[1]->hit())
          previous = true;
        //             if (m_smallcell && w.layerId() == 3)
        //                 if (neighbor[6]->hit())
        //                     previous = true;
      }
      if (neighbor[5] == 0) next = true;
      else {
        if ((neighbor[4]->hit()) || neighbor[5]->hit())
          next = true;
      }
      // if (previous && next) state |= CellHitContinuous;
      if (previous || next) state |= CellHitContinuous;

      //...Solve LR locally...
      if ((pattern == 34) || (pattern == 42) ||
          (pattern == 40) || (pattern == 10) ||
          (pattern == 35) || (pattern == 50))
        state |= CellHitPatternRight;
      else if ((pattern == 17) || (pattern == 21) ||
               (pattern == 20) || (pattern ==  5) ||
               (pattern == 19) || (pattern == 49))
        state |= CellHitPatternLeft;

      //...Store it...
      h->state(state);
    }
  }

  vector<const TWireHit*>
  TCDC::axialHits(void) const
  {
    vector<const TWireHit*> t;
    t.assign(_axialHits.begin(), _axialHits.end());
    return t;

    //     if (! mask) return _axialHits;
    //     else if (mask == CellHitFindingValid) return _axialHits;
    //     cout << "TCDC::axialHits !!! unsupported mask given" << endl;
    //  return _axialHits;
  }

  vector<const TWireHit*>
  TCDC::stereoHits(void) const
  {
    vector<const TWireHit*> t;
    t.assign(_stereoHits.begin(), _stereoHits.end());
    return t;

    //     if (! mask) return _stereoHits;
    //     else if (mask == CellHitFindingValid) return _stereoHits;
    //     cout << "TCDC::stereoHits !!! unsupported mask given" << endl;
    //     return _stereoHits;
  }

  vector<const TWireHit*>
  TCDC::hits(void) const
  {
    vector<const TWireHit*> t;
    t.assign(_hits.begin(), _hits.end());
    return t;

    //     if (! mask) return _hits;
    //     else if (mask == CellHitFindingValid) return _hits;
    //     cout << "TCDC::hits !!! unsupported mask given" << endl;
    //     return _hits;
  }

// vector<const TWireHit *>
// TCDC::badHits(void) const {
//     vector<const TWireHit *> t;
//     t.assign(_badHits.begin(), _badHits.end());
//     return t;

// //cnv     if (! updated()) update();
// //     if (_badHits.length()) return _badHits;

// //     //...Loop over RECCDC_WIRHIT bank...
// //    x unsigned nReccdc = BsCouTab(RECCDC_WIRHIT);
// //     for (unsigned i = 0; i < nReccdc; i++) {
// //         x struct reccdc_wirhit * h =
// //             (struct reccdc_wirhit *)
// //             BsGetEnt(RECCDC_WIRHIT, i + 1, BBS_No_Index);

// //         //...Check validity...
// //         if (h->m_stat & CellHitFindingValid) continue;

// //         //...Obtain a pointer to GEOCDC...
// //         x geocdc_wire * g =
// //             (geocdc_wire *) BsGetEnt(GEOCDC_WIRE, h->m_geo, BBS_No_Index);

// //         //...Get a pointer to a TCDCWire...
// //         TWire * w = _wires[g->m_ID - 1];

// //         //...Create TWireHit...
// //         _badHits.append(new TWireHit(w, h, _fudgeFactor));
// //     }

// //     return _badHits;
// }

  vector<const TWireHitMC*>
  TCDC::hitsMC(void) const
  {
    vector<const TWireHitMC*> t;
    t.assign(_hitsMC.begin(), _hitsMC.end());
    return t;
  }

  string
  TCDC::wireName(unsigned wireId) const
  {
    string as = "-";
    const TWire* const w = wire(wireId);
    if (w) {
      if (w->stereo())
        as = "=";
    } else {
      return "invalid_wire(" + TDebugUtilities::itostring(wireId) + ")";
    }
    return TDebugUtilities::itostring(layerId(wireId)) + as + TDebugUtilities::itostring(localId(wireId));
  }

  unsigned
  TCDC::localId(unsigned id) const
  {
    cout << "TCDC::localId !!! this function is not tested yet"
         << endl;
    unsigned iLayer = 0;
    unsigned nW = 0;
    unsigned nWLast = 0;
    bool nextLayer = true;
    while (nextLayer) {
      nWLast = nW;
      nW += layer(iLayer++)->nCells();
      if (id < (nW - 1))
        return id - nWLast;
      if (nW >= nWires())
        nextLayer = false;
    }
    cout << "TCDC::localId !!! no such a wire (id=" << id << endl;
    return TCDC_UNDEFINED;
  }

  unsigned
  TCDC::layerId(unsigned id) const
  {
    cout << "TCDC::layerId !!! this function is not tested yet"
         << endl;
    unsigned iLayer = 0;
    unsigned nW = 0;
    bool nextLayer = true;
    while (nextLayer) {
      nW += layer(iLayer++)->nCells();
      if (id < (nW - 1))
        return iLayer - 1;
      if (nW >= nWires())
        nextLayer = false;
    }
    cout << "TCDC::layerId !!! no such a wire (id=" << id << endl;
    return TCDC_UNDEFINED;
  }

  unsigned
  TCDC::layerId(unsigned , unsigned) const
  {
    cout << "TCDC::layerId !!! this function is not implemented yet"
         << endl;
    return TCDC_UNDEFINED;
  }

  unsigned
  TCDC::superLayerId(unsigned id) const
  {
    unsigned iLayer = 0;
    unsigned nW = 0;
    bool nextLayer = true;
    while (nextLayer) {
      const vector<TLayer*>& sl = * superLayer(iLayer);
      const unsigned nLayers = sl.size();
      for (unsigned i = 0; i < nLayers; i++)
        nW += sl[i]->nCells();

      if (id < (nW - 1))
        return iLayer;
      if (nW >= nWires())
        nextLayer = false;
    }
    cout << "TCDC::superLayerId !!! no such a wire (id=" << id
         << endl;
    return TCDC_UNDEFINED;
  }

  unsigned
  TCDC::localLayerId(unsigned id) const
  {
    unsigned iLayer = 0;
    unsigned nW = 0;
    bool nextLayer = true;
    while (nextLayer) {
      const vector<TLayer*>& sl = * superLayer(iLayer);
      const unsigned nLayers = sl.size();
      for (unsigned i = 0; i < nLayers; i++) {
        nW += sl[i]->nCells();
        if (id < (nW - 1))
          return i;
      }

      if (nW >= nWires())
        nextLayer = false;
    }
    cout << "TCDC::localLayerId !!! no such a wire (id=" << id
         << endl;
    return TCDC_UNDEFINED;
  }

  unsigned
  TCDC::axialStereoSuperLayerId(unsigned aors, unsigned i) const
  {
    unsigned is = 99;
    //    cout << "aors,i= " << aors <<" "<< i << std::endl;
    if (aors == 0) {       //axial
      if (i <=  7) {
        is = 0;
      } else if (i <= 13) {
        is = 1;
      } else if (i <= 19) {
        is = 2;
      } else if (i <= 25) {
        is = 3;
      } else if (i <= 31) {
        is = 4;
      }

    } else if (aors == 1) { //stereo
      if (i <=  5) {
        is = 0;
      } else if (i <= 11) {
        is = 1;
      } else if (i <= 17) {
        is = 2;
      } else if (i <= 23) {
        is = 3;
      }
    }

    assert(is != 99);
    return is;
  }

// void
// TCDC::driftDistance(TLink & l,
//                     const TTrack & t,
//                     unsigned flag,
//                     float t0Offset) {

//     //...No correction...
//     if (flag == 0) {
//         if (l.hit()) {
// //             l.drift(0, l.hit()->drift(0));
// //             l.drift(1, l.hit()->drift(1));
// //             l.dDrift(0, l.hit()->dDrift(0));
// //             l.dDrift(1, l.hit()->dDrift(1));
//             l.drift(l.hit()->drift(0), 0);
//             l.drift(l.hit()->drift(1), 1);
//             l.dDrift(l.hit()->dDrift(0), 0);
//             l.dDrift(l.hit()->dDrift(1), 1);
//         }
//         else {
// //             l.drift(0, 0.);
// //             l.drift(1, 0.);
// //             l.dDrift(0, 0.);
// //             l.dDrift(1, 0.);
//             l.drift(0., 0);
//             l.drift(0., 1);
//             l.dDrift(0., 0);
//             l.dDrift(0., 1);
//         }

//         return;
//     }

//     //...TOF correction...
//     float tof = 0.;
//     if (flag && 1) {
//         int imass = 3;
//         float tl = t.helix().a()[4];
//         float f = sqrt(1. + tl * tl);
//         float s = fabs(t.helix().curv()) * fabs(l.dPhi()) * f;
//         float p = f / fabs(t.helix().a()[2]);
//         calcdc_tof2_(& imass, & p, & s, & tof);
//     }

//     //...T0 correction....
//     if (! (flag && 2)) t0Offset = 0.;

//     //...Propagation corrections...
//     const TWireHit & h = * l.hit();
//     int wire = h.wire()->id();
//     HepGeom::Vector3D<double> tp = t.helix().momentum(l.dPhi());
//     float p[3] = {tp.x(), tp.y(), tp.z()};
//     const HepGeom::Point3D<double> & onWire = l.positionOnWire();
//     float x[3] = {onWire.x(), onWire.y(), onWire.z()};
// //cnv    float time = h.reccdc()->m_tdc + t0Offset - tof;
//     float time = 0;
//     float dist;
//     float edist;
//     int prop = (flag & 4);

//     //...Calculation with left side...
//     int side = -1;
//     if (side == 0) side = -1;
//     calcdc_driftdist_(& prop,
//                       & wire,
//                       & side,
//                       p,
//                       x,
//                       & time,
//                       & dist,
//                       & edist);
// //     l.drift(0, dist);
// //     l.dDrift(0, edist);
//     l.drift(dist, 0);
//     l.dDrift(edist, 0);

//     //...Calculation with left side...
//     side = 1;
//     calcdc_driftdist_(& prop,
//                       & wire,
//                       & side,
//                       p,
//                       x,
//                       & time,
//                       & dist,
//                       & edist);
// //     l.drift(1, dist);
// //     l.dDrift(1, edist);
//     l.drift(dist, 1);
//     l.dDrift(edist, 1);

//     //...tan(lambda) correction...
//     if (flag && 8) {
//         float tanl = abs(p[2]) / tp.perp();
//         float c;
//         if ((tanl >= 0.0) && (tanl < 0.5))      c = -0.48 * tanl + 1.3;
//         else if ((tanl >= 0.5) && (tanl < 1.0)) c = -0.28 * tanl + 1.2;
//         else if ((tanl >= 1.0) && (tanl < 1.5)) c = -0.16 * tanl + 1.08;
//         else                                    c =  0.84;

// //         l.dDrift(0, l.dDrift(0) * c);
// //         l.dDrift(1, l.dDrift(1) * c);
//         l.dDrift(l.dDrift(0) * c, 0);
//         l.dDrift(l.dDrift(1) * c, 1);
//     }
// }

  TCDC::~TCDC()
  {
    clear();

    delete [] _width;
    delete [] _r;
    delete [] _r2;
  }

  bool
  TCDC::neighbor(const TWire& w0, const TWire& w1) const
  {
    const int lyr0 = w0.layerId();
    const int lyr1 = w1.layerId();
    const int lyr = lyr0 - lyr1;

    if (abs(lyr) > 1) return false;
    if (w0.superLayerId() != w1.superLayerId()) return false;

    for (unsigned i = 0; i < 7; i++) {
      if (w0.neighbor(i)) {
        if (w0.neighbor(i)->id() == w1.id())
          return true;
      }
    }
    return false;
  }

  void
  TCDC::configure(void)
  {
  }

} // namespace Belle2
