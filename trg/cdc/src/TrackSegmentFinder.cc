//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : TrackSegmentFinder.cc
// Section  : TRG CDC
// Owner    : Jaebak Kim
// Email    : jbkim@hep.korea.ac.kr
//-----------------------------------------------------------------------------
// Description : A class to find Track Segments
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#define TRG_SHORT_NAMES
#define TRGCDC_SHORT_NAMES

#include <bitset>
#include "TVectorD.h"
#include "trg/trg/Debug.h"
#include "trg/trg/State.h"
#include "trg/trg/Channel.h"
#include "trg/trg/Utilities.h"
#include "trg/cdc/TRGCDC.h"
#include "trg/cdc/Cell.h"
#include "trg/cdc/Wire.h"
#include "trg/cdc/WireHit.h"
#include "trg/cdc/Segment.h"
#include "trg/cdc/SegmentHit.h"
#include "trg/cdc/TrackSegmentFinder.h"
#include "framework/datastore/StoreArray.h"
#include "framework/datastore/RelationArray.h"
#include "cdc/dataobjects/CDCSimHit.h"
#include "mdst/dataobjects/MCParticle.h"
#include "trg/cdc/FrontEnd.h"
#include "trg/cdc/Merger.h"
#include "trg/cdc/TRGCDC.h"

using namespace std;

namespace Belle2 {

//...For debug : yi...
// vector<TRGSignalVector * > dbgIn;
// vector<TRGSignalVector * > dbgOut;

  TRGCDCTrackSegmentFinder::TRGCDCTrackSegmentFinder(const TRGCDC& TRGCDC,
                                                     bool makeRootFile,
                                                     bool logicLUTFlag)
    : TRGBoard("", TRGClock("", 0, 0), TRGClock("", 0, 0), TRGClock("", 0, 0),
               TRGClock("", 0, 0)),
      _cdc(TRGCDC),
      m_logicLUTFlag(logicLUTFlag),
      m_makeRootFile(makeRootFile)
  {

    m_Trg_PI = 3.141592653589793;

    m_hitPatternInformation = new TClonesArray("TVectorD");
    m_particleEfficiency = new TClonesArray("TVectorD");
    m_tsInformation = new TClonesArray("TVectorD");
    m_nnPatternInformation = new TClonesArray("TVectorD");

    // For ROOT file
    TDirectory* currentDir = gDirectory;
    if (m_makeRootFile) {
      m_fileTSF = new TFile("TSF.root", "RECREATE");

      m_treeInputTSF = new TTree("m_treeInputTSF", "InputTSF");
      m_treeInputTSF->Branch("hitPatternInformation", &m_hitPatternInformation,
                             32000, 0);

      m_treeOutputTSF = new TTree("m_treeOutputTSF", "OutputTSF");
      m_treeOutputTSF->Branch("particleEfficiency", &m_particleEfficiency, 32000, 0);
      m_treeOutputTSF->Branch("tsInformation", &m_tsInformation, 32000, 0);

      // For neural network TSF. Filled only when TSF and priority is hit.
      m_treeNNTSF = new TTree("m_treeNNTSF", "NNTSF");
      m_treeNNTSF->Branch("nnPatternInformation", &m_nnPatternInformation, 32000, 0);
    }
    currentDir->cd();
  }


  TRGCDCTrackSegmentFinder::TRGCDCTrackSegmentFinder(const TRGCDC& TRGCDC,
                                                     const std::string& name,
                                                     boardType type,
                                                     const TRGClock& systemClock,
                                                     const TRGClock& dataClock,
                                                     const TRGClock& userClockInput,
                                                     const TRGClock& userClockOutput,
                                                     std::vector<TCSegment*>& tsSL)
    : TRGBoard(name, systemClock, dataClock, userClockInput, userClockOutput),
      _cdc(TRGCDC),
      _type(type),
      //     _tisb(0),
      _tosbE(0),
      _tosbT(0),
      _tsSL(tsSL)
  {

  }
  TRGCDCTrackSegmentFinder::~TRGCDCTrackSegmentFinder()
  {

    delete m_nnPatternInformation;
    delete m_tsInformation;
    delete m_particleEfficiency;
    delete m_hitPatternInformation;
    delete m_treeInputTSF;
    if (m_makeRootFile) delete m_fileTSF;

  }

  TRGCDCTrackSegmentFinder::boardType
  TRGCDCTrackSegmentFinder:: type(void) const
  {
    return _type;
  }
  void TRGCDCTrackSegmentFinder::terminate(void)
  {
    if (m_makeRootFile) {
      m_fileTSF->Write();
      m_fileTSF->Close();
    }
  }

  void TRGCDCTrackSegmentFinder::doit(std::vector<TRGCDCSegment*>& tss,
                                      const bool trackSegmentClockSimulation,
                                      std::vector<TRGCDCSegmentHit*>& segmentHits,
                                      std::vector<TRGCDCSegmentHit*>* segmentHitsSL)
  {
    TRGDebug::enterStage("Track Segment Finder");

    // Saves TS information
    //saveTSInformation(tss);

    //...Store TS hits...
    const unsigned n = tss.size();
    for (unsigned i = 0; i < n; i++) {
      TCSegment& s = * tss[i];
      s.simulate(trackSegmentClockSimulation, m_logicLUTFlag,
                 TRGCDC::getTRGCDC()->getCDCHitCollectionName());
      if (s.signal().active()) {
        TCSHit* th = new TCSHit(s);
        s.hit(th);
        segmentHits.push_back(th);
        segmentHitsSL[s.layerId()].push_back(th);
      }
    }

    // Save TSF results
    saveTSFResults(segmentHitsSL);

    // Saves NNTS information. Only when ts is hit.
    //saveNNTSInformation(tss);

    if (TRGDebug::level() > 1) {
      cout << TRGDebug::tab() << "TS hit list" << endl;
      string dumpOption = "trigger";
      if (TRGDebug::level() > 2)
        dumpOption = "detail";
      for (unsigned i = 0; i < _cdc.nSegments(); i++) {
        const TCSegment& s = _cdc.segment(i);
        if (s.signal().active())
          s.dump(dumpOption, TRGDebug::tab(4));
      }

      cout << TRGDebug::tab() << "TS hit list (2)" << endl;
      if (TRGDebug::level() > 2)
        dumpOption = "detail";
      for (unsigned i = 0; i < segmentHits.size(); i++) {
        const TCSHit& s = * segmentHits[i];
        s.dump(dumpOption, TRGDebug::tab(4));
      }

      cout << TRGDebug::tab() << "TS hit list (3)" << endl;
      if (TRGDebug::level() > 2)
        dumpOption = "detail";
      for (unsigned j = 0; j < _cdc.nSuperLayers(); j++) {
        for (unsigned i = 0; i < segmentHitsSL[j].size(); i++) {
          const vector<TCSHit*>& s = segmentHitsSL[j];
          for (unsigned k = 0; k < s.size(); k++)
            s[k]->dump(dumpOption, TRGDebug::tab(4));
        }
      }
    }

    TRGDebug::leaveStage("Track Segment Finder");
  }


  void TRGCDCTrackSegmentFinder::simulateBoard(void)
  {

    //... Clear signal bundle...
    //    if(_tisb){
    //      for(unsigned i=0;i<_tisb->size();i++)
    //        delete (* _tisb)[i];
    //      delete _tisb;
    //    }
    if (_tosbE) {
      for (unsigned i = 0; i < _tosbE->size(); i++)
        delete(* _tosbE)[i];
      delete _tosbE;
    }
    if (_tosbT) {
      for (unsigned i = 0; i < _tosbT->size(); i++)
        delete(* _tosbT)[i];
      delete _tosbT;
    }

    //... Clock..
    const TRGClock& dClock = TRGCDC::getTRGCDC()->dataClock();

    //... Make input signal bundle
    TRGSignalVector inputv(name() + "inputMerger", dClock);
    // const string ni= name()+"InputSignalBundle";
    //    _tisb = new TRGSignalBundle(ni, dClock);

    //TRGSignalVector *inputM= new TRGSignalVector(*( (*(*this)[0]->output())[0]));
    vector <TRGSignalVector*> inputM;
    vector <TRGSignalVector*> findOUTTrack;
    vector <TRGSignalVector*> findOUTEvt;
    vector <int> tmpCTimeListE;
    vector <int> changeTimeListE;
    vector <int> tmpCTimeListT;
    vector <int> changeTimeListT;
    inputM.resize((*this).size());
    //yi for(unsigned i=0;i<(*this).size();i++){
    for (unsigned i = 0; i < nInput(); i++) {
      //yi    inputM[i] = new TRGSignalVector(*( (*(*this)[i]->output())[0]));
      inputM[i] = new TRGSignalVector(* (* input(i)->signal())[0]);
    }

    //    TRGSignalVector *inputM= new TRGSignalVector(*( (*(*this)[0]->output())[0]));

    for (unsigned i = 0; i < inputM[0]->size(); i++) {

      TRGSignal msig = (*(inputM[0]))[i];
      inputv += msig;
    }

    vector<TRGSignalVector*> separateTS;
    TRGSignalVector* clockCounter = new TRGSignalVector(inputv[236]);
    for (int i = 0; i < 8; i++)
      clockCounter->push_back(inputv[236 + i]);
    separateTS.resize(16);
    //separateTS.resize((*this).size()*16);
    int nTS = separateTS.size();
    findOUTTrack.resize(nTS);
    findOUTEvt.resize(nTS);
    if (type() == outerType) {
      for (int i = 0; i < nTS; i++) {
        separateTS[i] = new TRGSignalVector(inputv[i + 208]);
        /// HitMap
        if (i == 0)
          separateTS[i]->push_back(inputv[255]);
        else
          separateTS[i]->push_back(inputv[i - 1]);
        separateTS[i]->push_back(inputv[i]);
        if (i == nTS)
          separateTS[i]->push_back(inputv[255]);
        else
          separateTS[i]->push_back(inputv[i + 1]);
        if (i == 0)
          separateTS[i]->push_back(inputv[255]);
        else
          separateTS[i]->push_back(inputv[i + 15]);
        separateTS[i]->push_back(inputv[i + 16]);
        separateTS[i]->push_back(inputv[i + 32]);
        if (i == 0)
          separateTS[i]->push_back(inputv[255]);
        else
          separateTS[i]->push_back(inputv[i + 47]);
        separateTS[i]->push_back(inputv[i + 48]);
        if (i == 0)
          separateTS[i]->push_back(inputv[255]);
        else
          separateTS[i]->push_back(inputv[i + 63]);
        separateTS[i]->push_back(inputv[i + 64]);
        if (i == nTS)
          separateTS[i]->push_back(inputv[255]);
        else
          separateTS[i]->push_back(inputv[i + 65]);

        /// priority timing
        separateTS[i]->push_back(inputv[4 * i + 80]);
        separateTS[i]->push_back(inputv[4 * i + 81]);
        separateTS[i]->push_back(inputv[4 * i + 82]);
        separateTS[i]->push_back(inputv[4 * i + 83]);
        /// fastest timing
        separateTS[i]->push_back(inputv[4 * i + 144]);
        separateTS[i]->push_back(inputv[4 * i + 145]);
        separateTS[i]->push_back(inputv[4 * i + 146]);
        separateTS[i]->push_back(inputv[4 * i + 147]);
        /// clock counter part
        const TRGSignalVector& cc = dClock.clockCounter();
        //      cc.dump();
        separateTS[i]->push_back(cc[0]);
        separateTS[i]->push_back(cc[1]);
        separateTS[i]->push_back(cc[2]);
        separateTS[i]->push_back(cc[3]);
        separateTS[i]->push_back(cc[4]);

      }
      // Board Type Check Start.(for future)
      for (int i = 0; i < nTS; i++) {
        findOUTTrack[i] = findTSHit(separateTS[i], i)[0];
        vector<int> tt = findOUTTrack[i]->stateChanges();
        if (tt.size())
          tmpCTimeListT.insert(tmpCTimeListT.end(), tt.begin(), tt.end());

        findOUTEvt[i] = findTSHit(separateTS[i], i)[1];
        vector<int> ee = findOUTEvt[i]->stateChanges();
        if (ee.size())
          tmpCTimeListE.insert(tmpCTimeListE.end(), ee.begin(), ee.end());

        //...iw for debug...
        // dbgIn.push_back(separateTS[i]);
        // dbgOut.push_back(findOUTTrack[i]);
      }

      for (unsigned i = 0; i < tmpCTimeListT.size(); i++) {
        bool nomatch = true;
        for (unsigned j = 0; j < changeTimeListT.size(); j++) {
          if (tmpCTimeListT[i] == changeTimeListT[j]) {
            nomatch = false;
            break;
          }
        }
        if (nomatch) changeTimeListT.push_back(tmpCTimeListT[i]);
      }

      TRGSignalVector* trackerOut = packerOuterTracker(findOUTTrack,
                                                       changeTimeListT, 6);
      (*trackerOut).insert((*trackerOut).end(), (*clockCounter).begin(),
                           (*clockCounter).end());
      trackerOut->name("TSF TrackerOut");

      const string noT = name() + "OutputSignalBundleTracker";
      _tosbT =  new TRGSignalBundle(noT, dClock);
      _tosbT->push_back(trackerOut);
      if (_tosbT) {
        (*_tosbT)[0]->name(noT);
        for (unsigned i = 0; i < (*(*_tosbT)[0]).size(); i++)
          (*(*_tosbT)[0])[i].name(noT + ":bit" + TRGUtilities::itostring(i));
        //      _tosbT->dump("","");
      }

      for (unsigned i = 0; i < tmpCTimeListE.size(); i++) {
        bool nomatch = true;
        for (unsigned j = 0; j < changeTimeListE.size(); j++) {
          if (tmpCTimeListE[i] == changeTimeListE[j]) {
            nomatch = false;
            break;
          }
        }
        if (nomatch) changeTimeListE.push_back(tmpCTimeListE[i]);
      }

      TRGSignalVector* evtOut = packerOuterEvt(findOUTEvt, changeTimeListE, 6);
      (*evtOut).insert((*evtOut).end(), (*clockCounter).begin(),
                       (*clockCounter).end());

      const string noE = name() + "OutputSignalBundleEvt";
      _tosbE =  new TRGSignalBundle(noE, dClock);
      _tosbE->push_back(evtOut);
      if (_tosbE) {
        (*_tosbE)[0]->name(noE);
        for (unsigned i = 0; i < (*(*_tosbE)[0]).size(); i++)
          (*(*_tosbE)[0])[i].name(noE + ":bit" + TRGUtilities::itostring(i));
        //      _tosbE->dump("","");
      }
      // Board Type Check End.
    }

  }

