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
// Filename : TRGCDCTrackSegmentSimulator.cc
// Section  : TRG CDC
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A program to display TRGCDC components
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#define TRG_SHORT_NAMES
#define TRGCDC_SHORT_NAMES

#define DEBUG_LEVEL     1
#define PROGRAM_NAME    "TRGCDCTrackSegmentSimulator"
#define PROGRAM_VERSION "version 0.00"
#define ENV_PATH        "BELLE2_LOCAL_DIR"
//#define CONFIG        "TRGCDCWireConfig_0_20101110_0836.dat"
#define CONFIG          "TRGCDCConfig_0_20101111_1051_2013beamtest.dat"

#include <iostream>
#include <fstream>
#include <string>
#include "trg/trg/Debug.h"
#include "trg/trg/Utilities.h"
#include "trg/trg/State.h"
#include "trg/trg/Channel.h"
#include "trg/cdc/TRGCDC.h"
#include "trg/cdc/Wire.h"
#include "trg/cdc/Segment.h"
#include "trg/cdc/TrackSegmentFinder.h"
#include "framework/gearbox/Gearbox.h"

#ifdef TRGCDC_DISPLAY
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

void readFile(const string& filename,
              TRGSignalBundle& isb,
              TRGSignalBundle& osbe,
              TRGSignalBundle& osbt);

int
main(int argc, char* argv[])
{

  cout << PROGRAM_NAME << " ... " << PROGRAM_VERSION << endl;
  // const string tab = "    ";

  //...Arguments...
  if (argc < 3) {
    cout << PROGRAM_NAME << " needs 3 arguments" << endl;
    return -1;
  }
  const unsigned dl = stoi(string(argv[1]));
  const unsigned sl = stoi(string(argv[2]));
  const string fn = string(argv[3]);

  TRGDebug::level(dl);

  //...Gearbox...
  const string path = getenv(ENV_PATH);
  const string patht = path + "/data/trg/cdc/";
  vector<std::string> backends;
  backends.push_back("file:");
  string filename = "geometry/Belle2.xml";
  Gearbox& gearbox = Gearbox::getInstance();
  gearbox.setBackends(backends);
  gearbox.open(filename);

  //...TRGCDC parameters...
  const string configFile = patht + CONFIG;
  const unsigned simulationMode = 3; // fast and firm
  const unsigned fastSimulationMode = 0;
  const unsigned firmwareSimulationMode = 0;
  const int firmwareSimulationStart = 0;
  const int firmwareSimulationStop = 32 * 32 - 1;
  const unsigned makeRootFile = 0;
  const bool perfect2DFinder = false;
  const bool perfect3DFinder = false;
  const string innerTSLUTFile = patht + "innerLUT_v2.2.coe";
  const string outerTSLUTFile = patht + "outerLUT_v2.2.coe";
  const string rootTRGCDCFile = "TRGCDC.root";
  const string rootFitter3DFile = "Fitter3D.root";
  const unsigned houghFinderPeakMin = 5;
  const string houghMappingFilePlus = patht + "HoughMappingPlus20140807.dat";
  const string houghMappingFileMinus = patht +
                                       "HoughMappingMinus20140808.dat";
  const unsigned houghDoit = 2;
  const unsigned fLogicLUTTSF = 0;
  const unsigned fLRLUT = 1;
  const unsigned fevtTime = 1;
  const unsigned fmclr = 0;
  const double inefficiency = 0;
  const unsigned fileTSF = 0;
  const unsigned fileETF = 0;
  const unsigned fverETF = 0;
  const unsigned fprintFirmETF = 0;
  const unsigned fileHough3D = 0;
  const unsigned finder3DMode = 2;
  const unsigned fileFitter3D = 0;
  const unsigned trgCDCDataInputMode = 0;

  //...TRGCDC...
  TRGCDC* cdc = TRGCDC::getTRGCDC(configFile,
                                  simulationMode,
                                  fastSimulationMode,
                                  firmwareSimulationMode,
                                  firmwareSimulationStart,
                                  firmwareSimulationStop,
                                  makeRootFile,
                                  perfect2DFinder,
                                  perfect3DFinder,
                                  innerTSLUTFile,
                                  outerTSLUTFile,
                                  rootTRGCDCFile,
                                  rootFitter3DFile,
                                  houghFinderPeakMin,
                                  houghMappingFilePlus,
                                  houghMappingFileMinus,
                                  houghDoit,
                                  fLogicLUTTSF,
                                  fLRLUT,
                                  fevtTime,
                                  fmclr,
                                  inefficiency,
                                  fileTSF,
                                  fileETF,
                                  fverETF,
                                  fprintFirmETF,
                                  fileHough3D,
                                  finder3DMode,
                                  fileFitter3D,
                                  trgCDCDataInputMode);

  //...TSF board...
  TRGCDCTrackSegmentFinder& tsfb = * cdc->tsfboard(sl);

  //...Read input signal bundle...
  const TRGClock& dc = TRGCDC::getTRGCDC()->dataClock();
  TRGSignalBundle isb("mgr data from chipscope", dc);;
  TRGSignalBundle osbt("output for trk form chipscope", dc);;
  TRGSignalBundle osbe("output for evt from chipscope", dc);;
  readFile(fn, isb, osbe, osbt);

  //...Set input signal bundle...
  const_cast<TRGChannel*>(tsfb.input(0))->signal(& isb);
  TRGSignalVector dummy("mgr dummy data", dc, isb[0]->size());
  TRGSignalBundle isbd("mgr dummy data", dc);;
  isbd.push_back(& dummy);
  for (unsigned i = 1; i < tsfb.nInput(); i++)
    const_cast<TRGChannel*>(tsfb.input(i))->signal(& isbd);

  //...Get output signal bundle...
  tsfb.simulate2();
  const TRGSignalBundle& osb = * tsfb.output(0)->signal();

  //...Compare output...
  osb.dump();
  osbt.dump();

#ifdef TRGCDC_DISPLAY
  //...Display...
  D->clear();
  D->show();
#endif

  //...Termination...
  cout << PROGRAM_NAME << " ... terminated" << endl;
}

