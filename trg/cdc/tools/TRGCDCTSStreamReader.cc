/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

//-----------------------------------------------------------------------------
// Description : A program to read Track Segment (TS) stream data
//-----------------------------------------------------------------------------

#define TRG_SHORT_NAMES
#define TRGCDC_SHORT_NAMES

#include <iostream>
#include <fstream>
#include <string>
#include "trg/trg/BitStream.h"
#include "trg/cdc/Wire.h"
#ifdef TRGCDC_DISPLAY
#include "framework/gearbox/Gearbox.h"
#include "trg/cdc/DisplayRphi.h"
#include "trg/cdc/DisplayHough.h"
namespace Belle2_TRGCDC {
  Belle2::TRGCDCDisplayRphi* D = 0;
}
#endif

using namespace std;
using namespace Belle2;
#ifdef TRGCDC_DISPLAY
using namespace Belle2_TRGCDC;
#endif

#define DEBUG_LEVEL     1
#define PROGRAM_NAME    "TRGCDCTSStreamReader"
#define PROGRAM_VERSION "version 0.03"
#define ENV_PATH        "BELLE2_LOCAL_DIR"
#define CONFIG          "TRGCDCWireConfig_0_20101110_0836.dat"

int
main(int argc, char* argv[])
{

  cout << PROGRAM_NAME << " ... " << PROGRAM_VERSION << endl;
  const string tab = "    ";

  //...Check arguments...
  if (argc < 2) {
    cout << PROGRAM_NAME << " !!! one argument necessary" << endl
         << tab << " 1 : Track Segment Stream data file" << endl;
    return -1;
  }

  //...1st argument...
  const string inname = argv[1];
  cout << tab << "Track Segment Stream : " << inname << endl;

  //...Open configuration data...
  ifstream infile(inname.c_str(), ios::in | ios::binary);
  if (infile.fail()) {
    cout << PROGRAM_NAME << " !!! can not open file" << endl
         << "    " << inname << endl;
    return -2;
  }

#ifdef TRGCDC_DISPLAY
  //...Gearbox...
  const string path = getenv(ENV_PATH);
  std::vector<std::string> m_backends;
  std::string m_filename = path + "/data/geometry/Belle2.xml";
  m_backends.push_back("file:");
  Gearbox& gearbox = Gearbox::getInstance();
  gearbox.setBackends(m_backends);
  gearbox.open(m_filename);

  //...TRGCDC...
  const string cname = path + "/data/trg/" + CONFIG;
//  TRGCDC * cdc = TRGCDC::getTRGCDC(cname, false, 100, 100);
  TRGCDC* cdc = TRGCDC::getTRGCDC(cname);
  const TRGClock& clock = cdc->systemClock();

  //...Wire layer ID for the central wires...
  unsigned LID[9] = {2, 10, 16, 22, 28, 34, 40, 46, 52};

  //...Display...
  D->clear();
  D->show();
#endif

  //...Prepare buffers...
  vector<int*> clks;
  vector<TRGBitStream*> bits[9];
  vector<const TCWire*> wires;

  //...Read stream...
  while (! infile.eof()) {

    //...Read record type...
    unsigned rec = 0;
    infile.read((char*) & rec, 4);
    unsigned siz = 0;
    infile.read((char*) & siz, 4);

    cout << "... " << hex << rec << "," << dec << siz << endl;

    //...Record manipulation...
    switch (rec) {

      case TRGBSRecord_Comment: {
        unsigned csiz = siz / 8;
        if (siz % 8) ++csiz;
        char* buf = new char[csiz + 1];
        infile.read(buf, csiz);
        buf[csiz] = 0;

        cout << "siz,csiz=" << siz << "," << csiz << endl;

        if (DEBUG_LEVEL)
          cout << "Comment : " << buf << endl;
        break;
      }

      case TRGBSRecord_BeginRun:
        if (DEBUG_LEVEL)
          cout << "BeginRun : " << siz << endl;
        break;

      case TRGBSRecord_EndRun:
        if (DEBUG_LEVEL)
          cout << "EndRun : " << siz << endl;
        break;

      case TRGBSRecord_BeginEvent:
#ifdef TRGCDC_DISPLAY
        D->clear();
        D->beginningOfEvent();
#endif
        for (unsigned i = 0; i < 9; i++) {
          for (unsigned j = 0; j < bits[i].size(); j++)
            delete bits[i][j];
          bits[i].clear();
        }
        for (unsigned i = 0; i < clks.size(); i++)
          delete clks[i];
        clks.clear();
        wires.clear();
        if (DEBUG_LEVEL)
          cout << "BeginEvent : " << siz << endl;
        break;

      case TRGBSRecord_EndEvent:
#ifdef TRGCDC_DISPLAY
        for (unsigned i = 0; i < 9; i++) {
          vector<TRGSignal> t = TRGBitStream::TRGBitStream2TRGSignal(
                                  clock,
                                  0,
                                  bits[i]);
          for (unsigned j = 0; j < t.size(); j++) {
            if (t[j].active())
              wires.push_back(cdc->wire(LID[i], j));
          }
        }
        D->area().append(wires);
        D->endOfEvent();
        D->run();

#endif
        if (DEBUG_LEVEL)
          cout << "EndEvent : " << siz << endl;
        break;

      case TRGBSRecord_Clock: {
        unsigned clk = 0;
        infile.read((char*) & clk, 4);
        clks.push_back(new int(clk));
        if (DEBUG_LEVEL)
          cout << "Clock : " << clk << endl;
        break;
      }

      case TRGBSRecord_SegmentSL0:
      case TRGBSRecord_SegmentSL1:
      case TRGBSRecord_SegmentSL2:
      case TRGBSRecord_SegmentSL3:
      case TRGBSRecord_SegmentSL4:
      case TRGBSRecord_SegmentSL5:
      case TRGBSRecord_SegmentSL6:
      case TRGBSRecord_SegmentSL7:
      case TRGBSRecord_SegmentSL8: {
        unsigned superLayer = rec - TRGBSRecord_SegmentSL0;
        unsigned csiz = siz / 8;
        if (siz % 8) ++csiz;
        char* buf = new char[csiz];
        infile.read(buf, csiz);
        TRGBitStream* bs = new TRGBitStream(buf, siz);
        bits[superLayer].push_back(bs);
        if (DEBUG_LEVEL)
          bs->dump();
        break;
      }

      default:
        cout << PROGRAM_NAME << " !!! unknown record found : "
             << "record type = " << hex << rec << endl;
        break;
    }
  }
  infile.close();

  //...Termination...
  cout << PROGRAM_NAME << " ... terminated" << endl;
}
