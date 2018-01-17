//---------------------------------------------------------
// $Id$
//---------------------------------------------------------
// Filename : MCMatcherTRGECLModule.h
// Section  : TRG ECL
// Owner    : InSoo Lee/Yuuji Unno
// Email    : islee@hep.hanyang.ac.kr / yunno@post.kek.jp
//---------------------------------------------------------
// Description : A MC Matching module for TRG ECL
//---------------------------------------------------------
// 0.00 : 2014/12/22 : First version
//                     Relation between MCParicle and
//                     TRGECLHit.
//---------------------------------------------------------
#include <trg/ecl/modules/MCMatcherTRGECL/MCMatcherTRGECLModule.h>

//framework headers
#include <framework/datastore/StoreObjPtr.h>
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>

//ecl package header
#include <ecl/dataobjects/ECLHit.h>

//ecl trg package header
#include "trg/ecl/dataobjects/TRGECLHit.h"
#include "trg/ecl/dataobjects/TRGECLDigi0.h"


#include <mdst/dataobjects/MCParticle.h>
#include <framework/datastore/RelationArray.h>


//root
#include <TVector3.h>

//C++ STL
#include <cstdlib>
#include <iomanip>
#include <math.h>
#include <time.h>
#include <iomanip>
#include <utility> //contains pair



using namespace std;
using namespace boost;
using namespace Belle2;
//using namespace ECL;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(MCMatcherTRGECL)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

MCMatcherTRGECLModule::MCMatcherTRGECLModule() : Module()
{
  // Set description

  setDescription("MCMatcherTRGECLModule");
  setPropertyFlags(c_ParallelProcessingCertified);
  _TCMap = new TrgEclMapping();


  for (int ii = 0; ii < 100; ii++) {
    TCId[ii] = 0;
    TCRawEnergy[ii] = 0;
    TCRawTiming[ii] = 0;
    SignalContribution[ii] = 0;
    BKGContribution[ii] = 0;
    TCIdHit[ii] = 0;
    TCHitEnergy[ii] = 0;
    TCHitTiming[ii] = 0;
    SignalContributionHit[ii] = 0;
    BKGContributionHit[ii] = 0;

    for (int icont = 0 ; icont < 3 ; icont ++) {


      maxEnergy[ii][icont] = -1;
      contribution[ii][icont] = 0;
      TCPrimaryIndex[ii][icont] = -1;
      XtalId[ii][icont] = -1 ;
      px[ii][icont] = 0;
      py[ii][icont] = 0;
      pz[ii][icont] = 0;
      pid[ii][icont] = 0;
      trackId[ii][icont] = 0;
      background_tag[ii][icont] = 0;
      mother[ii][icont] = 0 ;
      gmother[ii][icont] = 0;
      ggmother[ii][icont] = 0;
      gggmother[ii][icont] = 0;
      MCEnergy[ii][icont] = 0;

      ieclhit[ii][icont] = 0 ;

      background_tagHit[ii][icont] = 0;
      TCPrimaryIndexHit[ii][icont] = -1;
      XtalIdHit[ii][icont] = -1 ;
      pxHit[ii][icont] = 0;
      pyHit[ii][icont] = 0;
      pzHit[ii][icont] = 0;
      pidHit[ii][icont] = 0;
      trackIdHit[ii][icont] = 0;
      background_tagHit[ii][icont] = 0;
      motherHit[ii][icont] = 0 ;
      gmotherHit[ii][icont] = 0;
      ggmotherHit[ii][icont] = 0;
      gggmotherHit[ii][icont] = 0;
      MCEnergyHit[ii][icont] = 0;
      contributionHit[ii][icont] = 0;

    }
  }
}

MCMatcherTRGECLModule::~MCMatcherTRGECLModule()
{
  delete _TCMap;
}

void MCMatcherTRGECLModule::initialize()
{
  // Initialize variables
  m_nRun    = 0 ;
  m_nEvent  = 0 ;
  // CPU time start
  m_timeCPU = clock() * Unit::us;

  m_trgECLDigi0MC.registerInDataStore();
  m_trgECLHitMC.registerInDataStore();
  StoreArray<MCParticle> mcParticles;
  StoreArray<ECLHit> eclHitArray;

  mcParticles.registerRelationTo(eclHitArray);
  StoreArray<TRGECLDigi0> trgeclDigi0Array;
  trgeclDigi0Array.registerRelationTo(mcParticles);
  StoreArray<TRGECLHit> trgeclHitArray;
  trgeclHitArray.registerRelationTo(mcParticles);

}

