/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

//-----------------------------------------------------------------------------
// Description : A class to represent a CDC Tracker2D board
//-----------------------------------------------------------------------------

#define TRGCDC_SHORT_NAMES

#include <algorithm>
#include <iostream>
#include "trg/trg/Debug.h"
#include "trg/trg/State.h"
#include "trg/trg/Channel.h"
#include "trg/cdc/TRGCDC.h"
#include "trg/cdc/Wire.h"
#include "trg/cdc/Segment.h"
#include "trg/cdc/Tracker2D.h"
#include "trg/cdc/TrackSegmentFinder.h"

using namespace std;

namespace Belle2 {

  unsigned TRGCDCTracker2D::_nTSF = 0;
  vector<unsigned> TRGCDCTracker2D::_n;
  TRGState TRGCDCTracker2D::_ts(160 + 192 + 256 + 320 + 384);

  TRGCDCTracker2D::TRGCDCTracker2D(const std::string& name,
                                   const TRGClock& systemClock,
                                   const TRGClock& dataClock,
                                   const TRGClock& userClockInput,
                                   const TRGClock& userClockOutput)
    : TRGBoard(name, systemClock, dataClock, userClockInput, userClockOutput)
  {
  }

  TRGCDCTracker2D::~TRGCDCTracker2D()
  {
  }

  string
  TRGCDCTracker2D::version(void)
  {
    return ("TRGCDCTracker2D version 0.00");
  }

  void
  TRGCDCTracker2D::push_back(const TRGCDCTrackSegmentFinder* a)
  {
    std::vector<const TRGCDCTrackSegmentFinder*>::push_back(a);
  }

  void
  TRGCDCTracker2D::dump(const string& message, const string& pre) const
  {
    TRGBoard::dump(message, pre);
  }

  void
  TRGCDCTracker2D::simulate(void)
  {

    const string sn = "TRGCDC 2D simulate : " + name();
    TRGDebug::enterStage(sn);

    static bool first = true;
    if (first) {
      setConstants();
      first = false;
    }

    //...Delete old objects...
    if (nOutput())
      for (unsigned i = 0; i < nOutput(); i++)
        if (output(i)->signal())
          delete output(i)->signal();

    //...Create a combined input signal bundle from 5 TSF boards...
    TRGSignalBundle isb = TRGSignalBundle(name() +
                                          "-InputSignalBundle",
                                          clockData());
    for (unsigned i = 0; i < 5; i++) {
      TRGSignalBundle* b = input(i)->signal();
      if (! b)
        continue;
      for (unsigned j = 0; j < b->size(); j++) {
        isb.push_back((* b)[j]);
      }
    }

    if (TRGDebug::level())
      isb.dump("", TRGDebug::tab());

    //...Output signal bundle...
    const string no = name() + "OutputSignalBundle";
    TRGSignalBundle* osb = new TRGSignalBundle(no,
                                               clockData(),
                                               isb,
                                               256,
                                               _nTSF / 2 * 16,
                                               TCTracker2D::packer);
    output(0)->signal(osb);

    TRGDebug::leaveStage(sn);
  }

  TRGState
  TCTracker2D::packer(const TRGState& input,
                      TRGState& registers,
                      bool& logicStillActive)
  {

    //...Registers...
    //   Storing TSF hit history upto 16 clocks. #TSF is half of all TSF.
    //   reg[15 downto 0]      : TSF-0 history (SL0)
    //   reg[31 downto 16]     : TSF-1 history (SL0)
    //   ...
    //   reg[1279 downto 1264] : TSF-59 history (SL0)
    //   reg[1295 downto 1280] : TSF-? history (SL1)
    //   ...

    //...Shift registers (TSF hit history pipe)...
    for (unsigned i = 0; i < nTSF() / 2; i++) {
      TRGState s = registers.subset(i * 16, 16);
      s.shift(1);
      registers.set(i * 16, s);
    }

    //...Unpack input state...
    //   Get TSF hit information. The drift time information is ignored.
    unpacker(input, registers);

    //...Make TSF hit...
    hitInformation(registers);

    //...Do core logic...
//  HoughMappingPlus();
//  HoughMappingMinus();

    //...Make output...
    logicStillActive = registers.active();

    return TRGState(256);
  }

  void
  TCTracker2D:: unpacker(const TRGState& input,
                         TRGState& output)
  {

    const string sn = "TRGCDC 2D unpacker";
    TRGDebug::enterStage(sn);

    //...Constants...
//  const unsigned noHit = 0b11111111;
    const unsigned sizeSB = 420; // bits
    const unsigned sizeTS = 21;  // bits
    const unsigned sizeID = 8;   // bits
    const unsigned posID  = 13;  // bit position

    //...Store hit TSF id...
    unsigned tsfId[5][20]; // Max. 20 TSF ID is sent
    bool hitFound = false;
    for (unsigned i = 0; i < 5; i++) {
      for (unsigned j = 0; j < 20; j++) {
        unsigned jr = 20 - j - 1;
        tsfId[i][jr] = input.subset(i * sizeSB + sizeTS * jr + posID,
                                    sizeID);
        if (tsfId[i][jr] != 0) {
          output.set(tsfId[i][jr] * 16, true);
          hitFound = true;
        }
      }
    }

    if (TRGDebug::level() && hitFound) {
      input.dump("", TRGDebug::tab() + "input bits:");
      cout << TRGDebug::tab() << "TSF hit ID" << endl;
      for (unsigned i = 0; i < 5; i++) {
        cout << TRGDebug::tab() << "    ASL" << i
             << " < " << nTSF(i) / 2 << endl;
        cout << TRGDebug::tab() << "        ";
        for (unsigned j = 0; j < 20; j++) {
          cout << tsfId[i][j] << ",";
        }
        cout << endl;
      }
    }

    TRGDebug::leaveStage(sn);
  }

  void
  TCTracker2D::hitInformation(const TRGState& registers)
  {

    //...Clear info...
    _ts.clear();

    //...Set TSF hit information...
    for (unsigned i = 0; i < nTSF() / 2; i++) {
      bool active = registers.subset(i * 16, 16).active();
      if (active)
        _ts.set(i, true);
    }
  }

  void
  TCTracker2D::setConstants(void)
  {

    //...# of TSFs...
    const TRGCDC& cdc = * TRGCDC::getTRGCDC();
    _nTSF = 0;
    for (unsigned i = 0; i < 5; i++) {       // Ax only
      const unsigned n = cdc.nSegments(i * 2);
      _n.push_back(n);
      _nTSF += n;
    }

    //...Consistency check...
    if (_nTSF != nTSF()) {
      cout << "TRGCDCTracker2D !!! # of TSF is inconsistent internally"
           << endl;
      cout << "                      _nTSF,nTSF()=" << _nTSF << "," << nTSF()
           << endl;
    }
    for (unsigned i = 0; i < 5; i++) {
      if (_n[i] != nTSF(i)) {
        cout << "TRGCDCTracker2D !!! # of TSF is inconsistent internally"
             << endl;
        cout << "                      i,_n[i],nTSF(i)=" << i << ","
             << _n[i] << "," << nTSF(i) << endl;
      }

    }
    if (_nTSF != _ts.size()) {
      cout << "TRGCDCTracker2D !!! # of TSF is inconsistent internally"
           << endl;
      cout << "                    _nTSF,_ts.size()=" << _nTSF << ","
           << _ts.size() << endl;
    }
  }

} // namespace Belle2
