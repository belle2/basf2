/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

//-----------------------------------------------------------------------------
// Description : A class to find tracks usning Hough algorithm
//-----------------------------------------------------------------------------

#define TRG_SHORT_NAMES
#define TRGCDC_SHORT_NAMES

#define ENV_PATH      "BELLE2_LOCAL_DIR"

#include <stdlib.h>
#include <map>
#include <iostream>
#include <fstream>
#include "cdc/geometry/CDCGeometryPar.h"
#include "trg/trg/Debug.h"
#include "trg/trg/Utilities.h"
#include "trg/trg/Time.h"
#include "trg/trg/Signal.h"
#include "trg/trg/Constants.h"
#include "trg/cdc/TRGCDC.h"
#include "trg/cdc/Layer.h"
#include "trg/cdc/Cell.h"
#include "trg/cdc/Wire.h"
#include "trg/cdc/WireHit.h"
#include "trg/cdc/HoughFinder.h"
#include "trg/cdc/Segment.h"
#include "trg/cdc/SegmentHit.h"
#include "trg/cdc/Circle.h"
#include "trg/cdc/TRGCDCTrack.h"
#include "trg/cdc/Link.h"
#include "trg/cdc/Relation.h"
#include "trg/cdc/Fitter3DUtility.h"
#include "trg/cdc/Fitter3D.h"
#include "trg/cdc/Helix.h"
#include "trg/cdc/JLUT.h"
#include "trg/cdc/JSignal.h"
#include "trg/cdc/JSignalData.h"
#include "trg/cdc/WireHitMC.h"
#include "trg/cdc/TrackMC.h"
#include "trg/cdc/FrontEnd.h"
#include "trg/cdc/Merger.h"
#include "trg/cdc/LUT.h"
#include "trg/cdc/EventTime.h"

#ifdef TRGCDC_DISPLAY_HOUGH
#include "trg/cdc/DisplayRphi.h"
#include "trg/cdc/DisplayHough.h"
namespace Belle2_TRGCDC {
  Belle2::TRGCDCDisplayHough* H0 = 0;
  Belle2::TRGCDCDisplayHough* H1 = 0;
}
#endif

using namespace std;
#ifdef TRGCDC_DISPLAY_HOUGH
using namespace Belle2_TRGCDC;
#endif

namespace Belle2 {

  string
  TRGCDCHoughFinder::version(void) const
  {
    return string("TRGCDCHoughFinder 5.24");
  }

  TRGCDCHoughFinder::TRGCDCHoughFinder(const string& name,
                                       const TRGCDC& TRGCDC,
                                       unsigned peakMin,
                                       const string& mappingFilePlus,
                                       const string& mappingFileMinus,
                                       unsigned doit)
    : _name(name),
      _cdc(TRGCDC),
      _circleH("CircleHough"),
      _doit(doit),
      _peakFinder("PeakFinder"),
      _peakMin(peakMin)
  {

    _commonData = 0;

    //...Read Hough plane parameters from file
    const string fMap[2] = {mappingFilePlus, mappingFileMinus};
    const string fName[2] = {string("circle hough plus"), string("circle hough minus")};

    for (unsigned f = 0; f < 2; f++) {
      const string fn = fMap[f];
      ifstream infile(fn.c_str(), ios::in);
      if (infile.fail()) {
        B2FATAL("Cannot open Hough mapping file " << fn);
        return;
      }

      //...Ignore lines not starting with a digit...
      string ignores;
      while (!isdigit(infile.peek())) {
        getline(infile, ignores);
      }

      //...Read Hough plane cell number and limits...
      unsigned nX = 0;
      unsigned nY = 0;
      double Ymin = 0.;
      double Ymax = 0.;

      infile >> nX >> nY >> Ymin >> Ymax;
      infile.close();

      _plane[f] = new TCHPlaneMulti2(fName[f], _circleH,
                                     nX, 0, 2 * M_PI,
                                     nY, Ymin, Ymax,
                                     5);
    }

    //...Set charge...
    _plane[0]->charge(1);
    _plane[1]->charge(-1);

#ifdef TRGCDC_DISPLAY_HOUGH
    if (! H0)
      H0 = new TCDisplayHough("Plus");
    H0->link(* D);
    H0->clear();
    H0->show();
    H0->move(630, 0);
    if (! H1)
      H1 = new TCDisplayHough("Minus");
    H1->link(* D);
    H1->clear();
    H1->show();
    H0->move(1260, 0);
#endif

    //...Old mapping (trasan methode)...
    if (_doit == 0 || _doit == 1) {

      //...Create patterns...
      unsigned axialSuperLayerId = 0;
      for (unsigned i = 0; i < _cdc.nSegmentLayers(); i++) {
        const Belle2::TRGCDCLayer* l = _cdc.segmentLayer(i);
        const unsigned nWires = l->nCells();

        if (! nWires) continue;
        if ((* l)[0]->stereo()) continue;

        _plane[0]->preparePatterns(axialSuperLayerId, nWires);
        _plane[1]->preparePatterns(axialSuperLayerId, nWires);
        for (unsigned j = 0; j < nWires; j++) {
          const TCCell& w = * (* l)[j];
          const float x = w.xyPosition().x();
          const float y = w.xyPosition().y();

          _plane[0]->clear();
          _plane[0]->vote(x, y, +1, axialSuperLayerId, 1);
          _plane[0]->registerPattern(axialSuperLayerId, j);

          _plane[1]->clear();
          _plane[1]->vote(x, y, -1, axialSuperLayerId, 1);
          _plane[1]->registerPattern(axialSuperLayerId, j);
        }
        ++axialSuperLayerId;
      }
    }

    //...Kaiyu's methode...
    else {
      mappingByFile(mappingFilePlus, mappingFileMinus);
    }
  }

  TRGCDCHoughFinder::~TRGCDCHoughFinder()
  {
#ifdef TRGCDC_DISPLAY_HOUGH
    if (H0)
      delete H0;
    if (H1)
      delete H1;
    cout << "TRGCDCHoughFinder ... Hough displays deleted" << endl;
#endif
  }

