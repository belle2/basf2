/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : TRGCDCTSStreamModule.cc
// Section  : TRG CDC
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A trigger module for CDC
//-----------------------------------------------------------------------------
// 0.00 : 2012/02/02 : First version
//-----------------------------------------------------------------------------

#define TRGCDC_SHORT_NAMES

#include <iostream>
#include <fstream>

#include "trg/cdc/modules/tsstream/TRGCDCTSStreamModule.h"
#include "trg/trg/Debug.h"
#include "trg/trg/BitStream.h"
#include "trg/cdc/Wire.h"
#include "trg/cdc/Segment.h"
#include "trg/cdc/Layer.h"

using namespace std;

namespace Belle2 {

  /// register the basf2 TRGCDCTSStream module
  REG_MODULE(TRGCDCTSStream);

  string
  TRGCDCTSStreamModule::version() const
  {
    return string("TRGCDCTSStreamModule 0.00");
  }

  TRGCDCTSStreamModule::TRGCDCTSStreamModule()
    : Module::Module(),
      _debugLevel(0),
      _mode(0),
      _streamFilename("unknown"),
      _cdc(0),
      _out(0)
  {

    string desc = "TRGCDCTSStreamModule(" + version() + ")";
    setDescription(desc);

    addParam("DebugLevel",
             _debugLevel,
             "TRGCDCTSStream debug level",
             _debugLevel);

    addParam("Mode", _mode, "TRGCDCTSStream mode", _mode);

    addParam("OutputStreamFile",
             _streamFilename,
             "The filename of bit stream",
             _streamFilename);

    if (TRGDebug::level())
      cout << "TRGCDCTSStreamModule ... created" << endl;
  }

  TRGCDCTSStreamModule::~TRGCDCTSStreamModule()
  {

    if (_cdc)
      TRGCDC::getTRGCDC("good-bye");

    if (TRGDebug::level())
      cout << "TRGCDCTSStreamModule ... destructed " << endl;
  }

  void
  TRGCDCTSStreamModule::initialize()
  {
    if (TRGDebug::level()) {
      cout << "TRGCDCTSStreamModule::initialize ... options" << endl;
      cout << TRGDebug::tab(4) << "debug level = " << _debugLevel << endl;
      cout << TRGDebug::tab(4) << "       mode = " << _mode << endl;
      cout << TRGDebug::tab(4) << "output file = " << _streamFilename << endl;
    }

    if (_streamFilename != "unknown") {
      _out = new ofstream(_streamFilename.c_str(), ios::out | ios::binary);
      unsigned val = TRGBSRecord_Comment;
      _out->write((char*) & val, 4);
      const string cmt = "test data ";
      val = cmt.size() * 8;
      _out->write((char*) & val, 4);
      _out->write(cmt.c_str(), cmt.size());
    }
  }

  void
  TRGCDCTSStreamModule::beginRun()
  {

    _cdc = TRGCDC::getTRGCDC();

    //...Super layer loop...
    for (unsigned l = 0; l < _cdc->nSegmentLayers(); l++) {
      const Belle2::TRGCDCLayer* lyr = _cdc->segmentLayer(l);
      const unsigned nWires = lyr->nCells();

      //...Clear old pointers...
      _wires[l].clear();

      //...TS loop...
      for (unsigned i = 0; i < nWires; i++) {
        const TCSegment& s = (TCSegment&) * (* lyr)[i];
        _wires[l].push_back(s.wires()[5]);
      }
    }

    if (_out) {
      unsigned val = TRGBSRecord_BeginRun;
      _out->write((char*) & val, 4);
      val = 0;
      _out->write((char*) & val, 4);
    }

    if (TRGDebug::level())
      cout << "TRGCDCTSStreamModule ... beginRun called. TRGCDC version="
           << _cdc->version() << endl;
  }

  void
  TRGCDCTSStreamModule::event()
  {

    //...To dump wire hits...
    if (TRGDebug::level())
      _cdc->dump("trgWireCentralHits");

    if (_out) {
      unsigned val = TRGBSRecord_BeginEvent;
      _out->write((char*) & val, 4);
      val = 0;
      _out->write((char*) & val, 4);
    }

    //...Clock loop (from 0 to 99 cycles, about 800 ns)...
    for (unsigned c = 0; c < 100; c++) {

      if (_out) {
        unsigned val = TRGBSRecord_Clock;
        _out->write((char*) & val, 4);
        val = 32;
        _out->write((char*) & val, 4);
        val = c;
        _out->write((char*) & val, 4);
      }

      //...Super layer loop...
      for (unsigned l = 0; l < 9; l++) {

        //...Bit stream for this super layer...
        TRGBitStream stream;

        //...Wire loop...
        for (unsigned i = 0; i < _wires[l].size(); i++) {
          const TRGSignal& s = _wires[l][i]->signal();
          bool hit = s.state(c);
          stream.append(hit);
        }

        if (_out) {
          unsigned val = TRGBSRecord_SegmentSL0;
          val += l;
          _out->write((char*) & val, 4);
          val = stream.size();
          _out->write((char*) & val, 4);
          for (unsigned i = 0; i < stream.sizeInChar(); i++) {
            char cs = stream.c(i);
            _out->write(& cs, 1);
          }
        }

        if (TRGDebug::level()) {
          cout << "Super layer " << l << ", clock " << c << endl;
          stream.dump();
        }
      }
    }

    if (_out) {
      unsigned val = TRGBSRecord_EndEvent;
      _out->write((char*) & val, 4);
      val = 0;
      _out->write((char*) & val, 4);
    }

  }

  void
  TRGCDCTSStreamModule::endRun()
  {
    if (_out) {
      unsigned val = TRGBSRecord_EndRun;
      _out->write((char*) & val, 4);
      val = 0;
      _out->write((char*) & val, 4);
    }

    if (TRGDebug::level())
      cout << "TRGCDCTSStreamModule ... endRun called " << endl;
  }

  void
  TRGCDCTSStreamModule::terminate()
  {
    if (_out) {
      _out->flush();
      _out->close();
    }

    if (TRGDebug::level())
      cout << "TRGCDCTSStreamModule ... terminate called " << endl;
  }

} // namespace Belle2
