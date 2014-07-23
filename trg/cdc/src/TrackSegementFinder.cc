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

TRGCDCTrackSegmentFinder::TRGCDCTrackSegmentFinder(const TRGCDC & TRGCDC,
                                                   bool makeRootFile,
                                                   bool logicLUTFlag)
    : TRGBoard("", TRGClock("",0,0), TRGClock("",0,0), TRGClock("",0,0), TRGClock("",0,0)), 
      _cdc(TRGCDC),
      m_logicLUTFlag(logicLUTFlag),
      m_makeRootFile(makeRootFile) {

    m_Trg_PI = 3.141592653589793; 

    // For ROOT file
    if(m_makeRootFile) m_fileTSF = new TFile("TSF.root","RECREATE");

    m_treeInputTSF = new TTree("m_treeInputTSF", "InputTSF");
    m_hitPatternInformation = new TClonesArray("TVectorD");
    m_treeInputTSF->Branch("hitPatternInformation", &m_hitPatternInformation,32000,0);

    m_treeOutputTSF = new TTree("m_treeOutputTSF", "OutputTSF");
    m_particleEfficiency = new TClonesArray("TVectorD");
    m_treeOutputTSF->Branch("particleEfficiency", &m_particleEfficiency,32000,0);
    m_tsInformation = new TClonesArray("TVectorD");
    m_treeOutputTSF->Branch("tsInformation", &m_tsInformation,32000,0);

    // For neural network TSF. Filled only when TSF and priority is hit.
    m_treeNNTSF = new TTree("m_treeNNTSF", "NNTSF");
    m_nnPatternInformation = new TClonesArray("TVectorD");
    m_treeNNTSF->Branch("nnPatternInformation", &m_nnPatternInformation,32000,0);

}


TRGCDCTrackSegmentFinder::TRGCDCTrackSegmentFinder(const TRGCDC & TRGCDC,
						   const std::string & name,
						   boardType type,
						   const TRGClock & systemClock,
						   const TRGClock & dataClock,
						   const TRGClock & userClockInput,
						   const TRGClock & userClockOutput,
		std::vector<TCSegment*> & tsSL)
    : TRGBoard(name, systemClock,dataClock,userClockInput, userClockOutput),
     _cdc(TRGCDC),
     _type(type),
//     _tisb(0),
     _tosbE(0),
     _tosbT(0),
     _tsSL(tsSL){

    }
TRGCDCTrackSegmentFinder::~TRGCDCTrackSegmentFinder() {

    delete m_nnPatternInformation;
    delete m_treeNNTSF;
    delete m_tsInformation;
    delete m_particleEfficiency;
    delete m_treeOutputTSF;
    delete m_hitPatternInformation;
    delete m_treeInputTSF;
    if(m_makeRootFile) delete m_fileTSF;

}

TRGCDCTrackSegmentFinder::boardType
TRGCDCTrackSegmentFinder:: type(void) const{
    return _type;
}
void TRGCDCTrackSegmentFinder::terminate(void) {
    if(m_makeRootFile) {
        m_fileTSF->Write();
        m_fileTSF->Close();
    }
}

void TRGCDCTrackSegmentFinder::doit(std::vector<TRGCDCSegment* >& tss, const bool trackSegmentClockSimulation,
                                    std::vector<TRGCDCSegmentHit* >& segmentHits, std::vector<TRGCDCSegmentHit* >* segmentHitsSL) {
    TRGDebug::enterStage("Track Segment Finder");

    // Saves TS information
    saveTSInformation(tss);

//cout << "Hello World1" << endl;
    //...Store TS hits...
    const unsigned n = tss.size();
    for (unsigned i = 0; i < n; i++) {
        TCSegment & s = * tss[i];
        s.simulate(trackSegmentClockSimulation, m_logicLUTFlag);
        if (s.signal().active()) {
            TCSHit * th = new TCSHit(s);
            s.hit(th);
            segmentHits.push_back(th);
            segmentHitsSL[s.layerId()].push_back(th);
        }
    }

    // Save TSF results
    saveTSFResults(segmentHitsSL);

    // Saves NNTS information. Only when ts is hit.
    saveNNTSInformation(tss);

    if (TRGDebug::level() > 1) {
        cout << TRGDebug::tab() << "TS hit list" << endl;
        string dumpOption = "trigger";
        if (TRGDebug::level() > 2)
            dumpOption = "detail";
        for (unsigned i = 0; i < _cdc.nSegments(); i++) {
            const TCSegment & s = _cdc.segment(i);
            if (s.signal().active())
                s.dump(dumpOption, TRGDebug::tab(4));
        }

        cout << TRGDebug::tab() << "TS hit list (2)" << endl;
        if (TRGDebug::level() > 2)
            dumpOption = "detail";
        for (unsigned i = 0; i < segmentHits.size(); i++) {
            const TCSHit & s = * segmentHits[i];
            s.dump(dumpOption, TRGDebug::tab(4));
        }

        cout << TRGDebug::tab() << "TS hit list (3)" << endl;
        if (TRGDebug::level() > 2)
            dumpOption = "detail";
        for (unsigned j = 0; j < _cdc.nSuperLayers(); j++) {
            for (unsigned i = 0; i < segmentHitsSL[j].size(); i++) {
                const vector<TCSHit*> & s = segmentHitsSL[j];
                for (unsigned k = 0; k < s.size(); k++)
                    s[k]->dump(dumpOption, TRGDebug::tab(4));
            }
        }
    }

    TRGDebug::leaveStage("Track Segment Finder");
}