  vector <TRGSignalVector*>
  TSFinder::findTSHit(TRGSignalVector* eachInput, int tsid)
  {

    //variables for common
    const string na = "TSCandidate" + TRGUtilities::itostring(
                        tsid) + " in " + name();
    TCSegment* tsi = _tsSL[tsid];
    vector <TRGSignalVector*> result;

    //variables for EvtTime & Low pT
    vector<bool> fTimeVect;
    //  int tmpFTime = 0 ;

    //variables for Tracker & N.N
    vector <bool> tmpOutBool;

    TRGSignalVector* resultT = new TRGSignalVector(na, eachInput->clock(), 22);
    TRGSignalVector* resultE = new TRGSignalVector(na, eachInput->clock(), 10);
    TRGSignalVector* Hitmap = new TRGSignalVector(na + "HitMap",
                                                  eachInput->clock(), 0);
    TRGSignalVector pTime(na + "PriorityTime", eachInput->clock(), 0);
    TRGSignalVector fTime(na + "FastestTime", eachInput->clock(), 0);
    for (unsigned i = 0; i < 12; i++) {
      Hitmap->push_back((*eachInput)[i]);
      (*Hitmap)[i].widen(16);
    }
    for (unsigned i = 0; i < 4; i++) {
      pTime.push_back((*eachInput)[i + 12]);
      fTime.push_back((*eachInput)[i + 16]);
    }
    for (unsigned i = 0; i < 5; i++) {
      pTime.push_back((*eachInput)[i + 20]);
      fTime.push_back((*eachInput)[i + 20]);
    }

    vector <int> changeTime = Hitmap->stateChanges();

    int* LUTValue = new int[changeTime.size()];
    if (changeTime.size()) {
      int hitPosition = 0;
      bool fTimeBool[10];
      int tmpPTime = 0 ;
      int tmpCTime = 0 ;
      int tmpOutInt;
      fTime.state(changeTime[0]).copy2bool(fTimeBool);
      fTimeBool[9] = true;
      fTimeVect.insert(fTimeVect.begin(), fTimeBool, fTimeBool + 10);
      //tmpFTime = mkint(fTime.state(changeTime[0]));
      bool eOUT = true;
      for (unsigned i = 0; i < changeTime.size(); i++) {
        LUTValue[i] = tsi->LUT()->getValue(mkint(Hitmap->state(changeTime[i])));

        /// output for EvtTime & Low pT tracker module
        if ((LUTValue[i]) && (eOUT)) {
          resultE->set(fTimeVect, changeTime[i]);
          eOUT = false;
        }

        bool priority1rise = (*Hitmap)[6].riseEdge(changeTime[i]);
        bool priority2rise = ((*Hitmap)[7].riseEdge(changeTime[i]) |
                              (*Hitmap)[8].riseEdge(changeTime[i]));

        /// output for Tracker & N.N
        //ready for output
        if (priority1rise) {
          hitPosition = 3;
          tmpPTime = mkint(pTime.state(changeTime[i]));
          tmpCTime = changeTime[i];
        } else if (priority2rise) {
          if (!hitPosition) {
            tmpPTime = mkint(pTime.state(changeTime[i]));
            tmpCTime = changeTime[i];
            if ((*Hitmap)[0].state(changeTime[i])) hitPosition = 2;
            else hitPosition = 1;
          }
        }

        // output selection
        if ((hitPosition) && (LUTValue[i]) && ((changeTime[i] - tmpCTime) < 16)) {
          tmpOutInt = tsid * pow(2, 13) + tmpPTime * pow(2, 4) + LUTValue[i] * pow(2,
                      2) + hitPosition;
          tmpOutBool = mkbool(tmpOutInt, 22);
          if (hitPosition == 3) {
            if (priority1rise) resultT->set(tmpOutBool, changeTime[i]);
            else {
              if ((LUTValue[i] == 1) | (LUTValue[i] == 2)) {
                if (!((LUTValue[i - 1] == 1) |
                      (LUTValue[i - 1] == 2)))
                  resultT->set(tmpOutBool, changeTime[i]);
              } else {
                if (!(LUTValue[i - 1])) resultT->set(tmpOutBool, changeTime[i]);
              }
            }
          } else {
            if (priority2rise) resultT->set(tmpOutBool, changeTime[i]);
            else {
              if ((LUTValue[i] == 1) | (LUTValue[i] == 2)) {
                if (!((LUTValue[i - 1] == 1) |
                      (LUTValue[i - 1] == 2)))
                  resultT->set(tmpOutBool, changeTime[i]);
              } else {
                if (!(LUTValue[i])) resultT->set(tmpOutBool, changeTime[i]);
              }
            }
          }
        }

      }

    }
    result.push_back(resultT);
    result.push_back(resultE);

    delete [] LUTValue;
    delete Hitmap;

    return result;
  }

  TRGSignalVector*
  TSFinder::packerOuterTracker(vector<TRGSignalVector*>& hitList,
                               vector<int>& cList,
                               const unsigned maxHit)
  {
    TRGSignalVector* result =
      new TRGSignalVector("", (* hitList[0]).clock() , 21 * maxHit);

    for (unsigned ci = 0; ci < cList.size(); ci++) {
      unsigned cntHit = 0;
      for (unsigned hi = 0; hi < hitList.size(); hi++) {
        TRGState s = (* hitList[hi]).state(cList[ci]);
        if (s.active()) {
          if (cntHit >= maxHit) continue;
          for (unsigned j = 0; j < 21; j++) {
            if ((* hitList[hi])[j].state(cList[ci])) {
              (* result)[21 * (maxHit - 1) - (cntHit * 21) + j]
              .set(cList[ci], cList[ci] + 1);
            }
          }
          if (TRGDebug::level()) {
            TRGState t = hitList[hi]->state(cList[ci]).subset(13, 9);
            cout << TRGDebug::tab() << " hit found : TSF out local ID="
                 << unsigned(t) << "(" << t << ")" << endl;
          }

          ++cntHit;
          //            result->dump("", "??? ");
        }
      }
    }

    return result;
  }

  TRGSignalVector*
  TSFinder::packerOuterEvt(vector<TRGSignalVector*> hitList, vector<int> cList,
                           int maxHit)
  {

    //TRGSignalVector * result = new TRGSignalVector("",(*hitList[0]).clock() ,N+9*maxHit);
    TRGSignalVector* result = new TRGSignalVector("", (*hitList[0]).clock() ,
                                                  hitList.size() + 9 * maxHit);

    for (unsigned ci = 0; ci < cList.size(); ci++) {
      int cntHit = 0;
      for (unsigned hi = 0; hi < hitList.size(); hi++) {
        if ((*hitList[hi]).state(cList[ci]).active()) {
          (*result)[9 * maxHit + hi].set(cList[ci], cList[ci] + 1);
          if (cntHit >= maxHit) continue;
          for (unsigned j = 0; j < (((*hitList[hi])).size() - 1); j++) {
            if ((*hitList[hi])[j].state(cList[ci]))
              (*result)[9 * (maxHit - 1) - (cntHit * 9) + j].set(cList[ci], cList[ci] + 1);
          }
          cntHit++;
        }
      }
    }
    return result;
  }

  double
  TSFinder::mkint(TRGState bitInput)
  {
    double r = 0;
    bool* binput = new bool[bitInput.size()];
    bitInput.copy2bool(binput);
    for (unsigned i = 0; i < bitInput.size(); i++) {
      if (binput[i])
        r += pow(2, i);
    }
    delete[] binput;
    return r;
  }

  vector<bool>
  TSFinder::mkbool(int N, int bitSize)
  {
    vector<bool> boutput;
    boutput.resize(bitSize);
    int tmpint = N;
    for (unsigned i = 0; tmpint; i++) {
      if (tmpint % 2) boutput[i] = true;
      else boutput[i] = false;
      tmpint = tmpint / 2;
    }

    return boutput;
  }

//

  void TRGCDCTrackSegmentFinder::saveTSInformation(std::vector<TRGCDCSegment*>&
                                                   tss)
  {

    TClonesArray& hitPatternInformation = *m_hitPatternInformation;
    hitPatternInformation.Clear();

    StoreArray<CDCSimHit> SimHits;
    if (! SimHits.getEntries()) {
      //cout << "CDCTRGTSF !!! can not access to CDCSimHits" << endl;
      return;
    }
    StoreArray<MCParticle> mcParticles;
    if (! mcParticles.getEntries()) {
      //cout << "CDCTRGTSF !!! can not access to MCParticles" << endl;
      return;
    }

    //cout<<"Save TS information"<<endl;
    // Loop over all TSs. wireHit has only one wireSimHit. Meaning there is a particle overlap hit bug.
    const unsigned nTSs = tss.size();
    unsigned nHitTSs = 0;
    for (unsigned iTS = 0; iTS < nTSs; iTS++) {
      const TCSegment& ts = * tss[iTS];
      const TRGCDCWire* priority;
      const TRGCDCWire* secondPriorityL;
      const TRGCDCWire* secondPriorityR;
      // Find priority wires
      if (ts.wires().size() == 15) {
        priority = ts.wires()[0];
        secondPriorityR = ts.wires()[1];
        secondPriorityL = ts.wires()[2];
      } else {
        priority = ts.wires()[5];
        secondPriorityR = ts.wires()[6];
        secondPriorityL = ts.wires()[7];
      }
      // Find L/R, phi of priority wires
      vector<int> priorityLRs(3);
      vector<float> priorityPhis(3);
      TVector3 posOnTrack;
      TVector3 posOnWire;
      if (priority->hit() != 0) {
        int iSimHit = priority->hit()->iCDCSimHit();
        priorityLRs[0] = SimHits[iSimHit]->getPosFlag();
        posOnTrack = SimHits[iSimHit]->getPosTrack();
        posOnWire = SimHits[iSimHit]->getPosWire();
        priorityPhis[0] = (posOnTrack - posOnWire).Phi() + m_Trg_PI / 2 -
                          posOnWire.Phi();
        //cout<<ts.name()<<endl;
        //cout<<"Track: "<<posOnTrack.x()<<" "<<posOnTrack.y()<<" "<<posOnTrack.z()<<endl;
        //cout<<"Wire:  "<<posOnWire.x()<<" "<<posOnWire.y()<<" "<<posOnWire.z()<<endl;
        //cout<<"Before Phi: "<<(posOnTrack - posOnWire).Phi()<<" PosOnWirePhi: "<<posOnWire.Phi()<<" After Phi: "<<priorityPhis[0]<<endl;
        //cout<<"LR: "<<priorityLRs[0]<<endl;
      } else {
        priorityLRs[0] = -1;
        priorityPhis[0] = 9999;
      }
      if (secondPriorityR->hit() != 0) {
        int iSimHit = secondPriorityR->hit()->iCDCSimHit();
        priorityLRs[1] = SimHits[iSimHit]->getPosFlag();
        posOnTrack = SimHits[iSimHit]->getPosTrack();
        posOnWire = SimHits[iSimHit]->getPosWire();
        priorityPhis[1] = (posOnTrack - posOnWire).Phi() + m_Trg_PI / 2 -
                          posOnWire.Phi();
      } else {
        priorityLRs[1] = -1;
        priorityPhis[1] = 9999;
      }
      if (secondPriorityL->hit() != 0) {
        int iSimHit = secondPriorityL->hit()->iCDCSimHit();
        priorityLRs[2] = SimHits[iSimHit]->getPosFlag();
        posOnTrack = SimHits[iSimHit]->getPosTrack();
        posOnWire = SimHits[iSimHit]->getPosWire();
        priorityPhis[2] = (posOnTrack - posOnWire).Phi() + m_Trg_PI / 2 -
                          posOnWire.Phi();
      } else {
        priorityLRs[2] = -1;
        priorityPhis[2] = 9999;
      }

      const unsigned nWires = ts.wires().size();
      unsigned nHitWires = 0;
      // Find TSPatternInformation for each particle
      map<int, unsigned> particleTSPattern;
      // Loop over wires in TS
      for (unsigned iWire = 0; iWire < nWires; iWire++) {
        //...Copy signal from a wire...
        const TRGSignal& wireSignal = ts.wires()[iWire]->signal();
        if (wireSignal.active()) ++nHitWires;
        // Find MC particle of hit wire
        const TRGCDCWireHit* wireHit = ts.wires()[iWire]->hit();
        if (wireHit != 0) {
          MCParticle* particle = SimHits[wireHit->iCDCSimHit()]->getRelatedFrom<MCParticle>();
          int iMCParticle = (particle) ? particle->getArrayIndex() : mcParticles.getEntries();
          // If new particle
          if (particleTSPattern[iMCParticle] == 0) {
            particleTSPattern[iMCParticle] = 1 << iWire;
          } else {
            particleTSPattern[iMCParticle] |= 1 << iWire;
          }
        }
      } // Loop over wires in TS

      // Print pattern for each particle
      //for(map<int, unsigned >::const_iterator it = particleTSPattern.begin(); it != particleTSPattern.end(); it++ ) {
      //  bitset<15> printPattern((*it).second);
      //  cout<<ts.name()<<" MC Particle: "<< (*it).first <<" pattern: "<<printPattern<<endl;
      //}

      if (nHitWires != 0) {
        // Ignore TSPatterns that have 2 particles passing TS.
        if (particleTSPattern.size() == 1) {
          map<int, unsigned>::const_iterator it = particleTSPattern.begin();
          bitset<15> printPattern((*it).second);
          //cout<<ts.name()<<" MC Particle: "<< (*it).first <<" pattern: "<<printPattern<<endl;
          //cout<<(*it).first<<" "<<ts.superLayerId()<<" "<<printPattern<<endl;
          // Save TS Pattern information
          TVectorD tsPatternInformation(9);
          tsPatternInformation[0] = (*it).first;
          tsPatternInformation[1] = ts.superLayerId();
          tsPatternInformation[2] = double((*it).second);
          tsPatternInformation[3] = priorityLRs[0];
          tsPatternInformation[4] = priorityPhis[0];
          tsPatternInformation[5] = priorityLRs[1];
          tsPatternInformation[6] = priorityPhis[1];
          tsPatternInformation[7] = priorityLRs[2];
          tsPatternInformation[8] = priorityPhis[2];
          new(hitPatternInformation[nHitTSs++]) TVectorD(tsPatternInformation);
        }
        //cout<<ts.name()<<" has "<<nHitWires<<" hit wires."<<endl;
      }
    } // End of loop over all TSs
    if (m_makeRootFile) m_treeInputTSF->Fill();
    //cout<<"End saving TS information"<<endl;
  } // End of save function