void MCMatcherTRGECLModule::beginRun()
{
}

void MCMatcherTRGECLModule::event()
{


  StoreArray<MCParticle> mcParticles;
  PrimaryTrackMap eclPrimaryMap;
  eclPrimaryMap.clear();
  int nParticles = mcParticles.getEntries();
  //  cout << nParticles << endl;
  for (int iPart = 0; iPart < nParticles ; ++iPart) {
    if (mcParticles[iPart]->getMother() == NULL) {
      if (!mcParticles[iPart]->hasStatus(MCParticle::c_PrimaryParticle)) {
        if (!mcParticles[iPart]->hasStatus(MCParticle::c_StableInGenerator)) {
          continue;
        }
      }
    }



    bool adhoc_StableInGeneratorFlag(mcParticles[iPart]->hasStatus(MCParticle::c_StableInGenerator));

    if (mcParticles[iPart]->hasStatus(MCParticle::c_PrimaryParticle)
        && adhoc_StableInGeneratorFlag) {
      if (mcParticles[iPart]->getArrayIndex() == -1) {
        eclPrimaryMap.insert(pair<int, int>(iPart, iPart));
      } else {
        eclPrimaryMap.insert(pair<int, int>(mcParticles[iPart]->getArrayIndex(), mcParticles[iPart]->getArrayIndex()));
      }
    } else {
      if (mcParticles[iPart]->getMother() == NULL) continue;
      if (eclPrimaryMap.find(mcParticles[iPart]->getMother()->getArrayIndex()) != eclPrimaryMap.end()) {
        eclPrimaryMap.insert(
          pair<int, int>(mcParticles[iPart]->getArrayIndex(), eclPrimaryMap[mcParticles[iPart]->getMother()->getArrayIndex()]));

      }//if mother of mcParticles is stored.
    }//if c_StableInGenerator and c_PrimaryParticle

  }//for mcParticles

  //Connect TRGECLDigi0 and MCParticle using ECLHit
  StoreArray<ECLHit> eclHitArray("ECLHits");
  RelationArray eclHitRel(mcParticles, eclHitArray);
  StoreArray<TRGECLDigi0> trgeclDigi0Array;
  RelationArray trgeclDigi0ToMCPart(trgeclDigi0Array, mcParticles);
  int nHits_hit = eclHitArray.getEntries() - 1;
  //
  int ihit = 0;

  const int NofTCDigiHit = trgeclDigi0Array.getEntries();

  // cout << NofTCDigiHit << endl;

  for (int ii = 0; ii < NofTCDigiHit; ii++) {


    TRGECLDigi0* aTRGECLDigi0 = trgeclDigi0Array[ii];
    TCId[ihit]          = (aTRGECLDigi0->getTCId() - 1);
    TCRawTiming[ihit]    = aTRGECLDigi0 ->getRawTiming();

    int itimeindex = (int)(TCRawTiming[ihit] / 100 + 40);
    TCRawEnergy[ihit] = aTRGECLDigi0 ->getRawEnergy() / Unit::GeV;
    if (TCRawEnergy[ihit] < 0.1) {continue;} //0.05 GeV cut to save time.
    for (int hit = 0; hit < nHits_hit; hit++) {//Find relation of TRGECLDigi0 and ECLHit

      ECLHit* aECLHit =  eclHitArray[hit];;

      double hitE         =  aECLHit->getEnergyDep() / Unit::GeV;
      if (hitE < 0.1) {continue;} //to save time.
      int hitCellId         = aECLHit->getCellId() - 1;
      int hitTCId = _TCMap->getTCIdFromXtalId(hitCellId + 1) - 1;
      int timeindex = (int)((aECLHit ->getTimeAve()) / 100 + 40);
      int backtag = aECLHit ->getBackgroundTag();

      if (hitTCId != TCId[ihit]) {continue;}
      if (itimeindex != timeindex) {continue;}
      if (backtag == 0) { SignalContribution[ihit] =  SignalContribution[ihit] + hitE;}
      else if (backtag != 0) { BKGContribution[ihit] =  BKGContribution[ihit] + hitE;}


      if (TCId[ihit] == hitTCId && maxEnergy[ihit][0] < hitE) {

        ieclhit[ihit][0] = hit;
        maxEnergy[ihit][0] = hitE;
        contribution[ihit][0] = hitE;
        XtalId[ihit][0] = hitCellId ;
        background_tag[ihit][0] = backtag;

      }


      if (TCId[ihit] == hitTCId && maxEnergy[ihit][1] < hitE && hitE <  maxEnergy[ihit][0]) {
        ieclhit[ihit][1] = hit;
        maxEnergy[ihit][1] = hitE;
        contribution[ihit][1] = hitE;
        XtalId[ihit][1] = hitCellId ;
        background_tag[ihit][1] = backtag;
      }


      if (TCId[ihit] == hitTCId && maxEnergy[ihit][2] < hitE && hitE <  maxEnergy[ihit][1]) {
        ieclhit[ihit][2] = hit;
        maxEnergy[ihit][2] = hitE;
        contribution[ihit][2] = hitE;
        XtalId[ihit][2] = hitCellId ;
        background_tag[ihit][2] = backtag;

      }
    }
    //Find the relation of TRGECLDigi0 and MCParticle using ECLHit
    for (int index = 0; index < eclHitRel.getEntries(); index++) {
      int PrimaryIndex = -1;

      map<int, int>::iterator iter = eclPrimaryMap.find(eclHitRel[index].getFromIndex());

      if (iter != eclPrimaryMap.end()) {
        PrimaryIndex = iter->second;
      }
      int eclhitRelSize = eclHitRel[index].getToIndices().size();
      for (int pri_hit = 0; pri_hit <  eclhitRelSize ; pri_hit++) {
        int ieclHitRel = eclHitRel[index].getToIndex(pri_hit);
        if (ieclhit[ihit][0] == ieclHitRel) {
          TCPrimaryIndex[ihit][0] = PrimaryIndex;

        }
        if (ieclhit[ihit][1] == ieclHitRel) {
          TCPrimaryIndex[ihit][1] = PrimaryIndex;

        }
        if (ieclhit[ihit][2] == ieclHitRel) {
          TCPrimaryIndex[ihit][2] = PrimaryIndex;

        }
      }
    }


    trackId[ihit][0] = TCPrimaryIndex[ihit][0];
    trackId[ihit][1] = TCPrimaryIndex[ihit][1];
    trackId[ihit][2] = TCPrimaryIndex[ihit][2];


    //  cout <<ihit  <<" "  << trackId[ihit][0] << " " << trackId[ihit][1] << " " << trackId[ihit][2] << " " << endl;


    int mclist = 0;

    if (TCPrimaryIndex[ihit][0] > 0) {

      MCEnergy[ihit][0] = mcParticles[TCPrimaryIndex[ihit][0]]->getEnergy();
      pid[ihit][0] = mcParticles[TCPrimaryIndex[ihit][0]]->getPDG();
      px[ihit][0] = (mcParticles[TCPrimaryIndex[ihit][0]]->getMomentum()).X();
      py[ihit][0] = (mcParticles[TCPrimaryIndex[ihit][0]]->getMomentum()).Y();
      pz[ihit][0] = (mcParticles[TCPrimaryIndex[ihit][0]]->getMomentum()).Z();
      if (pid[ihit][0] != 0 && (mcParticles[TCPrimaryIndex[ihit][0]]->getMother())) {
        mother[ihit][0]  = mcParticles[TCPrimaryIndex[ihit][0]]->getMother() ->getPDG();
        mclist =  mcParticles[TCPrimaryIndex[ihit][0]]->getMother()-> getIndex();
      }
      if (mclist != 1 && mother[ihit][0] != 0 && (mcParticles[TCPrimaryIndex[ihit][0]]->getMother()->getMother())) {
        gmother[ihit][0] =  mcParticles[TCPrimaryIndex[ihit][0]]->getMother()->getMother() ->getPDG();
        mclist =  mcParticles[TCPrimaryIndex[ihit][0]]->getMother()->getMother()-> getIndex();

      }
      if (mclist != 1 && gmother[ihit][0] != 0 && (mcParticles[TCPrimaryIndex[ihit][0]]->getMother()->getMother()->getMother())) {
        ggmother[ihit][0] =  mcParticles[TCPrimaryIndex[ihit][0]]->getMother()->getMother()->getMother() ->getPDG();
        mclist =  mcParticles[TCPrimaryIndex[ihit][0]]->getMother()->getMother()->getMother()-> getIndex();


      }
      if (mclist != 1 && ggmother[ihit][0] != 0) {
        if (mcParticles[TCPrimaryIndex[ihit][0]]->getMother()->getMother()->getMother()->getMother()) {
          gggmother[ihit][0] =  mcParticles[TCPrimaryIndex[ihit][0]]->getMother()->getMother()->getMother()->getMother() ->getPDG();

        }
      }

    }
    mclist = 0;
    if (TCPrimaryIndex[ihit][1] > 0) {

      MCEnergy[ihit][1] = mcParticles[TCPrimaryIndex[ihit][1]]->getEnergy();
      pid[ihit][1] = mcParticles[TCPrimaryIndex[ihit][1]]->getPDG();
      px[ihit][1] = (mcParticles[TCPrimaryIndex[ihit][1]]->getMomentum()).X();
      py[ihit][1] = (mcParticles[TCPrimaryIndex[ihit][1]]->getMomentum()).Y();
      pz[ihit][1] = (mcParticles[TCPrimaryIndex[ihit][1]]->getMomentum()).Z();
      if (pid[ihit][1] != 0 && (mcParticles[TCPrimaryIndex[ihit][1]]->getMother())) {
        mother[ihit][1]  = mcParticles[TCPrimaryIndex[ihit][1]]->getMother() ->getPDG();
        mclist = mcParticles[TCPrimaryIndex[ihit][1]]->getMother()-> getIndex();
      }
      if (mclist != 1 && mother[ihit][1] != 0 && (mcParticles[TCPrimaryIndex[ihit][1]]->getMother()->getMother())) {
        gmother[ihit][1] =  mcParticles[TCPrimaryIndex[ihit][1]]->getMother()->getMother() ->getPDG();
        mclist =  mcParticles[TCPrimaryIndex[ihit][1]]->getMother()->getMother()-> getIndex();

      }
      if (mclist != 1 && gmother[ihit][1] != 0 && (mcParticles[TCPrimaryIndex[ihit][1]]->getMother()->getMother()->getMother())) {
        ggmother[ihit][1] =  mcParticles[TCPrimaryIndex[ihit][1]]->getMother()->getMother()->getMother() ->getPDG();
        mclist =  mcParticles[TCPrimaryIndex[ihit][1]]->getMother()->getMother()->getMother()-> getIndex();
      }
      if (mclist != 1 && ggmother[ihit][1] != 0) {
        if (mcParticles[TCPrimaryIndex[ihit][1]]->getMother()->getMother()->getMother()->getMother()) {
          gggmother[ihit][1] =  mcParticles[TCPrimaryIndex[ihit][1]]->getMother()->getMother()->getMother()->getMother() ->getPDG();
        }
      }
    }
    mclist = 0;
    if (TCPrimaryIndex[ihit][2] > 0) {

      MCEnergy[ihit][2] = mcParticles[TCPrimaryIndex[ihit][2]]->getEnergy();
      pid[ihit][2] = mcParticles[TCPrimaryIndex[ihit][2]]->getPDG();
      px[ihit][2] = (mcParticles[TCPrimaryIndex[ihit][2]]->getMomentum()).X();
      py[ihit][2] = (mcParticles[TCPrimaryIndex[ihit][2]]->getMomentum()).Y();
      pz[ihit][2] = (mcParticles[TCPrimaryIndex[ihit][2]]->getMomentum()).Z();
      if (pid[ihit][2] != 0 && (mcParticles[TCPrimaryIndex[ihit][2]]->getMother())) {
        mother[ihit][2]  = mcParticles[TCPrimaryIndex[ihit][2]]->getMother() ->getPDG();
        mclist = mcParticles[TCPrimaryIndex[ihit][2]]->getMother()-> getIndex();
      }
      if (mclist != 1 && mother[ihit][2] != 0 && (mcParticles[TCPrimaryIndex[ihit][2]]->getMother()->getMother())) {
        gmother[ihit][2] =  mcParticles[TCPrimaryIndex[ihit][2]]->getMother()->getMother() ->getPDG();
        mclist =  mcParticles[TCPrimaryIndex[ihit][2]]->getMother()->getMother()-> getIndex();

      }
      if (mclist != 1 && gmother[ihit][2] != 0 && (mcParticles[TCPrimaryIndex[ihit][2]]->getMother()->getMother()->getMother())) {
        ggmother[ihit][2] =  mcParticles[TCPrimaryIndex[ihit][2]]->getMother()->getMother()->getMother() ->getPDG();
        mclist =  mcParticles[TCPrimaryIndex[ihit][2]]->getMother()->getMother()->getMother()-> getIndex();
      }
      if (mclist != 1 && ggmother[ihit][2] != 0) {
        if (mcParticles[TCPrimaryIndex[ihit][2]]->getMother()->getMother()->getMother()->getMother()) {
          gggmother[ihit][2] =  mcParticles[TCPrimaryIndex[ihit][2]]->getMother()->getMother()->getMother()->getMother() ->getPDG();
        }
      }

    }
    trgeclDigi0ToMCPart.add(ii, TCPrimaryIndex[ihit][0]);
    ihit++;
  }


  int m_hitNum = 0;
  StoreArray<TRGECLDigi0MC> TCDigiArray;
  for (int ii = 0; ii < ihit; ii++) {

    if (TCRawEnergy[ii] < 0.1) {continue;}
    TCDigiArray.appendNew();
    m_hitNum = TCDigiArray.getEntries() - 1;

    TCDigiArray[m_hitNum]->setEventId(m_nEvent);
    TCDigiArray[m_hitNum]->setTCId(TCId[ii]);

    TCDigiArray[m_hitNum]->setRawEnergy(TCRawEnergy[ii]);
    TCDigiArray[m_hitNum]->setRawTiming(TCRawTiming[ii]);
    TCDigiArray[m_hitNum]->setTrackId(trackId[ii]);
    TCDigiArray[m_hitNum]->setCellId(XtalId[ii]);

    TCDigiArray[m_hitNum]->setPDG(pid[ii]);
    TCDigiArray[m_hitNum]->setMother(mother[ii]);
    TCDigiArray[m_hitNum]->setGMother(gmother[ii]);
    TCDigiArray[m_hitNum]->setGGMother(ggmother[ii]);
    TCDigiArray[m_hitNum]->setGGGMother(gggmother[ii]);

    TCDigiArray[m_hitNum]->setPX(px[ii]);
    TCDigiArray[m_hitNum]->setPY(py[ii]);
    TCDigiArray[m_hitNum]->setPZ(pz[ii]);
    TCDigiArray[m_hitNum]->setMCEnergy(MCEnergy[ii]);
    TCDigiArray[m_hitNum]->setContribution(contribution[ii]);
    TCDigiArray[m_hitNum]->setBackgroundTag(background_tag[ii]);
    TCDigiArray[m_hitNum]->setSignalContribution(SignalContribution[ii]);
    TCDigiArray[m_hitNum]->setBKGContribution(BKGContribution[ii]);
  }

  //Find relation of TRGECLHit and MCParticle using TRGECLDigi0

  StoreArray<TRGECLHit> trgeclHitArray;
  RelationArray trgeclHitToMCPart(trgeclHitArray, mcParticles);
  const int NofTCHit = trgeclHitArray.getEntries();


  for (int ii = 0; ii < NofTCHit; ii++) {

    TRGECLHit* aTRGECLHit = trgeclHitArray[ii];
    TCIdHit[ii]         = (aTRGECLHit->getTCId() - 1);
    TCHitTiming[ii]    = aTRGECLHit ->getTimeAve();
    TCHitEnergy[ii] =  aTRGECLHit -> getEnergyDep();
    int itimeindex = (int)(TCHitTiming[ii] / 100 + 40);

    for (int index = 0; index < trgeclDigi0ToMCPart.getEntries(); index++) {
      int idigi = index;
      int idigitimeindex = (int)(TCRawTiming[idigi] / 100 + 40);
      if (TCId[idigi] != TCIdHit[ii]) {continue;}
      if (itimeindex != idigitimeindex) {continue;}

      TCPrimaryIndexHit[ii][0] =  TCPrimaryIndex[idigi][0];
      contributionHit[ii][0] =  contribution[idigi][0];
      XtalIdHit[ii][0] =  XtalId[idigi][0] ;

      TCPrimaryIndexHit[ii][1] =  TCPrimaryIndex[idigi][1];
      contributionHit[ii][1] =  contribution[idigi][1];
      XtalIdHit[ii][1] =  XtalId[idigi][1] ;

      TCPrimaryIndexHit[ii][2] =  TCPrimaryIndex[idigi][2];
      contributionHit[ii][2] =  contribution[idigi][2];
      XtalIdHit[ii][2] =  XtalId[idigi][2] ;

      background_tagHit[ii][0] =  background_tag[idigi][0] ;
      background_tagHit[ii][1] =  background_tag[idigi][1] ;
      background_tagHit[ii][2] =  background_tag[idigi][2] ;

      SignalContributionHit[ii] =       SignalContribution[idigi];
      ;
      BKGContributionHit[ii] = BKGContribution[idigi];


      trackIdHit[ii][0] =      trackId[idigi][0];
      trackIdHit[ii][1] =      trackId[idigi][0];
      trackIdHit[ii][2] =      trackId[idigi][0];

      MCEnergyHit[ii][0] =   MCEnergy[idigi][0];
      pidHit[ii][0] =  pid[idigi][0] ;
      pxHit[ii][0] =  px[idigi][0];
      pyHit[ii][0] =  py[idigi][0];
      pzHit[ii][0] =  pz[idigi][0];
      gmotherHit[ii][0] =   gmother[idigi][0];
      ggmotherHit[ii][0] =   ggmother[idigi][0];
      gggmotherHit[ii][0] =   gggmother[idigi][0];

      MCEnergyHit[ii][1] =   MCEnergy[idigi][1];
      pidHit[ii][1] =  pid[idigi][1] ;
      pxHit[ii][1] =  px[idigi][1];
      pyHit[ii][1] =  py[idigi][1];
      pzHit[ii][1] =  pz[idigi][1];
      gmotherHit[ii][1] =   gmother[idigi][1];
      ggmotherHit[ii][1] =   ggmother[idigi][1];
      gggmotherHit[ii][1] =   gggmother[idigi][1];

      MCEnergyHit[ii][2] =   MCEnergy[idigi][2];
      pidHit[ii][2] =  pid[idigi][2] ;
      pxHit[ii][2] =  px[idigi][2];
      pyHit[ii][2] =  py[idigi][2];
      pzHit[ii][2] =  pz[idigi][2];
      gmotherHit[ii][2] =   gmother[idigi][2];
      ggmotherHit[ii][2] =   ggmother[idigi][2];
      gggmotherHit[ii][2] =   gggmother[idigi][2];
    }



    trgeclHitToMCPart.add(ii, TCPrimaryIndexHit[ii][0]);
  }

  m_hitNum = 0;
  StoreArray<TRGECLHitMC> TCHitArray;
  for (int ii = 0; ii < trgeclHitArray.getEntries(); ii++) {
    TCHitArray.appendNew();
    m_hitNum = TCHitArray.getEntries() - 1;

    TCHitArray[m_hitNum]->setEventId(m_nEvent);
    TCHitArray[m_hitNum]-> setTCId(TCIdHit[ii]);
    TCHitArray[m_hitNum]->setCellId(XtalIdHit[ii]);
    TCHitArray[m_hitNum]->setEnergyDep(TCHitEnergy[ii]);
    TCHitArray[m_hitNum]-> setTimeAve(TCHitTiming[ii]);
    TCHitArray[m_hitNum]-> setTrackId(trackIdHit[ii]);
    TCHitArray[m_hitNum]-> setPDG(pidHit[ii]);
    TCHitArray[m_hitNum]->setMother(motherHit[ii]);
    TCHitArray[m_hitNum]->setGMother(gmotherHit[ii]);
    TCHitArray[m_hitNum]->setGGMother(ggmotherHit[ii]);
    TCHitArray[m_hitNum]->setGGGMother(gggmotherHit[ii]);
    TCHitArray[m_hitNum]->setPX(pxHit[ii]);
    TCHitArray[m_hitNum]->setPY(pyHit[ii]);
    TCHitArray[m_hitNum]->setPZ(pzHit[ii]);
    TCHitArray[m_hitNum]->setMCEnergy(MCEnergyHit[ii]);
    TCHitArray[m_hitNum]->setBackgroundTag(background_tagHit[ii]);
    TCHitArray[m_hitNum]->setSignalContribution(SignalContributionHit[ii]);
    TCHitArray[m_hitNum]->setBKGContribution(BKGContributionHit[ii]);
    TCHitArray[m_hitNum]->setContribution(contributionHit[ii]);

  }

  m_nEvent++;

}


void MCMatcherTRGECLModule::endRun()
{
  m_nRun++;
}

void MCMatcherTRGECLModule::terminate()
{
}