void TRGCDCTrackSegmentFinder::simulateBoard(void){

    //... Clear signal bundle...
//    if(_tisb){
//      for(unsigned i=0;i<_tisb->size();i++)
//        delete (* _tisb)[i];
//      delete _tisb;
//    }
    if(_tosbE){
      for(unsigned i=0;i<_tosbE->size();i++)
        delete(* _tosbE)[i];
      delete _tosbE;
    }
    if(_tosbT){
      for(unsigned i=0;i<_tosbT->size();i++)
        delete(* _tosbT)[i];
      delete _tosbT;
    }

    //... Clock..
    const TRGClock &dClock = TRGCDC::getTRGCDC()->dataClock();

    //... Make input signal bundle
    TRGSignalVector input(name()+"inputMerger",dClock);
    const string ni= name()+"InputSignalBundle";
//    _tisb = new TRGSignalBundle(ni, dClock);

    TRGSignalVector * trackerOut;
    TRGSignalVector * evtOut;
    //TRGSignalVector *inputM= new TRGSignalVector(*( (*(*this)[0]->output())[0]));
    vector <TRGSignalVector*> inputM;
    vector <TRGSignalVector*> findOUTTrack;
    vector <TRGSignalVector*> findOUTEvt;
    vector <int> tmpCTimeListE;
    vector <int> changeTimeListE;
    vector <int> tmpCTimeListT;
    vector <int> changeTimeListT;
    inputM.resize((*this).size());
    for(unsigned i=0;i<(*this).size();i++){
	inputM[i] = new TRGSignalVector(*( (*(*this)[i]->output())[0]));
    }

//    TRGSignalVector *inputM= new TRGSignalVector(*( (*(*this)[0]->output())[0]));

    for (unsigned i=0;i<inputM[0]->size();i++){

      TRGSignal msig = (*(inputM[0]))[i];
      input += msig;
    }
  
    vector<TRGSignalVector*> separateTS;
    TRGSignalVector* clockCounter = new TRGSignalVector(input[236]);
    for(int i=0;i<8;i++){
      clockCounter->push_back(input[236+i]);
    }
    separateTS.resize(16);
    //separateTS.resize((*this).size()*16);
    int nTS=separateTS.size();
    findOUTTrack.resize(nTS);
    findOUTEvt.resize(nTS);
    if(type()==outerType){
      for(int i=0;i<nTS;i++){
        separateTS[i]= new TRGSignalVector(input[i+208]);
       /// HitMap
        if(i==0){
  	  separateTS[i]->push_back(input[255]);
        } else{
          separateTS[i]->push_back(input[i-1]);
        }
        separateTS[i]->push_back(input[i]);
        if(i==nTS){
          separateTS[i]->push_back(input[255]);
        }
        else{
          separateTS[i]->push_back(input[i+1]);
        }
        if(i==0){
          separateTS[i]->push_back(input[255]);
        }
        else{
        separateTS[i]->push_back(input[i+15]);
        }
        separateTS[i]->push_back(input[i+16]);
        separateTS[i]->push_back(input[i+32]);
        if(i==0){
          separateTS[i]->push_back(input[255]);
        }
        else{
          separateTS[i]->push_back(input[i+47]);
        }
        separateTS[i]->push_back(input[i+48]);
        if(i==0){
          separateTS[i]->push_back(input[255]);
        }
        else{
          separateTS[i]->push_back(input[i+63]);
        }
        separateTS[i]->push_back(input[i+64]);
        if(i==nTS){
          separateTS[i]->push_back(input[255]);
        }
        else{
          separateTS[i]->push_back(input[i+65]);
        }
  
  /// priority timing
        separateTS[i]->push_back(input[4*i+80]);
        separateTS[i]->push_back(input[4*i+81]);
        separateTS[i]->push_back(input[4*i+82]);
        separateTS[i]->push_back(input[4*i+83]);
  /// fastest timing
        separateTS[i]->push_back(input[4*i+144]);
        separateTS[i]->push_back(input[4*i+145]);
        separateTS[i]->push_back(input[4*i+146]);
        separateTS[i]->push_back(input[4*i+147]);
  /// clock counter part
        separateTS[i]->push_back(input[236]);
        separateTS[i]->push_back(input[237]);
        separateTS[i]->push_back(input[238]);
        separateTS[i]->push_back(input[239]);
        separateTS[i]->push_back(input[240]);
      
      }
// Board Type Check Start.(for future)
      for(int i=0;i<nTS;i++){
        findOUTTrack[i] = findTSHit(separateTS[i],i)[0];
        vector<int> tt = findOUTTrack[i]->stateChanges();
        if(tt.size()){
          tmpCTimeListT.insert(tmpCTimeListT.end(),tt.begin(),tt.end());
        }

        findOUTEvt[i] = findTSHit(separateTS[i],i)[1];
        vector<int> ee = findOUTEvt[i]->stateChanges();
        if(ee.size()){
          tmpCTimeListE.insert(tmpCTimeListE.end(),ee.begin(),ee.end());
        }
      }

      for (unsigned i=0; i<tmpCTimeListT.size();i++){
        bool nomatch = true;
        for (unsigned j=0;j<changeTimeListT.size();j++){
          if(tmpCTimeListT[i]==changeTimeListT[j]){ 
            nomatch = false;
            break;
          }
        }
        if(nomatch) changeTimeListT.push_back(tmpCTimeListT[i]);
      }

      trackerOut = packerOuterTracker(findOUTTrack, changeTimeListT, 6);
      (*trackerOut).insert((*trackerOut).end(),(*clockCounter).begin(),(*clockCounter).end());
      trackerOut->name("TSF TrackerOut");

      const string noT = name()+"OutputSignalBundleTracker";
      _tosbT =  new TRGSignalBundle(noT,dClock); 
      _tosbT->push_back(trackerOut);
      if(_tosbT){
      (*_tosbT)[0]->name(noT);
      for(unsigned i=0;i<(*(*_tosbT)[0]).size();i++){
        (*(*_tosbT)[0])[i].name(noT+":bit"+TRGUtilities::itostring(i));
      }
//      _tosbT->dump("","");
      }

      for (unsigned i=0; i<tmpCTimeListE.size();i++){
        bool nomatch = true;
        for (unsigned j=0;j<changeTimeListE.size();j++){
          if(tmpCTimeListE[i]==changeTimeListE[j]){ 
            nomatch = false;
            break;
          }
        }
        if(nomatch) changeTimeListE.push_back(tmpCTimeListE[i]);
      }
      evtOut = packerOuterEvt(findOUTEvt, changeTimeListE, 6);
      (*evtOut).insert((*evtOut).end(),(*clockCounter).begin(),(*clockCounter).end());

      const string noE = name()+"OutputSignalBundleEvt";
      _tosbE =  new TRGSignalBundle(noE,dClock); 
      _tosbE->push_back(evtOut);
      if(_tosbE){
      (*_tosbE)[0]->name(noE);
      for(unsigned i=0;i<(*(*_tosbE)[0]).size();i++){
        (*(*_tosbE)[0])[i].name(noE+":bit"+TRGUtilities::itostring(i));
      }
//      _tosbE->dump("","");
      }
// Board Type Check End.
    }

}

