/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//---------------------------------------------------------------
// $Id$
//---------------------------------------------------------------
// Filename : TrgEcl.cc
// Section  : TRG ECL
// Owner    : InSoo Lee / Yuuji Unno
// Email    : islee@hep.hanyang.ac.kr / yunno@post.kek.jp
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
//    _Triggerbit[0] |      7 |  14 downto 8  | Revoclk from FAM (LSB  = 125 ns)
//    (upto 12th     |      3 |  17 downto 15 | ECL-TRG Timing Source
//    bhabha bit)    |      1 |            18 | Physics TRG
//      (32bit)      |      1 |            19 | Belle type Bhabha
// ------------------|     14 |  33 downto 20 | Bhabha Type
//                   |     13 |  46 downto 34 | Total Energy
//                   |      1 |            47 | E low (Etot >0.5 GeV)
//                   |      1 |            48 | E High (Etot > 1.0 GeV)
//    _Triggerbit[1] |      1 |            49 | E lom (Etot > 3.0 GeV)
//                   |      7 |  56 dwonto 50 | ICN
//                   |      3 |  59 downto 57 | BG veto
//                   |      1 |            60 | Cluster Overflow
//                   |      1 |            61 | 3D Bhabha Trigger for Veto
//                   |      1 |         62    | N Cluster  >= 3, at least one Cluster >300 MeV (LAB), not 3D ECL Bhabha
// _________________ |      1 |         63    | one Cluster >= 2GeV(CM) with Theta Id = 4~14
//                   |      1 |         64    | one Cluster >= 2GeV(CM) with Theta Id = 2,3,15 or 16 and not a 3D ECL Bhabha
//                   |      1 |         65    | one Cluster >= 2GeV(CM) with Theta Id = 2, 3, 15 or 16 and not a 3D ECL Bhabha
//                   |      1 |         66    | one Cluster >= 2GeV(CM) with Theta Id = 1 or 17 and not a 3D ECL Bhabha
//                   |      1 |         67    | one Cluster >= 2GeV(CM) with Theta Id = 1 or 17 and a 3D ECL Bhabha
//    _Triggerbit[2] |      1 |         68    | exactly one Cluster >= 1GeV(CM) and one Cluster > 300  MeV (LAB ), in Theta Id 4~15(Barrel)
//                   |      1 |         69    | exactly one Cluster >= 1GeV(CM) and one Cluster > 300 MeV (LAB), in Theta Id 2, 3 or 16
//                   |      1 |         70    | 170 < delta phi(CM) < 190 degree, both Clusters > 250 MeV (LAB), and no 2GeV (CM) Cluster
//                   |      1 |         71    | 170 < delta phi(CM) < 190 degree, one Cluster < 250 MeV (LAB), the other Cluster > 250 MeV(LAB), and no 2GeV (CM) Cluster
//                   |      1 |         72    | 160 < delta phi(CM) < 200 degree, 160 < Sum Theta (CM)< 200 degree, no 2 GeV(CM) cluster
//                   |      1 |         74    | 3D Bhabha Trigger for selection
//                   |      1 |         75    | mumu bit
//                   |      1 |         76    | Bhabha prescale bit
//                   |      1 |         77    | E_tot > 20 GeV
//                   |      1 |         78    | N Cluster  >= 3, at least one Cluster >500 MeV (LAB) with Theta Id 2~16, not 3D ECL Bhabha
//                   |      1 |         79    | Only one Cluster >500 MeV (CM) with Theta Id 6~11 and no other CL >= 300 MeV(LAB) anywhere
//                   |      1 |  83 downto 80 | clock counter(set to be "0000" in tsim)
//                   |      1 |  85 downto 84 | event timing quality flag
//                   |      1 |         86    | 3D Bhabha Veto Intrk
//                   |      1 |  88 downto 87 | 3D Bhabha selection theta flag
//                   |      1 |         89    | ecltaub2b for 1x1 tau process (110<delta phi(CM)<250, 130<Thata Sum(CM)<230, Etot1to17(Lab)<7GeV, E(1CL)(Lab)<1.9GeV)
// ---------------------------------------------------------------------------------
//
//---------------------------------------------------------------
// $Log$
// 2017-02-23 version 1.0
// 2019-05-13 version 2.0 for Phase 3
// 2020-05-13 version 2.1 for Bug fixing
//---------------------------------------------------------------

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
  TimeWindow(250.0), OverlapWindow(0.0), _Clustering(1), _Bhabha(0),
  _EventTiming(1), _NofTopTC(3), _ClusterLimit(6), _Lowmultibit(0),
  _PrescaleFactor(0), _PrescaleCounter(0), _mumuThreshold(20),
  _n300MeVCluster(1), _ECLBurstThreshold(200)
{

  TCEnergy.clear();
  TCTiming.clear();
  TCBeamBkgTag.clear();
  HitTCId.clear();
  TCHitEnergy.clear();
  TCHitTiming.clear();
  TCHitBeamBkgTag.clear();


  TCEnergy.resize(576);
  TCTiming.resize(576);
  TCBeamBkgTag.resize(576);

  _Triggerbit[0] = 0;
  _Triggerbit[1] = 0;
  _Triggerbit[2] = 0;
  _Triggerbit[3] = 0;


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
  _2DBhabhaThresholdFWD  = {40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 30, 35}; // /100 MeV
  _2DBhabhaThresholdBWD  = {25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 30, 30}; // /100 MeV
  _3DBhabhaVetoThreshold = {30, 45}; //  /100 MeV
  _3DBhabhaSelectionThreshold = {20, 40}; //  /100 MeV
  _3DBhabhaVetoAngle = {160, 200, 165, 190}; //  /100 MeV
  _3DBhabhaSelectionAngle = {140, 220, 160, 200}; //  /100 MeV
  _mumuAngle = {160, 200, 165, 190}; //  degree
  _LowMultiThreshold = {10, 20, 25, 30}; // degree

  m_3DBhabhaVetoInTrackThetaRegion = {3, 15};
  m_EventTimingQualityThresholds = {5, 6}; // GeV

  m_taub2bAngleCut = {110, 250, 130, 230}; // degree
  m_taub2bEtotCut = 7; // GeV
  m_taub2bClusterECut1 = 1.9; // GeV
  m_taub2bClusterECut2 = 999; // GeV

  //ThetaRingSum.resize(3,std::vector<double>(36,0));
  //PhiRingSum.resize(17,0);

  //  obf_timing = new TrgEclTiming();
  obj_cluster = new TrgEclCluster();
  obj_beambkg = new TrgEclBeamBKG();
  obj_bhabha = new TrgEclBhabha();
  obj_timing = new TrgEclTiming();
  obj_map = new TrgEclMapping();
  obj_database = new TrgEclDataBase();
}
//
//
//
TrgEclMaster::~TrgEclMaster()
{
  delete obj_cluster;
  delete obj_beambkg;
  delete obj_bhabha;
  delete obj_timing;
  delete obj_map;
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
TrgEclMaster::initialize(int)
{
}
//
//
//
TrgEclMaster* TrgEclMaster::_ecl = 0;
//
//
//
TrgEclMaster*
TrgEclMaster::getTrgEclMaster(void)
{

  if (_ecl) { delete _ecl; }
  _ecl = new TrgEclMaster();

  return _ecl;
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
  TCTiming.clear();
  TCEnergy.clear();
  TCBeamBkgTag.clear();
  TCEnergy.resize(576);
  TCTiming.resize(576);
  TCBeamBkgTag.resize(576);

  StoreArray<TRGECLHit> trgeclHitArray;
  for (int ii = 0; ii < trgeclHitArray.getEntries(); ii++) {

    TRGECLHit* aTRGECLHit = trgeclHitArray[ii];
    int iTCID = (aTRGECLHit->getTCId() - 1);
    double HitTiming  = aTRGECLHit->getTimeAve();
    double HitEnergy  = aTRGECLHit->getEnergyDep();
    double HitBeamBkg = aTRGECLHit->getBeamBkgTag();

    TCTiming[iTCID].push_back(HitTiming);
    TCEnergy[iTCID].push_back(HitEnergy);
    TCBeamBkgTag[iTCID].push_back(HitBeamBkg);
  }
  //
  //
  int nBin = 8000 / (TimeWindow / 2) ; //8000/125
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

    check_window_start = iBin * (TimeWindow / 3) + fluctuation - 4000;
    WindowStart = check_window_start;
    check_window_end  = check_window_start + TimeWindow / 3;
    WindowEnd = WindowStart + TimeWindow;
    HitTCId.clear();
    TCHitTiming.clear();
    TCHitEnergy.clear();
    TCHitBeamBkgTag.clear();

    // prepare TC Hit in time window --
    for (int iTCId = 0; iTCId < 576; iTCId++) {
      const int hitsize =  TCTiming[iTCId].size();
      for (int ihit = 0; ihit < hitsize; ihit++) {
        if (TCTiming[iTCId][ihit] > check_window_start &&
            TCTiming[iTCId][ihit] < check_window_end) {
          HitTCId.push_back(iTCId + 1);

        }
      }
    }
    if (HitTCId.size() == 0) {continue;}
    else {
      HitTCId.clear();
      for (int iTCId = 0; iTCId < 576; iTCId++) {
        const int hitsize =  TCTiming[iTCId].size();
        for (int ihit = 0; ihit < hitsize; ihit++) {
          if (TCTiming[iTCId][ihit] > WindowStart &&
              TCTiming[iTCId][ihit] < WindowEnd) {
            HitTCId.push_back(iTCId + 1);
            TCHitTiming.push_back(TCTiming[iTCId][ihit]);
            TCHitEnergy.push_back(TCEnergy[iTCId][ihit]);
          }
        }
      }
      iBin  = iBin + 2;
    }
    int noftchit = HitTCId.size();
    if (noftchit == 0) {continue;}

    double eventtiming = 0;
    // Get EventTiming
    obj_timing->Setup(HitTCId, TCHitEnergy, TCHitTiming);
    obj_timing->SetNofTopTC(_NofTopTC);
    //    obj_timing->setEventTimingQualityThresholds(m_EventTimingQualityThresholds);

    eventtiming = obj_timing->GetEventTiming(_EventTiming);
    int timingsource = obj_timing->GetTimingSource();
    int EventTimingQualityFlag = obj_timing->getEventTimingQualityFlag();
    //--------------------------------------------------
    // Ring sum and Total Energy Sum
    //-------------------------------------------------
    std::vector<std::vector<double>>  thetaringsum;
    std::vector<double>  phiringsum;

    thetaringsum.clear();
    phiringsum.clear();
    thetaringsum.resize(3, std::vector<double>(36, 0));
    phiringsum.resize(17, 0);
    setRS(HitTCId, TCHitEnergy, phiringsum, thetaringsum);

    double E_phys = 0;
    double E_total = 0;
    int E_burst = 0;
    for (int iii = 0; iii <= 16; iii++) {
      if (iii > 0 && iii < 15) {E_phys += phiringsum[iii];}
      E_total += phiringsum[iii];
    }
    if (E_total == 0) {continue;}
    int ELow, EHigh, ELum;

    if (E_total > _ECLBurstThreshold / 10) {
      E_burst = 0x01;
    }
    if (E_phys > _TotalEnergy[0] / 10) { // GeV
      ELow = 0x01;
    }
    if (E_phys > _TotalEnergy[1] / 10) { // GeV
      EHigh = 0x01;
    }
    if (E_phys > _TotalEnergy[2] / 10) { // GeV
      ELum = 0x01;
    }
    //--------------
    // Clustering
    //--------------
    //
    // TrgEclCluster obj_cluster;
    obj_cluster->setClusteringMethod(_Clustering);
    obj_cluster->setClusterLimit(_ClusterLimit);
    obj_cluster->setEventId(m_nEvent);
    obj_cluster->setICN(HitTCId, TCHitEnergy, TCHitTiming);// Clustering
    obj_cluster->setICN(HitTCId); // Belle Cluster Counting

    int icn    = obj_cluster->getICNFwBr();
    int icnfwd = obj_cluster->getICNSub(0);
    int icnbr  = obj_cluster->getICNSub(1);
    int icnbwd = obj_cluster->getICNSub(2);
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
      TVector3 clusterposition(aTRGECLCluster->getPositionX(),
                               aTRGECLCluster->getPositionY(),
                               aTRGECLCluster->getPositionZ());
      ClusterTiming.push_back(clustertiming);
      ClusterEnergy.push_back(clusterenergy);
      MaxTCId.push_back(maxTCId);
    }
    //    const int ncluster = ClusterEnergy.size();

    makeLowMultiTriggerBit(MaxTCId, ClusterEnergy);
    //--------------
    // Bhabha veto
    //--------------
    obj_bhabha->set2DBhabhaThreshold(_2DBhabhaThresholdFWD, _2DBhabhaThresholdBWD);
    obj_bhabha->set3DBhabhaSelectionThreshold(_3DBhabhaSelectionThreshold);
    obj_bhabha->set3DBhabhaVetoThreshold(_3DBhabhaVetoThreshold);
    obj_bhabha->set3DBhabhaSelectionAngle(_3DBhabhaSelectionAngle);
    obj_bhabha->set3DBhabhaVetoAngle(_3DBhabhaVetoAngle);
    obj_bhabha->setmumuThreshold(_mumuThreshold);
    obj_bhabha->setmumuAngle(_mumuAngle);
    obj_bhabha->setTaub2bAngleCut(m_taub2bAngleCut);
    obj_bhabha->setTaub2bEtotCut(m_taub2bEtotCut);
    obj_bhabha->setTaub2bClusterECut(m_taub2bClusterECut1,
                                     m_taub2bClusterECut2);

    std::vector<double> vct_bhabha;
    vct_bhabha.clear();
    int bhabha2D = 0 ;
    int bhabha3D_veto = 0 ;
    int bhabha3D_sel = 0;
    int mumu = 0;
    int bhabha3DVetoInTrackFlag = 0;
    int bhabha3DSelectionThetaFlag = 0;
    int taub2bFlag = 0;

    bool b_2Dbhabha = obj_bhabha->GetBhabha00(phiringsum);
    vct_bhabha = obj_bhabha->GetBhabhaComb();
    if (b_2Dbhabha && (icn < 4)) {bhabha2D = 1;}
    bool b_3Dbhabha =  obj_bhabha->GetBhabha01();
    if (b_3Dbhabha) {bhabha3D_veto = 1;}
    bool b_3Dbhabha_sel =  obj_bhabha->GetBhabha02();
    if (b_3Dbhabha_sel) {bhabha3D_sel = 1;}
    bool b_mumu =  obj_bhabha->Getmumu();
    if (b_mumu) {mumu = 1;}
    bhabha3DVetoInTrackFlag = obj_bhabha->get3DBhabhaVetoInTrackFlag();
    bhabha3DSelectionThetaFlag = obj_bhabha->get3DBhabhaSelectionThetaFlag();
    taub2bFlag = obj_bhabha->GetTaub2b(E_total);
    //------------------------
    // Beam Background veto (Old cosmic veto)
    //------------------------
    int beambkgtag = 0;
    beambkgtag = obj_beambkg->GetBeamBkg(thetaringsum);

    int bhabhaprescale = 0;
    if (_PrescaleFactor == _PrescaleCounter) {
      bhabhaprescale = 1;
      _PrescaleCounter = 0;
    } else if (_PrescaleFactor > _PrescaleCounter) {
      _PrescaleCounter ++;
    }
    //-------------
    // Make ECL  Trigger Bit
    //-------------
    int hit = 1; // hit or not
    int Timing = (int)(eventtiming + 0.5);
    int RevoFAM = 0;
    int TimingSource = obj_timing->GetTimingSource(); //  FWD(0), Barrel(0), Backward(0);
    int etot = (int)(E_phys * 1000 + 0.5); // total Energy in theta ID [2~15]
    int physics = 0;
    if ((etot > 1000 || icn > 3) && !(bhabha2D == 1)) {physics = 1;}
    std::vector<int> bhabhabit;
    bhabhabit.clear();
    int bhabhabitsize = vct_bhabha.size();
    for (int ibhabha = 0; ibhabha < bhabhabitsize; ibhabha++) {
      bhabhabit.push_back((int)vct_bhabha[ibhabha]);
    }
    int ClusterOverflow = obj_cluster->getNofExceedCluster();
    int flagoverflow = 0;
    if (ClusterOverflow > 0) {
      flagoverflow = 1;
    }

    makeTriggerBit(hit, Timing, 0, timingsource, E_phys,
                   bhabha2D, physics, bhabhabit, icn, beambkgtag,
                   flagoverflow, bhabha3D_veto, _Lowmultibit,
                   bhabha3D_sel, mumu, bhabhaprescale, E_burst,
                   EventTimingQualityFlag,
                   bhabha3DVetoInTrackFlag,
                   bhabha3DSelectionThetaFlag,
                   taub2bFlag);

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
    trgEcltrgArray[m_hitEneNum]->setECLtoGDL(_Triggerbit[0], 0);
    trgEcltrgArray[m_hitEneNum]->setECLtoGDL(_Triggerbit[1], 1);
    trgEcltrgArray[m_hitEneNum]->setECLtoGDL(_Triggerbit[2], 2);
    trgEcltrgArray[m_hitEneNum]->setECLtoGDL(_Triggerbit[3], 3);

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
    trgEcltrgArray[m_hitEneNum]->setLowMultiBit(_Lowmultibit);
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
  TCTiming.clear();
  TCEnergy.clear();
  TCBeamBkgTag.clear();
  TCEnergy.resize(576);
  TCTiming.resize(576);
  TCBeamBkgTag.resize(576);

  StoreArray<TRGECLHit> trgeclHitArray;
  for (int ii = 0; ii < trgeclHitArray.getEntries(); ii++) {

    TRGECLHit* aTRGECLHit = trgeclHitArray[ii];
    int iTCID = (aTRGECLHit->getTCId() - 1);
    double HitTiming  = aTRGECLHit->getTimeAve();
    double HitEnergy  = aTRGECLHit->getEnergyDep();
    double HitBeamBkg = aTRGECLHit->getBeamBkgTag();

    TCTiming[iTCID].push_back(HitTiming);
    TCEnergy[iTCID].push_back(HitEnergy);
    TCBeamBkgTag[iTCID].push_back(HitBeamBkg);
  }
  //
  //
  int nBin = 2 * 8000 / TimeWindow ;
  double WindowStart = 0;
  double WindowEnd = 0;
  double fluctuation = ((gRandom->Uniform(-1, 0))) * 125;

  int startBin = nBin / 2 - 1; //start previous bin near 0s

  int endBin = nBin / 2 + 1; //start next bin near 0s

  if (_EventTiming == 0) {
    TimeWindow = 500;
    OverlapWindow = 0;
  }

  double maxE = 0;
  int max_bin = 0;

  for (int iBin = startBin ; iBin <= endBin; iBin ++) {
    WindowStart = iBin * (TimeWindow - OverlapWindow) + fluctuation - 4000;
    if (iBin == 0) {WindowStart = - 4000 + fluctuation;}
    WindowEnd = WindowStart + TimeWindow;

    double totalE = 0;
    // prepare TC Hit in time window --
    for (int iTCId = 0; iTCId < 576; iTCId++) {
      const int hitsize =  TCTiming[iTCId].size();
      for (int ihit = 0; ihit < hitsize; ihit++) {
        if (TCTiming[iTCId][ihit] > WindowStart &&
            TCTiming[iTCId][ihit] < WindowEnd) {
          totalE += TCEnergy[iTCId][ihit] ;
        }
      }
    }
    if (totalE == 0) {continue;}
    if (maxE < totalE) { //select the bin having the highest total energy
      maxE = totalE;
      max_bin = iBin;
    }
  }

  WindowStart = max_bin * (TimeWindow - OverlapWindow) + fluctuation - 4000;
  if (max_bin == 0) {WindowStart = - 4000 + fluctuation;}
  WindowEnd = WindowStart + TimeWindow;

  HitTCId.clear();
  TCHitTiming.clear();
  TCHitEnergy.clear();
  TCHitBeamBkgTag.clear();
  // prepare TC Hit in time window --
  for (int iTCId = 0; iTCId < 576; iTCId++) {
    const int hitsize =  TCTiming[iTCId].size();
    for (int ihit = 0; ihit < hitsize; ihit++) {
      if (TCTiming[iTCId][ihit] > WindowStart &&
          TCTiming[iTCId][ihit] < WindowEnd) {
        HitTCId.push_back(iTCId + 1);
        TCHitTiming.push_back(TCTiming[iTCId][ihit]);
        TCHitEnergy.push_back(TCEnergy[iTCId][ihit]);
        TCHitBeamBkgTag.push_back(TCBeamBkgTag[iTCId][ihit]);
      }
    }
  }

  int noftchit = HitTCId.size();
  if (noftchit == 0) { return;}

  double eventtiming = 0;
  // Get EventTiming
  obj_timing->Setup(HitTCId, TCHitEnergy, TCHitTiming);
  obj_timing->SetNofTopTC(_NofTopTC);
  obj_timing->setEventTimingQualityThresholds(m_EventTimingQualityThresholds);

  eventtiming = obj_timing->GetEventTiming(_EventTiming);
  int timingsource = obj_timing->GetTimingSource();

  int EventTimingQualityFlag = obj_timing->getEventTimingQualityFlag();
  //--------------------------------------------------
  // Ring sum and Total Energy Sum
  //-------------------------------------------------
  std::vector<std::vector<double>>  thetaringsum;
  std::vector<double>  phiringsum;

  thetaringsum.clear();
  phiringsum.clear();
  thetaringsum.resize(3, std::vector<double>(36, 0));
  phiringsum.resize(17, 0);
  setRS(HitTCId, TCHitEnergy, phiringsum, thetaringsum);

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
  int ELow, EHigh, ELum;
  if (E_total > _ECLBurstThreshold) {
    E_burst = 0x01;
  }

  if (E_phys > _TotalEnergy[0] / 10) { // GeV
    ELow = 0x01;
  }
  if (E_phys > _TotalEnergy[1] / 10) { // GeV
    EHigh = 0x01;
  }
  if (E_phys > _TotalEnergy[2] / 10) { // GeV
    ELum = 0x01;
  }
  //--------------
  // Clustering
  //--------------
  //
  // TrgEclCluster obj_cluster;
  obj_cluster->setClusteringMethod(_Clustering);
  obj_cluster->setClusterLimit(_ClusterLimit);
  obj_cluster->setEventId(m_nEvent);
  obj_cluster->setICN(HitTCId, TCHitEnergy, TCHitTiming);// Clustering
  obj_cluster->setICN(HitTCId); // Belle Cluster Counting

  int icn    = obj_cluster->getICNFwBr();
  int icnfwd = obj_cluster->getICNSub(0);
  int icnbr  = obj_cluster->getICNSub(1);
  int icnbwd = obj_cluster->getICNSub(2);

  int NofCluster1to17 =  obj_cluster->getNofCluster();
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
    TVector3 clusterposition(aTRGECLCluster->getPositionX(),
                             aTRGECLCluster->getPositionY(),
                             aTRGECLCluster->getPositionZ());
    ClusterTiming.push_back(clustertiming);
    ClusterEnergy.push_back(clusterenergy);
    MaxTCId.push_back(maxTCId);
  }
  //    int NofCluster =  obj_cluster->getNofCluster();
  makeLowMultiTriggerBit(MaxTCId, ClusterEnergy);
  //--------------
  // Bhabha veto (and mumu and tau b2b trigger)
  //--------------
  obj_bhabha->set2DBhabhaThreshold(_2DBhabhaThresholdFWD,
                                   _2DBhabhaThresholdBWD);
  obj_bhabha->set3DBhabhaSelectionThreshold(_3DBhabhaSelectionThreshold);
  obj_bhabha->set3DBhabhaVetoThreshold(_3DBhabhaVetoThreshold);
  obj_bhabha->set3DBhabhaSelectionAngle(_3DBhabhaSelectionAngle);
  obj_bhabha->set3DBhabhaVetoAngle(_3DBhabhaVetoAngle);
  obj_bhabha->setmumuThreshold(_mumuThreshold);
  obj_bhabha->setmumuAngle(_mumuAngle);
  obj_bhabha->set3DBhabhaVetoInTrackThetaRegion(m_3DBhabhaVetoInTrackThetaRegion);
  obj_bhabha->setTaub2bAngleCut(m_taub2bAngleCut);
  obj_bhabha->setTaub2bEtotCut(m_taub2bEtotCut);
  obj_bhabha->setTaub2bClusterECut(m_taub2bClusterECut1,
                                   m_taub2bClusterECut2);

  std::vector<double> vct_bhabha;
  vct_bhabha.clear();
  int bhabha2D = 0 ;
  int bhabha3D_veto = 0 ;
  int bhabha3D_sel = 0;
  int mumu = 0;
  int bhabha3DVetoInTrackFlag = -1;
  int bhabha3DSelectionThetaFlag = -1;
  int taub2bFlag = 0;

  bool b_2Dbhabha = obj_bhabha->GetBhabha00(phiringsum);
  vct_bhabha = obj_bhabha->GetBhabhaComb();
  if (b_2Dbhabha && (icn < 4)) {bhabha2D = 1;}
  bool b_3Dbhabha =  obj_bhabha->GetBhabha01();
  if (b_3Dbhabha) {bhabha3D_veto = 1;}
  bool b_3Dbhabha_sel =  obj_bhabha->GetBhabha02();
  if (b_3Dbhabha_sel) {bhabha3D_sel = 1;}
  bool b_mumu =  obj_bhabha->Getmumu();
  if (b_mumu) {mumu = 1;}
  int bhabhaprescale = 0;
  if (_PrescaleFactor == _PrescaleCounter) {
    bhabhaprescale = 1;
    _PrescaleCounter = 0;
  } else if (_PrescaleFactor > _PrescaleCounter) {
    _PrescaleCounter ++;
  }
  bhabha3DVetoInTrackFlag    = obj_bhabha->get3DBhabhaVetoInTrackFlag();
  bhabha3DSelectionThetaFlag = obj_bhabha->get3DBhabhaSelectionThetaFlag();
  taub2bFlag = (obj_bhabha->GetTaub2b(E_total)) ? 1 : 0;
  //------------------------
  // Beam Background veto
  //------------------------
  int beambkgtag = 0;
  beambkgtag = obj_beambkg->GetBeamBkg(thetaringsum);

  //-------------
  // Make ECL  Trigger Bit
  //-------------
  int hit = 1; // hit or not
  int Timing = (int)(eventtiming + 0.5);
  int RevoFAM = 0;
  int TimingSource = obj_timing->GetTimingSource(); //  FWD(0), Barrel(0), Backward(0);
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
  int ClusterOverflow = obj_cluster->getNofExceedCluster();
  int flagoverflow = 0;
  if (ClusterOverflow > 0) {
    flagoverflow = 1;
  }

  makeTriggerBit(hit, Timing, 0, timingsource, E_phys, bhabha2D,
                 physics, bhabhabit, icn, beambkgtag, flagoverflow,
                 bhabha3D_veto, _Lowmultibit, bhabha3D_sel, mumu,
                 bhabhaprescale, E_burst,
                 EventTimingQualityFlag,
                 bhabha3DVetoInTrackFlag,
                 bhabha3DSelectionThetaFlag,
                 taub2bFlag);
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
  //
  //  printf("bit = %i \n", bit);
  //----------------------
  //   if (0){ // check bit by "binary" output
  //     int xxx = bit;
  //     int yyy = 0;
  //     int iii = 0;
  //     int ans = 0;
  //     while (xxx > 0) {
  //       yyy = xxx % 2;
  //       ans = ans + yyy * pow(10,iii);
  //       xxx = xxx / 2;
  //       iii = iii++;
  //     }
  //     printf("xxx = %i \n", ans);
  //   }


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
  trgEcltrgArray[m_hitEneNum]->setECLtoGDL(_Triggerbit[0], 0);
  trgEcltrgArray[m_hitEneNum]->setECLtoGDL(_Triggerbit[1], 1);
  trgEcltrgArray[m_hitEneNum]->setECLtoGDL(_Triggerbit[2], 2);
  trgEcltrgArray[m_hitEneNum]->setECLtoGDL(_Triggerbit[3], 3);

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
  trgEcltrgArray[m_hitEneNum]->setLowMultiBit(_Lowmultibit);

  trgEcltrgArray[m_hitEneNum]->set3DBhabhaVetoInTrackFlag(obj_bhabha->get3DBhabhaVetoInTrackFlag());
  trgEcltrgArray[m_hitEneNum]->set3DBhabhaVetoClusterTCId(obj_bhabha->get3DBhabhaVetoClusterTCId(0), 0);
  trgEcltrgArray[m_hitEneNum]->set3DBhabhaVetoClusterTCId(obj_bhabha->get3DBhabhaVetoClusterTCId(1), 1);
  trgEcltrgArray[m_hitEneNum]->set3DBhabhaVetoClusterEnergy(obj_bhabha->get3DBhabhaVetoClusterEnergy(0), 0);
  trgEcltrgArray[m_hitEneNum]->set3DBhabhaVetoClusterEnergy(obj_bhabha->get3DBhabhaVetoClusterEnergy(1), 1);
  trgEcltrgArray[m_hitEneNum]->set3DBhabhaVetoClusterTiming(obj_bhabha->get3DBhabhaVetoClusterTiming(0), 0);
  trgEcltrgArray[m_hitEneNum]->set3DBhabhaVetoClusterTiming(obj_bhabha->get3DBhabhaVetoClusterTiming(1), 1);
  trgEcltrgArray[m_hitEneNum]->set3DBhabhaVetoClusterThetaId(obj_bhabha->get3DBhabhaVetoClusterThetaId(0), 0);
  trgEcltrgArray[m_hitEneNum]->set3DBhabhaVetoClusterThetaId(obj_bhabha->get3DBhabhaVetoClusterThetaId(1), 1);

  trgEcltrgArray[m_hitEneNum]->set3DBhabhaSelectionThetaFlag(obj_bhabha->get3DBhabhaSelectionThetaFlag());
  trgEcltrgArray[m_hitEneNum]->set3DBhabhaSelectionClusterTCId(obj_bhabha->get3DBhabhaSelectionClusterTCId(0), 0);
  trgEcltrgArray[m_hitEneNum]->set3DBhabhaSelectionClusterTCId(obj_bhabha->get3DBhabhaSelectionClusterTCId(1), 1);
  trgEcltrgArray[m_hitEneNum]->set3DBhabhaSelectionClusterEnergy(obj_bhabha->get3DBhabhaSelectionClusterEnergy(0), 0);
  trgEcltrgArray[m_hitEneNum]->set3DBhabhaSelectionClusterEnergy(obj_bhabha->get3DBhabhaSelectionClusterEnergy(1), 1);
  trgEcltrgArray[m_hitEneNum]->set3DBhabhaSelectionClusterTiming(obj_bhabha->get3DBhabhaSelectionClusterTiming(0), 0);
  trgEcltrgArray[m_hitEneNum]->set3DBhabhaSelectionClusterTiming(obj_bhabha->get3DBhabhaSelectionClusterTiming(1), 1);
  trgEcltrgArray[m_hitEneNum]->set3DBhabhaSelectionClusterThetaId(obj_bhabha->get3DBhabhaSelectionClusterThetaId(0), 0);
  trgEcltrgArray[m_hitEneNum]->set3DBhabhaSelectionClusterThetaId(obj_bhabha->get3DBhabhaSelectionClusterThetaId(1), 1);

  trgEcltrgArray[m_hitEneNum]->setEventTimingQualityFlag(obj_timing->getEventTimingQualityFlag());
  trgEcltrgArray[m_hitEneNum]->setEventTimingTCId(obj_timing->getEventTimingTCId());
  trgEcltrgArray[m_hitEneNum]->setEventTimingTCThetaId(obj_timing->getEventTimingTCThetaId());
  trgEcltrgArray[m_hitEneNum]->setEventTimingTCEnergy(obj_timing->getEventTimingTCEnergy());

  trgEcltrgArray[m_hitEneNum]->setEtot1to17(E_total);
  trgEcltrgArray[m_hitEneNum]->setTaub2bFlag(taub2bFlag);
  trgEcltrgArray[m_hitEneNum]->setTaub2bAngleFlag(obj_bhabha->getTaub2bAngleFlag());
  trgEcltrgArray[m_hitEneNum]->setTaub2bEtotFlag(obj_bhabha->getTaub2bEtotFlag());
  trgEcltrgArray[m_hitEneNum]->setTaub2bClusterEFlag(obj_bhabha->getTaub2bClusterEFlag());

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
      int iTCThetaId = obj_map->getTCThetaIdFromTCId(iTCId + 1) - 1 ;
      int iTCPhiId   = obj_map->getTCPhiIdFromTCId(iTCId + 1) - 1 ;
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
                             double etot, int bhabha2D, int physics,
                             std::vector<int> bhabhatype, int ICN, int BGVeto,
                             int ClusterOverflow, int bhabha3D, int lowmultibit,
                             int bhabha3D_sel, int mumubit, int prescale, int burst,
                             int EventTimingQualityFlag, int bhabha3DVetoInTrackFlag,
                             int bhabha3DSelectionThetaFlag,
                             int taub2bFlag)
{

  _Triggerbit[0] = 0;
  _Triggerbit[1] = 0;
  _Triggerbit[2] = 0;
  _Triggerbit[3] = 0;

  //  int physics = 0;
  int elow = 0;
  int ehigh = 0;
  int elum = 0;
  int bhabhaveto = 0;
  int Bhabhatype = bhabha2D;

  if (etot > 0.5) {
    elow = 0x01;
  }
  if (etot > 1.0) {
    ehigh = 0x01;
  }
  if (etot > 3.0) {
    elum = 0x01;
  }

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

  _Triggerbit[2] |= bit_taub2bflag;
  _Triggerbit[2] <<= 2;
  _Triggerbit[2] |= bit_bhabha3dselectionthetaflag;
  _Triggerbit[2] <<= 1;
  _Triggerbit[2] |= bit_bhabha3dvetointrackflag;
  _Triggerbit[2] <<= 2;
  _Triggerbit[2] |= bit_eventtimingqualityflag;
  _Triggerbit[2] <<= 4;
  _Triggerbit[2] |= bit_clkcc;
  _Triggerbit[2] <<= 2;
  _Triggerbit[2] |= bit_lowmulti2;
  _Triggerbit[2] <<= 1;
  _Triggerbit[2] |= bit_burst;
  _Triggerbit[2] <<= 1;
  _Triggerbit[2] |= bit_prescale;
  _Triggerbit[2] <<= 1;
  _Triggerbit[2] |= bit_mumu;
  _Triggerbit[2] <<= 1;
  _Triggerbit[2] |= bit_3DBhabha_sel;
  _Triggerbit[2] <<= 10;
  _Triggerbit[2] |= ((bit_lowmulti1) >> 2) & 0x3FF;

  _Triggerbit[1] |= (bit_lowmulti1 & 0x03);
  _Triggerbit[1] <<= 1;
  _Triggerbit[1] |= bit_3Dbhabha;
  _Triggerbit[1] <<= 1;
  _Triggerbit[1] |= bit_ClusterOverflow;
  _Triggerbit[1] <<= 3;
  _Triggerbit[1] |= bit_BGVeto;
  _Triggerbit[1] <<= 7;
  _Triggerbit[1] |= bit_ICN;
  _Triggerbit[1] <<= 1;
  _Triggerbit[1] |= bit_elum;
  _Triggerbit[1] <<= 1;
  _Triggerbit[1] |= bit_ehigh;
  _Triggerbit[1] <<= 1;
  _Triggerbit[1] |= bit_elow;
  _Triggerbit[1] <<= 13;
  _Triggerbit[1] |= bit_etot;
  _Triggerbit[1] <<= 2;
  _Triggerbit[1] |= ((bit_bhabhatype >> 12) & 0x03);

  _Triggerbit[0] |= (bit_bhabhatype & 0x0FFF);
  _Triggerbit[0] <<= 1;
  _Triggerbit[0] |= bit_2Dbhabha;
  _Triggerbit[0] <<= 1;
  _Triggerbit[0] |= bit_physics;
  _Triggerbit[0] <<= 3;
  _Triggerbit[0] |= bit_TimingSource;
  _Triggerbit[0] <<= 7;
  _Triggerbit[0] |= bit_RevoFAM;
  _Triggerbit[0] <<= 7;
  _Triggerbit[0] |= bit_Timing;
  _Triggerbit[0] <<= 1;
  _Triggerbit[0] |= bit_hit;

  // int tmp = (_Triggerbit[2] >> 24) & 0x03;
  // printf("%5i %5i %i\n", bit_bhabha3dselectionthetaflag, tmp, _Triggerbit[2]);
}
//
//
//
void
TrgEclMaster::makeLowMultiTriggerBit(std::vector<int> CenterTCId,
                                     std::vector<double> clusterenergy)
{

  //---------------------------------------------------------------------------------
  // ECL trigger
  //---------------------------------------------------------------------------------
  //Variable(in Tsim) | N(bit) |   Address     | Parameter
  //-------------------------------------- ------------------------------------------
  //                  |      1 |         62    |  N Cluster  >= 3, at least one Cluster >300 MeV (LAB), not 3D ECL Bhabha
  //                  |      1 |         63    | one Cluster >= 2GeV(CM) with Theta Id = 4~14
  //  _Lowmultibit    |      1 |         64    | one Cluster >= 2GeV(CM) with Theta Id = 2,3,15 or 16 and not a 3D ECL Bhabha
  //                  |      1 |         65    | one Cluster >= 2GeV(CM) with Theta Id = 2, 3, 15 or 16 and not a 3D ECL Bhabha
  //                  |      1 |         66    | one Cluster >= 2GeV(CM) with Theta Id = 1 or 17 and not a 3D ECL Bhabha
  //                  |      1 |         67    | one Cluster >= 2GeV(CM) with Theta Id = 1 or 17 and a 3D ECL Bhabha
  //                  |      1 |         68    | exactly one Cluster >= 1GeV(CM) and one Cluster > 300  MeV (LAB ), in Theta Id 4~15(Barrel)
  //                  |      1 |         69    | exactly one Cluster >= 1GeV(CM) and one Cluster > 300 MeV (LAB), in Theta Id 2, 3 or 16
  //                  |      1 |         70    | 170 < delta phi(CM) < 190 degree, both Clusters > 250 MeV (LAB), and no 2GeV (CM) Cluster
  //                  |      1 |         71    | 170 < delta phi(CM) < 190 degree, one Cluster < 250 MeV (LAB), the other Cluster > 250 MeV(LAB), and no 2GeV (CM) Cluster
  //                  |      1 |         72    | 160 < delta phi(CM) < 200 degree, 160 < Sum Theta (CM)< 200 degree, no 2 GeV(CM) cluster
  //                  |      1 |         73    | No 2GeV (CM) Cluster
  //---------------------------------------------------------------------------------
  _Lowmultibit = 0;
  int _nClust = CenterTCId.size();
  int _n300MeV = 0;
  int _n2GeV = 0;
  int _n2GeV414 = 0;
  int _n2GeV231516 = 0;
  int _n2GeV117 = 0;
  int _n1GeV415 = 0;
  int _n1GeV2316 = 0;
  int _n1GeV117 = 0;
  int _nClust216 = 0;
  int _n500MeV216 = 0;
  int _n500MeV611 = 0;
  for (int ic = 0; ic < _nClust; ic++) {
    if (clusterenergy[ic] > 0.3) {_n300MeV++;}
    int thetaid = obj_map->getTCThetaIdFromTCId(CenterTCId[ic]);
    int lut = obj_database->Get3DBhabhaLUT(CenterTCId[ic]);
    int thresh = 15 & lut;
    if (thetaid >= 2 && thetaid <= 16) {_nClust216++;}
    if (thetaid >= 6 && thetaid <= 11) {
      if (clusterenergy[ic] * 100 > 5 * thresh) {
        _n500MeV611++;
      }
    }

    if (clusterenergy[ic] > 0.5 && thetaid >= 2 && thetaid <= 16) {_n500MeV216++;}
    if (clusterenergy[ic] * 100 > (thresh * _LowMultiThreshold[1])) { //200 <MeV
      _n2GeV++;
      if (thetaid >= 4 && thetaid <= 14) {_n2GeV414++;}
      if (thetaid == 2 || thetaid == 3 || thetaid == 15 || thetaid == 16) {_n2GeV231516++;}
      if (thetaid == 1 || thetaid == 17) {_n2GeV117++;}
    }
    if (clusterenergy[ic] * 100 > thresh * _LowMultiThreshold[0]) { // 100 MeV
      if (thetaid >= 4 && thetaid <= 15) {_n1GeV415++;}
      if (thetaid == 2 || thetaid == 3 || thetaid == 16) {_n1GeV2316++;}
      if (thetaid == 1 || thetaid == 17) {_n1GeV117++;}
    }
  }
  //---------------------------------------------------------------------
  //..Trigger objects using back-to-back ECL clusters, plus Bhabha vetoes
  //  nPhiPairHigh nPhiPairLow n3DPair nECLBhabha nTrkBhabha

  int _nPhiPairHigh = 0;
  int _nPhiPairLow = 0;
  int _n3DPair = 0;
  int _nECLBhabha = 0;
  for (int i0 = 0; i0 < _nClust - 1; i0++) {
    for (int i1 = i0 + 1; i1 < _nClust; i1++) {
      int lut1 = obj_database->Get3DBhabhaLUT(CenterTCId[i0]);
      int lut2 = obj_database->Get3DBhabhaLUT(CenterTCId[i1]);

      int energy1 = 15 & lut1;
      int energy2 = 15 & lut2;
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

      // cout << dphi << " "  << thetaSum << endl;
      // cout << clusterenergy[i0] << " " << clusterenergy[i1] << endl;
      //  if (dphi > 180.) {dphi = 360 - dphi;}
      if (dphi > 170. && clusterenergy[i0] > _LowMultiThreshold[2] / 100
          && clusterenergy[i1] >  _LowMultiThreshold[2] / 100) {_nPhiPairHigh++;}
      if (dphi > 170. &&
          ((clusterenergy[i0] <  _LowMultiThreshold[2] / 100 &&
            clusterenergy[i1] >  _LowMultiThreshold[2] / 100) ||
           (clusterenergy[i0] >  _LowMultiThreshold[2] / 100 &&
            clusterenergy[i1] <  _LowMultiThreshold[2] / 100))) {_nPhiPairLow++;}
      //..3D
      if (dphi > 160. && thetaSum > 160. && thetaSum < 200) {_n3DPair++;}
      //..ecl Bhabha
      if (dphi > 160 &&
          thetaSum > 165 &&
          thetaSum < 190 &&
          clusterenergy[i0] * 100 > _3DBhabhaVetoThreshold[0] * energy1 &&
          clusterenergy[i1] * 100 > _3DBhabhaVetoThreshold[0]  * energy2 &&
          (clusterenergy[i0] * 100 > _3DBhabhaVetoThreshold[1]  * energy1 ||
           clusterenergy[i1] * 100 > _3DBhabhaVetoThreshold[1]  * energy2)) {
        _nECLBhabha++;

      }
    }
  }
  int bit1 = 0;
  int bit2 = 0;
  int bit3 = 0;
  int bit4 = 0;
  int bit5 = 0;
  int bit6 = 0;
  int bit7 = 0;
  int bit8 = 0;
  int bit9 = 0;
  int bit10 = 0;
  int bit11 = 0;
  int bit12 = 0;
  int bit13 = 0;
  int bit14 = 0;


  if (_nClust >= 3 && _n300MeV >= _n300MeVCluster && _nECLBhabha == 0) {
    bit1 = 0x01; //6
  }
  if (_n2GeV414 > 0) {
    bit2 = 0x01; //7
  }
  if (_n2GeV231516 && _nECLBhabha == 0) {
    bit3 = 0x01; //9
  }
  if (_n2GeV231516 && _nECLBhabha != 0) {
    bit4 = 0x01; //10
  }
  if (_n2GeV117 && _nECLBhabha == 0) {
    bit5 = 0x01; //11
  }
  if (_n2GeV117 && _nECLBhabha != 0) {
    bit6 = 0x01; //12
  }
  if (_n1GeV415 == 1 && _n300MeV == 1) {
    bit7 = 0x01; //13
  }
  if (_n1GeV2316 == 1 && _n300MeV == 1) {
    bit8 = 0x01; //14
  }
  if (_nPhiPairHigh > 0 && _n2GeV == 0) {
    bit9  = 0x01; //15
  }
  if (_nPhiPairLow > 0 && _n2GeV == 0) {
    bit10 = 0x01; //16
  }
  if (_n3DPair > 0 && _n2GeV == 0) {
    bit11 = 0x01; //17;
  }
  if (_n2GeV == 0) {
    bit12 = 0x01; //4
  }
  if (_nClust216 >= 3 && _n500MeV216 >  0 && _nECLBhabha == 0) {
    bit13 = 0x01; //6
  }
  if (_n500MeV611 == 1 && _n300MeV == 1) {
    bit14 = 0x01; //6
  }

  int  total_bit = 0;
  total_bit |= bit14;
  total_bit <<= 1;
  total_bit |= bit13;
  total_bit <<= 1;
  total_bit |= bit12;
  total_bit <<= 1;
  total_bit |= bit11;
  total_bit <<= 1;
  total_bit |= bit10;
  total_bit <<= 1;
  total_bit |= bit9;
  total_bit <<= 1;
  total_bit |= bit8;
  total_bit <<= 1;
  total_bit |= bit7;
  total_bit <<= 1;
  total_bit |= bit6;
  total_bit <<= 1;
  total_bit |= bit5;
  total_bit <<= 1;
  total_bit |= bit4;
  total_bit <<= 1;
  total_bit |= bit3;
  total_bit <<= 1;
  total_bit |= bit2;
  total_bit <<= 1;
  total_bit |= bit1;

  _Lowmultibit =  total_bit ;
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