  void TRGCDCTrackSegmentFinder::saveTSFResults(std::vector<TRGCDCSegmentHit*>*
                                                segmentHitsSL)
  {

    TClonesArray& particleEfficiency = *m_particleEfficiency;
    TClonesArray& tsInformation = *m_tsInformation;
    particleEfficiency.Clear();
    tsInformation.Clear();

    int mcInformation = 1;
    ///// Find efficiency for each particle.
    StoreArray<CDCSimHit> SimHits;
    if (! SimHits) {
      //cout << "CDCTRGTSF !!! can not access to CDCSimHits" << endl;
      mcInformation = 0;
    }
    StoreArray<MCParticle> mcParticles;
    if (! mcParticles) {
      //cout << "CDCTRGTSF !!! can not access to MCParticles" << endl;
      mcInformation = 0;
    }
    if (mcInformation) {
      RelationArray cdcSimHitRel(mcParticles, SimHits);
      // Make map for hit to mcParticle
      map<int, int> simHitsMCParticlesMap;
      // Loop over all particles
      for (int iPart = 0; iPart < cdcSimHitRel.getEntries(); iPart++) {
        // Loop over all hits for particle
        for (unsigned iHit = 0; iHit < cdcSimHitRel[iPart].getToIndices().size();
             iHit++) {
          //cout<<"From: "<<cdcSimHitRel[iPart].getFromIndex()<<" To: "<<cdcSimHitRel[iPart].getToIndex(iHit)<<endl;
          simHitsMCParticlesMap[cdcSimHitRel[iPart].getToIndex(iHit)] =
            cdcSimHitRel[iPart].getFromIndex();
        }
      }
      //// Print map
      //for(map<int, int >::iterator it = simHitsMCParticlesMap.begin(); it != simHitsMCParticlesMap.end(); it++) {
      //  cout<<"SimHit Index: "<<(*it).first<<" MCParticle Index: "<<(*it).second<<endl;
      //}
      // Find efficiency for each particle
      // Find hit TS SuperLayer for particles
      // particleNHiTS[iMCParticle] = hitTSSL
      map<int, unsigned> particleNHitTS;
      // Loop over all hit TSs
      for (int iSuperLayer = 0; iSuperLayer < 9; iSuperLayer++) {
        // map<int, bool> particleHitTS;
        for (unsigned iTS = 0; iTS < segmentHitsSL[iSuperLayer].size(); iTS++) {
          const TCSegment& ts = segmentHitsSL[iSuperLayer][iTS]->segment();
          unsigned nWires = ts.wires().size();
          for (unsigned iWire = 0; iWire < nWires; iWire++) {
            const TRGCDCWireHit* wireHit = ts.wires()[iWire]->hit();
            if (wireHit != 0) {
              int iMCParticle = simHitsMCParticlesMap[wireHit->iCDCSimHit()];
              if (particleNHitTS[iMCParticle] == 0) {
                unsigned hitTSSL;
                hitTSSL = 1 << iSuperLayer;
                particleNHitTS[iMCParticle] = hitTSSL;
              } else
                particleNHitTS[iMCParticle] |= 1 << iSuperLayer;
              //cout<<ts.name()<<" "<<ts.wires()[iWire]->name()<<" was hit.";
              //cout<<" Particle was "<<simHitsMCParticlesMap[wireHit->iCDCSimHit()]<<endl;
            } // If wire is hit
          } // End of loop over all wires in TS
        } // End of loop over all TS in super layer
      } // End of loop over all hit TSs
      //// Print all results
      //for( map<int, unsigned>::const_iterator it = particleNHitTS.begin(); it != particleNHitTS.end(); it++) {
      //  bitset<9> printSuperLayers((*it).second);
      //  cout<<"MC particle: "<<(*it).first<<" HitSuperLayers: "<<printSuperLayers<<" nCount: "<<printSuperLayers.count()<<endl;
      //}

      // Find last CDC hit for each MC particle
      // tsEfficiency[i][tsEfficiency,particle pT,#MCTS]
      vector<vector<float>> tsEfficiency;
      // Loop over all particles
      for (int iPart = 0; iPart < cdcSimHitRel.getEntries(); iPart++) {
        int lastWireHit = -1;
        // Loop over all hits for particle
        for (unsigned iHit = 0; iHit < cdcSimHitRel[iPart].getToIndices().size();
             iHit++) {
          int iSimHit = cdcSimHitRel[iPart].getToIndex(iHit);
          if (SimHits[iSimHit]->getWireID().getICLayer() > lastWireHit) lastWireHit =
              SimHits[iSimHit]->getWireID().getICLayer();
          //cout<<"Particle: "<<cdcSimHitRel[iPart].getFromIndex()<<" CDCSimHit: "<<iSimHit<<endl;
          //cout<<"SuperLayer: "<<SimHits[iSimHit]->getWireID().getISuperLayer()<<" wireLayer: "<<SimHits[iSimHit]->getWireID().getICLayer()<<endl;
        }
        //cout<<"iMCParticle: "<<cdcSimHitRel[iPart].getFromIndex()<<" Last wire Hit: "<<lastWireHit<<endl;
        // Calculate last superlayer
        int lastSLHit = 0;
        if (lastWireHit >= 53) lastSLHit = 9;
        else if (lastWireHit >= 47) lastSLHit = 8;
        else if (lastWireHit >= 41) lastSLHit = 7;
        else if (lastWireHit >= 35) lastSLHit = 6;
        else if (lastWireHit >= 29) lastSLHit = 5;
        else if (lastWireHit >= 23) lastSLHit = 4;
        else if (lastWireHit >= 17) lastSLHit = 3;
        else if (lastWireHit >= 11) lastSLHit = 2;
        else if (lastWireHit >= 5) lastSLHit = 1;
        // Get number of hit TS for particle
        int iMCParticle = cdcSimHitRel[iPart].getFromIndex();
        bitset<9> hitSuperLayers(particleNHitTS[iMCParticle]);
        int numberHitSuperLayers = hitSuperLayers.count();
        //cout<<"iMCParticle: "<< iMCParticle << " # hit TS: "<<numberHitSuperLayers<<" MC # TS: "<<lastSLHit<<endl;
        float mcPt = mcParticles[iMCParticle]->getMomentum().Perp();
        float efficiency;
        if (lastSLHit == 0) efficiency = -1;
        else efficiency = float(numberHitSuperLayers) / lastSLHit;
        //cout<<"Efficiency: "<<float(numberHitSuperLayers)/lastSLHit<<" MC pT: "<<mcPt<<endl;
        vector<float> tempEfficiency;
        tempEfficiency.resize(3);
        tempEfficiency[0] = efficiency;
        tempEfficiency[1] = mcPt;
        tempEfficiency[2] = lastSLHit;
        tsEfficiency.push_back(tempEfficiency);
      } // End of looping over all particles
      //// Print all TS efficiency
      //for(unsigned iEfficiency=0; iEfficiency<tsEfficiency.size(); iEfficiency++) {
      //  cout<<"Efficiency: "<<tsEfficiency[iEfficiency][0]<<" Pt: "<<tsEfficiency[iEfficiency][1]<<" #MCTS: "<<tsEfficiency[iEfficiency][2]<<endl;
      //}

      // Save TS efficiency for each particle
      for (unsigned iEfficiency = 0; iEfficiency < tsEfficiency.size();
           iEfficiency++) {
        TVectorD t_particleEfficiency(3);
        t_particleEfficiency[0] = tsEfficiency[iEfficiency][0];
        t_particleEfficiency[1] = tsEfficiency[iEfficiency][1];
        t_particleEfficiency[2] = tsEfficiency[iEfficiency][2];
        new(particleEfficiency[iEfficiency]) TVectorD(t_particleEfficiency);
      }

    } // End of no MC information

    // Save TS information
    // [FIXME] Doesn't work when second only priority is hit.
    // Loop over all hit TSs
    int iHitTS = 0;
    for (int iSuperLayer = 0; iSuperLayer < 9; iSuperLayer++) {
      for (unsigned iTS = 0; iTS < segmentHitsSL[iSuperLayer].size(); iTS++) {
        const TCSegment& ts = segmentHitsSL[iSuperLayer][iTS]->segment();
        //unsigned nWires = ts.wires().size();
        unsigned iWire = 5;
        if (iSuperLayer == 0) iWire = 0;
        const TRGCDCWireHit* wireHit = ts.wires()[iWire]->hit();
        if (wireHit != 0) {
          //cout<<"[TSF]: "<<ts.name()<<" was hit at ";
          unsigned nHits = ts.wires()[iWire]->signal().nSignals();
          for (unsigned iHit = 0; iHit < nHits; iHit++) {
            TVectorD tempTSInformation(3);
            tempTSInformation[0] = iSuperLayer;
            tempTSInformation[1] = ts.localId();
            tempTSInformation[2] = ts.wires()[iWire]->signal().stateChanges()[iHit];
            new(tsInformation[iHitTS++]) TVectorD(tempTSInformation);
            //cout<<ts.wires()[iWire]->signal().stateChanges()[iHit]<<", ";
            iHit++;
          }
          //cout<<endl;
        } // If wire is hit
      } // End of loop over all TS in super layer
    } // End of loop over all hit TSs

    if (m_makeRootFile) m_treeOutputTSF->Fill();

  } // End of saving TSF results



  void TRGCDCTrackSegmentFinder::saveNNTSInformation(
    std::vector<TRGCDCSegment*>& tss)
  {

    StoreArray<CDCSimHit> SimHits;
    if (! SimHits) {
      //cout << "CDCTRGTSF !!! can not access to CDCSimHits" << endl;
      return;
    }

    TClonesArray& nnPatternInformation = *m_nnPatternInformation;
    nnPatternInformation.Clear();

    // Save Timing information in ROOT file. Fill for each TS.
    // Loop over all TSs. wireHit has only one wireSimHit. Meaning there is a particle overlap hit bug.
    const unsigned nTSs = tss.size();
    unsigned indexSaving = 0;
    for (unsigned iTS = 0; iTS < nTSs; iTS++) {
      // If TS is hit
      const TCSegment& ts = * tss[iTS];
      const TCSHit* tsHit = ts.hit();
      if (tsHit) {
        const TRGCDCWire* priority;
        if (ts.wires().size() == 15) priority = ts.wires()[0];
        else priority = ts.wires()[5];
        // If priority wire is hit
        if (priority->hit()) {

          // Calculate timeWires
          // Fill wire timing. Not hit = 9999. Unit is ns.
          vector<float> wireTime;
          if (ts.superLayerId() == 0)
            wireTime.resize(15);
          else
            wireTime.resize(11);
          // Loop over all wires
          //cout<<ts.name();
          const unsigned nWires = ts.wires().size();
          for (unsigned iWire = 0; iWire < nWires; iWire++) {
            const TRGCDCWire* wire = ts.wires()[iWire];
            const TRGCDCWireHit* wireHit = ts.wires()[iWire]->hit();
            // If wire is hit
            if (wireHit) {
              // Only check first change. This could become a bug.
              wireTime[iWire] = wire->signal().stateChanges()[0];
              //cout<<ts.wires()[iWire]->name()<<" Clock: "<< ts.wires()[iWire]->signal().clock().frequency()<<" Drift lenght: "<<wireHit->drift()<<" Drift time: "<<ts.wires()[iWire]->signal().stateChanges()[0]<<endl;
            } else
              wireTime[iWire] = 9999;
            //cout<<" "<<wire->name();
          } // End loop over all wires
          //cout<<endl;
          //// Print all time of wires.
          //for(unsigned iWire = 0; iWire < wireTime.size(); iWire++) {
          //  cout<<"Wire: "<<iWire<<" Time: "<<wireTime[iWire]<<endl;
          //}
          // Get additional wire information for 6th layer
          //cout<<" JB: "<<ts.wires().back()->layerId()<<" "<<ts.wires().back()->localId()<<endl;
          int lastLayer = ts.wires().back()->layerId();
          int lastWire = ts.wires().back()->localId();
          int nWiresLayer = _cdc.layer(lastLayer + 1)->nCells();
          if (nWires == 15) {
            for (unsigned iWire = 0; iWire < 6; iWire++) {
              int wireIndex = lastWire - 4 + iWire;
              if (wireIndex < 0) wireIndex += nWiresLayer;
              if (wireIndex >= nWiresLayer) wireIndex -= nWiresLayer;
              //cout<<"Call: "<<(*_cdc.layer(lastLayer+1))[wireIndex]->localId()<<endl;
              const TRGCDCCell* wire = (*_cdc.layer(lastLayer + 1))[wireIndex];
              const TRGCDCCellHit* wireHit = wire->hit();
              // If wire is hit
              if (wireHit) {
                // Only check first change. This could become a bug.
                wireTime.push_back(wire->signal().stateChanges()[0]);
                //cout<<ts.wires()[iWire]->name()<<" Clock: "<< ts.wires()[iWire]->signal().clock().frequency()<<" Drift lenght: "<<wireHit->drift()<<" Drift time: "<<ts.wires()[iWire]->signal().stateChanges()[0]<<endl;
              } else
                wireTime.push_back(9999);
            } // Loop over all extra wires
          } else {
            for (unsigned iWire = 0; iWire < 5; iWire++) {
              int wireIndex = lastWire - 3 + iWire;
              if (wireIndex < 0) wireIndex += nWiresLayer;
              if (wireIndex >= nWiresLayer) wireIndex -= nWiresLayer;
              //cout<<"Call: "<<(*_cdc.layer(lastLayer+1))[wireIndex]->localId()<<endl;
              const TRGCDCCell* wire = (*_cdc.layer(lastLayer + 1))[wireIndex];
              const TRGCDCCellHit* wireHit = wire->hit();
              // If wire is hit
              if (wireHit) {
                // Only check first change. This could become a bug.
                wireTime.push_back(wire->signal().stateChanges()[0]);
                //cout<<ts.wires()[iWire]->name()<<" Clock: "<< ts.wires()[iWire]->signal().clock().frequency()<<" Drift lenght: "<<wireHit->drift()<<" Drift time: "<<ts.wires()[iWire]->signal().stateChanges()[0]<<endl;
              } else
                wireTime.push_back(9999);
            } // Loop over all extra wires
          }
          //ts.wires()[14]->layerId(), localId()
          //_cdc._layers[0][localId]->signal().stateChanges()[0]


          //// Find fastest timing
          //float fastestTime = 9999;
          //for(unsigned iWire=0; iWire<wireTime.size();iWire++) {
          //  if (fastestTime > wireTime[iWire]) fastestTime = wireTime[iWire];
          //}
          //// Calculate relative timing
          //for(unsigned iWire=0; iWire<wireTime.size();iWire++) {
          //  if(wireTime[iWire] != 9999)  wireTime[iWire] -= fastestTime;
          //}

          //// Print all time of wires.
          //for(unsigned iWire = 0; iWire < wireTime.size(); iWire++) {
          //  cout<<"Wire: "<<iWire<<" Time: "<<wireTime[iWire]<<endl;
          //}
          //cout<<ts.name()<<" is found and priority wire is hit"<<endl;
          // Calculate mc result
          float mcLRDriftTime = priority->signal().stateChanges()[0];
          if (priority->hit()->mcLR()) mcLRDriftTime *= -1;
          //cout<<ts.name()<<" LRDriftTime: "<<mcLRDriftTime<<endl;

          // Save timing information in Root
          TVectorD t_nnPatternInformation;
          if (ts.superLayerId() == 0) {
            //t_nnPatternInformation.ResizeTo(17);
            t_nnPatternInformation.ResizeTo(23);
            t_nnPatternInformation[0] = ts.superLayerId();
            t_nnPatternInformation[1] = mcLRDriftTime;
            //for(unsigned iWire=0; iWire<15; iWire++){
            for (unsigned iWire = 0; iWire < 21; iWire++)
              t_nnPatternInformation[iWire + 2] = wireTime[iWire];
            new(nnPatternInformation[indexSaving++]) TVectorD(t_nnPatternInformation);
          } else {
            //t_nnPatternInformation.ResizeTo(13);
            t_nnPatternInformation.ResizeTo(17);
            t_nnPatternInformation[0] = ts.superLayerId();
            t_nnPatternInformation[1] = mcLRDriftTime;
            //for(unsigned iWire=0; iWire<11; iWire++){
            for (unsigned iWire = 0; iWire < 15; iWire++)
              t_nnPatternInformation[iWire + 2] = wireTime[iWire];
            new(nnPatternInformation[indexSaving++]) TVectorD(t_nnPatternInformation);
          }

        } // End of if priority cell is hit
      } // End of if TS is hit
    } // End loop of all TSs

    if (m_makeRootFile) m_treeNNTSF->Fill();

  } // End of save function

  void
  TRGCDCTrackSegmentFinder::push_back(const TRGCDCMerger* a)
  {
    std::vector<const TRGCDCMerger*>::push_back(a);
  }


