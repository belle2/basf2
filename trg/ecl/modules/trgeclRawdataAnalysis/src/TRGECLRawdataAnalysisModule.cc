//---------------------------------------------------------------
// $Id$
//---------------------------------------------------------------
// Filename : TRGECLRawdataAnalysisModule.cc
// Section  : TRG ECL
// Owner    : InSoo Lee / Yuuji Unno / SungHyun Kim
// Email    : islee@hep.hanyang.ac.kr / yunno@post.kek.jp / sunghun.kim@belle2.org
//---------------------------------------------------------------
// Description : A trigger module for TRG ECL
//---------------------------------------------------------------
// 1.00 : 2017/08/01 : ECL Trigger Raw data analyzer.
//                     Save Cluster information
//                     using  Unpacker version 1.1
//---------------------------------------------------------------

//framework headers
#include <framework/datastore/StoreObjPtr.h>
#include <framework/gearbox/Unit.h>

#include <framework/logging/Logger.h>

#include <G4RunManager.hh>
#include "trg/ecl/TrgEclMaster.h"
//trg package headers
#include "trg/trg/Debug.h"
#include "trg/ecl/modules/trgeclRawdataAnalysis/TRGECLRawdataAnalysisModule.h"

#include <stdlib.h>

#include <stdlib.h>
#include <iostream>

using namespace std;

namespace Belle2 {
//
//
//! Register Module
  REG_MODULE(TRGECLRawdataAnalysis);
//
//
//
  string
  TRGECLRawdataAnalysisModule::version() const
  {
    return string("TRGECLAnalysisModule 1.00");
  }
//
//
//
  TRGECLRawdataAnalysisModule::TRGECLRawdataAnalysisModule()
    : Module::Module(),
      _debugLevel(0),  _Clustering(1)
  {

    string desc = "TRGECLRawdataAnalysisModule(" + version() + ")";
    setDescription(desc);
    //   setPropertyFlags(c_ParallelProcessingCertified | c_InitializeInProcess);

    addParam("DebugLevel", _debugLevel, "TRGECL debug level", _debugLevel);
    addParam("Clustering", _Clustering, "TRGECL Clustering method  0 : use only ICN, 1 : ICN + Energy(Defult)", _Clustering);

    obj_cluster = new TrgEclCluster();


    if (TRGDebug::level()) {
      std::cout << "TRGECLRawdataAnalysisModule ... created" << std::endl;
    }
  }
//
//
//
  TRGECLRawdataAnalysisModule::~TRGECLRawdataAnalysisModule()
  {
    delete obj_cluster;
    if (TRGDebug::level()) {
      std::cout << "TRGECLRawdataAnalysisModule ... destructed " << std::endl;

    }
  }
//
//
//
  void
  TRGECLRawdataAnalysisModule::initialize()
  {

    TRGDebug::level(_debugLevel);

    if (TRGDebug::level()) {
      std::cout << "TRGECLRawdataAnalysisModule::initialize ... options" << std::endl;
      std::cout << TRGDebug::tab(4) << "debug level = " << TRGDebug::level()
                << std::endl;
    }
    //
    //
    //
    m_nRun   = 0 ;
    m_nEvent = 0 ;
    m_hitNum = 0;
    m_hitTCNum = 0;

    m_TRGECLUnpackerStore.registerInDataStore();
    m_TRGECLCluster.registerInDataStore();
    m_TRGECLTiming.registerInDataStore();

  }
//
//
//
  void
  TRGECLRawdataAnalysisModule::beginRun()
  {

    if (TRGDebug::level()) {
      std::cout << "TRGECLRawdataAnalysisModule ... beginRun called " << std::endl;
    }
    //  _ecl = TrgEcl::getTrgEcl();

  }
//
//
//
  void
  TRGECLRawdataAnalysisModule::event()
  {

    if (TRGDebug::level()) {
      std::cout << "TRGECLMoudle ... event called" << std::endl;
    }

    std::vector<int> TCId;
    std::vector<double> TCTiming;
    std::vector<double> TCEnergy;

    TCId.clear();
    TCTiming.clear();
    TCEnergy.clear();

    int HitRevoFAM = 0;
    int HitFineTime = 0;


    //
    StoreArray<TRGECLUnpackerStore> trgeclUnpackerStoreArray;
    for (int ii = 0; ii < trgeclUnpackerStoreArray.getEntries(); ii++) {

      TRGECLUnpackerStore* TCHit = trgeclUnpackerStoreArray[ii];
      int  iTCID = (TCHit->getTCId() - 1);
      int HitTiming    = TCHit ->getTCTime();
      int HitEnergy =  TCHit -> getTCEnergy();
      HitRevoFAM = TCHit -> getRevoFAM();
      HitFineTime = TCHit -> getTCTime();

      if (iTCID == -1 && HitTiming == 0 && HitEnergy == 0) {continue;}

      TCId.push_back(iTCID + 1);
      TCTiming.push_back(HitTiming);
      TCEnergy.push_back(HitEnergy);



    }
    obj_cluster = new TrgEclCluster();
    if (TCId.size() > 0) {

      // TrgEclCluster obj_cluster;
      obj_cluster->setClusteringMethod(_Clustering);
      obj_cluster->setEventId(m_nEvent);
      obj_cluster->setICN(TCId, TCEnergy, TCTiming);
      //    Module::~TRGECLRa
      // int icn = obj_cluster->getICNFwBr();
      // int icnfwd  = obj_cluster->getICNSub(0);
      //      int icnbr   = obj_cluster->getICNSub(1);
      // int icnbwd = obj_cluster->getICNSub(2);
    }


    int Timing = ((HitFineTime >> 3) & 0xF) + ((HitRevoFAM & 0x7F) << 4);

    int m_hitNum = 0;
    StoreArray<TRGECLTiming> TimingArray;
    TimingArray.appendNew();
    m_hitNum = TimingArray.getEntries() - 1;
    TimingArray[m_hitNum]->setEventId(m_nEvent);
    TimingArray[m_hitNum]->setTiming(Timing);


    //

    //
    //
    m_nEvent++;
    //
    //
    //


  }
//
//
//
  void
  TRGECLRawdataAnalysisModule::endRun()
  {
    if (TRGDebug::level()) {
      std::cout << "TRGECLRawdataAnalysisModule ... endRun called " << std::endl;
    }
  }
//
//
//
  void
  TRGECLRawdataAnalysisModule::terminate()
  {
    if (TRGDebug::level()) {
      std::cout << "TRGECLRawdataAnalysisModule ... terminate called " << std::endl;
    }
  }
//
//
//
} // namespace Belle2
