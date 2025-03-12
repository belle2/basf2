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
#include <framework/database/DBObjPtr.h>

#include "trg/ecl/TrgEclMaster.h"
//trg package headers
#include "trg/trg/Debug.h"
#include "trg/ecl/modules/trgecl/TRGECLModule.h"
#include "trg/ecl/dbobjects/TRGECLETMPara.h"
#include "trg/ecl/dbobjects/TRGECLETMParameters.h"

#include <iostream>
#include <map>

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
    Module::Module(), m_debugLevel(0), m_Bhabha(0),
    m_Clustering(1), m_ClusterLimit(6), m_EventTiming(1),
    m_TimeWindow(250.0), m_OverlapWindow(125.0), m_NofTopTC(3),
    m_SelectEvent(1), m_ConditionDB(1)
  {

    string desc = "TRGECLModule(" + version() + ")";
    setDescription(desc);
    setPropertyFlags(c_ParallelProcessingCertified);

    addParam("DebugLevel",
             m_debugLevel,
             "TRGECL debug level",
             m_debugLevel);
    addParam("Bhabha",
             m_Bhabha,
             "TRGECL Bhabha method  0 : Belle I, 1 :belle II(default)",
             m_Bhabha);
    addParam("Clustering",
             m_Clustering,
             "TRGECL Clustering method  0 : use only ICN, 1 : ICN + Energy(Default)",
             m_Clustering);
    addParam("ClusterLimit",
             m_ClusterLimit,
             "The Limit number of cluster (Default:6)",
             m_ClusterLimit);
    addParam("EventTiming",
             m_EventTiming,
             "TRGECL EventTiming method  0 : Belle I, 1 : Energetic TC, 2 : Energy Weighted timing (default)",
             m_EventTiming);
    addParam("NofTopTC",
             m_NofTopTC,
             "TRGECL # of considered TC in energy weighted Timing method(Only work for EvenTiming Method 2)",
             m_NofTopTC);
    addParam("TimeWindow",
             m_TimeWindow,
             "TRGECL Trigger decision Time Window",
             m_TimeWindow);
    addParam("OverlapWindow",
             m_OverlapWindow,
             "TRGECL Trigger decision Time Window",
             m_OverlapWindow);
    addParam("EventSelect",
             m_SelectEvent,
             "TRGECL Select one trigger window for logic study",
             m_SelectEvent);
    addParam("ConditionDB",
             m_ConditionDB,
             "Flag to use Condition Database (0:=not use, 1:=use(default))",
             m_ConditionDB);
    //-----------------------------------------------
    addParam("ADCtoEnergy",
             m_ADCtoEnergy,
             "set conversion factor of ADC to Energy (GeV) : ",
             m_ADCtoEnergy);
    addParam("TotalEnergyCut",
             m_TotalEnergy,
             "set total energy cut(lowe, hie, lumie) in lab (GeV) : ",
             m_TotalEnergy);
    addParam("Bhabha2DThresholdFWD",
             m_2DBhabhaThresholdFWD,
             "set energy threshold(14 regions) of 2D Bhabha veto for FWD side in Lab. (GeV) : ",
             m_2DBhabhaThresholdFWD);
    addParam("Bhabha2DThresholdBWD",
             m_2DBhabhaThresholdBWD,
             "set energy threshold(14 regions) of 2D Bhabha veto for BWD side in Lab. (GeV) : ",
             m_2DBhabhaThresholdBWD);
    addParam("Bhabha3DVetoThreshold",
             m_3DBhabhaVetoThreshold,
             "set energy threshold(low, high) of 3D Bhabha veto in CMS (GeV) : ",
             m_3DBhabhaVetoThreshold);
    addParam("Bhabha3DVetoAngle",
             m_3DBhabhaVetoAngle,
             "set angle cut (phi diff L, H, and theta sum L, H) of 3D Bhabha veto in CMS(degree) : ",
             m_3DBhabhaVetoAngle);
    addParam("Bhabha3DSelectionThreshold",
             m_3DBhabhaSelectionThreshold,
             "set energy threshold(low, high) of 3D Bhabha selection in CMS (GeV) : ",
             m_3DBhabhaSelectionThreshold);
    addParam("Bhabha3DSelectionAngle",
             m_3DBhabhaSelectionAngle,
             "set angle cut (phi diff L, H, and theta sum L, H) of 3D Bhabha selection in CMS(degree) : ",
             m_3DBhabhaSelectionAngle);
    addParam("Bhabha3DSelectionPreScale",
             m_3DBhabhaSelectionPreScale,
             "set prescale (FW, BR, BW for Bhabha selection : ",
             m_3DBhabhaSelectionPreScale);
    addParam("mumuThreshold",
             m_mumuThreshold,
             "set cluster energy cut for mumu in CMS (GeV) : ",
             m_mumuThreshold);
    addParam("mumuAngle",
             m_mumuAngle,
             "set angle cut (phi diff L, H, and theta sum L, H) of mumu in CMS(degree) : ",
             m_mumuAngle);

    addParam("lmlCLELabCut",
             m_lmlCLELabCut,
             "set lml cluster energy thresholds(high, middle, low) in Lab in GeV : ",
             m_lmlCLELabCut);
    addParam("lmlCLECMSCut",
             m_lmlCLECMSCut,
             "set lml cluster energy thresholds(high, middle, low) in CMS in GeV : ",
             m_lmlCLECMSCut);
    addParam("lml00NCLforMinE",
             m_lml00NCLforMinE,
             "set lml00 the number of cluster for minimum energy cluster : ",
             m_lml00NCLforMinE);
    addParam("lml12NCLforMinE",
             m_lml12NCLforMinE,
             "set lml12 the number of cluster for minimum energy cluster : ",
             m_lml12NCLforMinE);
    addParam("lml13ThetaIdSelection",
             m_lml13ThetaIdSelection,
             "set lml13 theta ID selection : ",
             m_lml13ThetaIdSelection);

    addParam("ECLBurstThreshold",
             m_ECLBurstThreshold,
             "set ECL Burst Threshold in lab in GeV : ",
             m_ECLBurstThreshold);
    addParam("EventTimingQualityThreshold",
             m_EventTimingQualityThreshold,
             "set EventTimingQualityThreshold (low energy(GeV) and high energy(GeV))",
             m_EventTimingQualityThreshold);

    addParam("3DBhabhaVetoInTrackThetaRegion",
             m_3DBhabhaVetoInTrackThetaRegion,
             "set 3DBhabhaVetoInTrackThetaRegion (low TCID and high TCID)",
             m_3DBhabhaVetoInTrackThetaRegion);

    addParam("Taub2bAngleCut",
             m_taub2bAngleCut,
             "set Taub2bAngleCut(dphi low and high, theta sum low and high(degree)",
             m_taub2bAngleCut);
    addParam("Taub2bEtotCut",
             m_taub2bEtotCut,
             "set Taub2b total energy cut in TC ThetaID 1-17(GeV)",
             m_taub2bEtotCut);
    addParam("Taub2bCLELabCut",
             m_taub2bCLELabCut,
             "set Taub2b cluster energy cut in Lab (GeV) : ",
             m_taub2bCLELabCut);

    addParam("hie12BhabhaVetoAngle",
             m_hie12BhabhaVetoAngle,
             "set Angle Cut of Bhbaha Additional veto for hie (dphi low and high, theta sum low and high(degree)",
             m_hie12BhabhaVetoAngle);

    addParam("Taub2b2AngleCut",
             m_taub2b2AngleCut,
             "set Taub2b2AngleCut(dphi low and high, theta sum low and high(degree))",
             m_taub2b2AngleCut);
    addParam("Taub2b2EtotCut",
             m_taub2b2EtotCut,
             "set Taub2b2 total energy cut in TC ThetaID 1-17(GeV)",
             m_taub2b2EtotCut);
    addParam("Taub2b2CLELabCut",
             m_taub2b2CLELabCut,
             "set Taub2b2 cluster energy Cut(high, low) (GeV) : ",
             m_taub2b2CLELabCut);

    addParam("Taub2b3AngleCut",
             m_taub2b3AngleCut,
             "set Taub2b3AngleCut(dphi low and high, theta sum low and high in cms (degree))",
             m_taub2b3AngleCut);
    addParam("Taub2b3EtotCut",
             m_taub2b3EtotCut,
             "set Taub2b3 total energy cut in lab in ThetaID 1-17(GeV)",
             m_taub2b3EtotCut);
    addParam("Taub2b3CLEb2bLabCut",
             m_taub2b3CLEb2bLabCut,
             "set Taub2b3 cluster energy Cut in lab for one of b2b clusters (GeV) : ",
             m_taub2b3CLEb2bLabCut);
    addParam("Taub2b3CLELabCut",
             m_taub2b3CLELabCut,
             "set Taub2b3 cluster energy Cut(low, high) in lab for all clusters (GeV) : ",
             m_taub2b3CLELabCut);

    //-------------------------------------
    m_TotalEnergy.clear();
    m_2DBhabhaThresholdFWD.clear();
    m_2DBhabhaThresholdBWD.clear();
    m_3DBhabhaVetoThreshold.clear();
    m_3DBhabhaVetoAngle.clear();
    m_3DBhabhaSelectionThreshold.clear();
    m_3DBhabhaSelectionAngle.clear();
    m_3DBhabhaSelectionPreScale.clear();
    m_mumuAngle.clear();
    m_lmlCLELabCut.clear();
    m_lmlCLECMSCut.clear();
    m_EventTimingQualityThreshold.clear();
    m_3DBhabhaVetoInTrackThetaRegion.clear();
    m_hie12BhabhaVetoAngle.clear();
    m_taub2bAngleCut.clear();
    m_taub2b2AngleCut.clear();
    m_taub2b2CLELabCut.clear();
    m_taub2b3AngleCut.clear();
    m_taub2b3CLELabCut.clear();
    // lowe, hie, lume in Lab in GeV
    m_TotalEnergy = {0.5, 1.0, 3.0};
    // 2D Bhabha E cut in Lab in GeV (forward and backward sides)
    m_2DBhabhaThresholdFWD = {4.0, 4.0, 4.0, 4.0, 4.0, 4.0, 4.0, 4.0, 4.0, 4.0, 4.0, 4.0, 3.0, 3.5};
    m_2DBhabhaThresholdBWD = {2.5, 2.5, 2.5, 2.5, 2.5, 2.5, 2.5, 2.5, 2.5, 2.5, 2.5, 2.5, 3.0, 3.0};
    // 3D Bhabha
    m_3DBhabhaVetoThreshold = {3.0, 4.5}; // GeV
    m_3DBhabhaVetoAngle = {160, 200, 165, 190}; // degree
    // 3D Bhabha selection
    m_3DBhabhaSelectionThreshold = {2.5, 4.0}; // GeV
    m_3DBhabhaSelectionAngle = {140, 220, 160, 200}; // degree
    m_3DBhabhaSelectionPreScale = {1, 1, 1}; // pre-scale(no unit)
    // mumu
    m_mumuThreshold = 2.0; // GeV
    m_mumuAngle = {160, 200, 165, 190}; //  degree
    // lml
    m_lmlCLELabCut           = {0.5, 0.3, 0.25}; // in GeV
    m_lmlCLECMSCut           = {2.0, 1.0, 0.5};  // in GeV
    m_lml00NCLforMinE        = 1;
    m_lml12NCLforMinE        = 1;
    m_lml13ThetaIdSelection  = 2016;
    //
    m_ECLBurstThreshold = 20.0; // GeV
    //
    m_EventTimingQualityThreshold = {1.0, 20}; // GeV
    //
    m_3DBhabhaVetoInTrackThetaRegion = {3, 15};
    // taub2b
    m_taub2bAngleCut = {110, 250, 130, 230}; // degree
    m_taub2bEtotCut = 7.0; // GeV
    m_taub2bCLELabCut = 1.9; // GeV
    //
    m_hie12BhabhaVetoAngle = {150, 210, 160, 200}; // degree
    // taub2b2
    m_taub2b2AngleCut = {120, 240, 140, 220}; // degree
    m_taub2b2EtotCut = 7.0; // GeV
    m_taub2b2CLELabCut = {3.0, 0.162}; // GeV
    // taub2b3
    m_taub2b3AngleCut = {120, 240, 140, 220}; // degree
    m_taub2b3EtotCut = 7.0; // GeV
    m_taub2b3CLEb2bLabCut  = 0.14; // GeV
    m_taub2b3CLELabCut  = {0.12, 4.50}; // GeV
    //
    m_hie4LowCLELabCut = 0.5; // GeV

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

    TRGDebug::level(m_debugLevel);

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

    etm = new TrgEclMaster();

  }
  //
  //
  //
  void
  TRGECLModule::beginRun()
  {

    if (m_ConditionDB == 1) {

      //! read parameters from conditionDB
      DBObjPtr<TRGECLETMParameters> m_ETMParameters;

      const auto& dbParmap = m_ETMParameters->getparMap();

      B2DEBUG(100, "[TRGECLModule] valid N(par) in ETMParameters = "
              << m_ETMParameters->getnpar());
      B2DEBUG(100, "[TRGECLModule] all   N(par) in ETMParameters = "
              << dbParmap.size());

      m_ADCtoEnergy = getDBparmap(dbParmap, "adc2energy", 0);
      m_TotalEnergy = {
        getDBparmap(dbParmap, "lowe", 100),
        getDBparmap(dbParmap, "hie",  100),
        getDBparmap(dbParmap, "lume", 100)
      };
      m_2DBhabhaThresholdFWD = {
        getDBparmap(dbParmap, "bha2d_fw01", 100),
        getDBparmap(dbParmap, "bha2d_fw02", 100),
        getDBparmap(dbParmap, "bha2d_fw03", 100),
        getDBparmap(dbParmap, "bha2d_fw04", 100),
        getDBparmap(dbParmap, "bha2d_fw05", 100),
        getDBparmap(dbParmap, "bha2d_fw06", 100),
        getDBparmap(dbParmap, "bha2d_fw07", 100),
        getDBparmap(dbParmap, "bha2d_fw08", 100),
        getDBparmap(dbParmap, "bha2d_fw09", 100),
        getDBparmap(dbParmap, "bha2d_fw10", 100),
        getDBparmap(dbParmap, "bha2d_fw11", 100),
        getDBparmap(dbParmap, "bha2d_fw12", 100),
        getDBparmap(dbParmap, "bha2d_fw13", 100),
        getDBparmap(dbParmap, "bha2d_fw14", 100)
      };
      m_2DBhabhaThresholdBWD = {
        getDBparmap(dbParmap, "bha2d_bw01", 100),
        getDBparmap(dbParmap, "bha2d_bw02", 100),
        getDBparmap(dbParmap, "bha2d_bw03", 100),
        getDBparmap(dbParmap, "bha2d_bw04", 100),
        getDBparmap(dbParmap, "bha2d_bw05", 100),
        getDBparmap(dbParmap, "bha2d_bw06", 100),
        getDBparmap(dbParmap, "bha2d_bw07", 100),
        getDBparmap(dbParmap, "bha2d_bw08", 100),
        getDBparmap(dbParmap, "bha2d_bw09", 100),
        getDBparmap(dbParmap, "bha2d_bw10", 100),
        getDBparmap(dbParmap, "bha2d_bw11", 100),
        getDBparmap(dbParmap, "bha2d_bw12", 100),
        getDBparmap(dbParmap, "bha2d_bw13", 100),
        getDBparmap(dbParmap, "bha2d_bw14", 100)
      };
      m_3DBhabhaVetoThreshold = {
        getDBparmap(dbParmap, "bha3dveto_e_l", 100),
        getDBparmap(dbParmap, "bha3dveto_e_h", 100)
      };
      m_3DBhabhaVetoAngle = {
        (int) getDBparmap(dbParmap, "bha3dveto_phi_d_l",   0),
        (int) getDBparmap(dbParmap, "bha3dveto_phi_d_h",   0),
        (int) getDBparmap(dbParmap, "bha3dveto_theta_s_l", 0),
        (int) getDBparmap(dbParmap, "bha3dveto_theta_s_h", 0)
      };
      m_3DBhabhaSelectionThreshold = {
        getDBparmap(dbParmap, "bha3dsel_e_l", 100),
        getDBparmap(dbParmap, "bha3dsel_e_h", 100)
      };
      m_3DBhabhaSelectionAngle = {
        (int) getDBparmap(dbParmap, "bha3dsel_phi_d_l",   0),
        (int) getDBparmap(dbParmap, "bha3dsel_phi_d_h",   0),
        (int) getDBparmap(dbParmap, "bha3dsel_theta_s_l", 0),
        (int) getDBparmap(dbParmap, "bha3dsel_theta_s_h", 0)
      };
      m_3DBhabhaSelectionPreScale = {
        (int) getDBparmap(dbParmap, "bha3dsel_ps_fw", 0),
        (int) getDBparmap(dbParmap, "bha3dsel_ps_br", 0),
        (int) getDBparmap(dbParmap, "bha3dsel_ps_bw", 0)
      };
      m_mumuThreshold = getDBparmap(dbParmap, "mumu_e", 100);
      m_mumuAngle = {
        (int) getDBparmap(dbParmap, "mumu_phi_d_l",   0),
        (int) getDBparmap(dbParmap, "mumu_phi_d_h",   0),
        (int) getDBparmap(dbParmap, "mumu_theta_s_l", 0),
        (int) getDBparmap(dbParmap, "mumu_theta_s_h", 0)
      };
      m_lmlCLELabCut = {
        getDBparmap(dbParmap, "lml_cl_e_lab_1", 100),
        getDBparmap(dbParmap, "lml_cl_e_lab_2", 100),
        getDBparmap(dbParmap, "lml_cl_e_lab_3", 100)
      };
      m_lmlCLECMSCut = {
        getDBparmap(dbParmap, "lml_cl_e_cms_1", 100),
        getDBparmap(dbParmap, "lml_cl_e_cms_2", 100),
        getDBparmap(dbParmap, "lml_cl_e_cms_3", 100)
      };
      m_lml00NCLforMinE = (int) getDBparmap(dbParmap, "lml00_ncl", 100);
      m_lml12NCLforMinE = (int) getDBparmap(dbParmap, "lml12_ncl", 100);
      m_lml13ThetaIdSelection = (int) getDBparmap(dbParmap, "lml13_theta_id", 0);
      m_ECLBurstThreshold = getDBparmap(dbParmap, "e_burst", 100);
      m_EventTimingQualityThreshold = {
        getDBparmap(dbParmap, "event_timing_fine",  100),
        getDBparmap(dbParmap, "event_timing_super", 100)
      };
      m_3DBhabhaVetoInTrackThetaRegion = {
        (int) getDBparmap(dbParmap, "bha3d_theta_id_in_trk_l", 100),
        (int) getDBparmap(dbParmap, "bha3d_theta_id_in_trk_h",   0)
      };
      m_taub2bAngleCut = {
        (int) getDBparmap(dbParmap, "taub2b_phi_d_l",   0),
        (int) getDBparmap(dbParmap, "taub2b_phi_d_h",   0),
        (int) getDBparmap(dbParmap, "taub2b_theta_s_l", 0),
        (int) getDBparmap(dbParmap, "taub2b_theta_s_h", 0)
      };
      m_taub2bCLELabCut = getDBparmap(dbParmap, "taub2b_cl_e",  0);
      m_taub2bEtotCut   = getDBparmap(dbParmap, "taub2b_tot_e", 0);
      m_hie12BhabhaVetoAngle = {
        (int) getDBparmap(dbParmap, "hie12_phi_d_l",   0),
        (int) getDBparmap(dbParmap, "hie12_phi_d_h",   0),
        (int) getDBparmap(dbParmap, "hie12_theta_s_l", 0),
        (int) getDBparmap(dbParmap, "hie12_theta_s_h", 0)
      };
      m_taub2b2AngleCut = {
        (int) getDBparmap(dbParmap, "taub2b2_phi_d_l",   0),
        (int) getDBparmap(dbParmap, "taub2b2_phi_d_h",   0),
        (int) getDBparmap(dbParmap, "taub2b2_theta_s_l", 0),
        (int) getDBparmap(dbParmap, "taub2b2_theta_s_h", 0)
      };
      m_taub2b2EtotCut = getDBparmap(dbParmap, "taub2b2_tot_e", 0);
      m_taub2b2CLELabCut = {
        getDBparmap(dbParmap, "taub2b2_cl_e_endcap",  100),
        getDBparmap(dbParmap, "taub2b2_cl_e_all",     100)
      };
      m_taub2b3AngleCut = {
        (int) getDBparmap(dbParmap, "taub2b3_phi_d_l",   0),
        (int) getDBparmap(dbParmap, "taub2b3_phi_d_h",   0),
        (int) getDBparmap(dbParmap, "taub2b3_theta_s_l", 0),
        (int) getDBparmap(dbParmap, "taub2b3_theta_s_h", 0)
      };
      m_taub2b3EtotCut      = getDBparmap(dbParmap, "taub2b3_tot_e",    0);
      m_taub2b3CLEb2bLabCut = getDBparmap(dbParmap, "taub2b3_cl_e_b2b", 0);
      m_taub2b3CLELabCut = {
        getDBparmap(dbParmap, "taub2b3_cl_e_all_l", 100),
        getDBparmap(dbParmap, "taub2b3_cl_e_all_h", 100)
      };
      m_hie4LowCLELabCut = getDBparmap(dbParmap, "hie4_cl_e", 100);
    }
    //--------------------------------------------------------
    //
    //--------------------------------------------------------
    B2DEBUG(100, "[TRGECLModule] Clustering method (0 := BelleI, 1 := BelleII) = "
            << m_Clustering);

    B2DEBUG(100, "[TRGECLModule] ConditionDB usage (0 := no, 1 := yes) = "
            << m_ConditionDB);
    B2DEBUG(100, "[TRGECLModule] ADC to Energy in Lab in GeV = "
            << m_ADCtoEnergy);
    B2DEBUG(100, "[TRGECLModule] TotalEnergyCut (Low, middle, high) in Lab in GeV = ("
            << m_TotalEnergy[0] << ", "
            << m_TotalEnergy[1] << ", "
            << m_TotalEnergy[2] << ")");
    B2DEBUG(100, "[TRGECLModule] 2D BhaV E FW in GeV = ("
            << m_2DBhabhaThresholdFWD[0]  << ", "
            << m_2DBhabhaThresholdFWD[1]  << ", "
            << m_2DBhabhaThresholdFWD[2]  << ", "
            << m_2DBhabhaThresholdFWD[3]  << ", "
            << m_2DBhabhaThresholdFWD[4]  << ", "
            << m_2DBhabhaThresholdFWD[5]  << ", "
            << m_2DBhabhaThresholdFWD[6]  << ", "
            << m_2DBhabhaThresholdFWD[7]  << ", "
            << m_2DBhabhaThresholdFWD[8]  << ", "
            << m_2DBhabhaThresholdFWD[9]  << ", "
            << m_2DBhabhaThresholdFWD[10] << ", "
            << m_2DBhabhaThresholdFWD[11] << ", "
            << m_2DBhabhaThresholdFWD[12] << ", "
            << m_2DBhabhaThresholdFWD[13] << ")");
    B2DEBUG(100, "[TRGECLModule] 2D BhaV E BW in GeV = ("
            << m_2DBhabhaThresholdBWD[0]  << ", "
            << m_2DBhabhaThresholdBWD[1]  << ", "
            << m_2DBhabhaThresholdBWD[2]  << ", "
            << m_2DBhabhaThresholdBWD[3]  << ", "
            << m_2DBhabhaThresholdBWD[4]  << ", "
            << m_2DBhabhaThresholdBWD[5]  << ", "
            << m_2DBhabhaThresholdBWD[6]  << ", "
            << m_2DBhabhaThresholdBWD[7]  << ", "
            << m_2DBhabhaThresholdBWD[8]  << ", "
            << m_2DBhabhaThresholdBWD[9]  << ", "
            << m_2DBhabhaThresholdBWD[10] << ", "
            << m_2DBhabhaThresholdBWD[11] << ", "
            << m_2DBhabhaThresholdBWD[12] << ", "
            << m_2DBhabhaThresholdBWD[13] << ")");
    B2DEBUG(100, "[TRGECLModule] 3D Bhabha veto CL E Threshold in CMS in GeV (Low, High) = ("
            << m_3DBhabhaVetoThreshold[0] << ", "
            << m_3DBhabhaVetoThreshold[1] << ")");
    B2DEBUG(100, "[TRGECLModule] 3D Bhabha veto 2 CL angles in CMS in degree(dphi L, H, theta sum L, H) = ("
            << m_3DBhabhaVetoAngle[0] << ", "
            << m_3DBhabhaVetoAngle[1] << ", "
            << m_3DBhabhaVetoAngle[2] << ", "
            << m_3DBhabhaVetoAngle[3] << ")");
    B2DEBUG(100, "[TRGECLModule] 3D Bhabha selection CL E Threshold in CMS in GeV (Low, High) = ("
            << m_3DBhabhaSelectionThreshold[0] << ", "
            << m_3DBhabhaSelectionThreshold[1] << ")");
    B2DEBUG(100, "[TRGECLModule] 3D Bhabha selection 2 CL angles in CMS in degree(dphi L, H, theta sum L, H) = ("
            << m_3DBhabhaSelectionAngle[0] << ", "
            << m_3DBhabhaSelectionAngle[1] << ", "
            << m_3DBhabhaSelectionAngle[2] << ", "
            << m_3DBhabhaSelectionAngle[3] << ")");
    B2DEBUG(100, "[TRGECLModule] 3D Bhabha selection prescale(FW, BR, BW) = ("
            << m_3DBhabhaSelectionPreScale[0] << ", "
            << m_3DBhabhaSelectionPreScale[1] << ", "
            << m_3DBhabhaSelectionPreScale[2] << ")");
    B2DEBUG(100, "[TRGECLModule] mumu CL E Threshold in CMS in GeV = "
            << m_mumuThreshold);
    B2DEBUG(100, "[TRGECLModule] mumu 2 CL angles in CMS in degree(dphi L, H, theta sum L, H) = ("
            << m_mumuAngle[0] << ", "
            << m_mumuAngle[1] << ", "
            << m_mumuAngle[2] << ", "
            << m_mumuAngle[3] << ")");
    B2DEBUG(100, "[TRGECLModule] lml CL E cuts(high, mid, low) in Lab in GeV = ("
            << m_lmlCLELabCut[0] << ", "
            << m_lmlCLELabCut[1] << ", "
            << m_lmlCLELabCut[2] << ")");
    B2DEBUG(100, "[TRGECLModule] lml CL E cuts(high, mid, low) in CMS in GeV = ("
            << m_lmlCLECMSCut[0] << ", "
            << m_lmlCLECMSCut[1] << ", "
            << m_lmlCLECMSCut[2] << ")");
    B2DEBUG(100, "[TRGECLModule] lml00 the number of cluster for minimum energy cluster = "
            << m_lml00NCLforMinE);
    B2DEBUG(100, "[TRGECLModule] lml12 the number of cluster for minimum energy cluster = "
            << m_lml12NCLforMinE);
    B2DEBUG(100, "[TRGECLModule] lml13 ThetaID selection = "
            << m_lml13ThetaIdSelection);
    B2DEBUG(100, "[TRGECLModule] ECL Burst Threshold in lab. in GeV = "
            << m_ECLBurstThreshold);
    B2DEBUG(100, "[TRGECLModule] EventTimingQualityThreshold (low, high)= ("
            << m_EventTimingQualityThreshold[0] << ", "
            << m_EventTimingQualityThreshold[1] << ") (GeV)");
    B2DEBUG(100, "[TRGECLModule] 3DBhabhaVetoInTrackThetaRegion (low, high) = ("
            << m_3DBhabhaVetoInTrackThetaRegion[0] << ", "
            << m_3DBhabhaVetoInTrackThetaRegion[1] << ")");
    B2DEBUG(100, "[TRGECLModule] Taub2b two CL Angle cut in CM (degree) (dphi L, H, theta sum L, H ) = ("
            << m_taub2bAngleCut[0] << ", "
            << m_taub2bAngleCut[1] << ", "
            << m_taub2bAngleCut[2] << ", "
            << m_taub2bAngleCut[3] << ")");
    B2DEBUG(100, "[TRGECLModule] Taub2b Total Energy Cut in lab. (GeV) = "
            << m_taub2bEtotCut);
    B2DEBUG(100, "[TRGECLModule] Taub2b Cluster Energy Cut in lab. (GeV) = "
            << m_taub2bCLELabCut);
    B2DEBUG(100, "[TRGECLModule] hie12 Bhabha Add Angle cut in CM (degree) (dphi L, H, theta sum L, H) = ("
            << m_hie12BhabhaVetoAngle[0] << ", "
            << m_hie12BhabhaVetoAngle[1] << ", "
            << m_hie12BhabhaVetoAngle[2] << ", "
            << m_hie12BhabhaVetoAngle[3] << ")");
    B2DEBUG(100, "[TRGECLModule] Taub2b2 two CL Angle cut in CM (degree) (dphi L, H, theta sum L, H) = ("
            << m_taub2b2AngleCut[0] << ", "
            << m_taub2b2AngleCut[1] << ", "
            << m_taub2b2AngleCut[2] << ", "
            << m_taub2b2AngleCut[3] << ")");
    B2DEBUG(100, "[TRGECLModule] Taub2b2 Total Energy Cut in lab. (GeV) = "
            << m_taub2b2EtotCut);
    B2DEBUG(100, "[TRGECLModule] Taub2b2 Cluster energy cut(high, low) in lab. (GeV) = ("
            << m_taub2b2CLELabCut[0] << ", "
            << m_taub2b2CLELabCut[1] << ")");
    B2DEBUG(100, "[TRGECLModule] Taub2b3 two CL Angle cut in CM (degree) (dphi L, H, theta sum L, H) = ("
            << m_taub2b3AngleCut[0] << ", "
            << m_taub2b3AngleCut[1] << ", "
            << m_taub2b3AngleCut[2] << ", "
            << m_taub2b3AngleCut[3] << ")");
    B2DEBUG(100, "[TRGECLModule] Taub2b3 Total Energy Cut in lab. (GeV) = "
            << m_taub2b3EtotCut);
    B2DEBUG(100, "[TRGECLModule] Taub2b3 Cluster energy cut in lab. for one of b2b clusters (GeV) = "
            << m_taub2b3CLEb2bLabCut);
    B2DEBUG(100, "[TRGECLModule] Taub2b3 Cluster energy cut(low, high) in lab. for all clusters (GeV) = ("
            << m_taub2b3CLELabCut[0] << ", "
            << m_taub2b3CLELabCut[1] << ")");
    B2DEBUG(100, "[TRGECLModule] (hie4) CL E cut for minimum energy cluster = "
            << m_hie4LowCLELabCut);

    // set parameters to TRGECLMaster
    etm->initialize();
    etm->setClusterMethod(m_Clustering);
    etm->setClusterLimit(m_ClusterLimit);
    etm->setBhabhaMethod(m_Bhabha);
    etm->setEventTimingMethod(m_EventTiming);
    etm->setTimeWindow(m_TimeWindow);
    etm->setOverlapWindow(m_OverlapWindow);
    etm->setNofTopTC(m_NofTopTC);
    //
    etm->setADCtoEnergy(m_ADCtoEnergy);
    etm->setTotalEnergyThreshold(m_TotalEnergy);
    etm->set2DBhabhaThreshold(m_2DBhabhaThresholdFWD,
                              m_2DBhabhaThresholdBWD);
    etm->set3DBhabhaVetoThreshold(m_3DBhabhaVetoThreshold);
    etm->set3DBhabhaVetoAngle(m_3DBhabhaVetoAngle);
    etm->set3DBhabhaSelectionThreshold(m_3DBhabhaSelectionThreshold);
    etm->set3DBhabhaSelectionAngle(m_3DBhabhaSelectionAngle);
    etm->set3DBhabhaSelectionPreScale(m_3DBhabhaSelectionPreScale);
    etm->setmumuThreshold(m_mumuThreshold);
    etm->setmumuAngle(m_mumuAngle);
    etm->setlmlCLELabCut(m_lmlCLELabCut);
    etm->setlmlCLECMSCut(m_lmlCLECMSCut);
    etm->setlml00NCLforMinE(m_lml00NCLforMinE);
    etm->setlml12NCLforMinE(m_lml12NCLforMinE);
    etm->setlml13ThetaIdSelection(m_lml13ThetaIdSelection);
    etm->setECLBurstThreshold(m_ECLBurstThreshold);
    etm->setEventTimingQualityThreshold(m_EventTimingQualityThreshold);
    etm->set3DBhabhaVetoInTrackThetaRegion(m_3DBhabhaVetoInTrackThetaRegion);
    etm->setTaub2bAngleCut(m_taub2bAngleCut);
    etm->setTaub2bEtotCut(m_taub2bEtotCut);
    etm->setTaub2bCLELabCut(m_taub2bCLELabCut);
    etm->sethie12BhabhaVetoAngle(m_hie12BhabhaVetoAngle);
    etm->setTaub2b2Cut(m_taub2b2AngleCut,
                       m_taub2b2EtotCut,
                       m_taub2b2CLELabCut);
    etm->setTaub2b3Cut(m_taub2b3AngleCut,
                       m_taub2b3EtotCut,
                       m_taub2b3CLEb2bLabCut,
                       m_taub2b3CLELabCut);
    etm->sethie4LowCLELabCut(m_hie4LowCLELabCut);

    if (TRGDebug::level()) {
      std::cout << "TRGECLModule ... beginRun called " << std::endl;
    }

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

    // simulation
    if (m_SelectEvent == 0) {
      etm->simulate01(m_nEvent);
    } else if (m_SelectEvent == 1) {
      etm->simulate02(m_nEvent);
    }

    m_nEvent++;

  }
  //
  //
  //
  double
  TRGECLModule::getDBparmap(const std::map<std::string, double> dbParmap,
                            std::string parName,
                            double parAlternativeValue)
  {
    double par;
    if (dbParmap.count(parName)) {
      par = dbParmap.at(parName);
    } else {
      par = parAlternativeValue;
      B2WARNING("No key(\"" + parName +
                "\") for map in DB and set alternative value(" + parAlternativeValue +
                ")");
    }

    return par;
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
    delete etm;

    if (TRGDebug::level()) {
      std::cout << "TRGECLModule ... terminate called " << std::endl;
    }
  }
  //
  //
  //
} // namespace Belle2
