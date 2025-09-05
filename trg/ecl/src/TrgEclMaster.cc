/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

//---------------------------------------------------------------
// Description : A class to represent TRG ECL
//---------------------------------------------------------------------------------
//
//   ECL trigger bit
// ---------------------------------------------------------------------------------
// Variable(in Tsim) | N(bit) |   Address     | Parameter
// -------------------------------------- ------------------------------------------
//                   |      1 |         0     | TRG(Hit or not)
//                   |      7 |   7 downto 1  | Timing (LSB = 1 ns )
//   m_Triggerbit[0] |      7 |  14 downto 8  | Revoclk from FAM (LSB  = 125 ns)
//    (upto 12th     |      3 |  17 downto 15 | ECL-TRG Timing Source
//    bhabha bit)    |      1 |            18 | Physics TRG
//      (32bit)      |      1 |            19 | Belle type Bhabha
// ------------------|     14 |  33 downto 20 | Bhabha Type
//                   |     13 |  46 downto 34 | Total Energy
//                   |      1 |            47 | E low (Etot >0.5 GeV)
//                   |      1 |            48 | E High (Etot > 1.0 GeV)
//   m_Triggerbit[1] |      1 |            49 | E lom (Etot > 3.0 GeV)
//                   |      7 |  56 dwonto 50 | ICN
//                   |      3 |  59 downto 57 | BG veto
//                   |      1 |            60 | Cluster Overflow
//                   |      1 |            61 | 3D Bhabha Trigger for Veto
//                   |      1 |         62    | (lml0)  N Cluster  >= 3, at least one Cluster >300 MeV (LAB), not 3D ECL Bhabha
// _________________ |      1 |         63    | (lml1)  one Cluster >= 2GeV(CM) with Theta Id = 4~14
//                   |      1 |         64    | (lml2)  one Cluster >= 2GeV(CM) with Theta Id = 2,3,15 or 16 and not a 3D ECL Bhabha
//                   |      1 |         65    | (lml3)  one Cluster >= 2GeV(CM) with Theta Id = 2, 3, 15 or 16 and not a 3D ECL Bhabha
//                   |      1 |         66    | (lml4)  one Cluster >= 2GeV(CM) with Theta Id = 1 or 17 and not a 3D ECL Bhabha
//                   |      1 |         67    | (lml5)  one Cluster >= 2GeV(CM) with Theta Id = 1 or 17 and a 3D ECL Bhabha
//   m_Triggerbit[2] |      1 |         68    | (lml6)  exactly one Cluster >= 1GeV(CM) and one Cluster > 300  MeV (LAB ), in Theta Id 4~15(Barrel)
//                   |      1 |         69    | (lml7)  exactly one Cluster >= 1GeV(CM) and one Cluster > 300 MeV (LAB), in Theta Id 2, 3 or 16
//                   |      1 |         70    | (lml8)  170 < delta phi(CM) < 190 degree, both Clusters > 250 MeV (LAB), and no 2GeV (CM) Cluster
//                   |      1 |         71    | (lml9)  170 < delta phi(CM) < 190 degree, one Cluster < 250 MeV (LAB), the other Cluster > 250 MeV(LAB), and no 2GeV (CM) Cluster
//                   |      1 |         72    | (lml10) 160 < delta phi(CM) < 200 degree, 160 < Sum Theta (CM)< 200 degree, no 2 GeV(CM) cluster
//                   |      1 |         73    | (lml11) No 2GeV(CM) CL in an event
//                   |      1 |         74    | 3D Bhabha Trigger for selection
//                   |      1 |         75    | mumu bit
//                   |      1 |         76    | Bhabha prescale bit
//                   |      1 |         77    | E_tot > 20 GeV
//                   |      1 |         78    | (lml12) N Cluster  >= 3, at least one Cluster >500 MeV (LAB) with Theta Id 2~16, not 3D ECL Bhabha
//                   |      1 |         79    | (lml13) Only one Cluster >500 MeV (CM) with Theta Id 6~11 and no other CL >= 300 MeV(LAB) anywhere
//                   |      1 |  83 downto 80 | clock counter(set to be "0000" in tsim)
//                   |      1 |  85 downto 84 | event timing quality flag
//                   |      1 |         86    | 3D Bhabha Veto Intrk
//                   |      1 |  88 downto 87 | 3D Bhabha selection theta flag
//                   |      1 |         89    | [ecltaub2b] for 1x1 tau process : (110<delta phi(CM)<250, 130<Thata Sum(CM)<230, Etot1to17(Lab)<7GeV, E(1CL)(Lab)<1.9GeV)
//                   |      1 |         90    | [hie1] hie && 1CL veto(not (N(CL)=1 && CL in FW)) && 2CL veto-1(not (N(CL)=2 && 160 < dphi < 200 && 150 < sum theta < 250))
//                   |      1 |         91    | [hie2] hie && 1CL veto(not (N(CL)=1 && CL in FW)) && 2CL veto-2(not (N(CL)=2 && 160 < dphi < 200 || 150 < sum theta < 250))
//                   |      1 |         92    | [hie3] hie && 1CL veto(not (N(CL)=1 && CL in FW)) && 2CL veto-3(not (N(CL)=2 && CL_lowe in FW or BW)
//                   |      1 |         93    | [ecltaub2b v2] for 1x1 tau process : (120<delta phi(CM)<240, 140<Thata Sum(CM)<220, Etot1to17(Lab)<7GeV, N(CL) in endcap with E(CL)(3GeV) , E(CL)>0.165 for N(CL)==2, 1CL E(CL)>0.14 and 2CL E(CL)>0.165 for N(CL)==3,  2CL E(CL)>0.14 and 2CL E(CL)>0.165 for N(CL)==4,  1CL E(CL)>0.12 and 2CL E(CL)>0.14 and 2CL E(CL)>0.165 for N(CL)>4
//                   |      1 |         94    | [ecltaub2b v3] for 1x1 tau process : (120<delta phi(CM)<240, 140<Thata Sum(CM)<220, Etot1to17(Lab)<7GeV, E(CL)>0.140 in lab for one of 2CL in b2b CLs, CL ThetaID= 2-16 for b2b CLs, CL(E)>0.12GeV in lab for all CLs, CL(E)<4.5GeV in lab for all CLs)
//                   |      1 |         95    | [hie4] hie && 1CL veto(not (N(CL)=1 && CL in FW)) && 2CL veto-4(not (N(CL)=2 && CL_lowe in FW or BW) && E(CL_lowe)>0.5GeV)
// ---------------------------------------------------------------------------------

#define TRG_SHORT_NAMES
#define TRGECL_SHORT_NAMES

#include "framework/datastore/StoreArray.h"
#include "trg/ecl/TrgEclMaster.h"
#include "trg/ecl/TrgEclCluster.h"

