//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : Segment.cc
// Section  : TRG CDC
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to represent a wire in CDC.
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#define TRG_SHORT_NAMES
#define TRGCDC_SHORT_NAMES

#include <iostream>
#include "trg/trg/Utilities.h"
#include "trg/trg/Debug.h"
#include "trg/trg/SignalVector.h"
#include "trg/trg/State.h"
#include "trg/cdc/TRGCDC.h"
#include "trg/cdc/Wire.h"
#include "trg/cdc/WireHit.h"
#include "trg/cdc/Segment.h"
#include "trg/cdc/SegmentHit.h"
#include "trg/cdc/LUT.h"
#include "cdc/geometry/CDCGeometryPar.h"

#include <framework/datastore/StoreArray.h>
#include <cdc/dataobjects/CDCHit.h>
#include <trg/cdc/dataobjects/CDCTriggerSegmentHit.h>
#include <mdst/dataobjects/MCParticle.h>

#include <bitset>

using namespace std;

#define P3D HepGeom::Point3D<double>

namespace Belle2 {

  TRGCDCSegment::TRGCDCSegment(unsigned id,
                               const TCLayer& layer,
                               const TCWire& w,
                               const TRGClock& clock,
                               const std::string& TSLUTFile,
                               const std::vector<const TCWire*>& cells)
    : TCCell(id,
             layer.size(),
             layer,
             w.forwardPosition(),
             w.backwardPosition()),
      _wires(cells),
      _signal(std::string("TS_") + TRGUtil::itostring(id), clock),
      _storeHits{},
      m_TSLUTFileName(TSLUTFile)
  {
  }


  TRGCDCSegment::~TRGCDCSegment()
  {
  }

  void
  TRGCDCSegment::initialize()
  {
    m_TSLUT = TRGCDCLUT::getLUT(m_TSLUTFileName, _wires.size() + 1);
  }

  void
  TRGCDCSegment::dump(const string& msg,
                      const string& pre) const
  {
    cout << pre << name() << " (ptn=" << hitPattern() << ")" << endl;
    if ((msg.find("geometry") != string::npos) ||
        (msg.find("detail") != string::npos)) {
      cout << pre << "id " << id();
      cout << ",local " << localId();
      cout << ",layer " << layerId();
      cout << ",super layer " << superLayerId();
      cout << ",local layer " << localLayerId();
      cout << endl;
    }
    if ((msg.find("hit") != string::npos) ||
        (msg.find("detail") != string::npos)) {
      cout << pre << "Wires ";
      for (unsigned i = 0; i < _wires.size(); i++) {
        cout << _wires[i]->name();
        if (i < _wires.size() - 1)
          cout << ",";
        else
          cout << endl;
      }
      if (_hits.size() == 0) {
        cout << pre << "no wire hit" << endl;
      } else {
        cout << pre << "WHit dump : ";
        for (unsigned i = 0; i < _hits.size(); i++) {
          cout << _hits[i]->cell().name();
          if (i < _hits.size() - 1)
            cout << ",";
          else
            cout << endl;
        }
        for (unsigned i = 0; i < _hits.size(); i++) {
          _hits[i]->dump(msg, pre + "    ");
        }
      }
      if (hit()) {
        cout << pre << "SHit dump" << endl;
        hit()->dump(msg, pre + "    ");
      } else {
        cout << pre << "no TSHit" << endl;
      }
    }
//     if (msg.find("neighbor") != string::npos ||
//         msg.find("detail") != string::npos) {
//         for (unsigned i = 0; i < 7; i++)
//             if (neighbor(i))
//                 neighbor(i)->dump("", pre + TRGCDC::itostring(i) + "   ");
//     }
    if ((msg.find("trigger") != string::npos) ||
        (msg.find("detail") != string::npos)) {
      if (_signal.active())
        _signal.dump(msg, pre + "    ");
      else
        cout << pre << "no trigger signal" << endl;
    }
  }

  void
  TRGCDCSegment::clear(void)
  {
    TCCell::clear();
    _signal.clear();
    _hits.clear();
    _storeHits.clear();
  }

  string
  TRGCDCSegment::name(void) const
  {
    string t;
    if (axial())
      t = "-";
    else
      t = "=";
    string n0 = string("TS") + TRGUtil::itostring(layerId());
    string n1 = TRGUtil::itostring(localId());
    return n0 + t + n1;
  }

  void
  TCSegment::simulate(bool clockSimulation, bool logicLUTFlag,
                      string cdcCollectionName, string tsCollectionName)
  {
    //...Get wire informtion for speed-up...
    unsigned nHits = 0;
    for (unsigned i = 0, n = _wires.size(); i < n; i++) {
      if (_wires[i]->signal().active())
        ++nHits;
    }

    //..No wire hit case...
    if (nHits == 0)
      return;

    if (clockSimulation) {
      simulateWithClock(cdcCollectionName, tsCollectionName);
    } else {
      simulateWithoutClock(logicLUTFlag);
    }
  }