  void
  TRGCDCTrackSegmentFinder::simulateOuter(void)
  {

    const string sn = "TSF::simulateOuter : " + name();
    TRGDebug::enterStage(sn);

    //...Loop over mergers to create a super layer hit map...
    for (unsigned m = 0; m < nInput(); m++) {
      TRGSignalBundle* b = input(m)->signal();

      for (unsigned i = 0; i < 16; i++) {
        _secMap.push_back(& ((* b)[0][0][208 + i]));
        for (unsigned j = 0; j < 5; j++)
          _hitMap[j].push_back(& ((* b)[0][0][j * 16 + i]));
        for (unsigned j = 0; j < 4; j++)
          _priMap.push_back(& ((* b)[0][0][80 + i * 4 + j]));
        for (unsigned j = 0; j < 4; j++)
          _fasMap.push_back(& ((* b)[0][0][144 + i * 4 + j]));
      }

      for (unsigned i = 0; i < 4; i++)
        _edg0Map.push_back(& ((* b)[0][0][224 + i]));
      for (unsigned i = 0; i < 4; i++)
        _edg1Map.push_back(& ((* b)[0][0][224 + 4 + i]));
      for (unsigned i = 0; i < 4; i++)
        _edg2Map.push_back(& ((* b)[0][0][224 + 8 + i]));
    }

    //...Storage preparation...
    const unsigned nTSF = nInput() * 16;
    vector<TRGSignalVector*> trker[4];
    vector<int> tsfStateChanges;

    //...Form a TSF...
    for (unsigned t = 0; t < nTSF; t++) {

      const string n = name() + "-" + TRGUtilities::itostring(t);
      TRGSignalVector* s = new TRGSignalVector(n, clockData());
      _tsfIn.push_back(s);

      s->push_back(* _secMap[t]);

      if (t == 0) {
        s->push_back(* (_hitMap[0][nTSF - 1]));
        s->push_back(* (_hitMap[0][0]));
        s->push_back(* (_hitMap[0][1]));
        s->push_back(* (_hitMap[1][nTSF - 1]));
        s->push_back(* (_hitMap[1][0]));
        s->push_back(* (_hitMap[2][0]));
        s->push_back(* (_hitMap[3][nTSF - 1]));
        s->push_back(* (_hitMap[3][0]));
        s->push_back(* (_hitMap[4][nTSF - 1]));
        s->push_back(* (_hitMap[4][0]));
        s->push_back(* (_hitMap[4][1]));
      } else if (t == (nTSF - 1)) {
        s->push_back(* (_hitMap[0][nTSF - 2]));
        s->push_back(* (_hitMap[0][nTSF - 1]));
        s->push_back(* (_hitMap[0][0]));
        s->push_back(* (_hitMap[1][nTSF - 2]));
        s->push_back(* (_hitMap[1][nTSF - 1]));
        s->push_back(* (_hitMap[2][nTSF - 1]));
        s->push_back(* (_hitMap[3][nTSF - 2]));
        s->push_back(* (_hitMap[3][nTSF - 1]));

        s->push_back(* (_hitMap[4][nTSF - 2]));
        s->push_back(* (_hitMap[4][nTSF - 1]));
        s->push_back(* (_hitMap[4][0]));
      } else {
        s->push_back(* (_hitMap[0][t - 1]));
        s->push_back(* (_hitMap[0][t]));
        s->push_back(* (_hitMap[0][t + 1]));
        s->push_back(* (_hitMap[1][t - 1]));
        s->push_back(* (_hitMap[1][t]));
        s->push_back(* (_hitMap[2][t]));
        s->push_back(* (_hitMap[3][t - 1]));
        s->push_back(* (_hitMap[3][t]));
        s->push_back(* (_hitMap[4][t - 1]));
        s->push_back(* (_hitMap[4][t]));
        s->push_back(* (_hitMap[4][t + 1]));
      }

      //...Priority timing...
      s->push_back(* _priMap[t * 4 + 0]);
      s->push_back(* _priMap[t * 4 + 1]);
      s->push_back(* _priMap[t * 4 + 2]);
      s->push_back(* _priMap[t * 4 + 3]);

      //...Priority timing...
      priorityTiming(t, nTSF, * s, (* s)[6], (* s)[7], (* s)[8]);

      //...Fastest timing...
      fastestTimingOuter(t, nTSF, * s);

      //...Clock counter is omitted...

      //...Simulate TSF...
      vector<TRGSignalVector*> result = simulateOuter(s, t);
      _toBeDeleted.push_back(result[1]); // Event timing omitted now

      TRGSignalVector* forTracker0 = result[0];
      TRGSignalVector* forTracker1 = new TRGSignalVector(* forTracker0);

      _tsfOut.push_back(forTracker0);
      _tsfOut.push_back(forTracker1);


      //...State change list...
      vector<int> sc = forTracker0->stateChanges();
      for (unsigned i = 0; i < sc.size(); i++) {
        bool skip = false;
        for (unsigned j = 0; j < tsfStateChanges.size(); j++) {
          if (tsfStateChanges[j] == sc[i]) {
            skip = true;
            break;
          }
        }
        if (! skip) tsfStateChanges.push_back(sc[i]);
      }

      //...Store it for each tracker division...
      const unsigned pos = t / (nTSF / 4);
      if (pos == 0) {
        addID(* forTracker0, t + (nTSF / 4));
        addID(* forTracker1, t);
        trker[3].push_back(forTracker0);
        trker[0].push_back(forTracker1);
      } else if (pos == 1) {
        addID(* forTracker0, t);
        addID(* forTracker1, t - 1 * (nTSF / 4));
        trker[0].push_back(forTracker0);
        trker[1].push_back(forTracker1);
      } else if (pos == 2) {
        addID(* forTracker0, t - 1 * (nTSF / 4));
        addID(* forTracker1, t - 2 * (nTSF / 4));
        trker[1].push_back(forTracker0);
        trker[2].push_back(forTracker1);
      } else {
        addID(* forTracker0, t - 2 * (nTSF / 4));
        addID(* forTracker1, t - 3 * (nTSF / 4));
        trker[2].push_back(forTracker0);
        trker[3].push_back(forTracker1);
      }

      if (TRGDebug::level())
        if (forTracker0->active())
          cout << TRGDebug::tab() << "hit found : TSF out="
               << t << endl;
    }

    //...Sort state changes...
    std::sort(tsfStateChanges.begin(), tsfStateChanges.end());

    //...Output for 2D...
    for (unsigned i = 0; i < 4; i++) {
      string n = name() + "-trker" + TRGUtilities::itostring(i);
      TRGSignalVector* tOut = packerOuterTracker(trker[i],
                                                 tsfStateChanges,
                                                 20);
      tOut->name(n);
      TRGSignalBundle* b = new TRGSignalBundle(n, clockData());
      b->push_back(tOut);
      output(i)->signal(b);
      _toBeDeleted.push_back(tOut);

      if (TRGCDC::getTRGCDC()->firmwareSimulationMode() & 0x4)
        b->dumpCOE("",
                   TRGCDC::getTRGCDC()->firmwareSimulationStartDataClock(),
                   TRGCDC::getTRGCDC()->firmwareSimulationStopDataClock());

      //      b->dump();
    }

    TRGDebug::leaveStage(sn);
  }

  vector <TRGSignalVector*>
  TSFinder::simulateOuter(TRGSignalVector* in, unsigned tsid)
  {

    //variables for common
    const string na = "TSCandidate" + TRGUtilities::itostring(tsid) + " in " +
                      name();
    TCSegment* tsi = _tsSL[tsid];
    vector <TRGSignalVector*> result;

    //variables for EvtTime & Low pT
    vector<bool> fTimeVect;
    //  int tmpFTime = 0 ;

    //variables for Tracker & N.N
    vector <bool> tmpOutBool;

    //iwTRGSignalVector* resultT = new TRGSignalVector(na, in->clock(),22);
    TRGSignalVector* resultT = new TRGSignalVector(na, in->clock(), 13);
    TRGSignalVector* resultE = new TRGSignalVector(na, in->clock(), 10);
    TRGSignalVector* Hitmap = new TRGSignalVector(na + "HitMap", in->clock(), 0);
    TRGSignalVector pTime(na + "PriorityTime", in->clock(), 0);
    TRGSignalVector fTime(na + "FastestTime", in->clock(), 0);
    for (unsigned i = 0; i < 12; i++) {
      Hitmap->push_back((*in)[i]);
      (*Hitmap)[i].widen(16);
    }
    for (unsigned i = 0; i < 4; i++) {
      pTime.push_back((*in)[i + 12]);
      fTime.push_back((*in)[i + 16]);
    }

    //...Clock counter...
    const TRGSignalVector& cc = in->clock().clockCounter();
    for (unsigned i = 0; i < 5; i++) {
      pTime.push_back(cc[i]);
      fTime.push_back(cc[i]);
    }

    vector <int> changeTime = Hitmap->stateChanges();

    int* LUTValue = new int[changeTime.size()];
    if (changeTime.size()) {
      int hitPosition = 0;
      bool fTimeBool[10];
      int tmpPTime = 0 ;
      int tmpCTime = 0 ;
      int tmpOutInt;
      fTime.state(changeTime[0]).copy2bool(fTimeBool);
      fTimeBool[9] = true;
      fTimeVect.insert(fTimeVect.begin(), fTimeBool, fTimeBool + 10);
      //tmpFTime = mkint(fTime.state(changeTime[0]));
      bool eOUT = true;
      for (unsigned i = 0; i < changeTime.size(); i++) {
        LUTValue[i] = tsi->LUT()->getValue(mkint(Hitmap->state(changeTime[i])));

        /// output for EvtTime & Low pT tracker module
        if ((LUTValue[i]) && (eOUT)) {
          resultE->set(fTimeVect, changeTime[i]);
          eOUT = false;
        }

        bool priority1rise = (*Hitmap)[6].riseEdge(changeTime[i]);
        bool priority2rise = ((*Hitmap)[7].riseEdge(changeTime[i]) |
                              (*Hitmap)[8].riseEdge(changeTime[i]));

        /// output for Tracker & N.N
        //ready for output
        if (priority1rise) {
          hitPosition = 3;
          tmpPTime = mkint(pTime.state(changeTime[i]));
          tmpCTime = changeTime[i];
        } else if (priority2rise) {
          if (!hitPosition) {
            tmpPTime = mkint(pTime.state(changeTime[i]));
            tmpCTime = changeTime[i];
            if ((*Hitmap)[0].state(changeTime[i])) hitPosition = 2;
            else hitPosition = 1;
          }
        }

        // output selection
        if ((hitPosition) && (LUTValue[i]) && ((changeTime[i] - tmpCTime) < 16)) {
          //iw            tmpOutInt = tsid * pow(2, 13) + tmpPTime * pow(2, 4) +
          //iw                LUTValue[i] * pow(2,2) + hitPosition;
          tmpOutInt = tmpPTime * pow(2, 4) +
                      LUTValue[i] * pow(2, 2) + hitPosition;
          tmpOutBool = mkbool(tmpOutInt, 13);  // ID removed : iw
          if (hitPosition == 3) {
            if (priority1rise) resultT->set(tmpOutBool, changeTime[i]);
            else {
              if ((LUTValue[i] == 1) | (LUTValue[i] == 2)) {
                if (!((LUTValue[i - 1] == 1) |
                      (LUTValue[i - 1] == 2)))
                  resultT->set(tmpOutBool, changeTime[i]);
              } else {
                if (!(LUTValue[i - 1])) resultT->set(tmpOutBool, changeTime[i]);
              }
            }
          } else {
            if (priority2rise) resultT->set(tmpOutBool, changeTime[i]);
            else {
              if ((LUTValue[i] == 1) | (LUTValue[i] == 2)) {
                if (!((LUTValue[i - 1] == 1) |
                      (LUTValue[i - 1] == 2)))
                  resultT->set(tmpOutBool, changeTime[i]);
              } else {
                if (!(LUTValue[i])) resultT->set(tmpOutBool, changeTime[i]);
              }
            }
          }
        }

      }

    }
    result.push_back(resultT);
    result.push_back(resultE);

    delete [] LUTValue;
    delete Hitmap;

    return result;
  }

  void
  TRGCDCTrackSegmentFinder::priorityTiming(unsigned t,
                                           const unsigned nTSF,
                                           TRGSignalVector& s,
                                           const TRGSignal& center,
                                           const TRGSignal& right,
                                           const TRGSignal& left) const
  {

    unsigned rem = t % 16;

    if (rem != 0) {
      s.push_back(* _priMap[t * 4 + 0]);
      s.push_back(* _priMap[t * 4 + 1]);
      s.push_back(* _priMap[t * 4 + 2]);
      s.push_back(* _priMap[t * 4 + 3]);
      return;
    }

    //...Prepare signal vector for timing cells...
    TRGSignalVector tc("timing cell hit", clockData());
    tc.push_back(center);
    tc.push_back(right);
    tc.push_back(left);

    //...No timing hit case...
    if (! tc.active()) {
      s.push_back(* _priMap[t * 4 + 0]);
      s.push_back(* _priMap[t * 4 + 1]);
      s.push_back(* _priMap[t * 4 + 2]);
      s.push_back(* _priMap[t * 4 + 3]);
      return;
    }

    const string sn = "TSF priority timing outer";
    TRGDebug::enterStage(sn);

    //...Prepare signal vector for each timing cells...
    TRGSignalVector t0("t0", clockData()); // center
    t0.push_back(* _priMap[t * 4 + 0]);
    t0.push_back(* _priMap[t * 4 + 1]);
    t0.push_back(* _priMap[t * 4 + 2]);
    t0.push_back(* _priMap[t * 4 + 3]);

    TRGSignalVector t1("t1", clockData()); // left
    unsigned p = t / 16;
    if (t == 0)
      p = nTSF / 16 - 1;
    else p = p - 1;
    t1.push_back(* _edg0Map[p * 4 + 0]);
    t1.push_back(* _edg0Map[p * 4 + 1]);
    t1.push_back(* _edg0Map[p * 4 + 2]);
    t1.push_back(* _edg0Map[p * 4 + 3]);

    //...Loop over state changes...
    vector<TRGState*> outputTimingStates;
    vector<int> clkStates;
    vector<int> sc = tc.stateChanges();
    vector<bool> sb;
    for (unsigned i = 0; i < sc.size(); i++) {
      int clk = sc[i];
      TRGState stt = tc.state(clk);

      //...Priority cell hit case...
      bool sel = true; // true=t0, false=t1
      if (stt[0])
        sel = true;
      else if (stt[1] && (! stt[2]))
        sel = false;
      else if ((! stt[1]) && stt[2])
        sel = true;
      else {  // (stt[1] && stt[2])
        if (unsigned(t0.state(clk)) < unsigned(t1.state(clk)))
          sel = true;
        else
          sel = false;
      }

      if (sel) {
        outputTimingStates.push_back(new TRGState(t0.state(clk)));
        sb.push_back(false);
      } else {
        outputTimingStates.push_back(new TRGState(t1.state(clk)));
        sb.push_back(true);
      }
      clkStates.push_back(clk);
    }

    //...Create signals...
    const unsigned n = outputTimingStates.size();
    TRGSignalVector st("pri.timing", clockData(), 4);
    for (unsigned i = 0; i < n; i++) {
      if (sb[i])
        s[0].unset(clkStates[i], clkStates[i] + 1);
      st.set(* outputTimingStates[i], clkStates[i]);
      delete outputTimingStates[i];
    }

    //...Store signals...
    s.push_back(st[0]);
    s.push_back(st[1]);
    s.push_back(st[2]);
    s.push_back(st[3]);

    if (TRGDebug::level()) {
      tc.dump("", TRGDebug::tab() + "    ");
      t0.dump("", TRGDebug::tab() + "    ");
      t1.dump("", TRGDebug::tab() + "    ");
      st.dump("", TRGDebug::tab() + "    ");
      s.dump("",  TRGDebug::tab() + "    ");
    }

    TRGDebug::leaveStage(sn);
  }