  int
  TRGCDCHoughFinder::doFindingTrasan(vector<unsigned> peaks[],
                                     vector<TCTrack*>& trackList2D) const
  {

    const string sn = "Hough Finder Finding (trasan version)";
    TRGDebug::enterStage(sn);

    //...Initialization...
    _plane[0]->clear();
    _plane[1]->clear();

    //...Voting...
    unsigned nLayers = _cdc.nAxialSuperLayers();
    for (unsigned i = 0; i < nLayers; i++) {
      const vector<const TCSHit*> hits = _cdc.axialSegmentHits(i);
      for (unsigned j = 0; j < hits.size(); j++) {
        _plane[0]->vote(i, hits[j]->cell().localId());
        _plane[1]->vote(i, hits[j]->cell().localId());
      }
    }
    _plane[0]->merge();
    _plane[1]->merge();

#ifdef TRGCDC_DISPLAY_HOUGH
    string stg = "2D : Hough : Results of Peak Finding";
    string inf = "   ";
    H0->stage(stg);
    H0->information(inf);
    H0->clear();
    H0->area().append(_plane[0]);
    H0->show();
    H1->stage(stg);
    H1->information(inf);
    H1->clear();
    H1->area().append(_plane[1]);
    H1->show();
#endif

    //...Look for peaks which have 5 hits...
    _peakFinder.findPeaksTrasan(* _plane[0], _peakMin, false, peaks[0]);
    _peakFinder.findPeaksTrasan(* _plane[1], _peakMin, false, peaks[1]);

    //...Peak loop to pick up segment hits...
    // (no fit, using peak position only)
    for (unsigned pm = 0; pm < 2; pm++) {
      for (unsigned i = 0; i < peaks[pm].size(); i++) {
        const unsigned peakId = peaks[pm][i];

        //...Make a track...
        TCTrack* t = makeTrack(peakId, pm);
        trackList2D.push_back(t);

#ifdef TRGCDC_DISPLAY_HOUGH
        vector<const TCTrack*> cc;
        cc.push_back(t);
        const string stgNow = "doFinding : track made";
        const string infNow = "   ";
        D->clear();
        D->stage(stgNow);
        D->information(infNow);
        D->area().append(cc, Gdk::Color("#FF0066009900"));
        D->area().append(_cdc.hits());
        D->area().append(_cdc.segmentHits());
        D->show();
        D->run();
#endif
      }
    }

    TRGDebug::leaveStage(sn);
    return 0;
  }

  vector<TCLink*>
  TRGCDCHoughFinder::selectBestHits(const vector<TCLink*>& links) const
  {
    vector<TCLink*> bests;
    vector<TCLink*> layers[9];
    TCLink::separate(links, 9, layers);

    if (TRGDebug::level()) {
      for (unsigned i = 0; i < 9; i++) {
        cout << TRGDebug::tab() << "layer " << i << endl;
        TCLink::dump(layers[i], "", TRGDebug::tab(4));
      }
    }

    //...Select links to be removed...
    for (unsigned i = 0; i < 9; i++) {
      if (layers[i].size() == 0) continue;
      if (layers[i].size() == 1) {
        bests.push_back(layers[i][0]);
        continue;
      }

      TCLink* best = layers[i][0];
      int timeMin = (layers[i][0]->cell()->signal())[0]->time();
      for (unsigned j = 1; j < layers[i].size(); j++) {
        const TRGTime& t = * (layers[i][j]->cell()->signal())[0];
        if (t.time() < timeMin) {
          timeMin = t.time();
          best = layers[i][j];
        }
      }

      bests.push_back(best);
    }
    return bests;
  }

  int
  TRGCDCHoughFinder::doFittingTrasan(vector<unsigned> peaks[],
                                     vector<TRGCDCTrack*>& trackList2DFitted) const
  {

    const string sn = "Hough Finder Fitting (trasan version)";
    TRGDebug::enterStage(sn);

    //...Peak loop...
    unsigned nCircles = 0;
    for (unsigned pm = 0; pm < 2; pm++) {
      for (unsigned i = 0; i < peaks[pm].size(); i++) {
        const unsigned peakId = peaks[pm][i];

        //...Get segment hits...
        vector<TCLink*> links;
        vector<const TCSegment*> segments;
        const unsigned nLayers = _plane[pm]->nLayers();
        for (unsigned j = 0; j < nLayers; j++) {
          const vector<unsigned>& ptn =
            _plane[pm]->patternId(j, peakId);
          for (unsigned k = 0; k < ptn.size(); k++) {
            const TCSegment& s = _cdc.axialSegment(j, ptn[k]);
            segments.push_back(& s);
            if (s.hit()) {
              TCLink* l = new TCLink(0, s.hit());
              links.push_back(l);
            }
          }
        }

        //...Select best hits in each layer...
        const vector<TCLink*> bests = selectBestHits(links);

        //...Make a circle...
        TCCircle c(bests);
        c.fit();
        c.name("CircleFitted_" + TRGUtil::itostring(nCircles));
        ++nCircles;

        if (TRGDebug::level()) {
          cout << TRGDebug::tab() << "peak#" << nCircles << ":"
               << "plane" << pm << ",serialId=" << peakId << endl;
          cout << TRGDebug::tab() << "segments below" << endl;
          cout << TRGDebug::tab(4);
          for (unsigned j = 0; j < segments.size(); j++) {
            cout << segments[j]->name();
            if (j != (segments.size() - 1))
              cout << ",";
          }
          cout << endl;
          cout << TRGDebug::tab() << "best links below" << endl;
          TCLink::dump(bests, "", TRGDebug::tab(1));
          c.dump("detail", TRGDebug::tab() + "Circle> ");
        }

        //...Make a track...
        TCTrack& t = * new TCTrack(c);
        t.name("Track_" + TRGUtil::itostring(i));
        trackList2DFitted.push_back(& t);

        if (TRGDebug::level()) {
          t.relation().dump("", TRGDebug::tab());
          t.dump("detail", TRGDebug::tab(1));
        }

#ifdef TRGCDC_DISPLAY_HOUGH
        vector<const TCCircle*> cc;
        cc.push_back(& c);
        const string stg = "doFitting : trasan method";
        const string inf = "   ";
        D->clear();
        D->stage(stg);
        D->information(inf);
        D->area().append(_cdc.hits());
        D->area().append(_cdc.segmentHits());
        D->area().append(cc, Gdk::Color("blue"));
        D->show();
        D->run();
#endif
      }
    }

    TRGDebug::leaveStage(sn);

    return 0;
  }

  void
  TRGCDCHoughFinder::terminate()
  {
    if (_commonData) delete _commonData;
  }

