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
    addParam("3DBhabhaAddAngleCut",
             m_3DBhabhaAddAngleCut,
             "set Angle Cut of Bhbaha Additional veto for hie (dphi low and high, theta sum low and high(degree)",
             m_3DBhabhaAddAngleCut);
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

    addParam("Taub2b2AngleCut",
             m_taub2b2AngleCut,
             "set Taub2b2AngleCut(dphi low and high, theta sum low and high(degree))",
             m_taub2b2AngleCut);
    addParam("Taub2b2EtotCut",
             m_taub2b2EtotCut,
             "set Taub2b2 total energy cut in TC ThetaID 1-17(GeV)",
             m_taub2b2EtotCut);
    addParam("Taub2b2CLEEndcapCut",
             m_taub2b2CLEEndcapCut,
             "set Taub2b2 energy Cut (GeV) for cluseter in endcap (GeV) : ",
             m_taub2b2CLEEndcapCut);
    addParam("Taub2b2CLECut",
             m_taub2b2CLECut,
             "set Taub2b2 cluster energy Cut (GeV) : ",
             m_taub2b2CLECut);

    addParam("Taub2b3AngleCut",
             m_taub2b3AngleCut,
             "set Taub2b3AngleCut(dphi low and high, theta sum low and high in cms (degree))",
             m_taub2b3AngleCut);
    addParam("Taub2b3EtotCut",
             m_taub2b3EtotCut,
             "set Taub2b3 total energy cut in lab in ThetaID 1-17(GeV)",
             m_taub2b3EtotCut);
    addParam("Taub2b3CLEb2bCut",
             m_taub2b3CLEb2bCut,
             "set Taub2b3 cluster energy Cut in lab for one of b2b clusters (GeV) : ",
             m_taub2b3CLEb2bCut);
    addParam("Taub2b3CLELowCut",
             m_taub2b3CLELowCut,
             "set Taub2b3 cluster energy low Cut in lab for all clusters (GeV) : ",
             m_taub2b3CLELowCut);
    addParam("Taub2b3CLEHighCut",
             m_taub2b3CLEHighCut,
             "set Taub2b3 cluster energy high Cut in lab for all clusters (GeV) : ",
             m_taub2b3CLEHighCut);

    _2DBhabhaThresholdFWD.clear();
    _2DBhabhaThresholdBWD.clear();
    _3DBhabhaSelectionThreshold.clear();
    _3DBhabhaVetoThreshold.clear();
    _3DBhabhaSelectionAngle.clear();
    _3DBhabhaVetoAngle.clear();
    _mumuAngle.clear();
    m_3DBhabhaAddAngleCut.clear();
    m_taub2bAngleCut.clear();
    m_taub2b2AngleCut.clear();
    m_taub2b3AngleCut.clear();

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
    m_3DBhabhaAddAngleCut   = {150, 210, 160, 200}; // degree
    _LowMultiThreshold = {10, 20, 25, 30}; // degree
    _n300MeVCluster = 1;
    _ECLBurstThreshold = 20;
    m_3DBhabhaVetoInTrackThetaRegion = {3, 15};
    m_EventTimingQualityThresholds = {0.5, 10}; // GeV
    // taub2b
    m_taub2bAngleCut = {110, 250, 130, 230}; // degree
    m_taub2bEtotCut = 7; // GeV
    m_taub2bClusterECut1 = 1.9; // GeV
    m_taub2bClusterECut2 = 999; // GeV
    // taub2b2
    m_taub2b2AngleCut = {120, 240, 140, 220}; // degree
    m_taub2b2EtotCut = 7.0; // GeV
    m_taub2b2CLEEndcapCut = 3.0; // GeV
    m_taub2b2CLECut = 0.162; // GeV
    // taub2b3
    m_taub2b3AngleCut = {120, 240, 140, 220}; // degree
    m_taub2b3EtotCut = 7.0; // GeV
    m_taub2b3CLEb2bCut  = 0.14; // GeV
    m_taub2b3CLELowCut  = 0.12; // GeV
    m_taub2b3CLEHighCut = 4.50; // GeV

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
        };
        _3DBhabhaSelectionAngle = {
          (double)para.get3DBhabhaSelectionAngle(0),
          (double)para.get3DBhabhaSelectionAngle(1),
          (double)para.get3DBhabhaSelectionAngle(2),
          (double)para.get3DBhabhaSelectionAngle(3)
        };
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

    B2INFO("[TRGECLModule] Taub2b two CL Angle cut in CM (degree) (dphi L, H, theta sum L, H ) = ("
           << m_taub2bAngleCut[0] << ","
           << m_taub2bAngleCut[1] << ","
           << m_taub2bAngleCut[2] << ","
           << m_taub2bAngleCut[3] << ")");
    B2INFO("[TRGECLModule] Taub2b Total Energy Cut in lab. (GeV) = "
           << m_taub2bEtotCut);
    B2INFO("[TRGECLModule] Taub2b Cluster Energy Cut in lab. (GeV) : (E(CL1), E(CL2)) = ("
           << m_taub2bClusterECut1 << ", "
           << m_taub2bClusterECut2 << ")");
    B2INFO("[TRGECLModule] Bhabha Add Angle cut in CM (degree) (dphi L, H, theta sum L, H) = ("
           << m_3DBhabhaAddAngleCut[0] << ","
           << m_3DBhabhaAddAngleCut[1] << ","
           << m_3DBhabhaAddAngleCut[2] << ","
           << m_3DBhabhaAddAngleCut[3] << ")");

    B2INFO("[TRGECLModule] Taub2b2 two CL Angle cut in CM (degree) (dphi L, H, theta sum L, H) = ("
           << m_taub2b2AngleCut[0] << ","
           << m_taub2b2AngleCut[1] << ","
           << m_taub2b2AngleCut[2] << ","
           << m_taub2b2AngleCut[3] << ")");
    B2INFO("[TRGECLModule] Taub2b2 Total Energy Cut in lab. (GeV) = "
           << m_taub2b2EtotCut);
    B2INFO("[TRGECLModule] Taub2b2 Energy Cut in lab. for cluster in endcap (GeV) = "
           << m_taub2b2CLEEndcapCut);
    B2INFO("[TRGECLModule] Taub2b2 Cluster energy cut in lab. (GeV) = "
           << m_taub2b2CLECut);

    B2INFO("[TRGECLModule] Taub2b3 two CL Angle cut in CM (degree) (dphi L, H, theta sum L, H) = ("
           << m_taub2b3AngleCut[0] << ","
           << m_taub2b3AngleCut[1] << ","
           << m_taub2b3AngleCut[2] << ","
           << m_taub2b3AngleCut[3] << ")");
    B2INFO("[TRGECLModule] Taub2b3 Total Energy Cut in lab. (GeV) = "
           << m_taub2b3EtotCut);
    B2INFO("[TRGECLModule] Taub2b3 Cluster energy cut in lab. for one of b2b clusters (GeV) = "
           << m_taub2b3CLEb2bCut);
    B2INFO("[TRGECLModule] Taub2b3 Cluster energy low  cut in lab. for all clusters (GeV) = "
           << m_taub2b3CLELowCut);
    B2INFO("[TRGECLModule] Taub2b3 Cluster energy high cut in lab. for all clusters(GeV) = "
           << m_taub2b3CLEHighCut);

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
    _ecl->set3DBhabhaAddAngleCut(m_3DBhabhaAddAngleCut);
    _ecl->setTaub2bAngleCut(m_taub2bAngleCut);
    _ecl->setTaub2bEtotCut(m_taub2bEtotCut);
    _ecl->setTaub2bClusterECut(m_taub2bClusterECut1,
                               m_taub2bClusterECut2);
    _ecl->setTaub2b2Cut(m_taub2b2AngleCut,
                        m_taub2b2EtotCut,
                        m_taub2b2CLEEndcapCut,
                        m_taub2b2CLECut);
    _ecl->setTaub2b3Cut(m_taub2b3AngleCut,
                        m_taub2b3EtotCut,
                        m_taub2b3CLEb2bCut,
                        m_taub2b3CLELowCut,
                        m_taub2b3CLEHighCut);
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