vector <TRGSignalVector*>
TSFinder::findTSHit(TRGSignalVector* eachInput, int tsid){
   
//variables for common
  const string na = "TSCandidate" + TRGUtilities::itostring(tsid) + " in " + name();
  TCSegment * tsi = _tsSL[tsid];
  vector <TRGSignalVector*> result;

//variables for EvtTime & Low pT
  vector<bool> fTimeVect;
  bool fTimeBool[10];
  bool eOUT= true;
//  int tmpFTime = 0 ;

//variables for Tracker & N.N
  int hitPosition=0;
  int tmpPTime = 0 ;
  int tmpCTime = 0 ;
  vector <bool> tmpOutBool;
  int tmpOutInt;

  TRGSignalVector* resultT = new TRGSignalVector(na, eachInput->clock(),22);
  TRGSignalVector* resultE = new TRGSignalVector(na, eachInput->clock(),10);
  TRGSignalVector* Hitmap = new TRGSignalVector(na+"HitMap",eachInput->clock(),0);
  TRGSignalVector* pTime= new TRGSignalVector(na+"PriorityTime",eachInput->clock(),0);
  TRGSignalVector* fTime= new TRGSignalVector(na+"FastestTime",eachInput->clock(),0);
  for (unsigned i=0; i<12;i++){
    Hitmap->push_back((*eachInput)[i]);
    (*Hitmap)[i].widen(16);
  }
  for(unsigned i=0;i<4;i++){
    pTime->push_back((*eachInput)[i+12]);
    fTime->push_back((*eachInput)[i+16]);
  }
  for(unsigned i=0;i<5;i++){
    pTime->push_back((*eachInput)[i+20]);
    fTime->push_back((*eachInput)[i+20]);
  }

  vector <int> changeTime = Hitmap->stateChanges();

  int * LUTValue = new int[changeTime.size()];
  if(changeTime.size()){
    fTime->state(changeTime[0]).copy2bool(fTimeBool);
    fTimeBool[10]=true;
    fTimeVect.insert(fTimeVect.begin(),fTimeBool,fTimeBool+10);
    //tmpFTime = mkint(fTime->state(changeTime[0]));
    for(unsigned i=0;i<changeTime.size();i++){
      LUTValue[i] = tsi->nLUT()->getValue(mkint(Hitmap->state(changeTime[i])));

      /// output for EvtTime & Low pT tracker module
      if((LUTValue[i])&&(eOUT)){
        resultE->set(fTimeVect,changeTime[i]);
        eOUT= false;
      }
     
      bool priority1rise = (*Hitmap)[6].riseEdge(changeTime[i]);
      bool priority2rise = ((*Hitmap)[7].riseEdge(changeTime[i])|(*Hitmap)[8].riseEdge(changeTime[i]));

    /// output for Tracker & N.N
      //ready for output
      if(priority1rise){
	    hitPosition=3;
            tmpPTime= mkint(pTime->state(changeTime[i]));
    	    tmpCTime = changeTime[i];
      }else if(priority2rise){
        if(!hitPosition){
	  tmpPTime = mkint(pTime->state(changeTime[i]));
          tmpCTime = changeTime[i];
          if((*Hitmap)[0].state(changeTime[i])) hitPosition = 2;
          else hitPosition = 1;
        }
      }

      // output selection
      if((hitPosition)&&(LUTValue[i])&&((changeTime[i]-tmpCTime)<16)){
        tmpOutInt = tsid*pow(2,13)+tmpPTime*pow(2,4)+LUTValue[i]*pow(2,2)+hitPosition;
        tmpOutBool = mkbool(tmpOutInt, 22);
        if(hitPosition==3){
          if(priority1rise) resultT->set(tmpOutBool,changeTime[i]);
          else{
            if((LUTValue[i]==1)|(LUTValue[i]==2)){
              if((LUTValue[i-1]==1)|(LUTValue[i-1]==2)){
              }else resultT->set(tmpOutBool,changeTime[i]);
            }else{
              if(!(LUTValue[i-1])) resultT->set(tmpOutBool,changeTime[i]);
            }
          }
        }else{
          if(priority2rise) resultT->set(tmpOutBool,changeTime[i]);
          else{
            if((LUTValue[i]==1)|(LUTValue[i]==2)){
              if((LUTValue[i-1]==1)|(LUTValue[i-1]==2)){
              }else resultT->set(tmpOutBool,changeTime[i]);
            }else{
              if(!(LUTValue[i])) resultT->set(tmpOutBool,changeTime[i]);
            }
          }
        }
      }

    }

  }
result.push_back(resultT);
result.push_back(resultE);

  return result;
}