  double TRGCDCHoughFinder::calPhi(TRGCDCSegmentHit const* segmentHit, double eventTime)
  {
    const CDC::CDCGeometryPar& cdcp = CDC::CDCGeometryPar::Instance();
    unsigned localId = segmentHit->segment().center().localId();
    unsigned layerId = segmentHit->segment().center().layerId();
    int nWires = cdcp.nWiresInLayer(layerId) * 2;
    double rr = cdcp.senseWireR(layerId);
    double driftTime = segmentHit->segment().priorityTime();
    int lr = segmentHit->segment().LUT()->getValue(segmentHit->segment().lutPattern());
    return Fitter3DUtility::calPhi(localId, nWires, driftTime, eventTime, rr, lr);
  }

  void TRGCDCHoughFinder::calCosPhi(std::map<std::string, Belle2::TRGCDCJSignal>& mSignalStorage,
                                    std::map<std::string, Belle2::TRGCDCJLUT* >& mLutStorage)
  {

    /// Calculate phi
    Belle2::TRGCDCJSignalData* commonData = mSignalStorage.begin()->second.getCommonData();
    {
      mSignalStorage["invPhiAxMin"] = Belle2::TRGCDCJSignal(M_PI, mSignalStorage["phi_4"].getToReal(), commonData);
      mSignalStorage["invPhiAxMax"] = Belle2::TRGCDCJSignal(2 * M_PI, mSignalStorage["phi_4"].getToReal(), commonData);
    }

    for (unsigned iSt = 0; iSt < 5; iSt++) {
      string t_inputName = "phi_" + to_string(iSt);
      string t_outputName = "cosPhi_" + to_string(iSt);

      if (!mLutStorage.count(t_outputName)) {
        mLutStorage[t_outputName] = new Belle2::TRGCDCJLUT(t_outputName);
        mLutStorage[t_outputName]->setFloatFunction(
          [ = ](double aValue) -> double{return cos(aValue);},
          mSignalStorage[t_inputName],
          mSignalStorage["invPhiAxMin"], mSignalStorage["invPhiAxMax"], mSignalStorage[t_inputName].getToReal(),
          12, 12);
      };//if name

      mLutStorage[t_outputName]->operate(mSignalStorage[t_inputName], mSignalStorage[t_outputName]);
    } // end for
  }//calCosPhi

  void TRGCDCHoughFinder::calSinPhi(std::map<std::string, Belle2::TRGCDCJSignal>& mSignalStorage,
                                    std::map<std::string, Belle2::TRGCDCJLUT* >& mLutStorage)
  {
    Belle2::TRGCDCJSignalData* commonData = mSignalStorage.begin()->second.getCommonData();
    {
      mSignalStorage["invPhiAxMin"] = Belle2::TRGCDCJSignal(-M_PI / 2, mSignalStorage["phi_4"].getToReal(), commonData);
      mSignalStorage["invPhiAxMax"] = Belle2::TRGCDCJSignal(M_PI / 2, mSignalStorage["phi_4"].getToReal(), commonData);
    }

    for (unsigned iSt = 0; iSt < 5; iSt++) {
      string t_sininputName = "phi_" + to_string(iSt);
      string t_sinoutputName = "sinPhi_" + to_string(iSt);

      if (!mLutStorage.count(t_sinoutputName)) {
        mLutStorage[t_sinoutputName] = new Belle2::TRGCDCJLUT(t_sinoutputName);

        mLutStorage[t_sinoutputName]->setFloatFunction(
          [ = ](double aValue) -> double{return sin(aValue);},
          mSignalStorage[t_sininputName],
          mSignalStorage["invPhiAxMin"], mSignalStorage["invPhiAxMax"], mSignalStorage[t_sininputName].getToReal(),
          12, 12);
      };//if name

      mLutStorage[t_sinoutputName]->operate(mSignalStorage[t_sininputName], mSignalStorage[t_sinoutputName]);
    } // end for
  }//calSinPhi