  void
  TCSegment::simulateWithoutClock(bool logicLUTFlag)
  {
    TRGDebug::enterStage("TS sim");

    //...Get wire informtion...
    const unsigned n = _wires.size();
    unsigned nHits = 0;
    vector<TRGSignal> signals;
    for (unsigned i = 0; i < n; i++) {

      //...Store wire hit information...
      const TCWHit* h = _wires[i]->hit();
      if (h)
        _hits.push_back(h);

      //...Copy signal from a wire...
      const TRGSignal& s = _wires[i]->signal();
      signals.push_back(s);

      //...Widen it...
      const unsigned width = signals.back().clock().unit(1000);
      signals.back().widen(width);

      if (s.active())
        ++nHits;
    }

    if (logicLUTFlag == 0) {
      ///// TS Logic Finder
      //...Check number of hit wires...
      //cout<<"TSF: nHits is "<<nHits<<endl;
      if (nHits < 4) {
        TRGDebug::leaveStage("TS sim");
        return;
      }

      //...Signal simulation...
      TRGSignal l0, l1, l2, l3, l4;
      TRGSignal wo1, wo2, wo3, wo4;
      TRGSignal all;
      if (n == 11) {

        //...Simple simulation assuming 3:2:1:2:3 shape...
        l0 = signals[0] | signals[1] | signals[2];
        l1 = signals[3] | signals[4];
        l2 = signals[5];
        l3 = signals[6] | signals[7];
        l4 = signals[8] | signals[9] | signals[10];
        //l0.dump();
        //l1.dump();
        //l2.dump();
        //l3.dump();
        //l4.dump();
        wo1 = l1 & l3 & l4;
        wo2 = l0 & l3 & l4;
        wo3 = l0 & l1 & l4;
        wo4 = l0 & l1 & l3;
        all = l2 & (wo1 | wo2 | wo3 | wo4);

      } else if (n == 15) {

        //...Simple simulation assuming 1:2:3:4:5 shape...
        l0 = signals[0];
        l1 = signals[1] | signals[2];
        l2 = signals[3] | signals[4] | signals[5];
        l3 = signals[6] | signals[7] | signals[8] | signals[9];
        l4 = signals[10] | signals[11] | signals[12] | signals[13] | signals[14];
        wo1 = l2 & l3 & l4;
        wo2 = l1 & l3 & l4;
        wo3 = l1 & l2 & l4;
        wo4 = l1 & l2 & l3;
        all = l0 & (wo1 | wo2 | wo3 | wo4);
      }

      //...Coincidence of all layers...
//      TRGSignal all = l0 & l1 & l2 & l3 & l4;

      if (all.nEdges()) {
        //cout<<"TSF is found"<<endl;
        all.name(name());
        _signal = all;
        //cout<<all.name()<<":#signals="<<all.nSignals()<<endl;;
        //all.dump();
      }
      ///// End of TS logic finder
    }

    if (logicLUTFlag == 1) {
      ///// TS LUT finder
      //... Find hit wires ...
      vector<TRGSignal> hitSignals;
      for (unsigned iWire = 0; iWire < signals.size(); iWire++) {
        if (signals[iWire].active()) hitSignals.push_back(signals[iWire]);
      }
      //... Coincidence all hit wires ...
      TRGSignal allSignals;
      if (hitSignals.size() != 0) {
        allSignals = hitSignals[0];
        for (unsigned iHitWire = 1; iHitWire < hitSignals.size(); iHitWire++) {
          allSignals = allSignals & hitSignals[iHitWire];
        }
      }

      int lutValue = LUT()->getValue(lutPattern());
      if ((lutValue != 0) && (priority().signal().active() != 0)) {
        allSignals.name(name());
        _signal = allSignals;
      }
      ///// End of TS LUT finder
    }

    TRGDebug::leaveStage("TS sim");
  }