TRGSignalVector*
TSFinder::packerOuterTracker(vector<TRGSignalVector*> hitList, vector<int> cList, int maxHit){

  TRGSignalVector * result = new TRGSignalVector("",(*hitList[0]).clock() ,22*maxHit);

  for(unsigned ci=0; ci<cList.size();ci++){
    int cntHit=0;
    for(unsigned hi=0;hi<hitList.size();hi++){
      if((*hitList[hi]).state(cList[ci]).active()){
        if(cntHit>=maxHit) continue;
        for(unsigned j=0;j<((*hitList[hi])).size();j++){
          if((*hitList[hi])[j].state(cList[ci])){
            (*result)[22*(maxHit-1)-(cntHit*22)+j].set(cList[ci],cList[ci]+1);
          }
        }
        cntHit++;
      }
    }
  }
  return result;
}

TRGSignalVector*
TSFinder::packerOuterEvt(vector<TRGSignalVector*> hitList, vector<int> cList,int maxHit){

  //TRGSignalVector * result = new TRGSignalVector("",(*hitList[0]).clock() ,N+9*maxHit);
  TRGSignalVector * result = new TRGSignalVector("",(*hitList[0]).clock() ,hitList.size()+9*maxHit);

  for(unsigned ci=0; ci<cList.size();ci++){
    int cntHit=0;
    for(unsigned hi=0;hi<hitList.size();hi++){
      if((*hitList[hi]).state(cList[ci]).active()){
        (*result)[9*maxHit+hi].set(cList[ci],cList[ci]+1);
        if(cntHit>=maxHit) continue;
        for(unsigned j=0;j<(((*hitList[hi])).size()-1);j++){
          if((*hitList[hi])[j].state(cList[ci])){
            (*result)[9*(maxHit-1)-(cntHit*9)+j].set(cList[ci],cList[ci]+1);
          }
        }
        cntHit++;
      }
    }
  }
  return result;
}