  void TRGCDCHoughFinder::rPhi(std::map<std::string, Belle2::TRGCDCJSignal>& mSignalStorage,
                               std::map<std::string, Belle2::TRGCDCJLUT* >& mLutStorage)
  {
    Belle2::TRGCDCJSignalData* commonData = mSignalStorage.begin()->second.getCommonData();
    for (unsigned iSt = 0; iSt < 5; iSt++) {
      mSignalStorage["cosPhiMul_" + to_string(iSt)] <= mSignalStorage["cosPhi_" + to_string(iSt)] * mSignalStorage["cosPhi_" + to_string(
            iSt)];
    }

    for (unsigned iSt = 0; iSt < 5; iSt++) {
      mSignalStorage["sinPhiMul_" + to_string(iSt)] <= mSignalStorage["sinPhi_" + to_string(iSt)] * mSignalStorage["sinPhi_" + to_string(
            iSt)];
    }

    for (unsigned iSt = 0; iSt < 5; iSt++) {
      mSignalStorage["cossinPhiMul_" + to_string(iSt)] <= mSignalStorage["cosPhi_" + to_string(iSt)] * mSignalStorage["sinPhi_" +
          to_string(iSt)];
    }

    for (unsigned iSt = 0; iSt < 5; iSt++) {
      mSignalStorage["rcosPhiMul_" + to_string(iSt)] <= mSignalStorage["cosPhi_" + to_string(iSt)] * mSignalStorage["2Drr_" + to_string(
            iSt)];
    }

    for (unsigned iSt = 0; iSt < 5; iSt++) {
      mSignalStorage["rsinPhiMul_" + to_string(iSt)] <= mSignalStorage["sinPhi_" + to_string(iSt)] * mSignalStorage["2Drr_" + to_string(
            iSt)];
    }
    mSignalStorage["cossum_p1_0"] <= mSignalStorage["cosPhiMul_0"] + mSignalStorage["cosPhiMul_1"];
    mSignalStorage["cossum_p1_1"] <= mSignalStorage["cosPhiMul_2"] + mSignalStorage["cosPhiMul_3"];
    mSignalStorage["cossum_p1_2"] <= mSignalStorage["cosPhiMul_4"] ;
    mSignalStorage["cossum_p2_0"] <= mSignalStorage["cossum_p1_0"] + mSignalStorage["cossum_p1_1"];
    mSignalStorage["cossum_p2_1"] <= mSignalStorage["cossum_p1_2"] ;
    mSignalStorage["cossum"] <= mSignalStorage["cossum_p2_0"] + mSignalStorage["cossum_p2_1"];
    mSignalStorage["sinsum_p1_0"] <= mSignalStorage["sinPhiMul_0"] + mSignalStorage["sinPhiMul_1"];
    mSignalStorage["sinsum_p1_1"] <= mSignalStorage["sinPhiMul_2"] + mSignalStorage["sinPhiMul_3"];
    mSignalStorage["sinsum_p1_2"] <= mSignalStorage["sinPhiMul_4"] ;
    mSignalStorage["sinsum_p2_0"] <= mSignalStorage["sinsum_p1_0"] + mSignalStorage["sinsum_p1_1"];
    mSignalStorage["sinsum_p2_1"] <= mSignalStorage["sinsum_p1_2"] ;
    mSignalStorage["sinsum"] <= mSignalStorage["sinsum_p2_0"] + mSignalStorage["sinsum_p2_1"];
    mSignalStorage["cossinsum_p1_0"] <= mSignalStorage["cossinPhiMul_0"] + mSignalStorage["cossinPhiMul_1"];
    mSignalStorage["cossinsum_p1_1"] <= mSignalStorage["cossinPhiMul_2"] + mSignalStorage["cossinPhiMul_3"];
    mSignalStorage["cossinsum_p1_2"] <= mSignalStorage["cossinPhiMul_4"] ;
    mSignalStorage["cossinsum_p2_0"] <= mSignalStorage["cossinsum_p1_0"] + mSignalStorage["cossinsum_p1_1"];
    mSignalStorage["cossinsum_p2_1"] <= mSignalStorage["cossinsum_p1_2"] ;
    mSignalStorage["cossinsum"] <= mSignalStorage["cossinsum_p2_0"] + mSignalStorage["cossinsum_p2_1"];
    mSignalStorage["rcossum_p1_0"] <= mSignalStorage["rcosPhiMul_0"] + mSignalStorage["rcosPhiMul_1"];
    mSignalStorage["rcossum_p1_1"] <= mSignalStorage["rcosPhiMul_2"] + mSignalStorage["rcosPhiMul_3"];
    mSignalStorage["rcossum_p1_2"] <= mSignalStorage["rcosPhiMul_4"] ;
    mSignalStorage["rcossum_p2_0"] <= mSignalStorage["rcossum_p1_0"] + mSignalStorage["rcossum_p1_1"];
    mSignalStorage["rcossum_p2_1"] <= mSignalStorage["rcossum_p1_2"] ;
    mSignalStorage["rcossum"] <= mSignalStorage["rcossum_p2_0"] + mSignalStorage["rcossum_p2_1"];
    mSignalStorage["rsinsum_p1_0"] <= mSignalStorage["rsinPhiMul_0"] + mSignalStorage["rsinPhiMul_1"];
    mSignalStorage["rsinsum_p1_1"] <= mSignalStorage["rsinPhiMul_2"] + mSignalStorage["rsinPhiMul_3"];
    mSignalStorage["rsinsum_p1_2"] <= mSignalStorage["rsinPhiMul_4"] ;
    mSignalStorage["rsinsum_p2_0"] <= mSignalStorage["rsinsum_p1_0"] + mSignalStorage["rsinsum_p1_1"];
    mSignalStorage["rsinsum_p2_1"] <= mSignalStorage["rsinsum_p1_2"] ;
    mSignalStorage["rsinsum"] <= mSignalStorage["rsinsum_p2_0"] + mSignalStorage["rsinsum_p2_1"];
    mSignalStorage["hcx_p1_0"] <= mSignalStorage["rcossum"] * mSignalStorage["sinsum"];
    mSignalStorage["hcx_p1_1"] <= mSignalStorage["rsinsum"] * mSignalStorage["cossinsum"];
    mSignalStorage["hcx_p2"] <= mSignalStorage["hcx_p1_0"] - mSignalStorage["hcx_p1_1"];
    mSignalStorage["hcy_p1_0"] <= mSignalStorage["rsinsum"] * mSignalStorage["cossum"];
    mSignalStorage["hcy_p1_1"] <= mSignalStorage["rcossum"] * mSignalStorage["cossinsum"];
    mSignalStorage["hcy_p2"] <= mSignalStorage["hcy_p1_0"] - mSignalStorage["hcy_p1_1"];
    mSignalStorage["den_p1"] <= mSignalStorage["cossum"] * mSignalStorage["sinsum"];
    mSignalStorage["den_p2"] <= mSignalStorage["cossinsum"] * mSignalStorage["cossinsum"];
    mSignalStorage["den"] <= mSignalStorage["den_p1"] - mSignalStorage["den_p2"];
    {
      mSignalStorage["denMin"] = Belle2::TRGCDCJSignal(0.00315354, mSignalStorage["den"].getToReal(),
                                                       commonData);//den Min = 0.00630708*0.5
      mSignalStorage["denMax"] = Belle2::TRGCDCJSignal(1.332705, mSignalStorage["den"].getToReal(), commonData);//den Max = 0.88847*1.5
    }

    // Constrain den.
    {
      // Make ifElse data.
      vector<pair<Belle2::TRGCDCJSignal, vector<pair<Belle2::TRGCDCJSignal*, Belle2::TRGCDCJSignal> > > > t_data;
      // Compare
      Belle2::TRGCDCJSignal t_compare = Belle2::TRGCDCJSignal::comp(mSignalStorage["den"], ">", mSignalStorage["denMax"]);
      // Assignments
      vector<pair<Belle2::TRGCDCJSignal*, Belle2::TRGCDCJSignal> > t_assigns = {
        make_pair(&mSignalStorage["den_c"], mSignalStorage["denMax"])
      };
      // Push to data.
      t_data.push_back(make_pair(t_compare, t_assigns));
      // Compare
      t_compare = Belle2::TRGCDCJSignal::comp(mSignalStorage["den"], ">", mSignalStorage["denMin"]);
      // Assignments
      t_assigns = {
        make_pair(&mSignalStorage["den_c"], mSignalStorage["den"].limit(mSignalStorage["denMin"], mSignalStorage["denMax"]))
      };
      // Push to data.
      t_data.push_back(make_pair(t_compare, t_assigns));
      // Compare
      t_compare = Belle2::TRGCDCJSignal();
      // Assignments
      t_assigns = {
        make_pair(&mSignalStorage["den_c"], mSignalStorage["denMin"])
      };
      // Push to data.
      t_data.push_back(make_pair(t_compare, t_assigns));
      Belle2::TRGCDCJSignal::ifElse(t_data);
    }

    {
      unsigned long long t_int = mSignalStorage["den_c"].getMaxInt();
      double t_toReal = mSignalStorage["den_c"].getToReal();
      double t_actual = mSignalStorage["den_c"].getMaxActual();
      mSignalStorage["iDenMin"] = Belle2::TRGCDCJSignal(t_int, t_toReal, t_int, t_int, t_actual, t_actual, t_actual, -1, commonData);
      t_int = mSignalStorage["den_c"].getMinInt();
      t_actual = mSignalStorage["den_c"].getMinActual();
      mSignalStorage["iDenMax"] = Belle2::TRGCDCJSignal(t_int, t_toReal, t_int, t_int, t_actual, t_actual, t_actual, -1, commonData);
    }

    if (!mLutStorage.count("iDen")) {
      mLutStorage["iDen"] = new Belle2::TRGCDCJLUT("iDen");
      mLutStorage["iDen"]->setFloatFunction(
        [ = ](double aValue) -> double{return 0.5 / aValue;},
        mSignalStorage["den_c"],
        mSignalStorage["iDenMin"], mSignalStorage["iDenMax"], pow(1 / mSignalStorage["cossinsum"].getToReal(), 2),
        12, 12);
    }

    //Operate using LUT(iDen = 1/2/den)
    mLutStorage["iDen"]->operate(mSignalStorage["den_c"], mSignalStorage["iDen"]);

    mSignalStorage["hcx"] <= mSignalStorage["hcx_p2"] * mSignalStorage["iDen"];
    mSignalStorage["hcy"] <= mSignalStorage["hcy_p2"] * mSignalStorage["iDen"];

    mSignalStorage["hcx2"] <= mSignalStorage["hcx"] * mSignalStorage["hcx"];
    mSignalStorage["hcy2"] <= mSignalStorage["hcy"] * mSignalStorage["hcy"];
    mSignalStorage["sumhc"] <= mSignalStorage["hcx2"] + mSignalStorage["hcy2"];
    {
      mSignalStorage["sumhcMin"] = Belle2::TRGCDCJSignal(4489, mSignalStorage["sumhc"].getToReal(), commonData);
      mSignalStorage["sumhcMax"] = Belle2::TRGCDCJSignal(2560000, mSignalStorage["sumhc"].getToReal(), commonData);
    }

    // Constrain den.
    {
      // Make ifElse data.
      vector<pair<Belle2::TRGCDCJSignal, vector<pair<Belle2::TRGCDCJSignal*, Belle2::TRGCDCJSignal> > > > t_data;
      // Compare
      Belle2::TRGCDCJSignal t_compare = Belle2::TRGCDCJSignal::comp(mSignalStorage["sumhc"], ">", mSignalStorage["sumhcMax"]);
      // Assignments
      vector<pair<Belle2::TRGCDCJSignal*, Belle2::TRGCDCJSignal> > t_assigns = {
        make_pair(&mSignalStorage["sumhc_c"], mSignalStorage["sumhcMax"])
      };
      // Push to data.
      t_data.push_back(make_pair(t_compare, t_assigns));
      // Compare
      t_compare = Belle2::TRGCDCJSignal::comp(mSignalStorage["sumhc"], ">", mSignalStorage["sumhcMin"]);
      // Assignments
      t_assigns = {
        make_pair(&mSignalStorage["sumhc_c"], mSignalStorage["sumhc"].limit(mSignalStorage["sumhcMin"], mSignalStorage["sumhcMax"]))
      };
      // Push to data.
      t_data.push_back(make_pair(t_compare, t_assigns));
      // Compare
      t_compare = Belle2::TRGCDCJSignal();
      // Assignments
      t_assigns = {
        make_pair(&mSignalStorage["sumhc_c"], mSignalStorage["sumhcMin"])
      };
      // Push to data.
      t_data.push_back(make_pair(t_compare, t_assigns));
      Belle2::TRGCDCJSignal::ifElse(t_data);
    }
    {
      unsigned long long t_int = mSignalStorage["sumhc_c"].getMaxInt();
      double t_toReal = mSignalStorage["sumhc_c"].getToReal();
      double t_actual = mSignalStorage["sumhc_c"].getMaxActual();
      mSignalStorage["iRhoMax"] = Belle2::TRGCDCJSignal(t_int, t_toReal, t_int, t_int, t_actual, t_actual, t_actual, -1, commonData);
      t_int = mSignalStorage["sumhc_c"].getMinInt();
      t_actual = mSignalStorage["sumhc_c"].getMinActual();
      mSignalStorage["iRhoMin"] = Belle2::TRGCDCJSignal(t_int, t_toReal, t_int, t_int, t_actual, t_actual, t_actual, -1, commonData);
    }
    // Generate sqrt LUT sqrt(hcx*hcx + hcy*hcy)
    if (!mLutStorage.count("iRho")) {
      mLutStorage["iRho"] = new Belle2::TRGCDCJLUT("iRho");
      mLutStorage["iRho"]->setFloatFunction(
        [ = ](double aValue) -> double{return abs(sqrt(aValue));},
        mSignalStorage["sumhc_c"],
        mSignalStorage["iRhoMin"], mSignalStorage["iRhoMax"], mSignalStorage["sumhc"].getToReal(),
        24, 12);
    }
    // Operate using LUT(iRho = sqrt(sumhc))
    mLutStorage["iRho"]->operate(mSignalStorage["sumhc_c"], mSignalStorage["iRho"]);
    if (TRGDebug::level()) {
      cout << "<<<iRho>>>" << endl;
      mSignalStorage["iRho"].dump();
    }
  }//rphi

