//---------------------------------------------------------------
// $Id$
//---------------------------------------------------------------
// Filename : TRGECLModule.cc
// Section  : TRG ECL
// Owner    : InSu Lee / Yuuji Unno
// Email    : islee@hep.hanyang.ac.kr / yunno@post.kek.jp
//---------------------------------------------------------------
// Description : A trigger module for TRG ECL
//---------------------------------------------------------------
// 1.00 : 2010/11/18 : First version
//---------------------------------------------------------------

#define TRGECL_SHORT_NAMES

//framework headers
#include <framework/datastore/StoreObjPtr.h>
#include <framework/gearbox/Unit.h>

#include <framework/logging/Logger.h>

#include <G4RunManager.hh>
#include "trg/ecl/TrgEclMaster.h"
//trg package headers
#include "trg/trg/Debug.h"
#include "trg/ecl/modules/trgecl/TRGECLModule.h"
#include "trg/ecl/dbobjects/TRGECLETMPara.h"

#include <stdlib.h>
#include <iostream>

using namespace std;

namespace Belle2 {
//
//
//! Register Module
  REG_MODULE(TRGECL);
//
//
//
  string
  TRGECLModule::version() const
  {
    return string("TRGECLModule 1.00");
  }
//
//
//
  TRGECLModule::TRGECLModule(): Module::Module(), _debugLevel(0), _Bhabha(0), _Clustering(1), _ClusterLimit(6), _EventTiming(2),
    _TimeWindow(250.0), _OverlapWindow(125.0), _NofTopTC(3), _SelectEvent(1), _ConditionDB(0)
  {

    string desc = "TRGECLModule(" + version() + ")";
    setDescription(desc);
    setPropertyFlags(c_ParallelProcessingCertified);

    addParam("DebugLevel", _debugLevel, "TRGECL debug level", _debugLevel);
    addParam("Bhabha", _Bhabha, "TRGECL Bhabha method  0 : Belle I, 1 :belle II(defult)", _Bhabha);
    addParam("Clustering", _Clustering, "TRGECL Clustering method  0 : use only ICN, 1 : ICN + Energy(Defult)", _Clustering);
    addParam("ClusterLimit", _ClusterLimit, "The Limit number of cluster (Defult:6)", _ClusterLimit);

    addParam("EventTiming", _EventTiming,
             "TRGECL EventTiming method  0 : Belle I, 1 : Energetic TC, 2 : Energy Weighted timing (defult)", _EventTiming);
    addParam("NofTopTC", _NofTopTC, "TRGECL # of considered TC in energy weighted Timing method(Only work for EvenTiming Method 2)",
             _NofTopTC);

    addParam("TimeWindow", _TimeWindow, "TRGECL Trigger decision Time Window", _TimeWindow);
    addParam("OverlapWindow", _OverlapWindow, "TRGECL Trigger decision Time Window", _OverlapWindow);
    addParam("EventSelect", _SelectEvent, "TRGECL Select one trigger window for logic study", _SelectEvent);
    addParam("ConditionDB", _ConditionDB, "Flag to use Condition Database(Defult 0)", _ConditionDB);

    _2DBhabhaThresholdFWD.clear();
    _2DBhabhaThresholdBWD.clear();
    _3DBhabhaThreshold.clear();
    _TotalEnergy.clear();
    _LowMultiThreshold.clear();


    if (TRGDebug::level()) {
      std::cout << "TRGECLModule ... created" << std::endl;
    }
  }
//
//
//
  TRGECLModule::~TRGECLModule()
  {

    if (TRGDebug::level()) {
      std::cout << "TRGECLModule ... destructed " << std::endl;

    }
  }
//
//
//
  void
  TRGECLModule::initialize()
  {

    TRGDebug::level(_debugLevel);

    if (TRGDebug::level()) {
      std::cout << "TRGECLModule::initialize ... options" << std::endl;
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

    m_TRGECLHit.registerInDataStore();
    m_TRGECLTrg.registerInDataStore();
    m_TRGECLCluster.registerInDataStore();

  }
//
//
//
  void
  TRGECLModule::beginRun()
  {
    if (_ConditionDB == 1) {
      for (const auto& para : m_ETMPara) {
        _TotalEnergy = {(double)para.getELow(), (double)para.getELow(), (double)para.getELow()};
        for (int index = 0; index < 14; index ++) {
          _2DBhabhaThresholdFWD.push_back((double)para.get2DBhabhaFWD(index));
          _2DBhabhaThresholdBWD.push_back((double)para.get2DBhabhaBWD(index));
        }
        _3DBhabhaThreshold = {(double)para.get3DBhabhaThreshold(0), (double)para.get3DBhabhaThreshold(1)}; //  /100 MeV
        _LowMultiThreshold = {(double)para.getLowMultiThreshold(0) , (double)para.getLowMultiThreshold(1), (double)para.getLowMultiThreshold(2), (double)para.getLowMultiThreshold(3)}; //  /100 MeV
      }
    } else {

      _TotalEnergy = {5, 10, 30}; // /100 MeV
      _2DBhabhaThresholdFWD = {40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 30, 35}; // /100 MeV
      _2DBhabhaThresholdBWD  = {25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 30, 30}; // /100 MeV
      _3DBhabhaThreshold = {30, 45}; //  /10 MeV
      _LowMultiThreshold = {10, 20, 25, 30}; // /100 MeV
    }


    if (TRGDebug::level()) {
      std::cout << "TRGECLModule ... beginRun called " << std::endl;
    }
    //  _ecl = TrgEcl::getTrgEcl();

  }
//
//
//
  void
  TRGECLModule::event()
  {

    if (TRGDebug::level()) {
      std::cout << "TRGECLMoudle ... event called" << std::endl;
    }
    //
    // simulation
    //
    TrgEclMaster* _ecl = new TrgEclMaster();

    _ecl-> initialize(m_nEvent);
    _ecl-> setClusterMethod(_Clustering);
    _ecl-> setClusterLimit(_ClusterLimit);
    _ecl-> setBhabhaMethod(_Bhabha);
    _ecl-> setEventTimingMethod(_EventTiming);
    _ecl -> setTimeWindow(_TimeWindow);
    _ecl -> setOverlapWindow(_OverlapWindow);
    _ecl -> setNofTopTC(_NofTopTC);
    _ecl -> set2DBhabhaThreshold(_2DBhabhaThresholdFWD, _2DBhabhaThresholdBWD);
    _ecl -> set3DBhabhaThreshold(_3DBhabhaThreshold);
    _ecl -> setTotalEnergyThreshold(_TotalEnergy);
    _ecl -> setLowMultiplicityThreshold(_LowMultiThreshold);


    if (_SelectEvent == 0) {
      _ecl->simulate01(m_nEvent);
    } else if (_SelectEvent == 1) {
      _ecl->simulate02(m_nEvent);
    }
    // printf("TRGECLModule> eventId = %d \n", m_nEvent);

    //

    //
    //
    m_nEvent++;
    //
    //
    //

    delete _ecl;

  }
//
//
//
  void
  TRGECLModule::endRun()
  {
    if (TRGDebug::level()) {
      std::cout << "TRGECLModule ... endRun called " << std::endl;
    }
  }
//
//
//
  void
  TRGECLModule::terminate()
  {
    if (TRGDebug::level()) {
      std::cout << "TRGECLModule ... terminate called " << std::endl;
    }
  }
//
//
//
} // namespace Belle2
