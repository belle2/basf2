/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

//-----------------------------------------------------------------------------
// Description : A class to represent a wire in CDC.
//-----------------------------------------------------------------------------

#define TRG_SHORT_NAMES
#define TRGCDC_SHORT_NAMES

#include <iostream>
#include "trg/trg/Utilities.h"
#include "trg/trg/Debug.h"
#include "trg/cdc/Wire.h"
#include "trg/cdc/WireHit.h"
#include "trg/cdc/Segment.h"
#include "trg/cdc/SegmentHit.h"
#include "trg/cdc/LUT.h"

#include <framework/datastore/StoreArray.h>
#include <cdc/dataobjects/CDCHit.h>
#include <trg/cdc/dataobjects/CDCTriggerSegmentHit.h>
#include <mdst/dataobjects/MCParticle.h>

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
      m_wires(cells),
      m_signal(std::string("TS_") + TRGUtil::itostring(id), clock),
      m_storeHits{},
      m_TSLUTFileName(TSLUTFile)
  {
  }


  TRGCDCSegment::~TRGCDCSegment()
  {
  }

  void
  TRGCDCSegment::initialize()
  {
    m_TSLUT = TRGCDCLUT::getLUT(m_TSLUTFileName, m_wires.size() + 1);
  }

  void
  TRGCDCSegment::dump(const std::string& msg,
                      const std::string& pre) const
  {
    std::cout << pre << name() << " (ptn=" << hitPattern() << ")" << std::endl;
    if ((msg.find("geometry") != std::string::npos) ||
        (msg.find("detail") != std::string::npos)) {
      std::cout << pre << "id " << id();
      std::cout << ",local " << localId();
      std::cout << ",layer " << layerId();
      std::cout << ",super layer " << superLayerId();
      std::cout << ",local layer " << localLayerId();
      std::cout << std::endl;
    }
    if ((msg.find("hit") != std::string::npos) ||
        (msg.find("detail") != std::string::npos)) {
      std::cout << pre << "Wires ";
      for (unsigned i = 0; i < m_wires.size(); i++) {
        std::cout << m_wires[i]->name();
        if (i < m_wires.size() - 1)
          std::cout << ",";
        else
          std::cout << std::endl;
      }
      if (m_hits.size() == 0) {
        std::cout << pre << "no wire hit" << std::endl;
      } else {
        std::cout << pre << "WHit dump : ";
        for (unsigned i = 0; i < m_hits.size(); i++) {
          std::cout << m_hits[i]->cell().name();
          if (i < m_hits.size() - 1)
            std::cout << ",";
          else
            std::cout << std::endl;
        }
        for (unsigned i = 0; i < m_hits.size(); i++) {
          m_hits[i]->dump(msg, pre + "    ");
        }
      }
      if (hit()) {
        std::cout << pre << "SHit dump" << std::endl;
        hit()->dump(msg, pre + "    ");
      } else {
        std::cout << pre << "no TSHit" << std::endl;
      }
    }
//     if (msg.find("neighbor") != std::string::npos ||
//         msg.find("detail") != std::string::npos) {
//         for (unsigned i = 0; i < 7; i++)
//             if (neighbor(i))
//                 neighbor(i)->dump("", pre + TRGCDC::itostring(i) + "   ");
//     }
    if ((msg.find("trigger") != std::string::npos) ||
        (msg.find("detail") != std::string::npos)) {
      if (m_signal.active())
        m_signal.dump(msg, pre + "    ");
      else
        std::cout << pre << "no trigger signal" << std::endl;
    }
  }

  void
  TRGCDCSegment::clear(void)
  {
    TCCell::clear();
    m_signal.clear();
    m_hits.clear();
    m_storeHits.clear();
  }

  std::string
  TRGCDCSegment::name(void) const
  {
    std::string t;
    if (axial())
      t = "-";
    else
      t = "=";
    std::string n0 = std::string("TS") + TRGUtil::itostring(layerId());
    std::string n1 = TRGUtil::itostring(localId());
    return n0 + t + n1;
  }

  void
  TRGCDCSegment::simulate(bool clockSimulation, bool logicLUTFlag,
                          const std::string& cdcCollectionName, const std::string& tsCollectionName)
  {
    //...Get wire information for speed-up...
    unsigned nHits = 0;
    for (unsigned i = 0, n = m_wires.size(); i < n; i++) {
      if (m_wires[i]->signal().active())
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
  TRGCDCSegment::simulateWithoutClock(bool logicLUTFlag)
  {
    TRGDebug::enterStage("TS sim");

    //...Get wire information...
    const unsigned n = m_wires.size();
    unsigned nHits = 0;
    std::vector<TRGSignal> signals;
    signals.reserve(n);
    m_hits.reserve(n);
    for (unsigned i = 0; i < n; i++) {

      //...Store wire hit information...
      const TCWHit* h = m_wires[i]->hit();
      if (h)
        m_hits.push_back(h);

      //...Copy signal from a wire...
      const TRGSignal& s = m_wires[i]->signal();
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
      //std::cout<<"TSF: nHits is "<<nHits<<std::endl;
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
        //std::cout<<"TSF is found"<<std::endl;
        all.name(name());
        m_signal = all;
        //std::cout<<all.name()<<":#signals="<<all.nSignals()<<std::endl;;
        //all.dump();
      }
      ///// End of TS logic finder
    }

    if (logicLUTFlag == 1) {
      ///// TS LUT finder
      //... Find hit wires ...
      std::vector<TRGSignal> hitSignals;
      hitSignals.reserve(signals.size());
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
        m_signal = allSignals;
      }
      ///// End of TS LUT finder
    }

    TRGDebug::leaveStage("TS sim");
  }

  void
  TRGCDCSegment::simulateWithClock(std::string cdcCollectionName, std::string tsCollectionName)
  {
    // check LUT pattern without clock -> if there is no hit, skip clock simulation
    if (m_TSLUT->getValue(lutPattern()) == 0) return;

    TRGDebug::enterStage("TS sim with clock");

    StoreArray<CDCHit> cdcHits(cdcCollectionName);
    StoreArray<CDCTriggerSegmentHit> segmentHits(tsCollectionName);
    m_storeHits.reserve(segmentHits.getEntries());

    // get data clock of first and last hit
    const TRGClock& wireClock = m_wires[0]->signal().clock();
    int clkMin = 1000;
    int clkMax = -1000;
    for (unsigned i = 0, n = m_wires.size(); i < n; ++i) {
      const TRGSignal& s = m_wires[i]->signal();
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
          TRGTime rise = TRGTime(wireClock.absoluteTime(iclk), true, m_signal.clock());
          TRGTime fall = rise;
          fall.shift(1).reverse();
          m_signal |= TRGSignal(rise & fall);
          // get priority wire from position flag
          int ipr = (priorityPos == 3) ? 0 : priorityPos;
          const TRGCDCWire* priorityWire = (m_wires.size() == 15) ? m_wires[ipr] : m_wires[ipr + 5];
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
          for (unsigned iw = 0; iw < m_wires.size(); ++iw) {
            if (m_wires[iw]->signal().active(iclk - width, iclk + step)) {
              // priority wire has relation weight 2
              double weight = (m_wires[iw] == priorityWire) ? 2. : 1.;
              storeHit->addRelationTo(cdcHits[m_wires[iw]->hit()->iCDCHit()], weight);
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
  TRGCDCSegment::fastestTime() const
  {
    if ((LUT()->getValue(lutPattern()))) {
      float tmpFastTime = 9999;
      for (unsigned i = 0; i < m_wires.size(); i++) {
        if (m_wires[i]->signal().active()) {
          float dt = m_wires[i]->signal()[0]->time();
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
  TRGCDCSegment::fastestTime(int clk0) const
  {
    int fastest = 9999;
    for (unsigned iw = 0; iw < m_wires.size(); ++iw) {
      if (m_wires[iw]->signal().active()) {
        for (unsigned itdc = 0, edges = m_wires[iw]->signal().nEdges(); itdc < edges; itdc += 2) {
          float dt = m_wires[iw]->signal()[itdc]->time();
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
  TRGCDCSegment::foundTime() const
  {
    if ((LUT()->getValue(lutPattern()))) {
      float tmpFoundTime[5] = {9999, 9999, 9999, 9999, 9999};
      for (unsigned i = 0; i < m_wires.size(); i++) {
        if (!m_wires[i]->signal().active()) continue;
        float dt = m_wires[i]->signal()[0]->time();
        if (m_wires.size() == 11) {
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
      std::sort(tmpFoundTime, tmpFoundTime + 5);
      return tmpFoundTime[3];
    } else
      return -1;
  }

  float
  TRGCDCSegment::priorityTime() const
  {
    const TRGSignal& prioritySignal = priority().signal();
    if (prioritySignal.active()) {
      return prioritySignal[0]->time();
    }
    return -1;
  }

  int
  TRGCDCSegment::priorityPosition() const
  {
    if (center().signal().active()) {
      return 3;
    } else {
      const TRGCDCWire* priorityL;
      const TRGCDCWire* priorityR;
      if (m_wires.size() == 15) {
        priorityL = m_wires[2];
        priorityR = m_wires[1];
      } else {
        priorityL = m_wires[7];
        priorityR = m_wires[6];
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
  TRGCDCSegment::priorityPosition(int clk0, int clk1) const
  {
    if (center().signal().active(clk0, clk1)) {
      return 3;
    } else {
      const TRGCDCWire* priorityL;
      const TRGCDCWire* priorityR;
      if (m_wires.size() == 15) {
        priorityL = m_wires[2];
        priorityR = m_wires[1];
      } else {
        priorityL = m_wires[7];
        priorityR = m_wires[6];
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
  TRGCDCSegment::priority() const
  {
    int priority = priorityPosition();
    int offset = (m_wires.size() == 15) ? 0 : 5;
    if (priority == 1 || priority == 2)
      return *m_wires[offset + priority];
    return *m_wires[offset];
  }

  unsigned
  TRGCDCSegment::hitPattern() const
  {
    unsigned ptn = 0;
    for (unsigned i = 0; i < m_wires.size(); i++) {
      const TRGSignal& s = m_wires[i]->signal();
      if (s.active())
        ptn |= (1 << i);
    }
    return ptn;
  }
  std::vector<float>
  TRGCDCSegment::hitPatternTime() const
  {
    std::vector<float> timeptn;
    timeptn.reserve(m_wires.size());
    for (unsigned i = 0; i < m_wires.size(); i++) {
      const TRGSignal& s = m_wires[i]->signal();
      if (s.active())
        timeptn.push_back(s[0]->time());
      else
        timeptn.push_back(-1);
    }
    return timeptn;
  }

  unsigned
  TRGCDCSegment::hitPattern_adc() const
  {
    unsigned ptn = 0;
    for (unsigned i = 0; i < m_wires.size(); i++) {
      const TRGSignal& s = m_wires[i]->signal_adc();
      if (s.active())
        ptn |= (1 << i);
    }
    return ptn;
  }

  unsigned
  TRGCDCSegment::hitPattern(int clk0, int clk1) const
  {
    unsigned ptn = 0;
    for (unsigned i = 0; i < m_wires.size(); i++) {
      const TRGSignal& s = m_wires[i]->signal();
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
    const unsigned n = m_wires.size();
    for (unsigned i = 0; i < n; i++) {
      if (m_wires[i]->hasMember(a))
        return true;
    }
    return false;
  }

} // namespace Belle2