  void
  TRGCDCHoughFinder::mappingByFile(const string& mappingFilePlus,
                                   const string& mappingFileMinus)
  {
    const string sn = "mappingByFile";
    TRGDebug::enterStage(sn);

    const string* fMap[2] = {& mappingFilePlus, & mappingFileMinus};

    for (unsigned f = 0; f < 2; f++) {

      const string& fn = * fMap[f];
      ifstream infile(fn.c_str(), ios::in);
      if (infile.fail()) {
        cout << " !!! can not open file" << endl
             << "    " << fn << endl
             << "    Mapping aborted" << endl;
        return;
      }

      //...Map data storage...
      vector<unsigned> x;
      vector<unsigned> y;
      vector<vector<unsigned>> tsf;

      //...Ignore lines not starting with a digit...
      string ignores;
      while (!isdigit(infile.peek())) {
        getline(infile, ignores);
      }
      //...Skip the first line (read in constructor)...
      getline(infile, ignores);

      //...Read map file...
      string car;
      while (getline(infile, car)) {
        unsigned i = 0;
        unsigned b;
        istringstream in(car);
        vector<unsigned> slts;
        while (in >> b) {
          ++i;
          unsigned j = i % 2;
          if (i == 1)              // cell position x
            x.push_back(b);
          if (i == 2)              // cell position y (stored with offset 1)
            y.push_back(b - 1);
          if (j != 0 && i != 1)    // TSF SL
            slts.push_back(b);
          if (j != 1 && i != 2)    // TSF local ID
            slts.push_back(b);
        }
        tsf.push_back(slts);
      }
      infile.close();

      unsigned axialSuperLayerId = 0;
      for (unsigned i = 0; i < _cdc.nSegmentLayers(); i++) {
        const Belle2::TRGCDCLayer* l = _cdc.segmentLayer(i);
        const unsigned nWires = l->nCells();

        if (! nWires) continue;
        if ((* l)[0]->stereo()) continue;

        _plane[f]->preparePatterns(axialSuperLayerId, nWires);
        for (unsigned j = 0; j < nWires; j++) {

          _plane[f]->clear();

          //...loop over all hp cells...
          const unsigned n = x.size();
          for (unsigned k = 0; k < n; k++) {
            const int ix = x[k];
            const int iy = y[k];
            const unsigned sid = _plane[f]->serialId(ix, iy);
            for (unsigned itsf = 0; itsf < tsf[k].size();) {
              const unsigned sl = tsf[k][itsf];
              const unsigned id = tsf[k][itsf + 1];
              if ((i == sl) && (j == id)) {
                _plane[f]->setEntry(sid, axialSuperLayerId, 1);
              }
              itsf += 2;
            }
          }

          _plane[f]->registerPattern(axialSuperLayerId, j);
        }
        ++axialSuperLayerId;
      }
    }

    TRGDebug::leaveStage(sn);
  }