  void
  TRGCDCTrackSegmentFinder::fastestTimingOuter(unsigned t,
                                               const unsigned nTSF,
                                               TRGSignalVector& s) const
  {

    const unsigned rem = t % 16;

    if ((rem != 0)  && (rem != 15)) {
      s.push_back(* _fasMap[t * 4 + 0]);
      s.push_back(* _fasMap[t * 4 + 1]);
      s.push_back(* _fasMap[t * 4 + 2]);
      s.push_back(* _fasMap[t * 4 + 3]);
      return;
    }

    //...Check hit map if there is a hit...
    bool hit = false;
    for (unsigned i = 0; i < 11; i++) {
      if (s[i + 1].active()) {
        hit = true;
        break;
      }
    }

    //...No hit case...
    if (! hit) {
      s.push_back(* _fasMap[t * 4 + 0]);
      s.push_back(* _fasMap[t * 4 + 1]);
      s.push_back(* _fasMap[t * 4 + 2]);
      s.push_back(* _fasMap[t * 4 + 3]);
      return;
    }

    const string sn = "TSF fastest timing outer";
    TRGDebug::enterStage(sn);

    //...Prepare timing signal vectors
    TRGSignalVector t0("t0", clockData()); // main
    t0.push_back(* _fasMap[t * 4 + 0]);
    t0.push_back(* _fasMap[t * 4 + 1]);
    t0.push_back(* _fasMap[t * 4 + 2]);
    t0.push_back(* _fasMap[t * 4 + 3]);

    TRGSignalVector t1("t1", clockData()); // edge
    TRGSignal ht0("t0 hit", clockData());
    TRGSignal ht1("t1 hit", clockData());
    if (rem == 0) {
      unsigned n = t / 16;
      if (n == 0)
        n = nTSF / 16 - 1;
      else
        --n;

      t1.push_back(* _edg2Map[n * 4 + 0]);
      t1.push_back(* _edg2Map[n * 4 + 1]);
      t1.push_back(* _edg2Map[n * 4 + 2]);
      t1.push_back(* _edg2Map[n * 4 + 3]);

      ht0 = s[2];
      ht0 |= s[3];
      ht0 |= s[5];
      ht0 |= s[6];
      ht0 |= s[8];
      ht0 |= s[10];
      ht0 |= s[11];

      ht1 = s[1];
      ht1 |= s[4];
      ht1 |= s[7];
      ht1 |= s[9];
    } else {
      unsigned n = t / 16 + 1;
      if (n >= nTSF / 16)
        n = 0;

      t1.push_back(* _edg1Map[n * 4 + 0]);
      t1.push_back(* _edg1Map[n * 4 + 1]);
      t1.push_back(* _edg1Map[n * 4 + 2]);
      t1.push_back(* _edg1Map[n * 4 + 3]);

      ht0 = s[1];
      ht0 |= s[2];
      ht0 |= s[4];
      ht0 |= s[5];
      ht0 |= s[6];
      ht0 |= s[7];
      ht0 |= s[8];
      ht0 |= s[9];
      ht0 |= s[10];

      ht1 = s[3];
      ht1 |= s[11];
    }

    //...State changes...
    vector<int> sc = t0.stateChanges();
    vector<int> tmp = t1.stateChanges();
    sc.insert(sc.end(), tmp.begin(), tmp.end());
    std::sort(sc.begin(), sc.end());

    //...Loop over state changes...
    TRGSignalVector tm("fastest", clockData(), 4);
    int last = clockData().max();
    for (unsigned i = 0; i < sc.size(); i++) {
      if (sc[i] == last)
        continue;

      int clk = sc[i];

      TRGState ts0 = t0.state(clk);
      TRGState ts1 = t1.state(clk);
      unsigned tm0 = unsigned(ts0);
      unsigned tm1 = unsigned(ts1);
      bool th0 = ht0.state(clk);
      bool th1 = ht1.state(clk);

      if ((! th0) && (! th1))
        continue;
      else if (th0 && th1) {
        if (tm1 < tm0)
          tm.set(ts1, clk);
        else
          tm.set(ts0, clk);
      } else if (th0)
        tm.set(ts0, clk);
      else if (th1)
        tm.set(ts1, clk);

      last = clk;
    }

    //...Store signals...
    s.push_back(tm[0]);
    s.push_back(tm[1]);
    s.push_back(tm[2]);
    s.push_back(tm[3]);

    if (TRGDebug::level()) {
      ht0.name("t0 hit:" + ht0.name());
      ht0.dump("", TRGDebug::tab() + "    ");
      t0.dump("", TRGDebug::tab() + "    ");
      ht1.name("t1 hit:" + ht1.name());
      ht1.dump("", TRGDebug::tab() + "    ");
      t1.dump("", TRGDebug::tab() + "    ");
      tm.dump("", TRGDebug::tab() + "    ");
      s.dump("",  TRGDebug::tab() + "    ");
    }

    TRGDebug::leaveStage(sn);
  }

  void
  TRGCDCTrackSegmentFinder::simulateInner(void)
  {

    const string sn = "TSF::simulateInner : " + name();
    TRGDebug::enterStage(sn);

    //...Output for 2D : empty bundle temporary...
    for (unsigned i = 0; i < 4; i++) {
      string n = name() + "-trker" + TRGUtilities::itostring(i);
      TRGSignalVector* dummy = new TRGSignalVector(n, clockData(), 420);
      TRGSignalBundle* b = new TRGSignalBundle(n, clockData());
      b->push_back(dummy);
      output(i)->signal(b);
      _toBeDeleted.push_back(dummy);

      if (TRGCDC::getTRGCDC()->firmwareSimulationMode() & 0x4)
        b->dumpCOE("",
                   TRGCDC::getTRGCDC()->firmwareSimulationStartDataClock(),
                   TRGCDC::getTRGCDC()->firmwareSimulationStopDataClock());
    }

    TRGDebug::leaveStage(sn);
    return;


    //...Loop over mergers to create a super layer hit map...
    for (unsigned m = 0; m < nInput(); m++) {
      TRGSignalBundle* b = input(m)->signal();

      for (unsigned i = 0; i < 16; i++) {
        _secMap.push_back(& ((* b)[0][0][208 + i]));
        for (unsigned j = 0; j < 5; j++)
          _hitMap[j].push_back(& ((* b)[0][0][j * 16 + i]));
        for (unsigned j = 0; j < 4; j++)
          _priMap.push_back(& ((* b)[0][0][80 + i * 4 + j]));
        for (unsigned j = 0; j < 4; j++)
          _fasMap.push_back(& ((* b)[0][0][144 + i * 4 + j]));
      }

      for (unsigned i = 0; i < 4; i++)
        _edg0Map.push_back(& ((* b)[0][0][224 + i]));
      for (unsigned i = 0; i < 4; i++)
        _edg1Map.push_back(& ((* b)[0][0][224 + 4 + i]));
      for (unsigned i = 0; i < 4; i++)
        _edg2Map.push_back(& ((* b)[0][0][224 + 8 + i]));
      for (unsigned i = 0; i < 4; i++)
        _edg3Map.push_back(& ((* b)[0][0][224 + 12 + i]));
      for (unsigned i = 0; i < 4; i++)
        _edg4Map.push_back(& ((* b)[0][0][224 + 16 + i]));
    }

    //...Storage preparation...
    const unsigned nTSF = nInput() * 16;
    vector<TRGSignalVector*> trker[4];
    vector<int> tsfStateChanges;

    //...Form a TSF...
    for (unsigned t = 0; t < nTSF; t++) {

      const string n = name() + "-" + TRGUtilities::itostring(t);
      TRGSignalVector* s = new TRGSignalVector(n, clockData());
      _tsfIn.push_back(s);

      s->push_back(* _secMap[t]);

      if (t == 0) {
        s->push_back(* (_hitMap[0][0]));
        s->push_back(* (_hitMap[1][nTSF - 1]));
        s->push_back(* (_hitMap[1][0]));
        s->push_back(* (_hitMap[2][nTSF - 1]));
        s->push_back(* (_hitMap[2][0]));
        s->push_back(* (_hitMap[2][1]));
        s->push_back(* (_hitMap[3][nTSF - 2]));
        s->push_back(* (_hitMap[3][nTSF - 1]));
        s->push_back(* (_hitMap[3][0]));
        s->push_back(* (_hitMap[3][1]));
        s->push_back(* (_hitMap[4][nTSF - 2]));
        s->push_back(* (_hitMap[4][nTSF - 1]));
        s->push_back(* (_hitMap[4][0]));
        s->push_back(* (_hitMap[4][1]));
        s->push_back(* (_hitMap[4][2]));
      } else if (t == 1) {
        s->push_back(* (_hitMap[0][1]));
        s->push_back(* (_hitMap[1][0]));
        s->push_back(* (_hitMap[1][1]));
        s->push_back(* (_hitMap[2][0]));
        s->push_back(* (_hitMap[2][1]));
        s->push_back(* (_hitMap[2][2]));
        s->push_back(* (_hitMap[3][nTSF - 1]));
        s->push_back(* (_hitMap[3][0]));
        s->push_back(* (_hitMap[3][1]));
        s->push_back(* (_hitMap[3][2]));
        s->push_back(* (_hitMap[4][nTSF - 1]));
        s->push_back(* (_hitMap[4][0]));
        s->push_back(* (_hitMap[4][1]));
        s->push_back(* (_hitMap[4][2]));
        s->push_back(* (_hitMap[4][3]));
      } else if (t == (nTSF - 2)) {
        s->push_back(* (_hitMap[0][t]));
        s->push_back(* (_hitMap[1][t - 11]));
        s->push_back(* (_hitMap[1][t]));
        s->push_back(* (_hitMap[2][t - 1]));
        s->push_back(* (_hitMap[2][t]));
        s->push_back(* (_hitMap[2][0]));
        s->push_back(* (_hitMap[3][t - 2]));
        s->push_back(* (_hitMap[3][t - 1]));
        s->push_back(* (_hitMap[3][t]));
        s->push_back(* (_hitMap[3][0]));
        s->push_back(* (_hitMap[4][t - 2]));
        s->push_back(* (_hitMap[4][t - 1]));
        s->push_back(* (_hitMap[4][t]));
        s->push_back(* (_hitMap[4][t + 1]));
        s->push_back(* (_hitMap[4][0]));
      } else if (t == (nTSF - 1)) {
        s->push_back(* (_hitMap[0][t]));
        s->push_back(* (_hitMap[1][t - 11]));
        s->push_back(* (_hitMap[1][t]));
        s->push_back(* (_hitMap[2][t - 1]));
        s->push_back(* (_hitMap[2][t]));
        s->push_back(* (_hitMap[2][0]));
        s->push_back(* (_hitMap[3][t - 2]));
        s->push_back(* (_hitMap[3][t - 1]));
        s->push_back(* (_hitMap[3][t]));
        s->push_back(* (_hitMap[3][0]));
        s->push_back(* (_hitMap[4][t - 2]));
        s->push_back(* (_hitMap[4][t - 1]));
        s->push_back(* (_hitMap[4][t]));
        s->push_back(* (_hitMap[4][0]));
        s->push_back(* (_hitMap[4][1]));
      } else {
        s->push_back(* (_hitMap[0][t]));
        s->push_back(* (_hitMap[1][t - 11]));
        s->push_back(* (_hitMap[1][t]));
        s->push_back(* (_hitMap[2][t - 1]));
        s->push_back(* (_hitMap[2][t]));
        s->push_back(* (_hitMap[2][0]));
        s->push_back(* (_hitMap[3][t - 2]));
        s->push_back(* (_hitMap[3][t - 1]));
        s->push_back(* (_hitMap[3][t]));
        s->push_back(* (_hitMap[3][0]));
        s->push_back(* (_hitMap[4][t - 2]));
        s->push_back(* (_hitMap[4][t - 1]));
        s->push_back(* (_hitMap[4][t]));
        s->push_back(* (_hitMap[4][t + 1]));
        s->push_back(* (_hitMap[4][t + 2]));
      }

      //...Priority timing...
      s->push_back(* _priMap[t * 4 + 0]);
      s->push_back(* _priMap[t * 4 + 1]);
      s->push_back(* _priMap[t * 4 + 2]);
      s->push_back(* _priMap[t * 4 + 3]);

      //...Priority timing...
      priorityTiming(t, nTSF, * s, (* s)[1], (* s)[2], (* s)[3]);

      //...Fastest timing...
      fastestTimingInner(t, nTSF, * s);

      //...Clock counter is omitted...

      //...Simulate TSF...
      vector<TRGSignalVector*> result = simulateInner(* s, t);

      TRGSignalVector* forTracker = result[0];
      _tsfOut.push_back(forTracker);
      _toBeDeleted.push_back(result[1]);

      //...State change list...
      vector<int> sc = forTracker->stateChanges();
      for (unsigned i = 0; i < sc.size(); i++) {
        bool skip = false;
        for (unsigned j = 0; j < tsfStateChanges.size(); j++) {
          if (tsfStateChanges[j] == sc[i]) {
            skip = true;
            break;
          }
        }
        if (! skip) tsfStateChanges.push_back(sc[i]);
      }

      //...Store it for each tracker division...
      const unsigned pos = t / (nTSF / 4);
      if (pos == 0) {
        trker[0].push_back(forTracker);
        trker[1].push_back(forTracker);
      } else if (pos == 1) {
        trker[1].push_back(forTracker);
        trker[2].push_back(forTracker);
      } else if (pos == 2) {
        trker[2].push_back(forTracker);
        trker[3].push_back(forTracker);
      } else {
        trker[3].push_back(forTracker);
        trker[0].push_back(forTracker);
      }

      //...Test...
      // if (TRGDebug::level() && t < 16) {
      //     bool ok = ((* result[0]) == (* dbgOut[t])); // track part only

      //     if (ok) {
      //         cout << TRGDebug::tab() << name() << "...Comparison OK"
      //              << endl;
      //     }
      //     else {
      //         cout << TRGDebug::tab() << name()
      //              << "...Comparison is not OK" << endl;
      //         dbgOut[t]->dump("", "kt :");
      //         result[0]->dump("", "2d :");
      //         dbgIn[t]->dump("", "kt i:");
      //         s->dump("", "2d i:");
      //     }
      // }
    }

    //...Sort state changes...
    std::sort(tsfStateChanges.begin(), tsfStateChanges.end());

    //...Output for 2D...
    for (unsigned i = 0; i < 4; i++) {
      TRGSignalVector* tOut = packerOuterTracker(trker[i],
                                                 tsfStateChanges,
                                                 20);
      string n = name() + "trker" + TRGUtilities::itostring(i);
      TRGSignalBundle* b = new TRGSignalBundle(n, clockData());
      b->push_back(tOut);
      output(i)->signal(b);
    }

    TRGDebug::leaveStage(sn);
  }