void
readFile(const string& fn,
         TRGSignalBundle& isb,
         TRGSignalBundle& osbe,
         TRGSignalBundle& osbt)
{

  //...Open input file...
  ifstream ifile(fn.c_str(), ios::in);
  if (ifile.fail()) {
    cout << PROGRAM_NAME << " !!! can not open file" << endl
         << "    " << fn << endl;
    exit(-1);
  }

  //...Preparations...
  const TRGClock& dc = TRGCDC::getTRGCDC()->dataClock();
  const unsigned eSize = 625;
  const unsigned tSize = 1054 - 625;
  const unsigned mSize = 1309 - 1053 - 9;
  TRGSignalVector* eOut = new TRGSignalVector("output for evt", dc, eSize);
  TRGSignalVector* tOut = new TRGSignalVector("output for trk", dc, tSize);
  TRGSignalVector* mIn = new TRGSignalVector("input from mgr", dc, mSize);

  //...Read data...
  char b[10000];
  while (! ifile.eof()) {
    ifile.getline(b, 10000);
    string l(b);

    if (l.size() == 0) continue;
    if (b[0] == '#') continue;
    if (b[0] == 'S') continue;

    // string clock = TRGUtil::carstring(l);
    l = TRGUtil::cdrstring(l);
    // const string g = TRGUtil::carstring(l);
    l = TRGUtil::cdrstring(l);
    const string dat = TRGUtil::carstring(l);
    l = TRGUtil::cdrstring(l);

    //...Divide data...
    TRGState a(dat.c_str(), 0);
    TRGState e = a.subset(0, eSize);
    TRGState t = a.subset(eSize, tSize);
    TRGState m = a.subset(1054, mSize);
    TRGState c = a.subset(1054 + mSize, 9);
    unsigned cMgr = unsigned(c);

    eOut->set(e, cMgr);
    tOut->set(t, cMgr);
    mIn->set(m, cMgr);

    // cout << "clock=" << clock << ",dat:size=" << a.size() << ",e:size="
    //      << e.size() << ",t:size=" << t.size() << ",m:size=" << m.size()
    //      << endl;
  }

  isb.push_back(mIn);
  osbe.push_back(eOut);
  osbt.push_back(tOut);
}