  int
  TRGCDCHoughFinder::FindAndFit(std::vector<TRGCDCTrack*>& trackList2D,
                                std::vector<TRGCDCTrack*>& trackList2DFitted)
  {

    if (_doit == 0 || _doit == 1)
      return doFindingAndFittingTrasan(trackList2D, trackList2DFitted);
    else
      return doFindingAndFitting(trackList2D, trackList2DFitted);
  }

  int
  TRGCDCHoughFinder::doFindingAndFittingTrasan(std::vector<TRGCDCTrack*>& trackList2D,
                                               std::vector<TRGCDCTrack*>& trackList2DFitted)
  {

    const string sn = "HoughFinder::doFindingAndFitting (trasan version)";
    TRGDebug::enterStage(sn);

    vector<unsigned> peaks[2];
    doFindingTrasan(peaks, trackList2D);
    doFittingTrasan(peaks, trackList2DFitted);

    TRGDebug::leaveStage(sn);

    return 0;
  }

  int
  TRGCDCHoughFinder::doFindingAndFitting(std::vector<TRGCDCTrack*>& trackList2D,
                                         std::vector<TRGCDCTrack*>& trackList2DFitted)
  {

    const string sn = "HoughFinder::doFindingAndFitting (kaiyu version)";
    TRGDebug::enterStage(sn);

    vector<vector<unsigned>> peaks[2];
    doFinding(peaks, trackList2D);
    doFitting(trackList2D, trackList2DFitted);

    TRGDebug::leaveStage(sn);

    return 0;
  }

  int
  TRGCDCHoughFinder::doFinding(std::vector<std::vector<unsigned>> peaks[],
                               std::vector<TRGCDCTrack*>& trackList2D)
  {

    const string sn = "HoughFinder::doFinding";
    TRGDebug::enterStage(sn);

    //...Initialization...
    _plane[0]->clear();
    _plane[1]->clear();

    //...Voting...
    unsigned nLayers = _cdc.nAxialSuperLayers();
    for (unsigned i = 0; i < nLayers; i++) {
      const vector<const TCSHit*> hits = _cdc.axialSegmentHits(i);
      for (unsigned j = 0; j < hits.size(); j++) {
        _plane[0]->vote(i, hits[j]->cell().localId());
        _plane[1]->vote(i, hits[j]->cell().localId());
      }
    }
    _plane[0]->merge();
    _plane[1]->merge();

    //...Look for peaks which have 5 hits...
    _peakFinder.findPeaks(* _plane[0], _peakMin, peaks[0]);
    _peakFinder.findPeaks(* _plane[1], _peakMin, peaks[1]);

    //...Peak loop to make tracks (no fit, using hough position only)...
    for (unsigned pm = 0; pm < 2; pm++) {
      for (unsigned i = 0; i < peaks[pm].size(); i++) {
        const unsigned peakId = _plane[pm]->serialId(peaks[pm][i][0],
                                                     peaks[pm][i][1]);
        TCTrack* t = makeTrack(peakId, pm);
        trackList2D.push_back(t);

#ifdef TRGCDC_DISPLAY_HOUGH
        vector<const TCTrack*> cc;
        cc.push_back(t);
        const string stg = "doFinding : track made";
        const string inf = "   ";
        D->clear();
        D->stage(stg);
        D->information(inf);
        D->area().append(cc, Gdk::Color("#FF0066009900"));
        D->area().append(_cdc.hits());
        D->area().append(_cdc.segmentHits());
        D->show();
        D->run();
#endif
      }
    }

    TRGDebug::leaveStage(sn);
    return 0;
  }