  void
  TCSegment::simulateWithClock(string cdcCollectionName, string tsCollectionName)
  {
    // check LUT pattern without clock -> if there is no hit, skip clock simulation
    if (m_TSLUT->getValue(lutPattern()) == 0) return;

    TRGDebug::enterStage("TS sim with clock");

    StoreArray<CDCHit> cdcHits(cdcCollectionName);
    StoreArray<CDCTriggerSegmentHit> segmentHits(tsCollectionName);

    // get data clock of first and last hit
    const TRGClock& wireClock = _wires[0]->signal().clock();
    int clkMin = 1000;
    int clkMax = -1000;
    for (unsigned i = 0, n = _wires.size(); i < n; ++i) {
      const TRGSignal& s = _wires[i]->signal();
      if (s.active()) {
        int clk0 = s[0]->time();
        int clk1 = s[s.nEdges() - 2]->time();
        if (clk0 < clkMin) clkMin = clk0;
        if (clk1 > clkMax) clkMax = clk1;
      }
    }
    // loop over data clock cycles
    //const int step = wireClock.frequency() / TRGCDC::getTRGCDC()->dataClock().frequency();
    const int step = wireClock.frequency() / signal().clock().frequency();
    const int width = 16 * step;
    clkMin -= clkMin % step;
    clkMax -= clkMax % step;
    int lastLutValue = 0;
    int lastPriority = 0;
    int lastFastest = 0;
    for (int iclk = clkMin; iclk <= clkMax; iclk += step) {
      // check pattern in the last width clock cycles
      unsigned pattern = lutPattern(iclk - width, iclk + step);
      int lutValue = m_TSLUT->getValue(pattern);
      if (lutValue) {
        int priorityPos = priorityPosition(iclk - width, iclk + step);
        int fastest = fastestTime(iclk - width);
        // make a new hit if L/R changes to known, if priority changes to first
        // or if the fastest hit changes
        if ((lastLutValue == 3 && lutValue != 3) ||
            (lastPriority != 3 && priorityPos == 3) ||
            fastest != lastFastest) {
          // add new edge to signal
          TRGTime rise = TRGTime(wireClock.absoluteTime(iclk), true, _signal.clock());
          TRGTime fall = rise;
          fall.shift(1).reverse();
          _signal |= TRGSignal(rise & fall);
          // get priority wire from position flag
          int ipr = (priorityPos == 3) ? 0 : priorityPos;
          const TRGCDCWire* priorityWire = (_wires.size() == 15) ? _wires[ipr] : _wires[ipr + 5];
          // get priority time (first hit on priority wire in time window)
          int tdc = priorityWire->signal()[0]->time();
          if (tdc < iclk - width) {
            for (unsigned itdc = 2, edges = priorityWire->signal().nEdges(); itdc < edges; itdc += 2) {
              tdc = priorityWire->signal()[itdc]->time();
              if (tdc >= iclk - width) break;
            }
          }
          // create hit
          const CDCHit* priorityHit = cdcHits[priorityWire->hit()->iCDCHit()];
          const CDCTriggerSegmentHit* storeHit =
            segmentHits.appendNew(*priorityHit,
                                  id(),
                                  priorityPos,
                                  lutValue,
                                  tdc,
                                  fastest,
                                  iclk + step);
          addStoreHit(storeHit);
          // relation to all CDCHits in segment
          for (unsigned iw = 0; iw < _wires.size(); ++iw) {
            if (_wires[iw]->signal().active(iclk - width, iclk + step)) {
              // priority wire has relation weight 2
              double weight = (_wires[iw] == priorityWire) ? 2. : 1.;
              storeHit->addRelationTo(cdcHits[_wires[iw]->hit()->iCDCHit()], weight);
            }
          }
          // relation to MCParticles (same as priority hit)
          RelationVector<MCParticle> mcrel = priorityHit->getRelationsFrom<MCParticle>();
          for (unsigned imc = 0; imc < mcrel.size(); ++imc) {
            mcrel[imc]->addRelationTo(storeHit, mcrel.weight(imc));
          }
          // store values of this hit to compare with the next hit
          lastLutValue = lutValue;
          lastPriority = priorityPos;
          lastFastest = fastest;
        }
      }
    }

    TRGDebug::leaveStage("TS sim with clock");
  }

  float
  TCSegment::fastestTime() const
  {
    if ((LUT()->getValue(lutPattern()))) {
      float tmpFastTime = 9999;
      for (unsigned i = 0; i < _wires.size(); i++) {
        if (_wires[i]->signal().active()) {
          float dt = _wires[i]->signal()[0]->time();
          if (dt < tmpFastTime) {
            tmpFastTime = dt;
          }
        }
      }
      return tmpFastTime;
    } else
      return -1;
  }

  float
  TCSegment::fastestTime(int clk0) const
  {
    int fastest = 9999;
    for (unsigned iw = 0; iw < _wires.size(); ++iw) {
      if (_wires[iw]->signal().active()) {
        for (unsigned itdc = 0, edges = _wires[iw]->signal().nEdges(); itdc < edges; itdc += 2) {
          float dt = _wires[iw]->signal()[itdc]->time();
          if (dt >= clk0) {
            if (dt < fastest) fastest = dt;
            break;
          }
        }
      }
    }
    return fastest;
  }

