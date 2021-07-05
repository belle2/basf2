/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#define TRGECL_SHORT_NAMES

#include <framework/core/ModuleParamList.templateDetails.h>
#include <framework/core/ModuleParamList.h>

#include "trg/ecl/TrgEclMaster.h"
//trg package headers
#include "trg/trg/Debug.h"
#include "trg/ecl/modules/trgecl/TRGECLModule.h"
#include "trg/ecl/dbobjects/TRGECLETMPara.h"

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
  TRGECLModule::TRGECLModule():
    Module::Module(), _debugLevel(0), _Bhabha(0),
    _Clustering(1), _ClusterLimit(6), _EventTiming(1),
    _TimeWindow(250.0), _OverlapWindow(125.0), _NofTopTC(3),
    _SelectEvent(1), _ConditionDB(0), _mumuThreshold(20),
    _n300MeVCluster(1), _ECLBurstThreshold(200)
  {

    string desc = "TRGECLModule(" + version() + ")";
    setDescription(desc);
    setPropertyFlags(c_ParallelProcessingCertified);

    addParam("DebugLevel",
             _debugLevel,
             "TRGECL debug level",
             _debugLevel);
    addParam("Bhabha",
             _Bhabha,
             "TRGECL Bhabha method  0 : Belle I, 1 :belle II(defult)",
             _Bhabha);
    addParam("Clustering",
             _Clustering,
             "TRGECL Clustering method  0 : use only ICN, 1 : ICN + Energy(Defult)",
             _Clustering);
    addParam("ClusterLimit",
             _ClusterLimit,
             "The Limit number of cluster (Defult:6)",
             _ClusterLimit);
    addParam("EventTiming",
             _EventTiming,
             "TRGECL EventTiming method  0 : Belle I, 1 : Energetic TC, 2 : Energy Weighted timing (defult)",
             _EventTiming);
    addParam("NofTopTC",
             _NofTopTC,
             "TRGECL # of considered TC in energy weighted Timing method(Only work for EvenTiming Method 2)",
             _NofTopTC);
    addParam("TimeWindow",
             _TimeWindow,
             "TRGECL Trigger decision Time Window",
             _TimeWindow);
    addParam("OverlapWindow",
             _OverlapWindow,
             "TRGECL Trigger decision Time Window",
             _OverlapWindow);
    addParam("EventSelect",
             _SelectEvent,
             "TRGECL Select one trigger window for logic study",
             _SelectEvent);
    addParam("ConditionDB",
             _ConditionDB,
             "Flag to use Condition Database(Defult 0)",
             _ConditionDB);
    addParam("3DBhabhaVetoInTrackThetaRegion",
             m_3DBhabhaVetoInTrackThetaRegion,
             "set 3DBhabhaVetoInTrackThetaRegion (low TCID and high TCID)",
             m_3DBhabhaVetoInTrackThetaRegion);
    addParam("EventTimingQualityThresholds",
             m_EventTimingQualityThresholds,
             "set EventTimingQualityThresholds (low energy(GeV) and high energy(GeV))",
             m_EventTimingQualityThresholds);
    addParam("Taub2bAngleCut",
             m_taub2bAngleCut,
             "set Taub2bAngleCut(dphi low and high, theta sum low and high(degree)",
             m_taub2bAngleCut);
    addParam("Taub2bEtotCut",
             m_taub2bEtotCut,
             "set Taub2b total energy cut in TC ThetaID 1-17(GeV)",
             m_taub2bEtotCut);
    addParam("Taub2bClusterECut1",
             m_taub2bClusterECut1,
             "set Taub2b 1 cluster energy Cut (GeV) for CL1 : ",
             m_taub2bClusterECut1);
    addParam("Taub2bClusterECut2",
             m_taub2bClusterECut2,
             "set Taub2b 1 cluster energy Cut (GeV) for CL2 : ",
             m_taub2bClusterECut2);

    _2DBhabhaThresholdFWD.clear();
    _2DBhabhaThresholdBWD.clear();
    _3DBhabhaSelectionThreshold.clear();
    _3DBhabhaVetoThreshold.clear();
    _3DBhabhaSelectionAngle.clear();
    _3DBhabhaVetoAngle.clear();
    _mumuAngle.clear();
    m_taub2bAngleCut.clear();

    _TotalEnergy.clear();
    _LowMultiThreshold.clear();

    _TotalEnergy = {5, 10, 30}; // /100 MeV
    _2DBhabhaThresholdFWD = {40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 30, 35}; // /100 MeV
    _2DBhabhaThresholdBWD  = {25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 30, 30}; // /100 MeV
    _3DBhabhaVetoThreshold = {30, 45}; //  /100 MeV
    _3DBhabhaSelectionThreshold = {20, 40}; //  /100 MeV
    _3DBhabhaVetoAngle = {160, 200, 165, 190}; //  /100 MeV
    _3DBhabhaSelectionAngle = {140, 220, 160, 200}; //  /100 MeV
    _mumuThreshold = 20; //100 MeV
    _mumuAngle = {160, 200, 165, 190}; //  degree
    _LowMultiThreshold = {10, 20, 25, 30}; // degree
    _n300MeVCluster = 1;
    _ECLBurstThreshold = 20;
    m_3DBhabhaVetoInTrackThetaRegion = {3, 15};
    m_EventTimingQualityThresholds = {0.5, 10}; // GeV
    m_taub2bAngleCut = {110, 250, 130, 230}; // degree
    m_taub2bEtotCut = 7; // GeV
    m_taub2bClusterECut1 = 1.9; // GeV
    m_taub2bClusterECut2 = 999; // GeV

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
        _TotalEnergy = {(double)para.getELow(),
                        (double)para.getELow(),
                        (double)para.getELow()
                       };
        for (int index = 0; index < 14; index ++) {
          _2DBhabhaThresholdFWD.push_back((double)para.get2DBhabhaFWD(index));
          _2DBhabhaThresholdBWD.push_back((double)para.get2DBhabhaBWD(index));
        }
        _3DBhabhaVetoThreshold = {
          (double)para.get3DBhabhaVetoThreshold(0),
          (double)para.get3DBhabhaVetoThreshold(1)
        }; //  /100 MeV
        _3DBhabhaSelectionThreshold = {
          (double)para.get3DBhabhaSelectionThreshold(0),
          (double)para.get3DBhabhaSelectionThreshold(1)
        }; //  /100 MeV
        _3DBhabhaVetoAngle = {
          (double)para.get3DBhabhaVetoAngle(0),
          (double)para.get3DBhabhaVetoAngle(1),
          (double)para.get3DBhabhaVetoAngle(2),
          (double)para.get3DBhabhaVetoAngle(3)
        }; //  /100 MeV
        _3DBhabhaSelectionAngle = {
          (double)para.get3DBhabhaSelectionAngle(0),
          (double)para.get3DBhabhaSelectionAngle(1),
          (double)para.get3DBhabhaSelectionAngle(2),
          (double)para.get3DBhabhaSelectionAngle(3)
        }; //  /100 MeV
        _mumuThreshold =
          (double)(para.getmumuThreshold());
        _mumuAngle = {
          (double)para.getmumuAngle(0),
          (double)para.getmumuAngle(1),
          (double)para.getmumuAngle(2)
        };
        _LowMultiThreshold = {
          (double)para.getLowMultiThreshold(0),
          (double)para.getLowMultiThreshold(1),
          (double)para.getLowMultiThreshold(2),
          (double)para.getLowMultiThreshold(3)
        }; //  /100 MeV
      }
    }

    B2INFO("[TRGECLModule] 3DBhabhaVetoInTrackThetaRegion (low : high) = ("
           << m_3DBhabhaVetoInTrackThetaRegion[0] << " : "
           << m_3DBhabhaVetoInTrackThetaRegion[1] << ")");

    B2INFO("[TRGECLModule] EventTimingQualityThresholds (low : high)= ("
           << m_EventTimingQualityThresholds[0] << " : "
           << m_EventTimingQualityThresholds[1] << ") (GeV)");

    B2INFO("[TRGECLModule] Taub2b 2 CL Angle cut in CM (degree) (dphi L, H, theta sum L, H ) = ("
           << m_taub2bAngleCut[0] << ","
           << m_taub2bAngleCut[1] << ","
           << m_taub2bAngleCut[2] << ","
           << m_taub2bAngleCut[3] << ")");
    B2INFO("[TRGECLModule] Taub2b Total Energy Cut in lab. (GeV) = "
           << m_taub2bEtotCut);
    B2INFO("[TRGECLModule] Taub2b Cluster Energy Cut in lab. (GeV) : (E(CL1), E(CL2)) = ("
           << m_taub2bClusterECut1 << ", "
           << m_taub2bClusterECut2 << ")");

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

    _ecl->initialize(m_nEvent);
    _ecl->setClusterMethod(_Clustering);
    _ecl->setClusterLimit(_ClusterLimit);
    _ecl->setBhabhaMethod(_Bhabha);
    _ecl->setEventTimingMethod(_EventTiming);
    _ecl->setTimeWindow(_TimeWindow);
    _ecl->setOverlapWindow(_OverlapWindow);
    _ecl->setNofTopTC(_NofTopTC);
    _ecl->set2DBhabhaThreshold(_2DBhabhaThresholdFWD, _2DBhabhaThresholdBWD);
    _ecl->set3DBhabhaSelectionThreshold(_3DBhabhaSelectionThreshold);
    _ecl->set3DBhabhaVetoThreshold(_3DBhabhaVetoThreshold);
    _ecl->set3DBhabhaSelectionAngle(_3DBhabhaSelectionAngle);
    _ecl->set3DBhabhaVetoAngle(_3DBhabhaVetoAngle);
    _ecl->setmumuThreshold(_mumuThreshold);
    _ecl->setmumuAngle(_mumuAngle);
    _ecl->setTaub2bAngleCut(m_taub2bAngleCut);
    _ecl->setTaub2bEtotCut(m_taub2bEtotCut);
    _ecl->setTaub2bClusterECut(m_taub2bClusterECut1, m_taub2bClusterECut2);
    _ecl->setTotalEnergyThreshold(_TotalEnergy);
    _ecl->setLowMultiplicityThreshold(_LowMultiThreshold);
    _ecl->setn300MeVClusterThreshold(_n300MeVCluster);
    _ecl->setECLBurstThreshold(_ECLBurstThreshold);
    _ecl->set3DBhabhaVetoInTrackThetaRegion(m_3DBhabhaVetoInTrackThetaRegion);
    _ecl->setEventTimingQualityThresholds(m_EventTimingQualityThresholds);

    if (_SelectEvent == 0) {
      _ecl->simulate01(m_nEvent);
    } else if (_SelectEvent == 1) {
      _ecl->simulate02(m_nEvent);
    }
    // printf("TRGECLModule> eventId = %d \n", m_nEvent);
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