double
TSFinder::mkint(TRGState bitInput){
double r=0;
bool * binput = new bool[bitInput.size()];
bitInput.copy2bool(binput);
for(unsigned i=0;i<bitInput.size();i++){
if(binput[i]){
  r+= pow(2,i);
} 
}
delete[] binput;
return r;
}

vector<bool>
TSFinder::mkbool(int N, int bitSize){
  vector<bool> boutput;
  boutput.resize(bitSize);
  int tmpint=N;
  for(unsigned i=0;tmpint;i++){
   if(tmpint%2) boutput[i]=true;
   else boutput[i]=false;
   tmpint=tmpint/2;
  }

  return boutput;
}

//

void TRGCDCTrackSegmentFinder::saveTSInformation(std::vector<TRGCDCSegment* >& tss) {

    TClonesArray& hitPatternInformation = *m_hitPatternInformation;
    hitPatternInformation.Clear();

    StoreArray<CDCSimHit> SimHits;
    if (! SimHits) {
        //cout << "CDCTRGTSF !!! can not access to CDCSimHits" << endl;
        return;
    }
    StoreArray<MCParticle> mcParticles;
    if (! mcParticles) {
        //cout << "CDCTRGTSF !!! can not access to MCParticles" << endl;
        return;
    }
    RelationArray cdcSimHitRel(mcParticles, SimHits);
    // It is a one-one relationship. Confirmed with 100 events. [2013.08.05]
    map<int, int> simHitsMCParticlesMap;
    // Change RelationArray to a map
    // Loop over all particles
    for(int iPart=0; iPart < cdcSimHitRel.getEntries(); iPart++) {
        // Loop over all hits for particle
        for(unsigned iHit=0; iHit < cdcSimHitRel[iPart].getToIndices().size(); iHit++) {
            //cout<<"From: "<<cdcSimHitRel[iPart].getFromIndex()<<" To: "<<cdcSimHitRel[iPart].getToIndex(iHit)<<endl;
            simHitsMCParticlesMap[cdcSimHitRel[iPart].getToIndex(iHit)] = cdcSimHitRel[iPart].getFromIndex();
        }
    }
    //// Print map
    //for(map<int, int >::iterator it = simHitsMCParticlesMap.begin(); it != simHitsMCParticlesMap.end(); it++) {
    //  cout<<"SimHit Index: "<<(*it).first<<" MCParticle Index: "<<(*it).second<<endl;
    //}

    //cout<<"Save TS information"<<endl;
    // Loop over all TSs. wireHit has only one wireSimHit. Meaning there is a particle overlap hit bug.
    const unsigned nTSs = tss.size();
    unsigned nHitTSs = 0;
    for (unsigned iTS = 0; iTS < nTSs; iTS++) {
        const TCSegment & ts = * tss[iTS];
        const TRGCDCWire* priority;
        const TRGCDCWire* secondPriorityL;
        const TRGCDCWire* secondPriorityR;
        // Find priority wires
        if(ts.wires().size() == 15) {
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
        if(priority->hit() != 0) {
            int iSimHit = priority->hit()->iCDCSimHit();
            priorityLRs[0] = SimHits[iSimHit]->getPosFlag();
            posOnTrack = SimHits[iSimHit]->getPosTrack();
            posOnWire = SimHits[iSimHit]->getPosWire();
            priorityPhis[0] = (posOnTrack - posOnWire).Phi() + m_Trg_PI/2 - posOnWire.Phi();
            //cout<<ts.name()<<endl;
            //cout<<"Track: "<<posOnTrack.x()<<" "<<posOnTrack.y()<<" "<<posOnTrack.z()<<endl;
            //cout<<"Wire:  "<<posOnWire.x()<<" "<<posOnWire.y()<<" "<<posOnWire.z()<<endl;
            //cout<<"Before Phi: "<<(posOnTrack - posOnWire).Phi()<<" PosOnWirePhi: "<<posOnWire.Phi()<<" After Phi: "<<priorityPhis[0]<<endl;
            //cout<<"LR: "<<priorityLRs[0]<<endl;
        } else {
            priorityLRs[0] = -1;
            priorityPhis[0] = 9999;
        }
        if(secondPriorityR->hit() != 0) {
            int iSimHit = secondPriorityR->hit()->iCDCSimHit();
            priorityLRs[1] = SimHits[iSimHit]->getPosFlag();
            posOnTrack = SimHits[iSimHit]->getPosTrack();
            posOnWire = SimHits[iSimHit]->getPosWire();
            priorityPhis[1] = (posOnTrack - posOnWire).Phi() + m_Trg_PI/2 - posOnWire.Phi();
        } else {
            priorityLRs[1] = -1;
            priorityPhis[1] = 9999;
        }
        if(secondPriorityL->hit() != 0) {
            int iSimHit = secondPriorityL->hit()->iCDCSimHit();
            priorityLRs[2] = SimHits[iSimHit]->getPosFlag();
            posOnTrack = SimHits[iSimHit]->getPosTrack();
            posOnWire = SimHits[iSimHit]->getPosWire();
            priorityPhis[2] = (posOnTrack - posOnWire).Phi() + m_Trg_PI/2 - posOnWire.Phi();
        } else {
            priorityLRs[2] = -1;
            priorityPhis[2] = 9999;
        }

        const unsigned nWires = ts.wires().size();
        unsigned nHitWires = 0;
        // Find TSPatternInformation for each particle
        map<int, unsigned > particleTSPattern;
        // Loop over wires in TS
        for (unsigned iWire = 0; iWire < nWires; iWire++) {
            //...Copy signal from a wire...
            const TRGSignal & wireSignal = ts.wires()[iWire]->signal();
            if (wireSignal.active()) ++nHitWires;
            // Find MC particle of hit wire
            const TRGCDCWireHit* wireHit = ts.wires()[iWire]->hit();
            if(wireHit != 0) {
                int iMCParticle = simHitsMCParticlesMap[wireHit->iCDCSimHit()];
                // If new particle
                if(particleTSPattern[iMCParticle] == 0) {
                    unsigned tsPattern;
                    tsPattern = 1 << iWire;
                    particleTSPattern[iMCParticle] = tsPattern;
                } else {
                    particleTSPattern[iMCParticle] |= 1 << iWire;;
                }
                //cout<<ts.name()<<" "<<ts.wires()[iWire]->name()<<" was hit.";
                //cout<<" Particle was "<<mcParticles[simHitsMCParticlesMap[wireHit->iCDCSimHit()]]->getPDG()<<endl;
                //cout<<"iWire["<<iWire<<"] iCDCSimHit["<<wireHit->iCDCSimHit()<<"]: "<<simHitsMCParticlesMap[wireHit->iCDCSimHit()]<<endl;
            }
        } // Loop over wires in TS

        // Print pattern for each particle
        //for(map<int, unsigned >::const_iterator it = particleTSPattern.begin(); it != particleTSPattern.end(); it++ ) {
        //  bitset<15> printPattern((*it).second);
        //  cout<<ts.name()<<" MC Particle: "<< (*it).first <<" pattern: "<<printPattern<<endl;
        //}

        if(nHitWires != 0) {
            // Ignore TSPatterns that have 2 particles passing TS.
            if(particleTSPattern.size() == 1) {
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
    m_treeInputTSF->Fill();
    //cout<<"End saving TS information"<<endl;
} // End of save function



void TRGCDCTrackSegmentFinder::saveTSFResults(std::vector<TRGCDCSegmentHit* >* segmentHitsSL) {

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
        for(int iPart=0; iPart < cdcSimHitRel.getEntries(); iPart++) {
            // Loop over all hits for particle
            for(unsigned iHit=0; iHit < cdcSimHitRel[iPart].getToIndices().size(); iHit++) {
                //cout<<"From: "<<cdcSimHitRel[iPart].getFromIndex()<<" To: "<<cdcSimHitRel[iPart].getToIndex(iHit)<<endl;
                simHitsMCParticlesMap[cdcSimHitRel[iPart].getToIndex(iHit)] = cdcSimHitRel[iPart].getFromIndex();
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
        for( int iSuperLayer = 0; iSuperLayer < 9; iSuperLayer++) {
            map<int, bool> particleHitTS;
            for( unsigned iTS = 0; iTS < segmentHitsSL[iSuperLayer].size(); iTS++) {
                const TCSegment & ts = segmentHitsSL[iSuperLayer][iTS]->segment();
                unsigned nWires = ts.wires().size();
                for (unsigned iWire = 0; iWire < nWires; iWire++) {
                    const TRGCDCWireHit* wireHit = ts.wires()[iWire]->hit();
                    if(wireHit != 0) {
                        int iMCParticle = simHitsMCParticlesMap[wireHit->iCDCSimHit()];
                        if(particleNHitTS[iMCParticle] == 0) {
                            unsigned hitTSSL;
                            hitTSSL = 1 << iSuperLayer;
                            particleNHitTS[iMCParticle] = hitTSSL;
                        } else {
                            particleNHitTS[iMCParticle] |= 1 << iSuperLayer;
                        }
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
        vector<vector<float> > tsEfficiency;
        // Loop over all particles
        for(int iPart=0; iPart < cdcSimHitRel.getEntries(); iPart++) {
            int lastWireHit = -1;
            // Loop over all hits for particle
            for(unsigned iHit=0; iHit < cdcSimHitRel[iPart].getToIndices().size(); iHit++) {
                int iSimHit = cdcSimHitRel[iPart].getToIndex(iHit);
                if (SimHits[iSimHit]->getWireID().getICLayer() > lastWireHit) lastWireHit = SimHits[iSimHit]->getWireID().getICLayer();
                //cout<<"Particle: "<<cdcSimHitRel[iPart].getFromIndex()<<" CDCSimHit: "<<iSimHit<<endl;
                //cout<<"SuperLayer: "<<SimHits[iSimHit]->getWireID().getISuperLayer()<<" wireLayer: "<<SimHits[iSimHit]->getWireID().getICLayer()<<endl;
            }
            //cout<<"iMCParticle: "<<cdcSimHitRel[iPart].getFromIndex()<<" Last wire Hit: "<<lastWireHit<<endl;
            // Calculate last superlayer
            int lastSLHit = 0;
            if( lastWireHit >= 53 ) lastSLHit = 9;
            else if( lastWireHit >= 47 ) lastSLHit = 8;
            else if( lastWireHit >= 41 ) lastSLHit = 7;
            else if( lastWireHit >= 35 ) lastSLHit = 6;
            else if( lastWireHit >= 29 ) lastSLHit = 5;
            else if( lastWireHit >= 23 ) lastSLHit = 4;
            else if( lastWireHit >= 17 ) lastSLHit = 3;
            else if( lastWireHit >= 11 ) lastSLHit = 2;
            else if( lastWireHit >= 5 ) lastSLHit = 1;
            // Get number of hit TS for particle
            int iMCParticle = cdcSimHitRel[iPart].getFromIndex();
            bitset<9> hitSuperLayers(particleNHitTS[iMCParticle]);
            int numberHitSuperLayers = hitSuperLayers.count();
            //cout<<"iMCParticle: "<< iMCParticle << " # hit TS: "<<numberHitSuperLayers<<" MC # TS: "<<lastSLHit<<endl;
            float mcPt = mcParticles[iMCParticle]->getMomentum().Perp();
            float efficiency;
            if( lastSLHit == 0 ) efficiency = -1;
            else efficiency = float(numberHitSuperLayers)/lastSLHit;
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
        for(unsigned iEfficiency=0; iEfficiency<tsEfficiency.size(); iEfficiency++){
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
    for( int iSuperLayer = 0; iSuperLayer < 9; iSuperLayer++) {
        for( unsigned iTS = 0; iTS < segmentHitsSL[iSuperLayer].size(); iTS++) {
            const TCSegment & ts = segmentHitsSL[iSuperLayer][iTS]->segment();
            //unsigned nWires = ts.wires().size();
            unsigned iWire = 5;
            if(iSuperLayer == 0) iWire = 0;
            const TRGCDCWireHit* wireHit = ts.wires()[iWire]->hit();
            if(wireHit != 0) {
                //cout<<"[TSF]: "<<ts.name()<<" was hit at ";
                unsigned nHits = ts.wires()[iWire]->signal().nSignals();
                for(unsigned iHit=0; iHit<nHits; iHit++){
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

    m_treeOutputTSF->Fill();

} // End of saving TSF results



void TRGCDCTrackSegmentFinder::saveNNTSInformation(std::vector<TRGCDCSegment* >& tss) {

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
        const TCSegment & ts = * tss[iTS];
        const TCSHit* tsHit = ts.hit();
        if(tsHit) {
            const TRGCDCWire* priority;
            if(ts.wires().size() == 15) priority = ts.wires()[0];  
            else priority = ts.wires()[5];
            // If priority wire is hit
            if(priority->hit()) {

                // Calculate timeWires
                // Fill wire timing. Not hit = 9999. Unit is ns.
                vector<float> wireTime;
                if(ts.superLayerId() == 0) {
                    wireTime.resize(15); 
                } else {
                    wireTime.resize(11); 
                }
                // Loop over all wires
                //cout<<ts.name();
                const unsigned nWires = ts.wires().size();
                for (unsigned iWire = 0; iWire < nWires; iWire++) {
                    const TRGCDCWire* wire = ts.wires()[iWire];
                    const TRGCDCWireHit* wireHit = ts.wires()[iWire]->hit();
                    // If wire is hit
                    if(wireHit) {
                        // Only check first change. This could become a bug.
                        wireTime[iWire] = wire->signal().stateChanges()[0]; 
                        //cout<<ts.wires()[iWire]->name()<<" Clock: "<< ts.wires()[iWire]->signal().clock().frequency()<<" Drift lenght: "<<wireHit->drift()<<" Drift time: "<<ts.wires()[iWire]->signal().stateChanges()[0]<<endl;
                    } else {
                        wireTime[iWire] = 9999;
                    }
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
                int nWiresLayer = _cdc.layer(lastLayer+1)->nCells();
                if(nWires == 15) {
                    for(unsigned iWire =0; iWire < 6; iWire++) {
                        int wireIndex = lastWire - 4 + iWire;
                        if (wireIndex < 0) wireIndex += nWiresLayer;
                        if (wireIndex >= nWiresLayer) wireIndex -= nWiresLayer;
                        //cout<<"Call: "<<(*_cdc.layer(lastLayer+1))[wireIndex]->localId()<<endl;
                        const TRGCDCCell* wire = (*_cdc.layer(lastLayer+1))[wireIndex];
                        const TRGCDCCellHit* wireHit = wire->hit();
                        // If wire is hit
                        if(wireHit) {
                            // Only check first change. This could become a bug.
                            wireTime.push_back(wire->signal().stateChanges()[0]); 
                            //cout<<ts.wires()[iWire]->name()<<" Clock: "<< ts.wires()[iWire]->signal().clock().frequency()<<" Drift lenght: "<<wireHit->drift()<<" Drift time: "<<ts.wires()[iWire]->signal().stateChanges()[0]<<endl;
                        } else {
                            wireTime.push_back(9999);
                        }
                    } // Loop over all extra wires
                } else {
                    for(unsigned iWire =0; iWire < 5; iWire++) {
                        int wireIndex = lastWire - 3 + iWire;
                        if (wireIndex < 0) wireIndex += nWiresLayer;
                        if (wireIndex >= nWiresLayer) wireIndex -= nWiresLayer;
                        //cout<<"Call: "<<(*_cdc.layer(lastLayer+1))[wireIndex]->localId()<<endl;
                        const TRGCDCCell* wire = (*_cdc.layer(lastLayer+1))[wireIndex];
                        const TRGCDCCellHit* wireHit = wire->hit();
                        // If wire is hit
                        if(wireHit) {
                            // Only check first change. This could become a bug.
                            wireTime.push_back(wire->signal().stateChanges()[0]); 
                            //cout<<ts.wires()[iWire]->name()<<" Clock: "<< ts.wires()[iWire]->signal().clock().frequency()<<" Drift lenght: "<<wireHit->drift()<<" Drift time: "<<ts.wires()[iWire]->signal().stateChanges()[0]<<endl;
                        } else {
                            wireTime.push_back(9999);
                        }
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
                if(priority->hit()->mcLR()) mcLRDriftTime *= -1;
                //cout<<ts.name()<<" LRDriftTime: "<<mcLRDriftTime<<endl;

                // Save timing information in Root
                TVectorD t_nnPatternInformation;
                if(ts.superLayerId() == 0) {
                    //t_nnPatternInformation.ResizeTo(17);
                    t_nnPatternInformation.ResizeTo(23);
                    t_nnPatternInformation[0] = ts.superLayerId();
                    t_nnPatternInformation[1] = mcLRDriftTime;
                    //for(unsigned iWire=0; iWire<15; iWire++){
                    for(unsigned iWire=0; iWire<21; iWire++){
                        t_nnPatternInformation[iWire+2] = wireTime[iWire];
                    }
                    new(nnPatternInformation[indexSaving++]) TVectorD(t_nnPatternInformation);
                } else {
                    //t_nnPatternInformation.ResizeTo(13);
                    t_nnPatternInformation.ResizeTo(17);
                    t_nnPatternInformation[0] = ts.superLayerId();
                    t_nnPatternInformation[1] = mcLRDriftTime;
                    //for(unsigned iWire=0; iWire<11; iWire++){
                    for(unsigned iWire=0; iWire<15; iWire++){
                        t_nnPatternInformation[iWire+2] = wireTime[iWire];
                    }
                    new(nnPatternInformation[indexSaving++]) TVectorD(t_nnPatternInformation);
                }

            } // End of if priority cell is hit
        } // End of if TS is hit
    } // End loop of all TSs

    m_treeNNTSF->Fill();

} // End of save function

  void 
  TRGCDCTrackSegmentFinder::push_back(const TRGCDCMerger *a){
    std::vector<const TRGCDCMerger *>::push_back(a);
  }
} // namespace Belle2