  float
  TCSegment::foundTime() const
  {
    if ((LUT()->getValue(lutPattern()))) {
      float tmpFoundTime[5] = {9999, 9999, 9999, 9999, 9999};
      for (unsigned i = 0; i < _wires.size(); i++) {
        if (!_wires[i]->signal().active()) continue;
        float dt = _wires[i]->signal()[0]->time();
        if (_wires.size() == 11) {
          if (i < 3) {
            if (tmpFoundTime[0] > dt) tmpFoundTime[0] = dt;
          } else if (i < 5) {
            if (tmpFoundTime[1] > dt) tmpFoundTime[1] = dt;
          } else if (i == 5) {
            if (tmpFoundTime[2] > dt) tmpFoundTime[2] = dt;
          } else if (i < 8) {
            if (tmpFoundTime[3] > dt) tmpFoundTime[3] = dt;
          } else {
            if (tmpFoundTime[4] > dt) tmpFoundTime[4] = dt;
          }
        } else {
          if (i == 0) {
            if (tmpFoundTime[0] > dt) tmpFoundTime[0] = dt;
          } else if (i < 3) {
            if (tmpFoundTime[1] > dt) tmpFoundTime[1] = dt;
          } else if (i < 6) {
            if (tmpFoundTime[2] > dt) tmpFoundTime[2] = dt;
          } else if (i < 10) {
            if (tmpFoundTime[3] > dt) tmpFoundTime[3] = dt;
          } else {
            if (tmpFoundTime[4] > dt) tmpFoundTime[4] = dt;
          }
        }
      }
      sort(tmpFoundTime, tmpFoundTime + 5);
      return tmpFoundTime[3];
    } else
      return -1;
  }

  float
  TCSegment::priorityTime() const
  {
    const TRGSignal& prioritySignal = priority().signal();
    if (prioritySignal.active()) {
      return prioritySignal[0]->time();
    }
    return -1;
  }

  int
  TCSegment::priorityPosition() const
  {
    if (center().signal().active()) {
      return 3;
    } else {
      const TRGCDCWire* priorityL;
      const TRGCDCWire* priorityR;
      if (_wires.size() == 15) {
        priorityL = _wires[2];
        priorityR = _wires[1];
      } else {
        priorityL = _wires[7];
        priorityR = _wires[6];
      }
      if (priorityL->signal().active()) {
        if (priorityR->signal().active()) {
          if ((priorityL->signal()[0]->time()) >= (priorityR->signal()[0]->time())) return 1;
          else return 2;
        } else return 2;
      } else if (priorityR->signal().active()) {
        return 1;
      } else return 0;
    }
  }

  int
  TCSegment::priorityPosition(int clk0, int clk1) const
  {
    if (center().signal().active(clk0, clk1)) {
      return 3;
    } else {
      const TRGCDCWire* priorityL;
      const TRGCDCWire* priorityR;
      if (_wires.size() == 15) {
        priorityL = _wires[2];
        priorityR = _wires[1];
      } else {
        priorityL = _wires[7];
        priorityR = _wires[6];
      }
      if (priorityL->signal().active(clk0, clk1)) {
        if (priorityR->signal().active(clk0, clk1)) {
          if ((priorityL->signal()[0]->time()) >= (priorityR->signal()[0]->time())) return 1;
          else return 2;
        } else return 2;
      } else if (priorityR->signal().active(clk0, clk1)) {
        return 1;
      } else return 0;
    }
  }

  const TRGCDCWire&
  TCSegment::priority() const
  {
    int priority = priorityPosition();
    int offset = (_wires.size() == 15) ? 0 : 5;
    if (priority == 1 || priority == 2)
      return *_wires[offset + priority];
    return *_wires[offset];
  }

  unsigned
  TRGCDCSegment::hitPattern() const
  {
    unsigned ptn = 0;
    for (unsigned i = 0; i < _wires.size(); i++) {
      const TRGSignal& s = _wires[i]->signal();
      if (s.active())
        ptn |= (1 << i);
    }
    return ptn;
  }

  unsigned
  TRGCDCSegment::hitPattern(int clk0, int clk1) const
  {
    unsigned ptn = 0;
    for (unsigned i = 0; i < _wires.size(); i++) {
      const TRGSignal& s = _wires[i]->signal();
      if (s.active(clk0, clk1))
        ptn |= (1 << i);
    }
    return ptn;
  }

  unsigned
  TRGCDCSegment::lutPattern() const
  {
    unsigned outValue = (hitPattern()) * 2;
    if (priorityPosition() == 2) {
      outValue += 1;
    }
    return outValue;
  }

  unsigned
  TRGCDCSegment::lutPattern(int clk0, int clk1) const
  {
    unsigned outValue = (hitPattern(clk0, clk1)) * 2;
    if (priorityPosition(clk0, clk1) == 2) {
      outValue += 1;
    }
    return outValue;
  }

  bool
  TRGCDCSegment::hasMember(const std::string& a) const
  {
    const unsigned n = _wires.size();
    for (unsigned i = 0; i < n; i++) {
      if (_wires[i]->hasMember(a))
        return true;
    }
    return false;
  }

} // namespace Belle2