  void
  TRGCDCTrackSegmentFinder::fastestTimingInner(unsigned t,
                                               const unsigned nTSF,
                                               TRGSignalVector& s) const
  {

    const unsigned rem = t % 16;

    if ((rem > 1)  && (rem < 14)) {
      s.push_back(* _fasMap[t * 4 + 0]);
      s.push_back(* _fasMap[t * 4 + 1]);
      s.push_back(* _fasMap[t * 4 + 2]);
      s.push_back(* _fasMap[t * 4 + 3]);
      return;
    }

    //...Check hit map if there is a hit...
    bool hit = false;
    for (unsigned i = 0; i < 11; i++) {
      if (s[i + 1].active()) {
        hit = true;
        break;
      }
    }

    //...No hit case...
    if (! hit) {
      s.push_back(* _fasMap[t * 4 + 0]);
      s.push_back(* _fasMap[t * 4 + 1]);
      s.push_back(* _fasMap[t * 4 + 2]);
      s.push_back(* _fasMap[t * 4 + 3]);
      return;
    }

    const string sn = "TSF fastest timing inner";
    TRGDebug::enterStage(sn);

    //...Prepare timing signal vectors
    TRGSignalVector t0("t0", clockData()); // main
    t0.push_back(* _fasMap[t * 4 + 0]);
    t0.push_back(* _fasMap[t * 4 + 1]);
    t0.push_back(* _fasMap[t * 4 + 2]);
    t0.push_back(* _fasMap[t * 4 + 3]);

    TRGSignalVector t1("t1", clockData()); // edge
    TRGSignal ht0("t0 hit", clockData());
    TRGSignal ht1("t1 hit", clockData());
    if (rem == 0) {
      unsigned n = t / 16;
      if (n == 0)
        n = nTSF / 16 - 1;
      else
        --n;

      t1.push_back(* _edg3Map[n * 4 + 0]);
      t1.push_back(* _edg3Map[n * 4 + 1]);
      t1.push_back(* _edg3Map[n * 4 + 2]);
      t1.push_back(* _edg3Map[n * 4 + 3]);

      ht0 = s[0 + 1];
      ht0 |= s[2 + 1];
      ht0 |= s[4 + 1];
      ht0 |= s[5 + 1];
      ht0 |= s[8 + 1];
      ht0 |= s[9 + 1];
      ht0 |= s[12 + 1];
      ht0 |= s[13 + 1];

      ht1 = s[1 + 1];
      ht1 |= s[3 + 1];
      ht1 |= s[6 + 1];
      ht1 |= s[7 + 1];
      ht1 |= s[10 + 1];
      ht1 |= s[11 + 1];
    } else if (rem == 1) {
      unsigned n = t / 16;
      if (n == 0)
        n = nTSF / 16 - 1;
      else
        --n;

      t1.push_back(* _edg4Map[n * 4 + 0]);
      t1.push_back(* _edg4Map[n * 4 + 1]);
      t1.push_back(* _edg4Map[n * 4 + 2]);
      t1.push_back(* _edg4Map[n * 4 + 3]);

      // ht0 = s[0 + 1];
      ht0 = s[1 + 1];
      ht0 |= s[2 + 1];
      ht0 |= s[3 + 1];
      ht0 |= s[4 + 1];
      ht0 |= s[5 + 1];
      ht0 |= s[7 + 1];
      ht0 |= s[8 + 1];
      ht0 |= s[9 + 1];
      ht0 |= s[10 + 1];
      ht0 |= s[12 + 1];
      ht0 |= s[13 + 1];

      ht1 |= s[6 + 1];
      ht1 |= s[11 + 1];
    } else if (rem == 14) {
      unsigned n = t / 16 + 1;
      if (n >= nTSF / 16)
        n = 0;

      t1.push_back(* _edg1Map[n * 4 + 0]);
      t1.push_back(* _edg1Map[n * 4 + 1]);
      t1.push_back(* _edg1Map[n * 4 + 2]);
      t1.push_back(* _edg1Map[n * 4 + 3]);

      ht0 = s[0 + 1];
      ht0 |= s[1 + 1];
      ht0 |= s[2 + 1];
      ht0 |= s[3 + 1];
      ht0 |= s[4 + 1];
      ht0 |= s[5 + 1];
      ht0 |= s[6 + 1];
      ht0 |= s[7 + 1];
      ht0 |= s[8 + 1];
      ht0 |= s[9 + 1];
      ht0 |= s[10 + 1];
      ht0 |= s[11 + 1];
      ht0 |= s[12 + 1];

      ht1 = s[13 + 1];
    } else {
      unsigned n = t / 16 + 1;
      if (n >= nTSF / 16)
        n = 0;

      t1.push_back(* _edg2Map[n * 4 + 0]);
      t1.push_back(* _edg2Map[n * 4 + 1]);
      t1.push_back(* _edg2Map[n * 4 + 2]);
      t1.push_back(* _edg2Map[n * 4 + 3]);

      ht0 = s[0 + 1];
      ht0 |= s[1 + 1];
      ht0 |= s[2 + 1];
      ht0 |= s[3 + 1];
      ht0 |= s[4 + 1];
      ht0 |= s[6 + 1];
      ht0 |= s[7 + 1];
      ht0 |= s[8 + 1];
      ht0 |= s[10 + 1];
      ht0 |= s[11 + 1];

      ht1 = s[5 + 1];
      ht1 |= s[9 + 1];
      ht1 |= s[12 + 1];
      ht1 |= s[13 + 1];
    }

    //...State changes...
    vector<int> sc = t0.stateChanges();
    vector<int> tmp = t1.stateChanges();
    sc.insert(sc.end(), tmp.begin(), tmp.end());
    std::sort(sc.begin(), sc.end());

    //...Loop over state changes...
    TRGSignalVector tm("fastest", clockData(), 4);
    int last = clockData().max();
    for (unsigned i = 0; i < sc.size(); i++) {
      if (sc[i] == last)
        continue;

      int clk = sc[i];

      TRGState ts0 = t0.state(clk);
      TRGState ts1 = t1.state(clk);
      unsigned tm0 = unsigned(ts0);
      unsigned tm1 = unsigned(ts1);
      bool th0 = ht0.state(clk);
      bool th1 = ht1.state(clk);

      if ((! th0) && (! th1))
        continue;
      else if (th0 && th1) {
        if (tm1 < tm0)
          tm.set(ts1, clk);
        else
          tm.set(ts0, clk);
      } else if (th0)
        tm.set(ts0, clk);
      else if (th1)
        tm.set(ts1, clk);

      last = clk;
    }

    //...Store signals...
    s.push_back(tm[0]);
    s.push_back(tm[1]);
    s.push_back(tm[2]);
    s.push_back(tm[3]);

    if (TRGDebug::level()) {
      ht0.name("t0 hit:" + ht0.name());
      ht0.dump("", TRGDebug::tab() + "    ");
      t0.dump("", TRGDebug::tab() + "    ");
      ht1.name("t1 hit:" + ht1.name());
      ht1.dump("", TRGDebug::tab() + "    ");
      t1.dump("", TRGDebug::tab() + "    ");
      tm.dump("", TRGDebug::tab() + "    ");
      s.dump("",  TRGDebug::tab() + "    ");
    }

    TRGDebug::leaveStage(sn);
  }

  vector<TRGSignalVector*>
  TRGCDCTrackSegmentFinder::simulateInner(TRGSignalVector& s, unsigned)
  {

    // This is just a simple coincidence logic. Should be replaced by
    // real logic.

    const int width = 10;

    //...Layer hit...
    TRGSignal l0 = s[0 + 1].widen(width);
    TRGSignal l1 = s[1 + 1].widen(width) | s[2 + 1].widen(width);
    TRGSignal l2 = s[3 + 1].widen(width) | s[4 + 1].widen(width) |
                   s[5 + 1].widen(width);
    TRGSignal l3 = s[6 + 1].widen(width) | s[7 + 1].widen(width) |
                   s[8 + 1].widen(width) | s[9 + 1].widen(width);
    TRGSignal l4 = s[10 + 1].widen(width) | s[11 + 1].widen(width) |
                   s[12 + 1].widen(width) | s[13 + 1].widen(width);

    //...Layer coincidence...
    TRGSignal a0 = l1 & l2 & l3 & l4;
    TRGSignal a1 = l0 & l2 & l3 & l4;
    TRGSignal a2 = l0 & l1 & l3 & l4;
    TRGSignal a3 = l0 & l1 & l2 & l4;
    TRGSignal a4 = l0 & l1 & l2 & l3;

    //...Final hit... (detail hit pattern changes are ignored here)
    TRGSignal a = a0 | a1 | a2 | a3 | a4;

    //...Check timing cells...
    vector<int> sc = s.stateChanges();
    for (unsigned i = 0; i < sc.size(); i++) {
      int clk = sc[i];
      TRGState st = s.state(clk).subset(1, 3);

      if (st[0]) {
      } else {
        if (st[1] && st[2]) {
        } else if (st[1]) {
        } else if (st[2]) {
        }
      }
    }

    TRGSignalVector* r0 = new TRGSignalVector("tmp0", clockData(), 21);
    TRGSignalVector* r1 = new TRGSignalVector("tmp0", clockData(), 21);
    vector<TRGSignalVector*> v;
    v.push_back(r0);
    v.push_back(r1);
    return v;
  }

  void
  TRGCDCTrackSegmentFinder::addID(TRGSignalVector& s, unsigned id)
  {
    const TRGState sid(8, id);
    TRGSignalVector idv("TSFid", s.clock(), 8);

    //...Encode ID only when s is active...
    vector<int> sc = s.stateChanges();
    for (unsigned i = 0; i < sc.size(); i++) {
      if (s.active(sc[i]))
        idv.set(sid, sc[i]);
    }

    //...Merge ID vectors...
    s += idv;
  }

  void
  TRGCDCTrackSegmentFinder::simulate(void)
  {

    //...Delete old objects...
    for (unsigned i = 0; i < nOutput(); i++) {
      if (output(i))
        if (output(i)->signal())
          delete output(i)->signal();
    }
    for (unsigned i = 0; i < _tsfIn.size(); i++)
      delete _tsfIn[i];
    _tsfIn.clear();
    for (unsigned i = 0; i < _tsfOut.size(); i++)
      delete _tsfOut[i];
    _tsfOut.clear();
    for (unsigned i = 0; i < _toBeDeleted.size(); i++)
      delete _toBeDeleted[i];
    _toBeDeleted.clear();

    //...Clear old pointers...
    for (unsigned i = 0; i < 5; i++)
      _hitMap[i].clear();
    _priMap.clear();
    _fasMap.clear();
    _secMap.clear();
    _edg0Map.clear();
    _edg1Map.clear();
    _edg2Map.clear();
    _edg3Map.clear();
    _edg4Map.clear();

    if (_type == innerType)
      simulateInner();
    else
      simulateOuter();
  }

  void
  TRGCDCTrackSegmentFinder::simulate2(void)
  {

    const string sn = "TSF::simulate2 : " + name();
    TRGDebug::enterStage(sn);

    // if (_type == innerType) {
    //     cout << "??? tmp skip" << endl;
    //     TRGDebug::leaveStage(sn);
    //     return;
    // }

    //...Delete old objects...
    for (unsigned i = 0; i < nOutput(); i++) {
      if (output(i))
        if (output(i)->signal())
          delete output(i)->signal();
    }
    for (unsigned i = 0; i < _tsfIn.size(); i++)
      delete _tsfIn[i];
    _tsfIn.clear();
    for (unsigned i = 0; i < _tsfOut.size(); i++)
      delete _tsfOut[i];
    _tsfOut.clear();
    for (unsigned i = 0; i < _toBeDeleted.size(); i++)
      delete _toBeDeleted[i];
    _toBeDeleted.clear();

    //...Clear old pointers...
    for (unsigned i = 0; i < 5; i++)
      _hitMap[i].clear();
    _priMap.clear();
    _fasMap.clear();
    _secMap.clear();
    _edg0Map.clear();
    _edg1Map.clear();
    _edg2Map.clear();
    _edg3Map.clear();
    _edg4Map.clear();

    //...Storage preparation...
    const unsigned nTSF = nInput() * 16;
    vector<TRGSignalVector*> trker[4];
    vector<int> tsfStateChanges;

    //...Creates hit maps...
    if (_type == innerType)
      hitMapInner();
    else
      hitMapOuter();

    //...Form a TSF...
    for (unsigned t = 0; t < nTSF; t++) {

      const string n = name() + "-" + TRGUtilities::itostring(t);
      TRGSignalVector* s = new TRGSignalVector(n, clockData());
      _tsfIn.push_back(s);

      //...Make input signals...
      if (_type == innerType)
        inputInner(t, nTSF, s);
      else
        inputOuter(t, nTSF, s);

      //...Simulate TSF...
      vector<TRGSignalVector*> result = simulateTSF(s, t);
      vector<TRGSignalVector*> result2 = simulateTSF2(s, t);
      _toBeDeleted.push_back(result[1]); // Event timing omitted now

      if (TRGDebug::level()) {
        if (result.size() != result2.size()) {
          cout << "TSF::simulateTSF2 has different response(size)"
               << endl;
        } else {
          for (unsigned i = 0; i < result.size(); i++) {
            if ((* result[i]) != (* result2[i]))
              cout << "TSF::simulateTSF2 has different response"
                   << "(contents)" << endl;
          }
        }
      }

      TRGSignalVector* forTracker0 = result[0];
      TRGSignalVector* forTracker1 = new TRGSignalVector(* forTracker0);

      _tsfOut.push_back(forTracker0);
      _tsfOut.push_back(forTracker1);

      //...State change list...
      vector<int> sc = forTracker0->stateChanges();
      for (unsigned i = 0; i < sc.size(); i++) {
        bool skip = false;
        for (unsigned j = 0; j < tsfStateChanges.size(); j++) {
          if (tsfStateChanges[j] == sc[i]) {
            skip = true;
            break;
          }
        }
        if (! skip) tsfStateChanges.push_back(sc[i]);
      }

      //...Store it for each tracker division...
      const unsigned pos = t / (nTSF / 4);
      if (pos == 0) {
        addID(* forTracker0, t - 3 * (nTSF / 4));
        addID(* forTracker1, t);
        trker[3].push_back(forTracker0);
        trker[0].push_back(forTracker1);
      } else if (pos == 1) {
        addID(* forTracker0, t);
        addID(* forTracker1, t - 1 * (nTSF / 4));
        trker[0].push_back(forTracker0);
        trker[1].push_back(forTracker1);
      } else if (pos == 2) {
        addID(* forTracker0, t - 1 * (nTSF / 4));
        addID(* forTracker1, t - 2 * (nTSF / 4));
        trker[1].push_back(forTracker0);
        trker[2].push_back(forTracker1);
      } else {
        addID(* forTracker0, t - 2 * (nTSF / 4));
        addID(* forTracker1, t - 3 * (nTSF / 4));
        trker[2].push_back(forTracker0);
        trker[3].push_back(forTracker1);
      }

      if (TRGDebug::level())
        if (forTracker0->active())
          cout << TRGDebug::tab() << name() << " : TSF out="
               << t << endl;
    }

    //...Sort state changes...
    std::sort(tsfStateChanges.begin(), tsfStateChanges.end());

    //...Output for 2D...
    for (unsigned i = 0; i < 4; i++) {
      string n = name() + "-trker" + TRGUtilities::itostring(i);
      TRGSignalVector* tOut = packerForTracker(trker[i],
                                               tsfStateChanges,
                                               20);
      tOut->name(n);
      TRGSignalBundle* b = new TRGSignalBundle(n, clockData());
      b->push_back(tOut);
      output(i)->signal(b);
      _toBeDeleted.push_back(tOut);

      if (TRGCDC::getTRGCDC()->firmwareSimulationMode() & 0x4)
        b->dumpCOE("",
                   TRGCDC::getTRGCDC()->firmwareSimulationStartDataClock(),
                   TRGCDC::getTRGCDC()->firmwareSimulationStopDataClock());
      //      b->dump();
    }

    TRGDebug::leaveStage(sn);
  }

  void
  TRGCDCTrackSegmentFinder::hitMapInner(void)
  {

    //    dump("detail","??? ");

    //...Loop over mergers to create a super layer hit map...
    for (unsigned m = 0; m < nInput(); m++) {
      TRGSignalBundle* b = input(m)->signal();

      //        b->dump("", "??? ");

      for (unsigned i = 0; i < 16; i++) {
        _secMap.push_back(& ((* b)[0][0][208 + i]));
        for (unsigned j = 0; j < 5; j++) {
          _hitMap[j].push_back(& ((* b)[0][0][j * 16 + i]));

          //                _hitMap[j][i]->dump("", "??? " + TRGUtilities::itostring(i) + "-" + TRGUtilities::itostring(j));

        }
        for (unsigned j = 0; j < 4; j++)
          _priMap.push_back(& ((* b)[0][0][80 + i * 4 + j]));
        for (unsigned j = 0; j < 4; j++)
          _fasMap.push_back(& ((* b)[0][0][144 + i * 4 + j]));
      }

      for (unsigned i = 0; i < 4; i++)
        _edg0Map.push_back(& ((* b)[0][0][224 + i]));
      for (unsigned i = 0; i < 4; i++)
        _edg1Map.push_back(& ((* b)[0][0][224 + 4 + i]));
      for (unsigned i = 0; i < 4; i++)
        _edg2Map.push_back(& ((* b)[0][0][224 + 8 + i]));
      for (unsigned i = 0; i < 4; i++)
        _edg3Map.push_back(& ((* b)[0][0][224 + 12 + i]));
      for (unsigned i = 0; i < 4; i++)
        _edg4Map.push_back(& ((* b)[0][0][224 + 16 + i]));
    }
  }

