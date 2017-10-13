//---------------------------------------------------------------
// $Id$
//---------------------------------------------------------------
// Filename : TrgEcl.cc
// Section  : TRG ECL
// Owner    : InSoo Lee / Yuuji Unno
// Email    : islee@hep.hanyang.ac.kr / yunno@post.kek.jp
//---------------------------------------------------------------
// Description : A class to represent TRG ECL
//---------------------------------------------------------------
// $Log$
// 2017-02-23 version 1.0
//---------------------------------------------------------------

#define TRG_SHORT_NAMES
#define TRGECL_SHORT_NAMES

//#include <cstdlib>
#include "framework/datastore/StoreArray.h"
//#include "trg/trg/Debug.h"
//#include "trg/trg/Time.h"
//#include "trg/trg/Signal.h"
//#include "trg/trg/Link.h"
//#include "trg/trg/Utilities.h"
#include <framework/logging/Logger.h>
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
  TimeWindow(375.0), OverlapWindow(0.0), _Clustering(0), _Bhabha(0), _EventTiming(1), _NofTopTC(3)
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





  //ThetaRingSum.resize(3,std::vector<double>(36,0));
  //PhiRingSum.resize(17,0);

  //  obf_timing = new TrgEclTiming();
  obj_cluster = new TrgEclCluster();
  obj_beambkg = new TrgEclBeamBKG();
  obj_bhabha = new TrgEclBhabha();
  obj_timing = new TrgEclTiming();
  obj_map = new TrgEclMapping();


}
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
  return std::string("TrgEclMaster 1.00");
}
//
//
//
void
//iwasaki TrgEclMaster::initialize(int m_nEvent)
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

  //  if (! _ecl){
  //    std::cout << "TrgEclMaster::getTrgEcl !!! TrgEcl is not created yet" << std::endl;
  //  }

  if (_ecl) { delete _ecl; }
  _ecl = new TrgEclMaster();

  return _ecl;

}
void
TrgEclMaster::simulate01(int m_nEvent)
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
    double HitTiming    = aTRGECLHit ->getTimeAve();
    double HitEnergy =  aTRGECLHit -> getEnergyDep();
    double HitBeamBkg =  aTRGECLHit -> getBeamBkgTag();

    TCTiming[iTCID].push_back(HitTiming);
    TCEnergy[iTCID].push_back(HitEnergy);
    TCBeamBkgTag[iTCID].push_back(HitBeamBkg);

  }
  //
  //
  int nBin = 8000 / (TimeWindow / 3) ; //8000/125
  double WindowStart = 0;
  double WindowEnd = 0;
  double fluctuation = ((gRandom ->Uniform(-1, 0))) * 125;

  double check_window_start = 0;
  double check_window_end = 0;


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
        if (TCTiming[iTCId][ihit] > check_window_start && TCTiming[iTCId][ihit] < check_window_end) {
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
          if (TCTiming[iTCId][ihit] > WindowStart && TCTiming[iTCId][ihit] < WindowEnd) {
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
    obj_timing -> Setup(HitTCId, TCHitEnergy, TCHitTiming);
    obj_timing -> SetNofTopTC(_NofTopTC);
    eventtiming = obj_timing -> GetEventTiming(_EventTiming);

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

    double E_br = 0;
    double E_fwd = 0;
    double E_bwd = 0;
    double E_phys = 0;
    double E_total = 0;
    for (int iii = 0; iii <= 16; iii++) {
      if (iii > 1 && iii < 15) {E_phys += phiringsum[iii];}
      if (iii > 0 && iii < 3) {E_fwd += phiringsum[iii];}
      if (iii > 2 && iii < 15) {E_br += phiringsum[iii];}
      if (iii > 14) {E_bwd += phiringsum[iii];}
      E_total += phiringsum[iii];
    }
    if (E_total == 0) {continue;}



    //--------------
    // Clustering
    //--------------
    //
    // TrgEclCluster obj_cluster;
    obj_cluster->setClusteringMethod(_Clustering);
    obj_cluster->setEventId(m_nEvent);
    obj_cluster->setICN(HitTCId, TCHitEnergy, TCHitTiming);

    int icn = obj_cluster->getICNFwBr();
    int icnfwd  = obj_cluster->getICNSub(0);
    int icnbr   = obj_cluster->getICNSub(1);
    int icnbwd = obj_cluster->getICNSub(2);

    //    int NofCluster =  obj_cluster->getNofCluster();

    //--------------
    // Bhabha veto
    //--------------
    //
    //    obj_bhabha -> setup()
    std::vector<double> vct_bhabha;
    vct_bhabha.clear();
    bool boolBtoBTag  = false;
    if (_Bhabha == 0) { //belle I
      boolBtoBTag  =  obj_bhabha -> GetBhabha00(phiringsum);
      vct_bhabha = obj_bhabha -> GetBhabhaComb();
    } else if (_Bhabha == 1) {
      vct_bhabha.resize(18, 0);
      boolBtoBTag  =  obj_bhabha -> GetBhabha01();
    }
    int BtoBTag = 0;
    if (boolBtoBTag && icn < 4) {
      BtoBTag = 1;
    }
    //------------------------
    // Beam Background veto (Old cosmic veto)
    //------------------------
    bool boolBeamBkgTag = false;
    int beambkgtag = 0;
    boolBeamBkgTag =   obj_beambkg -> GetBeamBkg(thetaringsum);
    if (boolBeamBkgTag) {beambkgtag = 1;}
    //
    //
    //--------------
    // ECL trigger
    //--------------
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
    int bitEtot1       = 0x0001;
    int bitEtot2       = 0x0002;
    int bitEtot3       = 0x0004;
    int bitBhabha      = 0x0008;
    int bitPreBhabha   = 0x0010;
    int bitForwardICN  = 0x0100;
    int bitBeamBkgVeto = 0x0200;
    int bitTiming      = 0x0400;

    bool boolEtot[3] = {false};
    if (E_phys > 1.0) boolEtot[1] = true;
    bool boolBhabha = (boolBtoBTag && icn > 4);
    bool boolPreBhabha = false;
    bool boolForwardICN = icnfwd;
    bool boolBeamBkgVeto = boolBeamBkgTag;
    int bit = 0;
    //
    // bit 5-7
    bit = (icn >= 7) ? 0x0007 : icn;
    bit <<= 5;
    // bit 0
    bit |= boolEtot[0] ? bitEtot1 : 0;
    // bit 1
    bit |= boolEtot[1] ? bitEtot2 : 0;
    // bit 2
    bit |= boolEtot[2] ? bitEtot3 : 0;
    // bit 3
    bit |= boolBhabha  ? bitBhabha : 0;
    // bit 4
    bit |= boolPreBhabha   ? bitPreBhabha : 0;
    // bit 8
    bit |= boolForwardICN  ? bitForwardICN : 0;
    // bit 9
    bit |= boolBeamBkgVeto ? bitBeamBkgVeto : 0;
    // bit 10
    bit |= bitTiming;
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

    //----------------------
    //
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
    trgEcltrgArray[m_hitEneNum]->setECLtoGDL(bit);
    trgEcltrgArray[m_hitEneNum]->setBhabhaVeto(BtoBTag);
    trgEcltrgArray[m_hitEneNum]->setBeamBkgVeto(beambkgtag);
    trgEcltrgArray[m_hitEneNum]->setEventTiming(eventtiming);


  }

  return;
}

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
    double HitTiming    = aTRGECLHit ->getTimeAve();
    double HitEnergy =  aTRGECLHit -> getEnergyDep();
    double HitBeamBkg =  aTRGECLHit -> getBeamBkgTag();

    TCTiming[iTCID].push_back(HitTiming);
    TCEnergy[iTCID].push_back(HitEnergy);
    TCBeamBkgTag[iTCID].push_back(HitBeamBkg);

  }
  //
  //
  int nBin = 2 * 8000 / TimeWindow ;
  double WindowStart = 0;
  double WindowEnd = 0;
  double fluctuation = ((gRandom ->Uniform(-1, 0))) * 125;

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
        if (TCTiming[iTCId][ihit] > WindowStart && TCTiming[iTCId][ihit] < WindowEnd) {
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
      if (TCTiming[iTCId][ihit] > WindowStart && TCTiming[iTCId][ihit] < WindowEnd) {
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
  obj_timing -> Setup(HitTCId, TCHitEnergy, TCHitTiming);
  obj_timing -> SetNofTopTC(_NofTopTC);
  eventtiming = obj_timing -> GetEventTiming(_EventTiming);



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

  double E_br = 0;
  double E_fwd = 0;
  double E_bwd = 0;
  double E_phys = 0;
  double E_total = 0;
  for (int iii = 0; iii <= 16; iii++) {
    if (iii > 1 && iii < 15) {E_phys += phiringsum[iii];}
    if (iii > 0 && iii < 3) {E_fwd += phiringsum[iii];}
    if (iii > 2 && iii < 15) {E_br += phiringsum[iii];}
    if (iii > 14) {E_bwd += phiringsum[iii];}
    E_total += phiringsum[iii];
  }
  if (E_total == 0) {return;}



  //--------------
  // Clustering
  //--------------
  //
  // TrgEclCluster obj_cluster;
  obj_cluster->setClusteringMethod(_Clustering);
  obj_cluster->setEventId(m_nEvent);
  obj_cluster->setICN(HitTCId, TCHitEnergy, TCHitTiming);

  int icn = obj_cluster->getICNFwBr();
  int icnfwd  = obj_cluster->getICNSub(0);
  int icnbr   = obj_cluster->getICNSub(1);
  int icnbwd = obj_cluster->getICNSub(2);

  //    int NofCluster =  obj_cluster->getNofCluster();

  //--------------
  // Bhabha veto
  //--------------
  //
  //    obj_bhabha -> setup()
  std::vector<double> vct_bhabha;
  vct_bhabha.clear();
  bool boolBtoBTag  = false;
  if (_Bhabha == 0) { //belle I
    boolBtoBTag  =  obj_bhabha -> GetBhabha00(phiringsum);
    vct_bhabha = obj_bhabha -> GetBhabhaComb();
  } else if (_Bhabha == 1) {
    vct_bhabha.resize(18);
    boolBtoBTag  =  obj_bhabha -> GetBhabha01();

  }
  int BtoBTag = 0;
  if (boolBtoBTag && icn < 4) {
    BtoBTag = 1;
  }
  //------------------------
  // Beam Background veto (Old cosmic veto)
  //------------------------
  bool boolBeamBkgTag = false;
  int beambkgtag = 0;
  boolBeamBkgTag =   obj_beambkg -> GetBeamBkg(thetaringsum);
  if (boolBeamBkgTag) {beambkgtag = 1;}
  //
  //
  //--------------
  // ECL trigger
  //--------------
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
  int bitEtot1       = 0x0001;
  int bitEtot2       = 0x0002;
  int bitEtot3       = 0x0004;
  int bitBhabha      = 0x0008;
  int bitPreBhabha   = 0x0010;
  int bitForwardICN  = 0x0100;
  int bitBeamBkgVeto = 0x0200;
  int bitTiming      = 0x0400;

  bool boolEtot[3] = {false};
  if (E_phys > 1.0) boolEtot[1] = true;
  bool boolBhabha = (boolBtoBTag && icn > 4);
  bool boolPreBhabha = false;
  bool boolForwardICN = icnfwd;
  bool boolBeamBkgVeto = boolBeamBkgTag;
  int bit = 0;
  //
  // bit 5-7
  bit = (icn >= 7) ? 0x0007 : icn;
  bit <<= 5;
  // bit 0
  bit |= boolEtot[0] ? bitEtot1 : 0;
  // bit 1
  bit |= boolEtot[1] ? bitEtot2 : 0;
  // bit 2
  bit |= boolEtot[2] ? bitEtot3 : 0;
  // bit 3
  bit |= boolBhabha  ? bitBhabha : 0;
  // bit 4
  bit |= boolPreBhabha   ? bitPreBhabha : 0;
  // bit 8
  bit |= boolForwardICN  ? bitForwardICN : 0;
  // bit 9
  bit |= boolBeamBkgVeto ? bitBeamBkgVeto : 0;
  // bit 10
  bit |= bitTiming;
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

  //----------------------
  //
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
  trgEcltrgArray[m_hitEneNum]->setECLtoGDL(bit);
  trgEcltrgArray[m_hitEneNum]->setBhabhaVeto(BtoBTag);
  trgEcltrgArray[m_hitEneNum]->setBeamBkgVeto(beambkgtag);
  trgEcltrgArray[m_hitEneNum]->setEventTiming(eventtiming);




  return;
}

//
//
//
void
TrgEclMaster::setRS(std::vector<int> TCId, std::vector<double> TCHit, std::vector<double>& phiringsum,
                    std::vector<std::vector<double>>&  thetaringsum)
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
      int iTCThetaId = obj_map ->getTCThetaIdFromTCId(iTCId + 1) - 1 ;
      int iTCPhiId = obj_map ->getTCPhiIdFromTCId(iTCId + 1) - 1 ;
      phiringsum[iTCThetaId] += TCHit[iHit];
      if (iTCThetaId - 1 < 3) { //fwd
        thetaringsum[0][iTCPhiId] += TCHit[iHit];
      } else if (iTCThetaId > 14) { //bwd
        thetaringsum[2][iTCPhiId] += TCHit[iHit];
      } else { //barrel
        thetaringsum[1][iTCPhiId] += TCHit[iHit];
      }

    }

  }

}

//









//
//
//