  int
  TRGCDCHoughFinder::doFitting(std::vector<TRGCDCTrack*>& trackList2D,
                               std::vector<TRGCDCTrack*>& trackList2DFitted)
  {

    const string sn = "Hough Finder Fitting";
    TRGDebug::enterStage(sn);

    if (_commonData == 0) {
      _commonData = new Belle2::TRGCDCJSignalData();
    }

    //...Event time...
    bool fEvtTime = true;
    // cppcheck-suppress knownConditionTrueFalse
    double eventTime = fEvtTime ? _cdc.getEventTime() : 0;

    // Loop over all tracks
    for (unsigned int iTrack = 0; iTrack < trackList2D.size(); iTrack++) {
      TCTrack& aTrack = * new TCTrack(* trackList2D[iTrack]);
      trackList2DFitted.push_back(& aTrack);

      CDC::CDCGeometryPar& cdcp = CDC::CDCGeometryPar::Instance();
      double phi02D, Trg_PI;
      vector<double> nWires(9);
      vector<double> rr(9);
      vector<double> rr2D;
      vector<double> wirePhi2DError(5);
      wirePhi2DError[0] = 0.0085106;
      wirePhi2DError[1] = 0.0039841;
      wirePhi2DError[2] = 0.0025806;
      wirePhi2DError[3] = 0.0019084;
      wirePhi2DError[4] = 0.001514;
      vector<double> driftPhi2DError(5);
      driftPhi2DError[0] = 0.0085106;
      driftPhi2DError[1] = 0.0039841;
      driftPhi2DError[2] = 0.0025806;
      driftPhi2DError[3] = 0.0019084;
      driftPhi2DError[4] = 0.001514;
      Trg_PI = 3.141592653589793;
      // Check if all superlayers have one TS
      bool trackFull = 1;
      for (unsigned iSL = 0; iSL < _cdc.nSuperLayers(); iSL = iSL + 2) {
        // Check if all superlayers have one TS
        const vector<TCLink*>& links = aTrack.links(iSL);
        const unsigned nSegments = links.size();

        if (TRGDebug::level())
          cout << TRGDebug::tab() << "#segments in SL" << iSL << " : "
               << nSegments << endl;

        // Find if there is a TS with a priority hit.
        // Loop over all TS in same superlayer.
        bool priorityHitTS = 0;
        for (unsigned iTS = 0; iTS < nSegments; iTS++) {
          const TCSegment* _segment = dynamic_cast<const TCSegment*>(& links[iTS]->hit()->cell());
          if (_segment->center().hit() != 0)  priorityHitTS = 1;
        }
        if (nSegments != 1) {
          if (nSegments == 0) {
            trackFull = 0;
            if (TRGDebug::level())
              cout << TRGDebug::tab()
                   << "=> Not enough TS." << endl;
          } else {
            if (TRGDebug::level())
              cout << TRGDebug::tab()
                   << "=> multiple TS are assigned." << endl;
          }
        } else {
          if (priorityHitTS == 0) {
            trackFull = 0;
            if (TRGDebug::level())
              cout << TRGDebug::tab()
                   << "=> There are no priority hit TS" << endl;
          }
        }
      } // End superlayer loop
      if (trackFull == 0) {
        TRGCDCHelix helix(ORIGIN, CLHEP::HepVector(5, 0), CLHEP::HepSymMatrix(5, 0));
        CLHEP::HepVector helixParameters(5);
        helixParameters = aTrack.helix().a();
        aTrack.setFitted(0);
        aTrack.setHelix(helix);
        continue;
      }

      for (unsigned iSL = 0; iSL < 9; iSL++) {
        unsigned _layerId = _cdc.segment(iSL, 0).center().layerId();
        rr[iSL] = cdcp.senseWireR(_layerId);
        nWires[iSL] = cdcp.nWiresInLayer(_layerId) * 2;
      }
      rr2D = vector<double>({rr[0], rr[2], rr[4], rr[6], rr[8]});

      // Get wirePhi, Drift information
      vector<double>wirePhi(9);
      vector<double>driftLength(9);
      vector<double>LR(9);
      vector<double>lutLR(9);
      vector<double>mcLR(9);
      bool fmcLR = false, fLRLUT = true;
      for (unsigned iSL = 0; iSL < 9; iSL = iSL + 2) {
        const vector<TCLink*>& links = aTrack.links(iSL);
        const TCSegment* _segment = dynamic_cast<const TCSegment*>(& links[0]->hit()->cell());
        wirePhi[iSL] =  _segment->localId() / nWires[iSL] * 4 * Trg_PI;
        lutLR[iSL] = _segment->LUT()->getValue(_segment->lutPattern());
        mcLR[iSL] = _segment->hit()->mcLR();
        driftLength[iSL] = _segment->hit()->drift();
        // cppcheck-suppress knownConditionTrueFalse
        if (fmcLR)       LR[iSL] = mcLR[iSL];
        // cppcheck-suppress knownConditionTrueFalse
        else if (fLRLUT) LR[iSL] = lutLR[iSL];
        else LR[iSL] = 3;
      }//End superlayer loop
      // 2D fit values from IW 2D fitter
      phi02D = aTrack.helix().phi0();
      if (aTrack.charge() < 0) {
        phi02D = phi02D - Trg_PI;
        if (phi02D < 0) phi02D = phi02D + 2 * Trg_PI;
      }
      // Get 2D fit values from JB 2D fitter
      // Set phi2DError for 2D fit
      vector<double>phi2DError(5);
      for (unsigned iAx = 0; iAx < 5; iAx++) {
        if (LR[2 * iAx] != 2) phi2DError[iAx] = driftPhi2DError[iAx];
        else phi2DError[iAx] = wirePhi2DError[iAx];
      }
      // Calculate phi2D using driftTime.
      vector<double>phi2D(5);
      for (unsigned iAx = 0; iAx < 5; iAx++) {
        phi2D[iAx] = Fitter3DUtility::calPhi(wirePhi[iAx * 2],
                                             driftLength[iAx * 2],
                                             eventTime,
                                             rr[iAx * 2],
                                             LR[iAx * 2]);
        if (TRGDebug::level()) {
          cout << TRGDebug::tab() << "eventTime: " << eventTime << endl;
          for (int i = 0 ; i < 5 ; i++) {
            cout << TRGDebug::tab() << "phi2D: : " << phi2D[i] << endl;
            cout << TRGDebug::tab() << "wirePhi: " << wirePhi[i * 2]
                 << endl;
            cout << TRGDebug::tab() << "driftLength: " <<
                 driftLength[i * 2] << endl;
            cout << TRGDebug::tab() << "LR: " << LR[i * 2] << endl;
          }
        }
      }
      // Fit2D
      double rho, phi0, pt, chi2;
      rho = 0;
      phi0 = 0;
      pt = 0;
      chi2 = 0;
      vector<double>phi2DInvError(5);
      for (unsigned iAx = 0; iAx < 5; iAx++) {
        phi2DInvError[iAx] = 1 / phi2DError[iAx];
      }

      Fitter3DUtility::rPhiFitter(&rr2D[0], &phi2D[0], &phi2DInvError[0], rho, phi0, chi2); // By JB

      pt = 0.3 * 1.5 * rho / 100;

      if (TRGDebug::level()) {
        cout << TRGDebug::tab() << "rho: " << rho << endl;
        cout << TRGDebug::tab() << "pt:  " << pt << endl;
      }

      // update track parameters
      TRGCDCHelix helix(ORIGIN, CLHEP::HepVector(5, 0), CLHEP::HepSymMatrix(5, 0));
      CLHEP::HepVector a(5);
      a = aTrack.helix().a();
      a[1] = (aTrack.charge() < 0) ? fmod(phi0 + M_PI, 2 * M_PI) : phi0;
      a[2] = 1 / pt * aTrack.charge();
      helix.a(a);
      aTrack.setFitted(1);
      aTrack.setHelix(helix);
      aTrack.set2DFitChi2(chi2);

      // Change to Signals
      {
        /// LUT values
        double rhoMin = 67;
        double rhoMax = 1600;
        int phiBitSize = 12;
        int rhoBitSize = 12;
        vector<tuple<string, double, int, double, double, int> > t_values = {
          make_tuple("phi_0", phi2D[0], phiBitSize, -2 * M_PI, 2 * M_PI, 0),
          make_tuple("phi_1", phi2D[1], phiBitSize, -2 * M_PI, 2 * M_PI, 0),
          make_tuple("phi_2", phi2D[2], phiBitSize, -2 * M_PI, 2 * M_PI, 0),
          make_tuple("phi_3", phi2D[3], phiBitSize, -2 * M_PI, 2 * M_PI, 0),
          make_tuple("phi_4", phi2D[4], phiBitSize, -2 * M_PI, 2 * M_PI, 0),
          make_tuple("rho", rho, rhoBitSize, rhoMin, rhoMax, 0),
          make_tuple("2Drr_0", rr[0], 12, 18.8, 105.68, 0),
          make_tuple("2Drr_1", rr[2], 12, 18.8, 105.68, 0),
          make_tuple("2Drr_2", rr[4], 12, 18.8, 105.68, 0),
          make_tuple("2Drr_3", rr[6], 12, 18.8, 105.68, 0),
          make_tuple("2Drr_4", rr[8], 12, 18.8, 105.68, 0),
        };
        TRGCDCJSignal::valuesToMapSignals(t_values, _commonData, m_mSignalStorage);
      }
      TRGCDCHoughFinder::calCosPhi(m_mSignalStorage, m_mLutStorage);
      TRGCDCHoughFinder::calSinPhi(m_mSignalStorage, m_mLutStorage);
      TRGCDCHoughFinder::rPhi(m_mSignalStorage, m_mLutStorage);
      // Name signals only once.
      bool updateSecondName = false;
      if ((*m_mSignalStorage.begin()).second.getName() == "") {
        for (auto it = m_mSignalStorage.begin(); it != m_mSignalStorage.end(); it++) {
          (*it).second.setName((*it).first);
        }
        updateSecondName = true;
      }

      if (updateSecondName) {
        // Print Vhdl
        if ((*m_mSignalStorage.begin()).second.getName() != "") {
          if (_commonData->getPrintedToFile() == 0) {
            if (_commonData->getPrintVhdl() == 0) {
              _commonData->setVhdlOutputFile("Fitter2D.vhd");
              _commonData->setPrintVhdl(1);
            } else {
              _commonData->setPrintVhdl(0);
              _commonData->entryVhdlCode();
              _commonData->signalsVhdlCode();
              _commonData->buffersVhdlCode();
              _commonData->printToFile();
              // Print LUTs.
              for (map<string, TRGCDCJLUT*>::iterator it = m_mLutStorage.begin(); it != m_mLutStorage.end(); ++it) {
                it->second->makeCOE(it->first + ".coe");
              }
            }
          }
        }
      }

#ifdef TRGCDC_DISPLAY_HOUGH
      vector<const TCTrack*> cc;
      cc.push_back(& aTrack);
      const string stg = "doFitting : Kaiyu method";
      const string inf = "   ";
      D->clear();
      D->stage(stg);
      D->information(inf);
      D->area().append(_cdc.hits());
      D->area().append(_cdc.segmentHits());
      D->area().append(cc, Gdk::Color("blue"));
      D->show();
      D->run();
#endif
    }

    TRGDebug::leaveStage(sn);

    return 0;
  }