  void
  TRGCDCTrackSegmentFinder::hitMapOuter(void)
  {

    //...Loop over mergers to create a super layer hit map...
    for (unsigned m = 0; m < nInput(); m++) {
      TRGSignalBundle* b = input(m)->signal();

      for (unsigned i = 0; i < 16; i++) {
        _secMap.push_back(& ((* b)[0][0][208 + i]));
        for (unsigned j = 0; j < 5; j++)
          _hitMap[j].push_back(& ((* b)[0][0][j * 16 + i]));
        for (unsigned j = 0; j < 4; j++)
          _priMap.push_back(& ((* b)[0][0][80 + i * 4 + j]));
        for (unsigned j = 0; j < 4; j++)
          _fasMap.push_back(& ((* b)[0][0][144 + i * 4 + j]));
      }

      for (unsigned i = 0; i < 4; i++)
        _edg0Map.push_back(& ((* b)[0][0][224 + i]));
      for (unsigned i = 0; i < 4; i++)
        _edg1Map.push_back(& ((* b)[0][0][224 + 4 + i]));
      for (unsigned i = 0; i < 4; i++)
        _edg2Map.push_back(& ((* b)[0][0][224 + 8 + i]));
    }
  }

  void
  TRGCDCTrackSegmentFinder::inputInner(const unsigned t,
                                       const unsigned nTSF,
                                       TRGSignalVector* s)
  {
    s->push_back(* _secMap[t]);

    if (t == 0) {
      s->push_back(* (_hitMap[0][0]));
      s->push_back(* (_hitMap[1][nTSF - 1]));
      s->push_back(* (_hitMap[1][0]));
      s->push_back(* (_hitMap[2][nTSF - 1]));
      s->push_back(* (_hitMap[2][0]));
      s->push_back(* (_hitMap[2][1]));
      s->push_back(* (_hitMap[3][nTSF - 2]));
      s->push_back(* (_hitMap[3][nTSF - 1]));
      s->push_back(* (_hitMap[3][0]));
      s->push_back(* (_hitMap[3][1]));
      s->push_back(* (_hitMap[4][nTSF - 2]));
      s->push_back(* (_hitMap[4][nTSF - 1]));
      s->push_back(* (_hitMap[4][0]));
      s->push_back(* (_hitMap[4][1]));
      s->push_back(* (_hitMap[4][2]));
    } else if (t == 1) {
      s->push_back(* (_hitMap[0][1]));
      s->push_back(* (_hitMap[1][0]));
      s->push_back(* (_hitMap[1][1]));
      s->push_back(* (_hitMap[2][0]));
      s->push_back(* (_hitMap[2][1]));
      s->push_back(* (_hitMap[2][2]));
      s->push_back(* (_hitMap[3][nTSF - 1]));
      s->push_back(* (_hitMap[3][0]));
      s->push_back(* (_hitMap[3][1]));
      s->push_back(* (_hitMap[3][2]));
      s->push_back(* (_hitMap[4][nTSF - 1]));
      s->push_back(* (_hitMap[4][0]));
      s->push_back(* (_hitMap[4][1]));
      s->push_back(* (_hitMap[4][2]));
      s->push_back(* (_hitMap[4][3]));
    } else if (t == (nTSF - 2)) {
      s->push_back(* (_hitMap[0][t]));
      s->push_back(* (_hitMap[1][t - 11]));
      s->push_back(* (_hitMap[1][t]));
      s->push_back(* (_hitMap[2][t - 1]));
      s->push_back(* (_hitMap[2][t]));
      s->push_back(* (_hitMap[2][0]));
      s->push_back(* (_hitMap[3][t - 2]));
      s->push_back(* (_hitMap[3][t - 1]));
      s->push_back(* (_hitMap[3][t]));
      s->push_back(* (_hitMap[3][0]));
      s->push_back(* (_hitMap[4][t - 2]));
      s->push_back(* (_hitMap[4][t - 1]));
      s->push_back(* (_hitMap[4][t]));
      s->push_back(* (_hitMap[4][t + 1]));
      s->push_back(* (_hitMap[4][0]));
    } else if (t == (nTSF - 1)) {
      s->push_back(* (_hitMap[0][t]));
      s->push_back(* (_hitMap[1][t - 11]));
      s->push_back(* (_hitMap[1][t]));
      s->push_back(* (_hitMap[2][t - 1]));
      s->push_back(* (_hitMap[2][t]));
      s->push_back(* (_hitMap[2][0]));
      s->push_back(* (_hitMap[3][t - 2]));
      s->push_back(* (_hitMap[3][t - 1]));
      s->push_back(* (_hitMap[3][t]));
      s->push_back(* (_hitMap[3][0]));
      s->push_back(* (_hitMap[4][t - 2]));
      s->push_back(* (_hitMap[4][t - 1]));
      s->push_back(* (_hitMap[4][t]));
      s->push_back(* (_hitMap[4][0]));
      s->push_back(* (_hitMap[4][1]));
    } else {
      //        _hitMap[1][t - 1]->dump("", "??? " + TRGUtilities::itostring(1) + "-" + TRGUtilities::itostring(t - 1));

      TRGSignal& ts = * _hitMap[1][t - 1];

      s->push_back(* (_hitMap[0][t]));
      //      s->push_back(* (_hitMap[1][t - 11])); // Why this makes SegV?
      s->push_back(ts);
      s->push_back(* (_hitMap[1][t]));
      s->push_back(* (_hitMap[2][t - 1]));
      s->push_back(* (_hitMap[2][t]));
      s->push_back(* (_hitMap[2][0]));
      s->push_back(* (_hitMap[3][t - 2]));
      s->push_back(* (_hitMap[3][t - 1]));
      s->push_back(* (_hitMap[3][t]));
      s->push_back(* (_hitMap[3][0]));
      s->push_back(* (_hitMap[4][t - 2]));
      s->push_back(* (_hitMap[4][t - 1]));
      s->push_back(* (_hitMap[4][t]));
      s->push_back(* (_hitMap[4][t + 1]));
      s->push_back(* (_hitMap[4][t + 2]));
    }

    //...Priority timing...
//    s->push_back(* _priMap[t * 4 + 0]);
//    s->push_back(* _priMap[t * 4 + 1]);
//    s->push_back(* _priMap[t * 4 + 2]);
//    s->push_back(* _priMap[t * 4 + 3]);

    //...Priority timing...
    priorityTiming(t, nTSF, * s, (* s)[1], (* s)[2], (* s)[3]);

    //...Fastest timing...
    fastestTimingInner(t, nTSF, * s);

    //...Clock counter is omitted...
  }

  void
  TRGCDCTrackSegmentFinder::inputOuter(const unsigned t,
                                       const unsigned nTSF,
                                       TRGSignalVector* s)
  {
    s->push_back(* _secMap[t]);

    if (t == 0) {
      s->push_back(* (_hitMap[0][nTSF - 1]));
      s->push_back(* (_hitMap[0][0]));
      s->push_back(* (_hitMap[0][1]));
      s->push_back(* (_hitMap[1][nTSF - 1]));
      s->push_back(* (_hitMap[1][0]));
      s->push_back(* (_hitMap[2][0]));
      s->push_back(* (_hitMap[3][nTSF - 1]));
      s->push_back(* (_hitMap[3][0]));
      s->push_back(* (_hitMap[4][nTSF - 1]));
      s->push_back(* (_hitMap[4][0]));
      s->push_back(* (_hitMap[4][1]));
    } else if (t == (nTSF - 1)) {
      s->push_back(* (_hitMap[0][nTSF - 2]));
      s->push_back(* (_hitMap[0][nTSF - 1]));
      s->push_back(* (_hitMap[0][0]));
      s->push_back(* (_hitMap[1][nTSF - 2]));
      s->push_back(* (_hitMap[1][nTSF - 1]));
      s->push_back(* (_hitMap[2][nTSF - 1]));
      s->push_back(* (_hitMap[3][nTSF - 2]));
      s->push_back(* (_hitMap[3][nTSF - 1]));

      s->push_back(* (_hitMap[4][nTSF - 2]));
      s->push_back(* (_hitMap[4][nTSF - 1]));
      s->push_back(* (_hitMap[4][0]));
    } else {
      s->push_back(* (_hitMap[0][t - 1]));
      s->push_back(* (_hitMap[0][t]));
      s->push_back(* (_hitMap[0][t + 1]));
      s->push_back(* (_hitMap[1][t - 1]));
      s->push_back(* (_hitMap[1][t]));
      s->push_back(* (_hitMap[2][t]));
      s->push_back(* (_hitMap[3][t - 1]));
      s->push_back(* (_hitMap[3][t]));
      s->push_back(* (_hitMap[4][t - 1]));
      s->push_back(* (_hitMap[4][t]));
      s->push_back(* (_hitMap[4][t + 1]));
    }

    //...Priority timing...
//    s->push_back(* _priMap[t * 4 + 0]);
//    s->push_back(* _priMap[t * 4 + 1]);
//    s->push_back(* _priMap[t * 4 + 2]);
//    s->push_back(* _priMap[t * 4 + 3]);

    //...Priority timing...
    priorityTiming(t, nTSF, * s, (* s)[6], (* s)[7], (* s)[8]);

    //...Fastest timing...
    fastestTimingOuter(t, nTSF, * s);

    //...Clock counter is omitted...
  }

  TRGSignalVector*
  TSFinder::packerForTracker(vector<TRGSignalVector*>& hitList,
                             vector<int>& cList,
                             const unsigned maxHit)
  {

    TRGSignalVector* result =
      new TRGSignalVector("", (* hitList[0]).clock() , 21 * maxHit);

    for (unsigned ci = 0; ci < cList.size(); ci++) {
      unsigned cntHit = 0;
      for (unsigned hi = 0; hi < hitList.size(); hi++) {
        TRGState s = (* hitList[hi]).state(cList[ci]);
        if (s.active()) {
          if (cntHit >= maxHit) continue;
          for (unsigned j = 0; j < 21; j++) {
            if ((* hitList[hi])[j].state(cList[ci])) {
              (* result)[21 * (maxHit - 1) - (cntHit * 21) + j]
              .set(cList[ci], cList[ci] + 1);
            }
          }
          if (TRGDebug::level()) {
            TRGState t = hitList[hi]->state(cList[ci]).subset(13, 8);
            cout << TRGDebug::tab() << " hit found : TSF out local ID="
                 << unsigned(t) << "(" << t << ")" << endl;
          }

          ++cntHit;
          //            result->dump("", "??? ");
        }
      }
    }

    return result;
  }

  vector <TRGSignalVector*>
  TSFinder::simulateTSFOld(TRGSignalVector* in, unsigned tsid)
  {

    //variables for common
    const string na = "TSF" + TRGUtilities::itostring(tsid) + " in " +
                      name();
    TCSegment* tsi = _tsSL[tsid];
    vector <TRGSignalVector*> result;

    //variables for EvtTime & Low pT
    vector<bool> fTimeVect;
    //  int tmpFTime = 0 ;

    //variables for Tracker & N.N
    vector <bool> tmpOutBool;

    //iwTRGSignalVector* resultT = new TRGSignalVector(na, in->clock(),22);
    TRGSignalVector* resultT = new TRGSignalVector(na, in->clock(), 13);
    TRGSignalVector* resultE = new TRGSignalVector(na, in->clock(), 10);
    TRGSignalVector* Hitmap = new TRGSignalVector(na + "HitMap", in->clock(), 0);
    TRGSignalVector pTime(na + "PriorityTime", in->clock(), 0);
    TRGSignalVector fTime(na + "FastestTime", in->clock(), 0);
    TRGSignal* pri0 = 0;
    TRGSignal* pri1 = 0;
    TRGSignal* pri2 = 0;
    if (_type == innerType) {
      for (unsigned i = 0; i < 16; i++) {
        Hitmap->push_back((* in)[i]);
        (* Hitmap)[i].widen(16);
      }
      for (unsigned i = 0; i < 4; i++) {
        pTime.push_back((* in)[i + 16]);
        fTime.push_back((* in)[i + 20]);
      }
      pri0 = & (*Hitmap)[1];
      pri1 = & (*Hitmap)[2];
      pri2 = & (*Hitmap)[3];
    } else {
      for (unsigned i = 0; i < 12; i++) {
        Hitmap->push_back((* in)[i]);
        (* Hitmap)[i].widen(16);
      }
      for (unsigned i = 0; i < 4; i++) {
        pTime.push_back((* in)[i + 12]);
        fTime.push_back((* in)[i + 16]);
      }
      pri0 = & (*Hitmap)[6];
      pri1 = & (*Hitmap)[7];
      pri2 = & (*Hitmap)[8];
    }

    //...Clock counter...
    const TRGSignalVector& cc = in->clock().clockCounter();
    for (unsigned i = 0; i < 5; i++) {
      pTime.push_back(cc[i]);
      fTime.push_back(cc[i]);
    }

    vector <int> changeTime = Hitmap->stateChanges();

    int* LUTValue = new int[changeTime.size()];
    if (changeTime.size()) {

      const string fn = "TSF::simulateTSF:tsid=" + to_string(tsid);
      TRGDebug::enterStage(fn);

      int hitPosition = 0;
      bool fTimeBool[10];
      int tmpPTime = 0 ;
      int tmpCTime = 0 ;
      int tmpOutInt;
      fTime.state(changeTime[0]).copy2bool(fTimeBool);
      fTimeBool[9] = true;
      fTimeVect.insert(fTimeVect.begin(), fTimeBool, fTimeBool + 10);
      //tmpFTime = mkint(fTime.state(changeTime[0]));
      bool eOUT = true;
      for (unsigned i = 0; i < changeTime.size(); i++) {
        int ct = changeTime[i];

//          LUTValue[i] = tsi->LUT()->getValue(mkint(Hitmap->state(ct)));
        LUTValue[i] = tsi->LUT()->getValue(unsigned(Hitmap->state(ct)));

        /// output for EvtTime & Low pT tracker module
        if ((LUTValue[i]) && (eOUT)) {
          resultE->set(fTimeVect, ct);
          eOUT = false;
        }

        bool priority1rise = pri0->riseEdge(ct);
        bool priority2rise = pri1->riseEdge(ct) | pri2->riseEdge(ct);

        /// output for Tracker & N.N
        //ready for output
        if (priority1rise) {
          hitPosition = 3;
//              tmpPTime = mkint(pTime.state(ct));
          tmpPTime = unsigned(pTime.state(ct));
          tmpCTime = ct;
        } else if (priority2rise) {
          if (!hitPosition) {
//                  tmpPTime = mkint(pTime.state(ct));
            tmpPTime = unsigned(pTime.state(ct));
            tmpCTime = ct;
            if ((*Hitmap)[0].state(ct)) hitPosition = 2;
            else hitPosition = 1;
          }
        }

        // output selection
        if ((hitPosition) && (LUTValue[i]) && ((ct - tmpCTime) < 16)) {
          tmpOutInt = tmpPTime * pow(2, 4) +
                      LUTValue[i] * pow(2, 2) + hitPosition;
          tmpOutBool = mkbool(tmpOutInt, 13);  // ID removed : iw

          if (hitPosition == 3) {
            if (priority1rise) {
              resultT->set(tmpOutBool, ct);
            } else {
              if ((LUTValue[i] == 1) | (LUTValue[i] == 2)) {
                if (!((LUTValue[i - 1] == 1) |
                      (LUTValue[i - 1] == 2)))
                  resultT->set(tmpOutBool, ct);
              } else {
                if (!(LUTValue[i - 1])) resultT->set(tmpOutBool, ct);
              }
            }
          } else {
            if (priority2rise) resultT->set(tmpOutBool, ct);
            else {
              if ((LUTValue[i] == 1) | (LUTValue[i] == 2)) {
                if (!((LUTValue[i - 1] == 1) |
                      (LUTValue[i - 1] == 2)))
                  resultT->set(tmpOutBool, ct);
              } else {
                if (!(LUTValue[i])) resultT->set(tmpOutBool, ct);
              }
            }
          }
        }

        if (TRGDebug::level() > 1) {
          cout << TRGDebug::tab() << "clk=" << ct
               << ", pattern=" << Hitmap->state(ct)
               << ", LUT=" << LUTValue[i]
               << ", pri=" << priority1rise
               << ", sec=" << priority2rise
               << ", hitPos=" << hitPosition
               << ", (clk-tmpCTime)=" << (ct - tmpCTime)
               << endl;
        }
      }

      TRGDebug::leaveStage(fn);
    }

    result.push_back(resultT);
    result.push_back(resultE);

    delete [] LUTValue;
    delete Hitmap;

    return result;
  }