#include "trg/ecl/dataobjects/TRGECLTrg.h"
#include "trg/ecl/dataobjects/TRGECLHit.h"
#include "trg/ecl/dataobjects/TRGECLCluster.h"
//
#include <math.h>
#include <TRandom3.h>
//
//
using namespace std;
using namespace Belle2;
//
//
//
TrgEclMaster::TrgEclMaster():
  m_TimeWindow(250.0), m_OverlapWindow(0.0), m_Clustering(1),
  m_EventTiming(1), m_NofTopTC(3), m_ClusterLimit(6), m_Triggerbit{0, 0, 0, 0},
  m_Lowmultibit(0), m_PrescaleFactor(0), m_PrescaleCounter(0)
{

  m_obj_cluster  = new TrgEclCluster();
  m_obj_beambkg  = new TrgEclBeamBKG();
  m_obj_bhabha   = new TrgEclBhabha();
  m_obj_timing   = new TrgEclTiming();
  m_obj_map      = new TrgEclMapping();
  m_obj_database = new TrgEclDataBase();

}
//
//
//
TrgEclMaster::~TrgEclMaster()
{
  delete m_obj_cluster;
  delete m_obj_beambkg;
  delete m_obj_bhabha;
  delete m_obj_timing;
  delete m_obj_map;
  delete m_obj_database;
}
//
//
//
std::string
TrgEclMaster::name(void) const
{
  return "TrgEclMaster";
}
//
//
//
std::string
TrgEclMaster::version(void) const
{
  return std::string("TrgEclMaster 2.1");
}
//
//
//
void
TrgEclMaster::initialize()
{
  m_TCEnergy.clear();
  m_TCTiming.clear();
  m_TCBeamBkgTag.clear();
  m_HitTCId.clear();
  m_TCHitEnergy.clear();
  m_TCHitTiming.clear();
  m_TCHitBeamBkgTag.clear();

  m_TCEnergy.resize(576);
  m_TCTiming.resize(576);
  m_TCBeamBkgTag.resize(576);

  // conversion factor of ADC to Energy in Lab in GeV
  m_ADCtoEnergy = 0.00525;
  // lowe, hie, lume in Lab in GeV
  m_TotalEnergy = {0.5, 1.0, 3.0};
  // 2D Bhabha E cut in Lab in GeV (forward and backward sides)
  m_2DBhabhaThresholdFWD  =
  {4.0, 4.0, 4.0, 4.0, 4.0, 4.0, 4.0, 4.0, 4.0, 4.0, 4.0, 4.0, 3.0, 3.5};
  m_2DBhabhaThresholdBWD  =
  {2.5, 2.5, 2.5, 2.5, 2.5, 2.5, 2.5, 2.5, 2.5, 2.5, 2.5, 2.5, 3.0, 3.0};
  //
  m_3DBhabhaVetoAngle      = {160, 200, 165, 190}; //  /100 MeV
  m_3DBhabhaVetoThreshold = {30, 45}; //  /100 MeV
  m_3DBhabhaSelectionThreshold = {20, 40}; //  /100 MeV
  m_3DBhabhaSelectionAngle = {140, 220, 160, 200}; //  /100 MeV
  m_3DBhabhaSelectionPreScale = {1, 1, 1};
  //
  m_mumuThreshold          = 2.0; // GeV
  m_mumuAngle              = {160, 200, 165, 190}; //  degree
  //
  m_lmlCLELabCut           = {0.5, 0.3, 0.25}; // in GeV
  m_lmlCLECMSCut           = {2.0, 1.0, 0.5};  // in GeV
  m_lml00NCLforMinE        = 1;
  m_lml12NCLforMinE        = 1;
  m_lml13ThetaIdSelection  = 2016;
  //
  m_ECLBurstThreshold = 20.0; // GeV
  //
  m_EventTimingQualityThreshold = {1.0, 20.0}; // GeV
  //
  m_3DBhabhaVetoInTrackThetaRegion = {3, 15};
  // taub2b cut
  m_taub2bAngleCut  = {110, 250, 130, 230}; // degree
  m_taub2bEtotCut   = 7.0; // GeV
  m_taub2bCLELabCut = 1.9; // GeV
  // hie1,2 BhabhaVeto angle
  m_hie12BhabhaVetoAngle = {150, 210, 160, 200}; // degree
  // taub2b2 cut
  m_taub2b2AngleCut  = {120, 240, 140, 220}; // degree
  m_taub2b2EtotCut   = 7.0; // GeV
  m_taub2b2CLELabCut = {3.0, 0.162}; // GeV
  //taub2b3 cut
  m_taub2b3AngleCut     = {120, 240, 140, 220}; // degree
  m_taub2b3EtotCut      = 7.0; // GeV
  m_taub2b3CLEb2bLabCut = 0.14; // GeV
  m_taub2b3CLELabCut    = {0.12, 4.5}; // GeV
  // hie4
  m_hie4LowCLELabCut = 0.5; //GeV

}
//========================================================
//
//========================================================
void
TrgEclMaster::simulate01(int m_nEvent) // Firmware simulator(time window 250 ns )
{
  //  TrgEclFAM* obj_trgeclfam = new TrgEclFAM();
  //   obj_trgeclfam->setup(m_nEvent, 1);
  // setPRS(obj_trgeclfam);
  //
  //----------
  // TC Etot
  //----------
  //
  // Energy sum of forward and barrel except for extreme forward
  // so Etot is sum of "phi ring ID" = 1-14
  // Etot threshold are
  // 1.0 GeV for Etot01
  // 0.5 GeV for Etot02nt
  // 3.0 GeV for Etot03
  //
  // Read FAM Output
  m_TCTiming.clear();
  m_TCEnergy.clear();
  m_TCBeamBkgTag.clear();
  m_TCEnergy.resize(576);
  m_TCTiming.resize(576);
  m_TCBeamBkgTag.resize(576);

  StoreArray<TRGECLHit> trgeclHitArray;
  for (int ii = 0; ii < trgeclHitArray.getEntries(); ii++) {

    TRGECLHit* aTRGECLHit = trgeclHitArray[ii];
    int iTCID = (aTRGECLHit->getTCId() - 1);
    double HitTiming  = aTRGECLHit->getTimeAve();
    double HitEnergy  = aTRGECLHit->getEnergyDep();
    double HitBeamBkg = aTRGECLHit->getBeamBkgTag();

    m_TCTiming[iTCID].push_back(HitTiming);
    m_TCEnergy[iTCID].push_back(HitEnergy);
    m_TCBeamBkgTag[iTCID].push_back(HitBeamBkg);
  }
  //
  //
  int nBin = 8000 / (m_TimeWindow / 2) ; //8000/125
  /* cppcheck-suppress variableScope */
  double WindowStart;
  /* cppcheck-suppress variableScope */
  double WindowEnd;
  double fluctuation = ((gRandom->Uniform(-1, 0))) * 125;
  /* cppcheck-suppress variableScope */
  double check_window_start;
  /* cppcheck-suppress variableScope */
  double check_window_end;

  for (int iBin = 0 ; iBin < nBin; iBin ++) {

    check_window_start = iBin * (m_TimeWindow / 3) + fluctuation - 4000;
    WindowStart = check_window_start;
    check_window_end  = check_window_start + m_TimeWindow / 3;
    WindowEnd = WindowStart + m_TimeWindow;
    m_HitTCId.clear();
    m_TCHitTiming.clear();
    m_TCHitEnergy.clear();
    m_TCHitBeamBkgTag.clear();

    // prepare TC Hit in time window --
    for (int iTCId = 0; iTCId < 576; iTCId++) {
      const int hitsize =  m_TCTiming[iTCId].size();
      for (int ihit = 0; ihit < hitsize; ihit++) {
        if (m_TCTiming[iTCId][ihit] > check_window_start &&
            m_TCTiming[iTCId][ihit] < check_window_end) {
          m_HitTCId.push_back(iTCId + 1);

        }
      }
    }
    if (m_HitTCId.size() == 0) {continue;}
    else {
      m_HitTCId.clear();
      for (int iTCId = 0; iTCId < 576; iTCId++) {
        const int hitsize =  m_TCTiming[iTCId].size();
        for (int ihit = 0; ihit < hitsize; ihit++) {
          if (m_TCTiming[iTCId][ihit] > WindowStart &&
              m_TCTiming[iTCId][ihit] < WindowEnd) {
            m_HitTCId.push_back(iTCId + 1);
            m_TCHitTiming.push_back(m_TCTiming[iTCId][ihit]);
            m_TCHitEnergy.push_back(m_TCEnergy[iTCId][ihit]);
          }
        }
      }
      iBin  = iBin + 2;
    }
    int noftchit = m_HitTCId.size();
    if (noftchit == 0) {continue;}

    double eventtiming = 0;
    // Get EventTiming
    m_obj_timing->Setup(m_HitTCId, m_TCHitEnergy, m_TCHitTiming);
    m_obj_timing->setNofTopTC(m_NofTopTC);
    m_obj_timing->setEventTimingQualityThreshold(m_EventTimingQualityThreshold);

    eventtiming = m_obj_timing->getEventTiming(m_EventTiming);
    int timingsource = m_obj_timing->getTimingSource();
    int EventTimingQualityFlag = m_obj_timing->getEventTimingQualityFlag();
    //--------------------------------------------------
    // Ring sum and Total Energy Sum
    //-------------------------------------------------
    std::vector<std::vector<double>>  thetaringsum;
    std::vector<double>  phiringsum;

    thetaringsum.clear();
    phiringsum.clear();
    thetaringsum.resize(3, std::vector<double>(36, 0));
    phiringsum.resize(17, 0);
    setRS(m_HitTCId, m_TCHitEnergy, phiringsum, thetaringsum);

    double E_phys = 0;
    double E_total = 0;
    int E_burst = 0;
    for (int iii = 0; iii <= 16; iii++) {
      if (iii > 0 && iii < 15) {E_phys += phiringsum[iii];}
      E_total += phiringsum[iii];
    }
    if (E_total == 0) {continue;}
    int ELow = 0, EHigh = 0, ELum = 0;

    if (E_total > m_ECLBurstThreshold / 10) {
      E_burst = 0x01;
    }
    if (E_phys > m_TotalEnergy[0]) { // GeV
      ELow = 0x01;
    }
    if (E_phys > m_TotalEnergy[1]) { // GeV
      EHigh = 0x01;
    }
    if (E_phys > m_TotalEnergy[2]) { // GeV
      ELum = 0x01;
    }
    //--------------
    // Clustering
    //--------------
    m_obj_cluster->initialize();
    m_obj_cluster->setClusteringMethod(m_Clustering);
    m_obj_cluster->setClusterLimit(m_ClusterLimit);
    m_obj_cluster->setEventId(m_nEvent);
    m_obj_cluster->setICN(m_HitTCId, m_TCHitEnergy, m_TCHitTiming);// Clustering
    m_obj_cluster->setICN(m_HitTCId); // Belle Cluster Counting

    int icn    = m_obj_cluster->getICNFwBr();
    int icnfwd = m_obj_cluster->getICNSub(0);
    int icnbr  = m_obj_cluster->getICNSub(1);
    int icnbwd = m_obj_cluster->getICNSub(2);
    //--------------
    // Bhabha veto
    //--------------
    setBhabhaParameter();

    std::vector<double> vct_bhabha;
    vct_bhabha.clear();
    int bhabha2D = 0 ;
    int bhabha3D_veto = 0 ;
    int bhabha3D_sel = 0;
    int mumu = 0;
    int bhabha3DVetoInTrackFlag = 0;
    int bhabha3DSelectionThetaFlag = 0;
    int taub2bFlag = 0;
    int taub2b2Flag = 0;
    int taub2b3Flag = 0;

    bool b_2Dbhabha = m_obj_bhabha->getBhabha00(phiringsum);
    vct_bhabha = m_obj_bhabha->getBhabhaComb();
    if (b_2Dbhabha && (icn < 4)) {bhabha2D = 1;}
    bool b_3Dbhabha =  m_obj_bhabha->getBhabha01();
    if (b_3Dbhabha) {bhabha3D_veto = 1;}
    bool b_3Dbhabha_sel =  m_obj_bhabha->getBhabha02();
    if (b_3Dbhabha_sel) {bhabha3D_sel = 1;}
    bool b_mumu =  m_obj_bhabha->getmumu();
    if (b_mumu) {mumu = 1;}
    bhabha3DVetoInTrackFlag = m_obj_bhabha->get3DBhabhaVetoInTrackFlag();
    bhabha3DSelectionThetaFlag = m_obj_bhabha->get3DBhabhaSelectionThetaFlag();
    taub2bFlag  = (m_obj_bhabha->getTaub2b(E_total))  ? 1 : 0;
    taub2b2Flag = (m_obj_bhabha->getTaub2b2(E_total)) ? 1 : 0;
    taub2b3Flag = (m_obj_bhabha->getTaub2b3(E_total)) ? 1 : 0;

    //------------------------
    // Beam Background veto (Old cosmic veto)
    //------------------------
    int beambkgtag = 0;
    beambkgtag = m_obj_beambkg->getBeamBkg(thetaringsum);

    int bhabhaprescale = 0;
    if (m_PrescaleFactor == m_PrescaleCounter) {
      bhabhaprescale = 1;
      m_PrescaleCounter = 0;
    } else if (m_PrescaleFactor > m_PrescaleCounter) {
      m_PrescaleCounter ++;
    }
    //--------------
    // Low Multiplicity bit
    //--------------
    std::vector<double> ClusterTiming;
    std::vector<double> ClusterEnergy;
    std::vector<int> MaxTCId;
    ClusterTiming.clear();
    ClusterEnergy.clear();
    MaxTCId.clear();
    StoreArray<TRGECLCluster> trgeclClusterArray;
    for (int ii = 0; ii < trgeclClusterArray.getEntries(); ii++) {
      TRGECLCluster* aTRGECLCluster = trgeclClusterArray[ii];
      int maxTCId    = aTRGECLCluster->getMaxTCId();
      double clusterenergy  = aTRGECLCluster->getEnergyDep();
      double clustertiming  =  aTRGECLCluster->getTimeAve();
      ClusterTiming.push_back(clustertiming);
      ClusterEnergy.push_back(clusterenergy);
      MaxTCId.push_back(maxTCId);
    }
    makeLowMultiTriggerBit(MaxTCId, ClusterEnergy, bhabha3D_veto);
    //-------------
    // Make ECL  Trigger Bit
    //-------------
    int hit = 1; // hit or not
    int Timing = (int)(eventtiming + 0.5);
    int RevoFAM = 0;
    int TimingSource = m_obj_timing->getTimingSource(); //  FWD(0), Barrel(0), Backward(0);
    int etot = (int)(E_phys * 1000 + 0.5); // total Energy in theta ID [2~15]
    int physics = 0;
    if ((etot > 1000 || icn > 3) && !(bhabha2D == 1)) {physics = 1;}
    std::vector<int> bhabhabit;
    bhabhabit.clear();
    int bhabhabitsize = vct_bhabha.size();
    for (int ibhabha = 0; ibhabha < bhabhabitsize; ibhabha++) {
      bhabhabit.push_back((int)vct_bhabha[ibhabha]);
    }
    int ClusterOverflow = m_obj_cluster->getNofExceedCluster();
    int flagoverflow = 0;
    if (ClusterOverflow > 0) {
      flagoverflow = 1;
    }

    makeTriggerBit(hit, Timing, 0, timingsource, E_phys, ELow, EHigh, ELum,
                   bhabha2D, physics, bhabhabit, icn, beambkgtag, flagoverflow,
                   bhabha3D_veto, m_Lowmultibit, bhabha3D_sel, mumu,
                   bhabhaprescale, E_burst,
                   EventTimingQualityFlag,
                   bhabha3DVetoInTrackFlag,
                   bhabha3DSelectionThetaFlag,
                   taub2bFlag,
                   0,
                   taub2b2Flag,
                   taub2b3Flag);

    int m_hitEneNum = 0;
    StoreArray<TRGECLTrg> trgEcltrgArray;
    trgEcltrgArray.appendNew();
    m_hitEneNum = trgEcltrgArray.getEntries() - 1;
    //-------------
    // Store
    //-------------
    trgEcltrgArray[m_hitEneNum]->setEventId(m_nEvent);
    trgEcltrgArray[m_hitEneNum]->setPRS01(phiringsum[0]);
    trgEcltrgArray[m_hitEneNum]->setPRS02(phiringsum[1]);
    trgEcltrgArray[m_hitEneNum]->setPRS03(phiringsum[2]);
    trgEcltrgArray[m_hitEneNum]->setPRS04(phiringsum[3]);
    trgEcltrgArray[m_hitEneNum]->setPRS05(phiringsum[4]);
    trgEcltrgArray[m_hitEneNum]->setPRS06(phiringsum[5]);
    trgEcltrgArray[m_hitEneNum]->setPRS07(phiringsum[6]);
    trgEcltrgArray[m_hitEneNum]->setPRS08(phiringsum[7]);
    trgEcltrgArray[m_hitEneNum]->setPRS09(phiringsum[8]);
    trgEcltrgArray[m_hitEneNum]->setPRS10(phiringsum[9]);
    trgEcltrgArray[m_hitEneNum]->setPRS11(phiringsum[10]);
    trgEcltrgArray[m_hitEneNum]->setPRS12(phiringsum[11]);
    trgEcltrgArray[m_hitEneNum]->setPRS13(phiringsum[12]);
    trgEcltrgArray[m_hitEneNum]->setPRS14(phiringsum[13]);
    trgEcltrgArray[m_hitEneNum]->setPRS15(phiringsum[14]);
    trgEcltrgArray[m_hitEneNum]->setPRS16(phiringsum[15]);
    trgEcltrgArray[m_hitEneNum]->setPRS17(phiringsum[16]);
    //
    trgEcltrgArray[m_hitEneNum]->setEtot(E_phys);
    trgEcltrgArray[m_hitEneNum]->setNofTCHit(noftchit);
    //
    trgEcltrgArray[m_hitEneNum]->setBhabha01(vct_bhabha[0]);
    trgEcltrgArray[m_hitEneNum]->setBhabha02(vct_bhabha[1]);
    trgEcltrgArray[m_hitEneNum]->setBhabha03(vct_bhabha[2]);
    trgEcltrgArray[m_hitEneNum]->setBhabha04(vct_bhabha[3]);
    trgEcltrgArray[m_hitEneNum]->setBhabha05(vct_bhabha[4]);
    trgEcltrgArray[m_hitEneNum]->setBhabha06(vct_bhabha[5]);
    trgEcltrgArray[m_hitEneNum]->setBhabha07(vct_bhabha[6]);
    trgEcltrgArray[m_hitEneNum]->setBhabha08(vct_bhabha[7]);
    trgEcltrgArray[m_hitEneNum]->setBhabha09(vct_bhabha[8]);
    trgEcltrgArray[m_hitEneNum]->setBhabha10(vct_bhabha[9]);
    trgEcltrgArray[m_hitEneNum]->setBhabha11(vct_bhabha[10]);
    trgEcltrgArray[m_hitEneNum]->setBhabha12(vct_bhabha[11]);
    trgEcltrgArray[m_hitEneNum]->setBhabha13(vct_bhabha[12]);
    trgEcltrgArray[m_hitEneNum]->setBhabha14(vct_bhabha[13]);
    trgEcltrgArray[m_hitEneNum]->setBhabha15(vct_bhabha[14]);
    trgEcltrgArray[m_hitEneNum]->setBhabha16(vct_bhabha[15]);
    trgEcltrgArray[m_hitEneNum]->setBhabha17(vct_bhabha[16]);
    trgEcltrgArray[m_hitEneNum]->setBhabha18(vct_bhabha[17]);
    //
    trgEcltrgArray[m_hitEneNum]->setICN(icn);
    trgEcltrgArray[m_hitEneNum]->setICNFw(icnfwd);
    trgEcltrgArray[m_hitEneNum]->setICNBr(icnbr);
    trgEcltrgArray[m_hitEneNum]->setICNBw(icnbwd);
    //
    trgEcltrgArray[m_hitEneNum]->setECLtoGDL(m_Triggerbit[0], 0);
    trgEcltrgArray[m_hitEneNum]->setECLtoGDL(m_Triggerbit[1], 1);
    trgEcltrgArray[m_hitEneNum]->setECLtoGDL(m_Triggerbit[2], 2);
    trgEcltrgArray[m_hitEneNum]->setECLtoGDL(m_Triggerbit[3], 3);

    trgEcltrgArray[m_hitEneNum]->setBhabhaVeto(bhabha2D);
    trgEcltrgArray[m_hitEneNum]->setBeamBkgVeto(beambkgtag);
    trgEcltrgArray[m_hitEneNum]->setEventTiming(eventtiming);

    trgEcltrgArray[m_hitEneNum]->setHit(hit);
    trgEcltrgArray[m_hitEneNum]->setRevoclk(RevoFAM);
    trgEcltrgArray[m_hitEneNum]->setTimingSource(TimingSource);
    trgEcltrgArray[m_hitEneNum]->setPhysics(physics) ;
    trgEcltrgArray[m_hitEneNum]->set2DBhabha(bhabha2D);
    trgEcltrgArray[m_hitEneNum]->set3DBhabha(bhabha3D_veto);
    trgEcltrgArray[m_hitEneNum]->set3DBhabhaSel(bhabha3D_sel);
    trgEcltrgArray[m_hitEneNum]->setmumuBit(mumu);
    trgEcltrgArray[m_hitEneNum]->setBhabhaPrescaleBit(bhabhaprescale);

    trgEcltrgArray[m_hitEneNum]->setELow(ELow)  ;
    trgEcltrgArray[m_hitEneNum]->setEHihg(EHigh);
    trgEcltrgArray[m_hitEneNum]->setELum(ELum)  ;
    trgEcltrgArray[m_hitEneNum]->setClusterOverflow(ClusterOverflow) ;
    trgEcltrgArray[m_hitEneNum]->setLowMultiBit(m_Lowmultibit);
  }

  return;
}
//========================================================
//
//========================================================
void
TrgEclMaster::simulate02(int m_nEvent) // select one window for analyze trigger logic
{
  //  TrgEclFAM* obj_trgeclfam = new TrgEclFAM();
  //   obj_trgeclfam->setup(m_nEvent, 1);
  // setPRS(obj_trgeclfam);
  //
  //----------
  // TC Etot
  //----------
  //
  // Energy sum of forward and barrel except for extreme forward
  // so Etot is sum of "phi ring ID" = 1-14
  // Etot threshold are
  // 1.0 GeV for Etot01
  // 0.5 GeV for Etot02nt
  // 3.0 GeV for Etot03
  //
  // Read FAM Output
  m_TCTiming.clear();
  m_TCEnergy.clear();
  m_TCBeamBkgTag.clear();
  m_TCEnergy.resize(576);
  m_TCTiming.resize(576);
  m_TCBeamBkgTag.resize(576);

  StoreArray<TRGECLHit> trgeclHitArray;
  for (int ii = 0; ii < trgeclHitArray.getEntries(); ii++) {

    TRGECLHit* aTRGECLHit = trgeclHitArray[ii];
    int iTCID = (aTRGECLHit->getTCId() - 1);
    double HitTiming  = aTRGECLHit->getTimeAve();
    double HitEnergy  = aTRGECLHit->getEnergyDep();
    double HitBeamBkg = aTRGECLHit->getBeamBkgTag();

    m_TCTiming[iTCID].push_back(HitTiming);
    m_TCEnergy[iTCID].push_back(HitEnergy);
    m_TCBeamBkgTag[iTCID].push_back(HitBeamBkg);
  }
  //
  //
  int nBin = 2 * 8000 / m_TimeWindow ;
  double WindowStart = 0;
  double WindowEnd = 0;
  double fluctuation = ((gRandom->Uniform(-1, 0))) * 125;

  int startBin = nBin / 2 - 1; //start previous bin near 0s

  int endBin = nBin / 2 + 1; //start next bin near 0s

  if (m_EventTiming == 0) {
    m_TimeWindow = 500;
    m_OverlapWindow = 0;
  }

  double maxE = 0;
  int max_bin = 0;

  for (int iBin = startBin ; iBin <= endBin; iBin ++) {
    WindowStart = iBin * (m_TimeWindow - m_OverlapWindow) + fluctuation - 4000;

    if (iBin == 0) {WindowStart = - 4000 + fluctuation;}
    WindowEnd = WindowStart + m_TimeWindow;

    double totalE = 0;
    // prepare TC Hit in time window --
    for (int iTCId = 0; iTCId < 576; iTCId++) {
      const int hitsize =  m_TCTiming[iTCId].size();
      for (int ihit = 0; ihit < hitsize; ihit++) {
        if (m_TCTiming[iTCId][ihit] > WindowStart &&
            m_TCTiming[iTCId][ihit] < WindowEnd) {
          totalE += m_TCEnergy[iTCId][ihit] ;
        }
      }
    }
    if (totalE == 0) {continue;}
    if (maxE < totalE) { //select the bin having the highest total energy
      maxE = totalE;
      max_bin = iBin;
    }
  }

  WindowStart = max_bin * (m_TimeWindow - m_OverlapWindow) + fluctuation - 4000;
  if (max_bin == 0) {WindowStart = - 4000 + fluctuation;}
  WindowEnd = WindowStart + m_TimeWindow;

  m_HitTCId.clear();
  m_TCHitTiming.clear();
  m_TCHitEnergy.clear();
  m_TCHitBeamBkgTag.clear();

  // prepare TC Hit in time window --
  for (int iTCId = 0; iTCId < 576; iTCId++) {
    const int hitsize =  m_TCTiming[iTCId].size();
    for (int ihit = 0; ihit < hitsize; ihit++) {
      if (m_TCTiming[iTCId][ihit] > WindowStart &&
          m_TCTiming[iTCId][ihit] < WindowEnd) {
        m_HitTCId.push_back(iTCId + 1);
        m_TCHitTiming.push_back(m_TCTiming[iTCId][ihit]);
        m_TCHitEnergy.push_back(m_TCEnergy[iTCId][ihit]);
        m_TCHitBeamBkgTag.push_back(m_TCBeamBkgTag[iTCId][ihit]);
      }
    }
  }

  int noftchit = m_HitTCId.size();
  if (noftchit == 0) { return;}

  double eventtiming = 0;
  // Get EventTiming
  m_obj_timing->Setup(m_HitTCId, m_TCHitEnergy, m_TCHitTiming);
  m_obj_timing->setNofTopTC(m_NofTopTC);
  m_obj_timing->setEventTimingQualityThreshold(m_EventTimingQualityThreshold);

  eventtiming = m_obj_timing->getEventTiming(m_EventTiming);
  int timingsource = m_obj_timing->getTimingSource();

  int EventTimingQualityFlag = m_obj_timing->getEventTimingQualityFlag();
  //--------------------------------------------------
  // Ring sum and Total Energy Sum
  //-------------------------------------------------
  std::vector<std::vector<double>>  thetaringsum;
  std::vector<double>  phiringsum;

  thetaringsum.clear();
  phiringsum.clear();
  thetaringsum.resize(3, std::vector<double>(36, 0));
  phiringsum.resize(17, 0);
  setRS(m_HitTCId, m_TCHitEnergy, phiringsum, thetaringsum);

  //double E_br;  //variable not used
  //double E_fwd; //variable not used
  //double E_bwd; //variable not used
  double E_phys = 0;
  double E_total = 0;
  int E_burst = 0;

  for (int iii = 0; iii <= 16; iii++) {
    if (iii > 0 && iii < 15) {E_phys += phiringsum[iii];}
    //if (iii < 3) {E_fwd += phiringsum[iii];} //TODO variable not used, should be?
    //if (iii > 2 && iii < 15) {E_br += phiringsum[iii];} //TODO not used, should be?
    //if (iii > 14) {E_bwd += phiringsum[iii];} //TODO not used, should be?
    E_total += phiringsum[iii];
  }

  if (E_total == 0) {return;}
  int ELow = 0, EHigh = 0, ELum = 0;
  if (E_total > m_ECLBurstThreshold) {
    E_burst = 0x01;
  }

  if (E_phys > m_TotalEnergy[0]) { // GeV
    ELow = 0x01;
  }
  if (E_phys > m_TotalEnergy[1]) { // GeV
    EHigh = 0x01;
  }
  if (E_phys > m_TotalEnergy[2]) { // GeV
    ELum = 0x01;
  }
  //--------------
  // Clustering
  //--------------
  m_obj_cluster->initialize();
  m_obj_cluster->setClusteringMethod(m_Clustering);
  m_obj_cluster->setClusterLimit(m_ClusterLimit);
  m_obj_cluster->setEventId(m_nEvent);
  m_obj_cluster->setICN(m_HitTCId, m_TCHitEnergy, m_TCHitTiming);// Clustering
  m_obj_cluster->setICN(m_HitTCId); // Belle Cluster Counting

  int icn    = m_obj_cluster->getICNFwBr();
  int icnfwd = m_obj_cluster->getICNSub(0);
  int icnbr  = m_obj_cluster->getICNSub(1);
  int icnbwd = m_obj_cluster->getICNSub(2);

  int NofCluster1to17 =  m_obj_cluster->getNofCluster();
  //--------------
  // Bhabha veto (and mumu and tau b2b trigger)
  //--------------
  setBhabhaParameter();

  std::vector<double> vct_bhabha;
  vct_bhabha.clear();
  int bhabha2D = 0 ;
  int bhabha3D_veto = 0 ;
  int bhabha3D_sel = 0;
  int mumu = 0;
  int bhabha3DVetoInTrackFlag = -1;
  int bhabha3DSelectionThetaFlag = -1;
  int taub2bFlag = 0;
  int taub2b2Flag = 0;
  int taub2b3Flag = 0;

  bool b_2Dbhabha = m_obj_bhabha->getBhabha00(phiringsum);
  vct_bhabha = m_obj_bhabha->getBhabhaComb();
  if (b_2Dbhabha && (icn < 4)) {bhabha2D = 1;}
  bool b_3Dbhabha =  m_obj_bhabha->getBhabha01();
  if (b_3Dbhabha) {bhabha3D_veto = 1;}
  bool b_3Dbhabha_sel =  m_obj_bhabha->getBhabha02();
  if (b_3Dbhabha_sel) {bhabha3D_sel = 1;}
  bool b_mumu =  m_obj_bhabha->getmumu();
  if (b_mumu) {mumu = 1;}
  int bhabhaprescale = 0;
  if (m_PrescaleFactor == m_PrescaleCounter) {
    bhabhaprescale = 1;
    m_PrescaleCounter = 0;
  } else if (m_PrescaleFactor > m_PrescaleCounter) {
    m_PrescaleCounter ++;
  }
  bhabha3DVetoInTrackFlag    = m_obj_bhabha->get3DBhabhaVetoInTrackFlag();
  bhabha3DSelectionThetaFlag = m_obj_bhabha->get3DBhabhaSelectionThetaFlag();
  taub2bFlag  = (m_obj_bhabha->getTaub2b(E_total)) ? 1 : 0;
  taub2b2Flag = (m_obj_bhabha->getTaub2b2(E_total)) ? 1 : 0;
  taub2b3Flag = (m_obj_bhabha->getTaub2b3(E_total)) ? 1 : 0;

  //------------------------
  // additional Bhabha veto
  //------------------------
  int bhabha_addition = m_obj_bhabha->getBhabhaAddition();
  //------------------------
  // hie with additional Bhabha veto
  //------------------------
  int beambkgtag = 0;
  beambkgtag = m_obj_beambkg->getBeamBkg(thetaringsum);

  int bit_hie_bhav = 0;
  if (E_phys > 1.0) {
    bit_hie_bhav |= (~bhabha_addition & 0x01) & (~bhabha_addition >> 4 & 0x01); // hie4
    bit_hie_bhav <<= 1;
    bit_hie_bhav |= (~bhabha_addition & 0x01) & (~bhabha_addition >> 3 & 0x01); // hie3
    bit_hie_bhav <<= 1;
    bit_hie_bhav |= (~bhabha_addition & 0x01) & (~bhabha_addition >> 2 & 0x01); // hie2
    bit_hie_bhav <<= 1;
    bit_hie_bhav |= (~bhabha_addition & 0x01) & (~bhabha_addition >> 1 & 0x01); // hie1
  }
  //--------------
  // Low Multiplicity bit
  //--------------
  std::vector<double> ClusterTiming;
  std::vector<double> ClusterEnergy;
  std::vector<int> MaxTCId;
  std::vector<int> MaxThetaId;
  ClusterTiming.clear();
  ClusterEnergy.clear();
  MaxTCId.clear();
  MaxThetaId.clear();
  StoreArray<TRGECLCluster> trgeclClusterArray;
  for (int ii = 0; ii < trgeclClusterArray.getEntries(); ii++) {
    TRGECLCluster* aTRGECLCluster = trgeclClusterArray[ii];
    int maxTCId    = aTRGECLCluster->getMaxTCId();
    int maxThetaId = aTRGECLCluster->getMaxThetaId();
    double clusterenergy  = aTRGECLCluster->getEnergyDep();
    double clustertiming  =  aTRGECLCluster->getTimeAve();
    ClusterTiming.push_back(clustertiming);
    ClusterEnergy.push_back(clusterenergy);
    MaxTCId.push_back(maxTCId);
    MaxThetaId.push_back(maxThetaId);
  }
  makeLowMultiTriggerBit(MaxTCId, ClusterEnergy, bhabha3D_veto);
  //-------------
  // Make ECL  Trigger Bit
  //-------------
  int hit = 1; // hit or not
  int Timing = (int)(eventtiming + 0.5);
  int RevoFAM = 0;
  int TimingSource = m_obj_timing->getTimingSource(); //  FWD(0), Barrel(0), Backward(0);
  int etot = (int)(E_phys * 1000 + 0.5); // total Energy in theta ID [2~15]

  //int bhabha2D = BtoBTag ;
  int physics = 0;
  if ((etot > 1000 || icn > 3) && !(bhabha2D == 1)) {physics = 1;}
  std::vector<int> bhabhabit;
  bhabhabit.clear();
  int bhabhabitsize = vct_bhabha.size();
  for (int ibhabha = 0; ibhabha < bhabhabitsize; ibhabha++) {
    bhabhabit.push_back((int)vct_bhabha[ibhabha]);
  }
  int ClusterOverflow = m_obj_cluster->getNofExceedCluster();
  int flagoverflow = 0;
  if (ClusterOverflow > 0) {
    flagoverflow = 1;
  }

  makeTriggerBit(hit, Timing, 0, timingsource, E_phys, ELow, EHigh, ELum,
                 bhabha2D, physics, bhabhabit, icn, beambkgtag, flagoverflow,
                 bhabha3D_veto, m_Lowmultibit, bhabha3D_sel, mumu,
                 bhabhaprescale, E_burst,
                 EventTimingQualityFlag,
                 bhabha3DVetoInTrackFlag,
                 bhabha3DSelectionThetaFlag,
                 taub2bFlag,
                 bit_hie_bhav,
                 taub2b2Flag,
                 taub2b3Flag);

  //----------------------------------------------------
  // ECL trigger
  //----------------------------------------------------
  // Integer GDL "Output word to GDL:"
  //  "bit0-2 = Etot1,2,3"
  //  "bit3 = Bhabha,"
  //  "bit4 = prescaled Bhabha,"
  //  "bit5-8 = ICN(3bits)+FORWARD(1bit) OR ICN(3+1 carry),"
  //  "bit9 = cosmic,"
  //  "bit10 = neutral timing trigger"
  //
  //------------------------------
  //          2         10     16
  //------------------------------
  //  1 0000000000001     1     1   Etot1
  //  2 0000000000010     2     2   Etot2
  //  3 0000000000100     4     4   Etot3
  //  4 0000000001000     8     8   Bhabha
  //  5 0000000010000    16    10   preBhabha
  //  6 0000000100000    32    20   ICN
  //  7 0000001000000    64    40   ICN
  //  8 0000010000000   128    80   ICN
  //  9 0000100000000   256   100   ForwardICN
  // 10 0001000000000   512   200   BeamBkgVeto
  // 11 0010000000000  1024   400   Timing
  //------------------------------
  // int bitEtot1       = 0x0001;
  // int bitEtot2       = 0x0002;
  // int bitEtot3       = 0x0004;
  // int bitBhabha      = 0x0008;
  // int bitPreBhabha   = 0x0010;
  // int bitForwardICN  = 0x0100;
  // int bitBeamBkgVeto = 0x0200;
  // int bitTiming      = 0x0400;

  // bool boolEtot[3] = {false};
  // if (E_phys > 1.0) boolEtot[1] = true;
  // bool boolBhabha = (boolBtoBTag && icn > 4);
  // bool boolPreBhabha = false;
  // bool boolForwardICN = icnfwd;
  // bool boolBeamBkgVeto = boolBeamBkgTag;
  // int bit = 0;
  // //
  // // bit 5-7
  // bit = (icn >= 7) ? 0x0007 : icn;
  // bit <<= 5;
  // // bit 0
  // bit |= boolEtot[0] ? bitEtot1 : 0;
  // // bit 1
  // bit |= boolEtot[1] ? bitEtot2 : 0;
  // // bit 2
  // bit |= boolEtot[2] ? bitEtot3 : 0;
  // // bit 3
  // bit |= boolBhabha  ? bitBhabha : 0;
  // // bit 4
  // bit |= boolPreBhabha   ? bitPreBhabha : 0;
  // // bit 8
  // bit |= boolForwardICN  ? bitForwardICN : 0;
  // // bit 9
  // bit |= boolBeamBkgVeto ? bitBeamBkgVeto : 0;
  // // bit 10
  // bit |= bitTiming;


  int m_hitEneNum = 0;
  StoreArray<TRGECLTrg> trgEcltrgArray;
  trgEcltrgArray.appendNew();
  m_hitEneNum = trgEcltrgArray.getEntries() - 1;
  //----------------------------------------------------
  // Store
  //----------------------------------------------------
  trgEcltrgArray[m_hitEneNum]->setEventId(m_nEvent);
  trgEcltrgArray[m_hitEneNum]->setPRS01(phiringsum[0]);
  trgEcltrgArray[m_hitEneNum]->setPRS02(phiringsum[1]);
  trgEcltrgArray[m_hitEneNum]->setPRS03(phiringsum[2]);
  trgEcltrgArray[m_hitEneNum]->setPRS04(phiringsum[3]);
  trgEcltrgArray[m_hitEneNum]->setPRS05(phiringsum[4]);
  trgEcltrgArray[m_hitEneNum]->setPRS06(phiringsum[5]);
  trgEcltrgArray[m_hitEneNum]->setPRS07(phiringsum[6]);
  trgEcltrgArray[m_hitEneNum]->setPRS08(phiringsum[7]);
  trgEcltrgArray[m_hitEneNum]->setPRS09(phiringsum[8]);
  trgEcltrgArray[m_hitEneNum]->setPRS10(phiringsum[9]);
  trgEcltrgArray[m_hitEneNum]->setPRS11(phiringsum[10]);
  trgEcltrgArray[m_hitEneNum]->setPRS12(phiringsum[11]);
  trgEcltrgArray[m_hitEneNum]->setPRS13(phiringsum[12]);
  trgEcltrgArray[m_hitEneNum]->setPRS14(phiringsum[13]);
  trgEcltrgArray[m_hitEneNum]->setPRS15(phiringsum[14]);
  trgEcltrgArray[m_hitEneNum]->setPRS16(phiringsum[15]);
  trgEcltrgArray[m_hitEneNum]->setPRS17(phiringsum[16]);
  //
  trgEcltrgArray[m_hitEneNum]->setEtot(E_phys);
  trgEcltrgArray[m_hitEneNum]->setNofTCHit(noftchit);
  //
  trgEcltrgArray[m_hitEneNum]->setBhabha01(vct_bhabha[0]);
  trgEcltrgArray[m_hitEneNum]->setBhabha02(vct_bhabha[1]);
  trgEcltrgArray[m_hitEneNum]->setBhabha03(vct_bhabha[2]);
  trgEcltrgArray[m_hitEneNum]->setBhabha04(vct_bhabha[3]);
  trgEcltrgArray[m_hitEneNum]->setBhabha05(vct_bhabha[4]);
  trgEcltrgArray[m_hitEneNum]->setBhabha06(vct_bhabha[5]);
  trgEcltrgArray[m_hitEneNum]->setBhabha07(vct_bhabha[6]);
  trgEcltrgArray[m_hitEneNum]->setBhabha08(vct_bhabha[7]);
  trgEcltrgArray[m_hitEneNum]->setBhabha09(vct_bhabha[8]);
  trgEcltrgArray[m_hitEneNum]->setBhabha10(vct_bhabha[9]);
  trgEcltrgArray[m_hitEneNum]->setBhabha11(vct_bhabha[10]);
  trgEcltrgArray[m_hitEneNum]->setBhabha12(vct_bhabha[11]);
  trgEcltrgArray[m_hitEneNum]->setBhabha13(vct_bhabha[12]);
  trgEcltrgArray[m_hitEneNum]->setBhabha14(vct_bhabha[13]);
  trgEcltrgArray[m_hitEneNum]->setBhabha15(vct_bhabha[14]);
  trgEcltrgArray[m_hitEneNum]->setBhabha16(vct_bhabha[15]);
  trgEcltrgArray[m_hitEneNum]->setBhabha17(vct_bhabha[16]);
  trgEcltrgArray[m_hitEneNum]->setBhabha18(vct_bhabha[17]);
  //
  trgEcltrgArray[m_hitEneNum]->setICN(icn);
  trgEcltrgArray[m_hitEneNum]->setICNFw(icnfwd);
  trgEcltrgArray[m_hitEneNum]->setICNBr(icnbr);
  trgEcltrgArray[m_hitEneNum]->setICNBw(icnbwd);
  //
  trgEcltrgArray[m_hitEneNum]->setECLtoGDL(m_Triggerbit[0], 0);
  trgEcltrgArray[m_hitEneNum]->setECLtoGDL(m_Triggerbit[1], 1);
  trgEcltrgArray[m_hitEneNum]->setECLtoGDL(m_Triggerbit[2], 2);
  trgEcltrgArray[m_hitEneNum]->setECLtoGDL(m_Triggerbit[3], 3);

  trgEcltrgArray[m_hitEneNum]->setBhabhaVeto(bhabha2D);
  trgEcltrgArray[m_hitEneNum]->setBeamBkgVeto(beambkgtag);
  trgEcltrgArray[m_hitEneNum]->setEventTiming(eventtiming);

  trgEcltrgArray[m_hitEneNum]->setHit(hit);
  trgEcltrgArray[m_hitEneNum]->setRevoclk(RevoFAM);
  trgEcltrgArray[m_hitEneNum]->setTimingSource(TimingSource);
  trgEcltrgArray[m_hitEneNum]->setPhysics(physics) ;
  trgEcltrgArray[m_hitEneNum]->set2DBhabha(bhabha2D);
  trgEcltrgArray[m_hitEneNum]->set3DBhabha(bhabha3D_veto);
  trgEcltrgArray[m_hitEneNum]->set3DBhabhaSel(bhabha3D_sel);
  trgEcltrgArray[m_hitEneNum]->setmumuBit(mumu);
  trgEcltrgArray[m_hitEneNum]->setBhabhaPrescaleBit(bhabhaprescale);

  trgEcltrgArray[m_hitEneNum]->setELow(ELow)  ;
  trgEcltrgArray[m_hitEneNum]->setEHihg(EHigh);
  trgEcltrgArray[m_hitEneNum]->setELum(ELum)  ;
  trgEcltrgArray[m_hitEneNum]->setClusterOverflow(ClusterOverflow) ;
  trgEcltrgArray[m_hitEneNum]->setLowMultiBit(m_Lowmultibit);

  trgEcltrgArray[m_hitEneNum]->set3DBhabhaVetoInTrackFlag(m_obj_bhabha->get3DBhabhaVetoInTrackFlag());
  trgEcltrgArray[m_hitEneNum]->set3DBhabhaVetoClusterTCId(m_obj_bhabha->get3DBhabhaVetoClusterTCId(0), 0);
  trgEcltrgArray[m_hitEneNum]->set3DBhabhaVetoClusterTCId(m_obj_bhabha->get3DBhabhaVetoClusterTCId(1), 1);
  trgEcltrgArray[m_hitEneNum]->set3DBhabhaVetoClusterEnergy(m_obj_bhabha->get3DBhabhaVetoClusterEnergy(0), 0);
  trgEcltrgArray[m_hitEneNum]->set3DBhabhaVetoClusterEnergy(m_obj_bhabha->get3DBhabhaVetoClusterEnergy(1), 1);
  trgEcltrgArray[m_hitEneNum]->set3DBhabhaVetoClusterTiming(m_obj_bhabha->get3DBhabhaVetoClusterTiming(0), 0);
  trgEcltrgArray[m_hitEneNum]->set3DBhabhaVetoClusterTiming(m_obj_bhabha->get3DBhabhaVetoClusterTiming(1), 1);
  trgEcltrgArray[m_hitEneNum]->set3DBhabhaVetoClusterThetaId(m_obj_bhabha->get3DBhabhaVetoClusterThetaId(0), 0);
  trgEcltrgArray[m_hitEneNum]->set3DBhabhaVetoClusterThetaId(m_obj_bhabha->get3DBhabhaVetoClusterThetaId(1), 1);

  trgEcltrgArray[m_hitEneNum]->set3DBhabhaSelectionThetaFlag(m_obj_bhabha->get3DBhabhaSelectionThetaFlag());
  trgEcltrgArray[m_hitEneNum]->set3DBhabhaSelectionClusterTCId(m_obj_bhabha->get3DBhabhaSelectionClusterTCId(0), 0);
  trgEcltrgArray[m_hitEneNum]->set3DBhabhaSelectionClusterTCId(m_obj_bhabha->get3DBhabhaSelectionClusterTCId(1), 1);
  trgEcltrgArray[m_hitEneNum]->set3DBhabhaSelectionClusterEnergy(m_obj_bhabha->get3DBhabhaSelectionClusterEnergy(0), 0);
  trgEcltrgArray[m_hitEneNum]->set3DBhabhaSelectionClusterEnergy(m_obj_bhabha->get3DBhabhaSelectionClusterEnergy(1), 1);
  trgEcltrgArray[m_hitEneNum]->set3DBhabhaSelectionClusterTiming(m_obj_bhabha->get3DBhabhaSelectionClusterTiming(0), 0);
  trgEcltrgArray[m_hitEneNum]->set3DBhabhaSelectionClusterTiming(m_obj_bhabha->get3DBhabhaSelectionClusterTiming(1), 1);
  trgEcltrgArray[m_hitEneNum]->set3DBhabhaSelectionClusterThetaId(m_obj_bhabha->get3DBhabhaSelectionClusterThetaId(0), 0);
  trgEcltrgArray[m_hitEneNum]->set3DBhabhaSelectionClusterThetaId(m_obj_bhabha->get3DBhabhaSelectionClusterThetaId(1), 1);

  trgEcltrgArray[m_hitEneNum]->setEventTimingQualityFlag(m_obj_timing->getEventTimingQualityFlag());
  trgEcltrgArray[m_hitEneNum]->setEventTimingTCId(m_obj_timing->getEventTimingTCId());
  trgEcltrgArray[m_hitEneNum]->setEventTimingTCThetaId(m_obj_timing->getEventTimingTCThetaId());
  trgEcltrgArray[m_hitEneNum]->setEventTimingTCEnergy(m_obj_timing->getEventTimingTCEnergy());

  trgEcltrgArray[m_hitEneNum]->setEtot1to17(E_total);
  trgEcltrgArray[m_hitEneNum]->setTaub2bFlag(taub2bFlag);
  trgEcltrgArray[m_hitEneNum]->setTaub2bAngleFlag(m_obj_bhabha->getTaub2bAngleFlag());
  trgEcltrgArray[m_hitEneNum]->setTaub2bEtotFlag(m_obj_bhabha->getTaub2bEtotFlag());
  trgEcltrgArray[m_hitEneNum]->setTaub2bClusterEFlag(m_obj_bhabha->getTaub2bClusterEFlag());

  trgEcltrgArray[m_hitEneNum]->setNofCluster1to17(NofCluster1to17);

  trgEcltrgArray[m_hitEneNum]->setDataClockWindowStartTime(WindowStart);

  return;
}
//========================================================
//
//========================================================
void
TrgEclMaster::setRS(std::vector<int> TCId,
                    std::vector<double> TCHit,
                    std::vector<double>& phiringsum,
                    std::vector<std::vector<double>>& thetaringsum)
{
  //
  //
  // TC Phi & Theta ring sum
  //
  //----------------------------------------
  //               FW    BR     BW   total
  //----------------------------------------
  //TC phi ring     3    12     2      17
  //ID             1-3  4-15  16-17
  //TC Theta ring   32    36    32
  //ID             1-32  1-36  1-32

  //----------------------------------------
  //
  thetaringsum.resize(3, std::vector<double>(36, 0.));
  phiringsum.resize(17);
  const int size_hit = TCHit.size();
  for (int iHit = 0; iHit < size_hit; iHit++) {
    int iTCId = TCId[iHit] - 1;
    if (TCHit[iHit] > 0) {
      int iTCThetaId = m_obj_map->getTCThetaIdFromTCId(iTCId + 1) - 1 ;
      int iTCPhiId   = m_obj_map->getTCPhiIdFromTCId(iTCId + 1) - 1 ;
      phiringsum[iTCThetaId] += TCHit[iHit];
      if (iTCThetaId < 3) {
        //fwd
        if (iTCThetaId != 0) {
          thetaringsum[0][iTCPhiId] += TCHit[iHit];
        }
      } else if (iTCThetaId < 15) {
        //barrel
        thetaringsum[1][iTCPhiId] += TCHit[iHit];
      } else {
        //bwd
        thetaringsum[2][iTCPhiId] += TCHit[iHit];
      }

    }

  }

}
//========================================================
//
//========================================================
void
TrgEclMaster::makeTriggerBit(int hit, int Timing, int RevoFAM, int TimingSource,
                             double etot, int elow, int ehigh, int elum,
                             int bhabha2D, int physics,
                             std::vector<int> bhabhatype, int ICN, int BGVeto,
                             int ClusterOverflow, int bhabha3D, int lowmultibit,
                             int bhabha3D_sel, int mumubit, int prescale, int burst,
                             int EventTimingQualityFlag, int bhabha3DVetoInTrackFlag,
                             int bhabha3DSelectionThetaFlag,
                             int taub2bFlag,
                             int bit_hie_bhav,
                             int taub2b2Flag,
                             int taub2b3Flag)
{

  m_Triggerbit[0] = 0;
  m_Triggerbit[1] = 0;
  m_Triggerbit[2] = 0;
  m_Triggerbit[3] = 0;

  int bhabhaveto = 0;
  int Bhabhatype = bhabha2D;

  if (bhabhatype.size() > 14) {
    for (int ibhabha = 0; ibhabha < 13; ibhabha++) {
      int type = 0x00;
      if (bhabhatype[ibhabha] == 1) {type = 0x01;}

      Bhabhatype |= type;
      Bhabhatype <<= 1;

    }
  }

  int bit_hit = hit & 0x01;
  int bit_Timing = (Timing & 0x7F) ;
  int bit_RevoFAM = (RevoFAM & 0x7F) ;
  int bit_TimingSource  = (TimingSource & 0x07) ;
  int bit_physics = (physics &  0x01) ;
  int bit_2Dbhabha = (bhabhaveto & 0x01) ;
  int bit_bhabhatype = (Bhabhatype & 0x3FFF);
  int bit_etot = (((int)etot) & 0x1FFF) ;
  int bit_elow = (elow & 0x01);
  int bit_ehigh = (ehigh & 0x01) ;
  int bit_elum = (elum & 0x01) ;
  int bit_ICN = (ICN & 0x7F) ;
  int bit_BGVeto = (BGVeto & 0x07) ;
  int bit_ClusterOverflow = (ClusterOverflow & 0x01);
  int bit_3Dbhabha = (bhabha3D & 0x01);

  int bit_lowmulti1 = lowmultibit & 0x0FFF;
  int bit_lowmulti2 = (lowmultibit >>= 12) & 0x3;
  int bit_3DBhabha_sel = bhabha3D_sel & 0x01;
  int bit_mumu = mumubit & 0x01;
  int bit_prescale = prescale & 0x01;
  int bit_burst = burst & 0x01;
  int bit_clkcc = 0;  // 4 bits for revo counter (set to be 0 in tsim)
  int bit_eventtimingqualityflag = EventTimingQualityFlag & 0x03;
  int bit_bhabha3dvetointrackflag = 0;
  if (bhabha3D == 1) {
    bit_bhabha3dvetointrackflag = bhabha3DVetoInTrackFlag & 0x01;
  }
  int bit_bhabha3dselectionthetaflag = 0;
  if (bhabha3D_sel == 1) {
    bit_bhabha3dselectionthetaflag = bhabha3DSelectionThetaFlag & 0x03;
  }
  int bit_taub2bflag = taub2bFlag & 0x01;
  int bit_taub2b2flag = taub2b2Flag & 0x01;
  int bit_taub2b3flag = taub2b3Flag & 0x01;

  int bit_hie_bhav_hie123 = bit_hie_bhav & 0x7;      // for hie1, hie2, hie3
  int bit_hie_bhav_hie4   = (bit_hie_bhav >> 3) & 0x1; // for hie4

  m_Triggerbit[2] |= bit_hie_bhav_hie4;
  m_Triggerbit[2] <<= 1;
  m_Triggerbit[2] |= bit_taub2b3flag;
  m_Triggerbit[2] <<= 1;
  m_Triggerbit[2] |= bit_taub2b2flag;
  m_Triggerbit[2] <<= 3;
  m_Triggerbit[2] |= bit_hie_bhav_hie123;
  m_Triggerbit[2] <<= 1;
  m_Triggerbit[2] |= bit_taub2bflag;
  m_Triggerbit[2] <<= 2;
  m_Triggerbit[2] |= bit_bhabha3dselectionthetaflag;
  m_Triggerbit[2] <<= 1;
  m_Triggerbit[2] |= bit_bhabha3dvetointrackflag;
  m_Triggerbit[2] <<= 2;
  m_Triggerbit[2] |= bit_eventtimingqualityflag;
  m_Triggerbit[2] <<= 4;
  m_Triggerbit[2] |= bit_clkcc;
  m_Triggerbit[2] <<= 2;
  m_Triggerbit[2] |= bit_lowmulti2;
  m_Triggerbit[2] <<= 1;
  m_Triggerbit[2] |= bit_burst;
  m_Triggerbit[2] <<= 1;
  m_Triggerbit[2] |= bit_prescale;
  m_Triggerbit[2] <<= 1;
  m_Triggerbit[2] |= bit_mumu;
  m_Triggerbit[2] <<= 1;
  m_Triggerbit[2] |= bit_3DBhabha_sel;
  m_Triggerbit[2] <<= 10;
  m_Triggerbit[2] |= ((bit_lowmulti1) >> 2) & 0x3FF;

  m_Triggerbit[1] |= (bit_lowmulti1 & 0x03);
  m_Triggerbit[1] <<= 1;
  m_Triggerbit[1] |= bit_3Dbhabha;
  m_Triggerbit[1] <<= 1;
  m_Triggerbit[1] |= bit_ClusterOverflow;
  m_Triggerbit[1] <<= 3;
  m_Triggerbit[1] |= bit_BGVeto;
  m_Triggerbit[1] <<= 7;
  m_Triggerbit[1] |= bit_ICN;
  m_Triggerbit[1] <<= 1;
  m_Triggerbit[1] |= bit_elum;
  m_Triggerbit[1] <<= 1;
  m_Triggerbit[1] |= bit_ehigh;
  m_Triggerbit[1] <<= 1;
  m_Triggerbit[1] |= bit_elow;
  m_Triggerbit[1] <<= 13;
  m_Triggerbit[1] |= bit_etot;
  m_Triggerbit[1] <<= 2;
  m_Triggerbit[1] |= ((bit_bhabhatype >> 12) & 0x03);

  m_Triggerbit[0] |= (bit_bhabhatype & 0x0FFF);
  m_Triggerbit[0] <<= 1;
  m_Triggerbit[0] |= bit_2Dbhabha;
  m_Triggerbit[0] <<= 1;
  m_Triggerbit[0] |= bit_physics;
  m_Triggerbit[0] <<= 3;
  m_Triggerbit[0] |= bit_TimingSource;
  m_Triggerbit[0] <<= 7;
  m_Triggerbit[0] |= bit_RevoFAM;
  m_Triggerbit[0] <<= 7;
  m_Triggerbit[0] |= bit_Timing;
  m_Triggerbit[0] <<= 1;
  m_Triggerbit[0] |= bit_hit;

}
//
//
//
void
TrgEclMaster::makeLowMultiTriggerBit(std::vector<int> CenterTCId,
                                     std::vector<double> clusterenergy,
                                     int Bhabha3DVeto)
{

  //----------------------------------------------------------------------------
  // ECL trigger
  //----------------------------------------------------------------------------
  // Variable in tsim is m_Lowmultibit
  //----------------------------------------------------------------------------
  //   N  |Address| documentation
  // (bit)|in FW  |
  //----------------------------------------------------------------------------
  //   1  |   62  |(lml0)  N Cluster  >= 3, at least one Cluster >300 MeV (LAB), not 3D ECL Bhabha
  //   1  |   63  |(lml1)  one Cluster >= 2GeV(CM) with Theta Id = 4~14
  //   1  |   64  |(lml2)  one Cluster >= 2GeV(CM) with Theta Id = 2,3,15 or 16 and not a 3D ECL Bhabha
  //   1  |   65  |(lml3)  one Cluster >= 2GeV(CM) with Theta Id = 2, 3, 15 or 16 and not a 3D ECL Bhabha
  //   1  |   66  |(lml4)  one Cluster >= 2GeV(CM) with Theta Id = 1 or 17 and not a 3D ECL Bhabha
  //   1  |   67  |(lml5)  one Cluster >= 2GeV(CM) with Theta Id = 1 or 17 and a 3D ECL Bhabha
  //   1  |   68  |(lml6)  exactly one Cluster >= 1GeV(CM) and one Cluster > 300  MeV (LAB ), in Theta Id 4~15(Barrel)
  //   1  |   69  |(lml7)  exactly one Cluster >= 1GeV(CM) and one Cluster > 300 MeV (LAB), in Theta Id 2, 3 or 16
  //   1  |   70  |(lml8)  170 < delta phi(CM) < 190 degree, both Clusters > 250 MeV (LAB), and no 2GeV (CM) Cluster
  //   1  |   71  |(lml9)  170 < delta phi(CM) < 190 degree, one Cluster < 250 MeV (LAB), the other Cluster > 250 MeV(LAB), and no 2GeV (CM) Cluster
  //   1  |   72  |(lml10) 160 < delta phi(CM) < 200 degree, 160 < Sum Theta (CM)< 200 degree, no 2 GeV(CM) cluster
  //   1  |   73  |(lml11) No 2GeV (CM) Cluster
  //   1  |   78  |(lml12) N Cluster  >= 3, at least one Cluster >500 MeV (LAB) with Theta Id 2~16, not 3D ECL Bhabha
  //   1  |   79  |(lml13) Only one Cluster >500 MeV (CM) with Theta Id 6~11 and no other CL >= 300 MeV(LAB) anywhere
  //----------------------------------------------------------------------------
  m_Lowmultibit = 0;
  int _nClust = CenterTCId.size();
  int _n300MeV = 0;
  int _n2GeV = 0;
  int _n2GeV414 = 0;
  int _n2GeV231516 = 0;
  int _n2GeV117 = 0;
  int _n1GeV415 = 0;
  int _n1GeV2316 = 0;
  int _nClust216 = 0;
  int _n500MeV216 = 0;
  int _n500MeV611 = 0;
  for (int ic = 0; ic < _nClust; ic++) {
    if (clusterenergy[ic] > m_lmlCLELabCut[1]) {
      _n300MeV++;
    }
    int thetaid = m_obj_map->getTCThetaIdFromTCId(CenterTCId[ic]);
    int lut = m_obj_database->get3DBhabhaLUT(CenterTCId[ic]);
    double thresh = 0.1 * (double)(15 & lut);
    if (thetaid >= 2 && thetaid <= 16) {_nClust216++;}

    // lml13ThetaIdBit is decimal, but indicates thetaID with binary
    // In lml13ThetaIdBit = 2016 case, (thetaID 17 to 1) = 0 0000 0111 1110 0000
    int lml13ThetaIdBit = (m_lml13ThetaIdSelection >> (thetaid - 1)) & 0x1;
    if (lml13ThetaIdBit == 1) {
      if (clusterenergy[ic] > thresh * m_lmlCLECMSCut[2]) {
        _n500MeV611++;
      }
    }
    // lml12
    if (clusterenergy[ic] > m_lmlCLELabCut[0] &&
        thetaid >=  2 &&
        thetaid <= 16) {
      _n500MeV216++;
    }

    if (clusterenergy[ic] > thresh * m_lmlCLECMSCut[0]) {   // GeV
      _n2GeV++;
      if (thetaid >= 4 && thetaid <= 14) {_n2GeV414++;}
      if (thetaid == 2 || thetaid == 3 || thetaid == 15 || thetaid == 16) {_n2GeV231516++;}
      if (thetaid == 1 || thetaid == 17) {_n2GeV117++;}
    }

    if (clusterenergy[ic] > thresh * m_lmlCLECMSCut[1]) {   // GeV
      if (thetaid >= 4 && thetaid <= 15) {_n1GeV415++;}
      if (thetaid == 2 || thetaid == 3 || thetaid == 16) {_n1GeV2316++;}
    }
  }
  //---------------------------------------------------------------------
  //..Trigger objects using back-to-back ECL clusters, plus Bhabha vetoes
  //  nPhiPairHigh nPhiPairLow n3DPair nECLBhabha nTrkBhabha

  int _nPhiPairHigh = 0;
  int _nPhiPairLow = 0;
  int _n3DPair = 0;
  for (int i0 = 0; i0 < _nClust - 1; i0++) {
    for (int i1 = i0 + 1; i1 < _nClust; i1++) {

      int lut1 = m_obj_database->get3DBhabhaLUT(CenterTCId[i0]);
      int lut2 = m_obj_database->get3DBhabhaLUT(CenterTCId[i1]);

      lut1 >>= 4;
      lut2 >>= 4;
      int phi1 = 511 & lut1;
      int phi2 = 511 & lut2;
      lut1 >>= 9;
      lut2 >>= 9;
      int theta1 = lut1;
      int theta2 = lut2;

      //..back to back in phi
      int dphi = abs(phi1 - phi2);
      if (dphi > 180) {dphi = 360 - dphi;}
      int thetaSum = theta1 + theta2;

      // lml8
      if (dphi > 170. &&
          clusterenergy[i0] > m_lmlCLELabCut[2] &&
          clusterenergy[i1] > m_lmlCLELabCut[2]) {
        _nPhiPairHigh++;
      }
      // lml9
      if (dphi > 170. &&
          ((clusterenergy[i0] < m_lmlCLELabCut[2] &&
            clusterenergy[i1] > m_lmlCLELabCut[2]) ||
           (clusterenergy[i0] > m_lmlCLELabCut[2] &&
            clusterenergy[i1] < m_lmlCLELabCut[2]))) {
        _nPhiPairLow++;
      }
      // lml10
      if (dphi > 160. &&
          thetaSum > 160. &&
          thetaSum < 200) {
        _n3DPair++;
      }
    }
  }

  int bitlml0 = 0;
  int bitlml1 = 0;
  int bitlml2 = 0;
  int bitlml3 = 0;
  int bitlml4 = 0;
  int bitlml5 = 0;
  int bitlml6 = 0;
  int bitlml7 = 0;
  int bitlml8 = 0;
  int bitlml9 = 0;
  int bitlml10 = 0;
  int bitlml11 = 0;
  int bitlml12 = 0;
  int bitlml13 = 0;

  if (_nClust >= 3 &&
      _n300MeV >= m_lml00NCLforMinE &&
      Bhabha3DVeto == 0) {
    bitlml0 = 0x01;
  }
  if (_n2GeV414 > 0) {
    bitlml1 = 0x01;
  }
  if (_n2GeV231516 && Bhabha3DVeto == 0) {
    bitlml2 = 0x01;
  }
  if (_n2GeV231516 && Bhabha3DVeto != 0) {
    bitlml3 = 0x01;
  }
  if (_n2GeV117 && Bhabha3DVeto == 0) {
    bitlml4 = 0x01;
  }
  if (_n2GeV117 && Bhabha3DVeto != 0) {
    bitlml5 = 0x01;
  }
  if (_n1GeV415 == 1 && _n300MeV == 1) {
    bitlml6 = 0x01;
  }
  if (_n1GeV2316 == 1 && _n300MeV == 1) {
    bitlml7 = 0x01;
  }
  if (_nPhiPairHigh > 0 && _n2GeV == 0) {
    bitlml8  = 0x01;
  }
  if (_nPhiPairLow > 0 && _n2GeV == 0) {
    bitlml9 = 0x01;
  }
  if (_n3DPair > 0 && _n2GeV == 0) {
    bitlml10 = 0x01;
  }
  if (_n2GeV == 0) {
    bitlml11 = 0x01;
  }
  if (_nClust216 >= 3 &&
      _n500MeV216 >= m_lml12NCLforMinE &&
      Bhabha3DVeto == 0) {
    bitlml12 = 0x01;
  }
  if (_n500MeV611 == 1 && _n300MeV == 1) {
    bitlml13 = 0x01;
  }

  int  total_bit = 0;
  total_bit |= bitlml13;
  total_bit <<= 1;
  total_bit |= bitlml12;
  total_bit <<= 1;
  total_bit |= bitlml11;
  total_bit <<= 1;
  total_bit |= bitlml10;
  total_bit <<= 1;
  total_bit |= bitlml9;
  total_bit <<= 1;
  total_bit |= bitlml8;
  total_bit <<= 1;
  total_bit |= bitlml7;
  total_bit <<= 1;
  total_bit |= bitlml6;
  total_bit <<= 1;
  total_bit |= bitlml5;
  total_bit <<= 1;
  total_bit |= bitlml4;
  total_bit <<= 1;
  total_bit |= bitlml3;
  total_bit <<= 1;
  total_bit |= bitlml2;
  total_bit <<= 1;
  total_bit |= bitlml1;
  total_bit <<= 1;
  total_bit |= bitlml0;

  m_Lowmultibit =  total_bit ;
}
//
//
//
double TrgEclMaster::setTotalEnergy(std::vector<double> phisum)
{

  double E_phys = 0;
  for (int iii = 0; iii <= 16; iii++) {
    if (iii > 0 && iii < 15) {E_phys += phisum[iii];}
  }
  return E_phys;
}
//
//
//
void TrgEclMaster::setBhabhaParameter(void)
{

  m_obj_bhabha->set2DBhabhaThreshold(m_2DBhabhaThresholdFWD,
                                     m_2DBhabhaThresholdBWD);

  m_obj_bhabha->set3DBhabhaVetoThreshold(m_3DBhabhaVetoThreshold);
  m_obj_bhabha->set3DBhabhaVetoAngle(m_3DBhabhaVetoAngle);

  m_obj_bhabha->set3DBhabhaSelectionThreshold(m_3DBhabhaSelectionThreshold);
  m_obj_bhabha->set3DBhabhaSelectionAngle(m_3DBhabhaSelectionAngle);
  m_obj_bhabha->set3DBhabhaSelectionPreScale(m_3DBhabhaSelectionPreScale);

  m_obj_bhabha->setmumuThreshold(m_mumuThreshold);
  m_obj_bhabha->setmumuAngle(m_mumuAngle);

  m_obj_bhabha->set3DBhabhaVetoInTrackThetaRegion(m_3DBhabhaVetoInTrackThetaRegion);

  m_obj_bhabha->setTaub2bAngleCut(m_taub2bAngleCut);
  m_obj_bhabha->setTaub2bEtotCut(m_taub2bEtotCut);
  m_obj_bhabha->setTaub2bCLELabCut(m_taub2bCLELabCut);

  m_obj_bhabha->sethie12BhabhaVetoAngle(m_hie12BhabhaVetoAngle);

  m_obj_bhabha->setTaub2b2Cut(m_taub2b2AngleCut,
                              m_taub2b2EtotCut,
                              m_taub2b2CLELabCut);
  m_obj_bhabha->setTaub2b3Cut(m_taub2b3AngleCut,
                              m_taub2b3EtotCut,
                              m_taub2b3CLEb2bLabCut,
                              m_taub2b3CLELabCut);

  m_obj_bhabha->sethie4LowCLELabCut(m_hie4LowCLELabCut);

}