  TCTrack*
  TRGCDCHoughFinder::makeTrack(const unsigned peakId, const unsigned pm) const
  {

    //...Cal. pt and phi from the cell number...
    const TCHTransformationCircle& tc =
      (TCHTransformationCircle&) _plane[pm]->transformation();
    unsigned x, y;
    _plane[pm]->id(peakId, x, y);
    const TRGPoint2D hp = _plane[pm]->position(x, y);
    const TRGPoint2D cc = tc.circleCenter(hp);
    const double r = cc.y();
    const double phi = cc.x();

    //...Make a circle...
    TCCircle c(r, phi, pm ? -1. : 1., * _plane[pm]);
    c.name("circle_" + TRGUtil::itostring(int(peakId) * (pm ? -1 : 1)));

    if (TRGDebug::level()) {
      cout << TRGDebug::tab() << "plane" << pm << ",serialId=" << peakId
           << endl;
      c.dump("detail", TRGDebug::tab() + "Circle> ");
    }

    //...Get segment hits...
    vector<TCLink*> links;
    vector<const TCSegment*> segments;
    const unsigned nLayers = _plane[pm]->nLayers();
    for (unsigned j = 0; j < nLayers; j++) {
      const vector<unsigned>& ptn =
        _plane[pm]->patternId(j, peakId);
      for (unsigned k = 0; k < ptn.size(); k++) {
        const TCSegment& s = _cdc.axialSegment(j, ptn[k]);
        segments.push_back(& s);
        if (s.hit()) {
          TCLink* l = new TCLink(0, s.hit());
          links.push_back(l);
        }
      }
    }
    c.append(links);

    if (TRGDebug::level()) {
      cout << TRGDebug::tab() << "attched segments below" << endl;
      cout << TRGDebug::tab(4);
      for (unsigned j = 0; j < segments.size(); j++) {
        cout << segments[j]->name();
        if (j != (segments.size() - 1))
          cout << ",";
      }
      cout << endl;
    }

    //...Make a track...
    TCTrack* t = new TCTrack(c);
    t->name("track_" + TRGUtil::itostring(int(peakId) * (pm ? -1 : 1)));

    if (TRGDebug::level()) {
      t->relation().dump("", TRGDebug::tab());
      t->dump("detail");
    }

    return t;
  }

} // namespace Belle2