  vector <TRGSignalVector*>
  TSFinder::simulateTSF(TRGSignalVector* in, unsigned tsid)
  {

    //variables for common
    const string na = "TSF" + TRGUtilities::itostring(tsid) + " in " +
                      name();
    TCSegment* tsi = _tsSL[tsid];
    vector <TRGSignalVector*> result;

    //variables for EvtTime & Low pT
    vector<bool> fTimeVect;
    //  int tmpFTime = 0 ;

    //variables for Tracker & N.N
    vector <bool> tmpOutBool;

    TRGSignalVector* resultT = new TRGSignalVector(na, in->clock(), 13);
    TRGSignalVector* resultE = new TRGSignalVector(na, in->clock(), 10);
    TRGSignalVector* Hitmap = new TRGSignalVector(na + "HitMap",
                                                  in->clock(),
                                                  0);
    TRGSignalVector pTime(na + "PriorityTime", in->clock(), 0);
    TRGSignalVector fTime(na + "FastestTime", in->clock(), 0);
    TRGSignal* pri0 = 0;
    TRGSignal* pri1 = 0;
    TRGSignal* pri2 = 0;
    if (_type == innerType) {
      for (unsigned i = 0; i < 16; i++) {
        Hitmap->push_back((* in)[i]);
        (* Hitmap)[i].widen(16);
      }
      for (unsigned i = 0; i < 4; i++) {
        pTime.push_back((* in)[i + 16]);
        fTime.push_back((* in)[i + 20]);
      }
      pri0 = & (*Hitmap)[1];
      pri1 = & (*Hitmap)[2];
      pri2 = & (*Hitmap)[3];
    } else {
      for (unsigned i = 0; i < 12; i++) {
        Hitmap->push_back((* in)[i]);
        (* Hitmap)[i].widen(16);
      }
      for (unsigned i = 0; i < 4; i++) {
        pTime.push_back((* in)[i + 12]);
        fTime.push_back((* in)[i + 16]);
      }
      pri0 = & (*Hitmap)[6];
      pri1 = & (*Hitmap)[7];
      pri2 = & (*Hitmap)[8];
    }

    //...Clock counter...
    const TRGSignalVector& cc = in->clock().clockCounter();
    for (unsigned i = 0; i < 5; i++) {
      pTime.push_back(cc[i]);
      fTime.push_back(cc[i]);
    }

    vector<int> changeTime = Hitmap->stateChanges();

    int* LUTValue = new int[changeTime.size()];
    int oldLUT = 0;
    int lastFastHit = in->clock().min();
    if (changeTime.size()) {

      const string fn = "TSF::simulateTSF:tsid=" + to_string(tsid);
      TRGDebug::enterStage(fn);

      int hitPosition = 0;
      int tmpPTime = 0 ;
      int tmpCTime = 0 ;
      int tmpOutInt;
      TRGState ft(10);
      bool eOut = false;
      for (unsigned i = 0; i < changeTime.size(); i++) {

        int ct = changeTime[i];
        TRGState st = Hitmap->state(ct);

        //...LUT...
        // 0:no hit, 1:right, 2:left, 3:LR unknown hit
        LUTValue[i] = tsi->LUT()->getValue(unsigned(st));

        //...Any wire hit (for the fastest timing)...
        bool active = st.active();
        if (active) {
          const int timeCounter = ct - lastFastHit;

          //...Record as the fastest timing hit...
          if (timeCounter > 15) {
            lastFastHit = ct;
            TRGState ftnow = fTime.state(ct);
            ftnow += TRGState(1, 1);
            ft = ftnow;
            eOut = true;
          }
        }

        /// output for EvtTime & Low pT tracker module
        if ((LUTValue[i])) {
          if (eOut) {
            resultE->set(fTimeVect, ct);
            eOut = false;
          }
        }

        bool priority1rise = pri0->riseEdge(ct);
        bool priority2rise = pri1->riseEdge(ct) | pri2->riseEdge(ct);

        /// output for Tracker & N.N
        //ready for output
        if (priority1rise) {
          hitPosition = 3;
          tmpPTime = unsigned(pTime.state(ct));
          tmpCTime = ct;
        } else if (priority2rise) {
          if (!hitPosition) {
            tmpPTime = unsigned(pTime.state(ct));
            tmpCTime = ct;
            if ((*Hitmap)[0].state(ct)) hitPosition = 2;
            else hitPosition = 1;
          }
        }

        // output selection
        if ((ct - tmpCTime) < 16) {
          if ((hitPosition) && (LUTValue[i])) {
            //iw            tmpOutInt = tsid * pow(2, 13) + tmpPTime * pow(2, 4) +
            //iw                LUTValue[i] * pow(2,2) + hitPosition;
            tmpOutInt = tmpPTime * pow(2, 4) +
                        LUTValue[i] * pow(2, 2) + hitPosition;
            tmpOutBool = mkbool(tmpOutInt, 13);  // ID removed : iw

            if (hitPosition == 3) {
              if (priority1rise) {
                resultT->set(tmpOutBool, ct);
                oldLUT = LUTValue[i];
              } else {
                if ((LUTValue[i] == 1) | (LUTValue[i] == 2)) {
                  if (!((oldLUT == 1) |
                        (oldLUT == 2))) {
                    resultT->set(tmpOutBool, ct);
                    oldLUT = LUTValue[i];
                  }
                } else {
                  if (!(LUTValue[i - 1])) {
                    resultT->set(tmpOutBool, ct);
                    oldLUT = LUTValue[i];
                  }
                }
              }
            } else {
              if ((priority2rise) && (!oldLUT)) resultT->set(tmpOutBool, ct);
              else {
                if ((LUTValue[i] == 1) | (LUTValue[i] == 2)) {
                  if (!((oldLUT == 1) |
                        (oldLUT == 2))) {
                    resultT->set(tmpOutBool, ct);
                    oldLUT = LUTValue[i];
                  }
                } else {
                  if (!(oldLUT)) {
                    resultT->set(tmpOutBool, ct);
                    oldLUT = LUTValue[i];
                  }
                }
              }
            }
          }
        } else {
          oldLUT = 0;
          hitPosition = 0;
        }

        if (TRGDebug::level() > 1) {
          cout << TRGDebug::tab() << "clk=" << ct
               << ", pattern=" << st
               << ", LUT=" << LUTValue[i]
               << ", pri=" << priority1rise
               << ", sec=" << priority2rise
               << ", hitPos=" << hitPosition
               << ", (clk-tmpCTime)=" << (ct - tmpCTime)
               << endl;
        }
      }
      TRGDebug::leaveStage(fn);
    }

    if (TRGDebug::level()) {
      vector<int> sc = resultT->stateChanges();
      vector<unsigned> lv;
      for (unsigned i = 0; i < sc.size(); i++) {
        //...LUT value...
        unsigned l = unsigned(resultT->state(sc[i]).subset(2, 2));
        lv.push_back(l);
        cout << "clk=" << sc[i] << " LUT output[" << i << "]=" << l << endl;
      }
      bool found1or2 = false;
      bool found3 = false;
      for (unsigned i = 0; i < lv.size(); i++) {
        if (found1or2 && (lv[i] > 0))
          cout << "!!! simulateTSF something wrong(found1or2)" << endl;
        if (found3 && (lv[i] == 3))
          cout << "!!! simulateTSF something wrong(found3)" << endl;

        if ((lv[i] == 1) & (lv[i] == 2))
          found1or2 = true;
        else if (lv[i] == 3)
          found3 = true;
      }
    }

    result.push_back(resultT);
    result.push_back(resultE);

    delete [] LUTValue;
    delete Hitmap;

    return result;
  }
  vector <TRGSignalVector*>
  TSFinder::simulateTSF2(TRGSignalVector* in, unsigned tsid)
  {

    //variables for common
    const string na = "TSF" + TRGUtilities::itostring(tsid) + " in " +
                      name();
    TCSegment* tsi = _tsSL[tsid];
    vector <TRGSignalVector*> result;

    //variables for EvtTime & Low pT
    vector<bool> fTimeVect;
    //  int tmpFTime = 0 ;

    //variables for Tracker & N.N
    TRGSignalVector* resultT = new TRGSignalVector(na, in->clock(), 13);
    TRGSignalVector* resultE = new TRGSignalVector(na, in->clock(), 10);
    TRGSignalVector* Hitmap = new TRGSignalVector(na + "HitMap",
                                                  in->clock(),
                                                  0);
    TRGSignalVector pTime(na + "PriorityTime", in->clock(), 0);
    TRGSignalVector fTime(na + "FastestTime", in->clock(), 0);
    TRGSignal* pri0 = 0;
    TRGSignal* pri1 = 0;
    TRGSignal* pri2 = 0;

    if (_type == innerType) {
      for (unsigned i = 0; i < 16; i++) {
        Hitmap->push_back((* in)[i]);
        (* Hitmap)[i].widen(16);
      }
      for (unsigned i = 0; i < 4; i++) {
        pTime.push_back((* in)[i + 16]);
        fTime.push_back((* in)[i + 20]);
      }
      pri0 = & (*Hitmap)[1];
      pri1 = & (*Hitmap)[2];
      pri2 = & (*Hitmap)[3];
    } else {
      for (unsigned i = 0; i < 12; i++) {
        Hitmap->push_back((* in)[i]);
        (* Hitmap)[i].widen(16);
      }
      for (unsigned i = 0; i < 4; i++) {
        pTime.push_back((* in)[i + 12]);
        fTime.push_back((* in)[i + 16]);
      }
      pri0 = & (*Hitmap)[6];
      pri1 = & (*Hitmap)[7];
      pri2 = & (*Hitmap)[8];
    }

    //...Clock counter...
    const TRGSignalVector& cc = in->clock().clockCounter();
    for (unsigned i = 0; i < 5; i++) {
      pTime.push_back(cc[i]);
      fTime.push_back(cc[i]);
    }

    vector<int> changeTime = Hitmap->stateChanges();

    vector<unsigned> luts;
    int lastFastHit = in->clock().min();

    if (changeTime.size()) {

      const string fn = "TSF::simulateTSF2:tsid=" + to_string(tsid);
      TRGDebug::enterStage(fn);

      TRGState ft(10);
      bool eOut = false;

      unsigned stateHitPos = 0; // 0:wait, 1:active1st, 2:active2nd
      unsigned hitPos = 0;
      unsigned timing = 0;
      int timePosFound = 0;
      int counterPos = 0;

      unsigned stateLR = 0; // 0:wait, 1:LUT12, 2:LUT3
//      int timeLRFound = 0;
//      int counterLR = 0;

      for (unsigned i = 0; i < changeTime.size(); i++) {

        const int ct = changeTime[i];
        const TRGState st = Hitmap->state(ct);

        //...LUT...
        // 0:no hit, 1:right, 2:left, 3:LR unknown hit
        const unsigned lut = tsi->LUT()->getValue(unsigned(st));
        luts.push_back(lut);

        //...Any wire hit (for the fastest timing)...
        const bool active = st.active();
        if (active) {
          const int timeCounter = ct - lastFastHit;

          //...Record as the fastest timing hit...
          if (timeCounter > 15) {
            lastFastHit = ct;
            TRGState ftnow = fTime.state(ct);
            ftnow += TRGState(1, 1);
            ft = ftnow;
            eOut = true;
          }
        }

        // output for EvtTime & Low pT tracker module
        if (lut) {
          if (eOut) {
            resultE->set(fTimeVect, ct);
            eOut = false;
          }
        }

        //...Hit position...
        const bool hit1st = pri0->riseEdge(ct);
        const bool hit2nd0 = pri1->riseEdge(ct);
        const bool hit2nd1 = pri2->riseEdge(ct);
        const bool hit2nd = hit2nd0 || hit2nd1;

        //...Hit position state machine...

        //...State : wait...
        if (stateHitPos == 0) {
          hitPos = 0;
          timing = 0;
          timePosFound = 0;
          if (hit1st) {
            hitPos = 3;
            timing = unsigned(pTime.state(ct));
            timePosFound = ct;
            stateHitPos = 1;
          } else if (hit2nd) {
            if ((* Hitmap)[0].state(ct))
              hitPos = 2;
            else
              hitPos = 1;
            timing = unsigned(pTime.state(ct));
            timePosFound = ct;
            stateHitPos = 2;
          }
        }

        //...State : active1st...
        else if (stateHitPos == 1) {
          counterPos = ct - timePosFound;
          if (counterPos > 15)
            stateHitPos = 0;
        }

        //...State : active2nd...
        else if (stateHitPos == 2) {
          if (hit1st) {
            hitPos = 3;
            timing = unsigned(pTime.state(ct));
            timePosFound = ct;
            stateHitPos = 1;
          } else {
            counterPos = ct - timePosFound;
            if (counterPos > 15)
              stateHitPos = 0;
          }
        }

        //...State unknown...
        else {
          cout << "TSF::simulateTSF2 !!! strange state in hit position"
               << endl;
        }

        //...L/R decision state machine...

        //...State : wait...
        if (stateLR == 0) {
          if (lut != 0) {
            const unsigned val = (timing << 4) | (lut << 2) | hitPos;
            const TRGState output(13, val);
            resultT->set(output, ct);
//                  timeLRFound = ct;
            if ((lut == 1) || (lut == 2))
              stateLR = 1;
            else
              stateLR = 2;

            if (TRGDebug::level()) {
              if (((lut == 1) || (lut == 2)) && (stateHitPos == 0))
                cout << TRGDebug::tab()
                     << "!!! state machines incosistent" << endl;
            }
          }
        }

        //...State : LUT12...
        else if (stateLR == 1) {
//              counterLR = ct - timeLRFound;
          if (counterPos > 15)
            stateLR = 0;
        }

        //...State : LUT3...
        else if (stateLR == 2) {
          if (lut) {
            if ((lut == 1) || (lut == 2)) {
              const unsigned val = (timing << 4) | (lut << 2) |
                                   hitPos;
              const TRGState output(13, val);
              resultT->set(output, ct);
//                      timeLRFound = ct;
              stateLR = 1;
            } else {
//                      counterLR = ct - timeLRFound;
              if (counterPos > 15)
                stateLR = 0;
            }
          }
        }

        //...State unknown...
        else {
          cout << "TSF::simulateTSF2 !!! strange state in L/R decision"
               << endl;
        }

        if (TRGDebug::level() > 1) {
          cout << TRGDebug::tab() << "clk=" << ct
               << ", ptn=" << st
               << ", LUT=" << lut
               << ", pri=" << hit1st
               << ", sec=" << hit2nd
               << ", hPos=" << hitPos
               << ", ctr=" << counterPos // << "," << counterLR
               << ", states=" << stateHitPos << "," << stateLR
               << endl;
        }
      }
      TRGDebug::leaveStage(fn);
    }

    if (TRGDebug::level()) {
      vector<int> sc = resultT->stateChanges();
      vector<unsigned> lv;
      for (unsigned i = 0; i < sc.size(); i++) {
        //...LUT value...
        unsigned l = unsigned(resultT->state(sc[i]).subset(2, 2));
        lv.push_back(l);
        cout << "clk=" << sc[i] << " LUT output[" << i << "]=" << l << endl;
      }
    }

    result.push_back(resultT);
    result.push_back(resultE);

    delete Hitmap;

    return result;
  }

} // namespace Belle2
