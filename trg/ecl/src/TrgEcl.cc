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
#include "trg/ecl/TrgEcl.h"
#include "trg/ecl/TrgEclCluster.h"
#include "trg/ecl/TrgEclMapping.h"

#include "trg/ecl/dataobjects/TRGECLDigi.h"
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
TrgEcl::TrgEcl():
  bitECLtoGDL(0)
{
  for (int iTCId = 0; iTCId < 576; iTCId++) {
    for (int iTimeindex = 0; iTimeindex < 64; iTimeindex++) {
      Timing[iTCId][iTimeindex] = 0;
      Energy[iTCId][iTimeindex] = 0;
      for (int ibin = 0; ibin < 160; ibin++) {
        HitTC[ibin][iTCId][iTimeindex] = 0;
      }
    }
  }
  for (int iii = 0; iii < 5; iii++) {
    Etop5[iii] = 0;
    Ttop5[iii] = 0;

  }

  obj_cluster = new TrgEclCluster();

}
TrgEcl::~TrgEcl()
{
  delete obj_cluster;
}
//
//
//
std::string
TrgEcl::name(void) const
{
  return "TrgEcl";
}
//
//
//
std::string
TrgEcl::version(void) const
{
  return std::string("TrgEcl 1.00");
}
//
//
//
void
//iwasaki TrgEcl::initialize(int m_nEvent)
TrgEcl::initialize(int)
{


}
//
//
//
TrgEcl* TrgEcl::_ecl = 0;
//
//
//
TrgEcl*
TrgEcl::getTrgEcl(void)
{

  //  if (! _ecl){
  //    std::cout << "TrgEcl::getTrgEcl !!! TrgEcl is not created yet" << std::endl;
  //  }

  if (_ecl) { delete _ecl; }
  _ecl = new TrgEcl();

  return _ecl;

}
//
//
//
void
TrgEcl::setPRS(int iBin)
{
  //
  //
  // TC Phi ring sum
  //
  //----------------------------------------
  //               FW    BR     BW   total
  //----------------------------------------
  //TC phi ring     3    12     2      17
  //ID             0-2  3-14  15-16
  //----------------------------------------
  //
  for (int iTCThetaId = 0; iTCThetaId < 17; iTCThetaId++) {
    for (int itime = 0; itime < 64 ; itime++) {
      _PhiRingSum[iTCThetaId] = 0;
    }
  }
  TrgEclMapping* map = new TrgEclMapping();

  for (int iTCId = 0; iTCId < 576; iTCId++) {
    for (int itime = 0; itime < 64; itime++) {
      for (int iTCThetaId = 0; iTCThetaId < 17; iTCThetaId++) {
        if (HitTC[iBin][iTCId][itime] == 1) {
          if (iTCThetaId == map ->getTCThetaIdFromTCId(iTCId + 1)) {

            _PhiRingSum[iTCThetaId] += Energy[iTCId][itime];
          }
        }
      }
    }
  }
}



//
//
//
void
TrgEcl::simulate(int m_nEvent)
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
  //-----------------------------------------
  // Calculate event timing
  //-----------------------------------------

  for (int iTCId = 0; iTCId < 576; iTCId++) {
    for (int iTimeindex = 0; iTimeindex < 64; iTimeindex++) {
      Timing[iTCId][iTimeindex] = 0;
      Energy[iTCId][iTimeindex] = 0;
      for (int ibin = 0; ibin < 160; ibin++) {
        HitTC[ibin][iTCId][iTimeindex] = 0;
      }
    }
  }

  StoreArray<TRGECLHit> trgeclHitArray;
  for (int ii = 0; ii < trgeclHitArray.getEntries(); ii++) {

    TRGECLHit* aTRGECLHit = trgeclHitArray[ii];
    int TCID = (aTRGECLHit->getCellId() - 1);
    double TCHitTiming    = aTRGECLHit ->getTimeAve();
    double TCHitEnergy =  aTRGECLHit -> getEnergyDep();
    int itimeindex = (int)(TCHitTiming / 100 + 40);

    Timing[TCID][itimeindex] = TCHitTiming;
    Energy[TCID][itimeindex] = TCHitEnergy;

  }




  for (int iBin = 0; iBin < 64 ; iBin ++) {
    EventTiming[iBin] = 0;
  }

  getEventTiming(1); //1: belle , 2: Energetic , 3 : Energy weighted

  for (int iBin = 0 ; iBin < 64; iBin ++) {

    if (EventTiming[iBin] == 0) {continue;}
    //  cout << "hmm???" << endl;
    setPRS(iBin);


    //--------------------------------------------------

    double E_tot = 0;
    for (int iii = 1; iii <= 14; iii++) { E_tot += _PhiRingSum[iii]; }

    //----------
    // Bhabha*
    //----------
    //
    // // Threshold for each combination of PRS
    // //



    vector<int> k01 = {5,  1, 2, 3 , 16, 17};    // (1)  F1+F2 + F3 + B1+B2
    vector<int> k02 = {2,  3, 15}; // (2)  F3 + C12
    vector<int> k03 = {2,  2, 3};                // (3)  F2 + F3
    vector<int> k04 = {1,  4};    // (4)  C1 + backward gap
    vector<int> k05 = {3,  4, 14, 15};        // (5)  C1+C11+C12
    vector<int> k06 = {3,  5, 14, 15};         // (6)  C2+C11+C12
    vector<int> k07 = {3,  4, 5, 14};        // (7)  C1+C2+C11
    vector<int> k08 = {3,  5, 13, 14};        // (8)  C2+C10+C11
    vector<int> k09 = {3,  5, 12, 13};         // (9)  C2+C9+C10
    vector<int> k10 = {3,  5, 6, 13};             // (10) C2+C3+C10
    vector<int> k11 = {3,  5, 6, 12};                // (11) C2+C3+C9
    vector<int> k12 = {3,  6, 7, 12};         // (9)  C3+C4+C9
    vector<int> k13 = {3,  6, 7, 11};             // (10) C3+C4+C8
    vector<int> k14 = {3,  7, 8, 11};                // (11) C4+C5+C8
    vector<int> k15 = {3,  8, 10, 11};         // (9)  C5+C7+C8
    vector<int> k16 = {2,  8, 9, 10};             // (10) C5+C6+C7
    vector<int> k17 = {1, 14, 15};                // (11) C11+C12 +forward gap
    vector<int> k18 = {1, 16};                // (11) B1 + forward gap

    vector<double> vct_bhabha;
    for (int iii = 0; iii < 18; iii++) {vct_bhabha.push_back(0.0);}
    for (int iii = 1; iii <= k01[0]; iii++) { vct_bhabha[0]  += _PhiRingSum[k01[iii] - 1];}
    for (int iii = 1; iii <= k02[0]; iii++) { vct_bhabha[1]  += _PhiRingSum[k02[iii] - 1]; }
    for (int iii = 1; iii <= k03[0]; iii++) { vct_bhabha[2]  += _PhiRingSum[k03[iii] - 1]; }
    for (int iii = 1; iii <= k04[0]; iii++) { vct_bhabha[3]  += _PhiRingSum[k04[iii] - 1]; }
    for (int iii = 1; iii <= k05[0]; iii++) { vct_bhabha[4]  += _PhiRingSum[k05[iii] - 1]; }
    for (int iii = 1; iii <= k06[0]; iii++) { vct_bhabha[5]  += _PhiRingSum[k06[iii] - 1]; }
    for (int iii = 1; iii <= k07[0]; iii++) { vct_bhabha[6]  += _PhiRingSum[k07[iii] - 1]; }
    for (int iii = 1; iii <= k08[0]; iii++) { vct_bhabha[7]  += _PhiRingSum[k08[iii] - 1]; }
    for (int iii = 1; iii <= k09[0]; iii++) { vct_bhabha[8]  += _PhiRingSum[k09[iii] - 1]; }
    for (int iii = 1; iii <= k10[0]; iii++) { vct_bhabha[9]  += _PhiRingSum[k10[iii] - 1]; }
    for (int iii = 1; iii <= k11[0]; iii++) { vct_bhabha[10] += _PhiRingSum[k11[iii] - 1]; }
    for (int iii = 1; iii <= k12[0]; iii++) { vct_bhabha[11] += _PhiRingSum[k12[iii] - 1]; }
    for (int iii = 1; iii <= k13[0]; iii++) { vct_bhabha[12] += _PhiRingSum[k13[iii] - 1]; }
    for (int iii = 1; iii <= k14[0]; iii++) { vct_bhabha[13] += _PhiRingSum[k14[iii] - 1]; }
    for (int iii = 1; iii <= k15[0]; iii++) { vct_bhabha[14] += _PhiRingSum[k15[iii] - 1]; }
    for (int iii = 1; iii <= k16[0]; iii++) { vct_bhabha[15] += _PhiRingSum[k16[iii] - 1]; }
    for (int iii = 1; iii <= k17[0]; iii++) { vct_bhabha[16] += _PhiRingSum[k17[iii] - 1]; }
    for (int iii = 1; iii <= k18[0]; iii++) { vct_bhabha[17] += _PhiRingSum[k18[iii] - 1]; }



    bool boolBhabhaStar =
      (vct_bhabha[0]  > 5.0 ||
       vct_bhabha[1]  > 3.0 ||
       vct_bhabha[2]  > 5.0 ||
       vct_bhabha[3]  > 4.0 ||
       vct_bhabha[4]  > 5.0 ||
       vct_bhabha[5]  > 5.0 ||
       vct_bhabha[6]  > 5.0 ||
       vct_bhabha[7]  > 5.0 ||
       vct_bhabha[8]  > 5.0 ||
       vct_bhabha[9]  > 5.0 ||
       vct_bhabha[10] > 5.0 ||
       vct_bhabha[11] > 5.0 ||
       vct_bhabha[12] > 5.0 ||
       vct_bhabha[13] > 5.0 ||
       vct_bhabha[14] > 5.0 ||
       vct_bhabha[15] > 5.0 ||
       vct_bhabha[16] > 3.0 ||
       vct_bhabha[17] > 3.0);
    //----------
    // ICN
    //----------
    //
    // TrgEclCluster obj_cluster;
    obj_cluster->setICN(HitTC[iBin]);


    int ICN = obj_cluster->getICNFwBr();
    int ICNForward  = obj_cluster->getICNSub(0);
    int ICNBarrel   = obj_cluster->getICNSub(1);
    int ICNBackward = obj_cluster->getICNSub(2);


    int m_hitNum = 0;
    int NofCluster =  obj_cluster->getNofCluster();
    for (int incluster = 0; incluster < NofCluster ; incluster++) {

      int NofTCinCluster = obj_cluster->getNofTCinCluster(incluster);
      double ClusterEnergy = obj_cluster->getClusterEnergy(incluster);
      double ClusterTiming = obj_cluster->getClusterTiming(incluster);
      double ClusterPositionX = (obj_cluster->getClusterPosition(incluster)).X();
      double ClusterPositionY = (obj_cluster->getClusterPosition(incluster)).Y();
      double ClusterPositionZ = (obj_cluster->getClusterPosition(incluster)).Z();

      if (ClusterEnergy == 0 && ClusterTiming == 0) {continue;}

      StoreArray<TRGECLCluster> ClusterArray;
      ClusterArray.appendNew();
      m_hitNum = ClusterArray.getEntries() - 1;
      ClusterArray[m_hitNum]->setEventId(m_nEvent);
      ClusterArray[m_hitNum]->setClusterId(incluster);
      ClusterArray[m_hitNum]->setNofTCinCluster(NofTCinCluster);
      ClusterArray[m_hitNum]->setEnergyDep(ClusterEnergy);
      ClusterArray[m_hitNum]->setTimeAve(ClusterTiming);
      ClusterArray[m_hitNum]->setPositionX(ClusterPositionX);
      ClusterArray[m_hitNum]->setPositionY(ClusterPositionY);
      ClusterArray[m_hitNum]->setPositionZ(ClusterPositionZ);

    }


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
    if (E_tot > 1.0) boolEtot[1] = true;
    bool boolBhabha = (boolBhabhaStar && ICN > 4);
    bool boolPreBhabha = false;
    bool boolForwardICN = obj_cluster->getICNSub(0);
    bool boolBeamBkgVeto = obj_cluster->getBeamBkgVeto();
    //
    // bit 5-7
    bitECLtoGDL = (ICN >= 7) ? 0x0007 : ICN;
    bitECLtoGDL <<= 5;
    // bit 0
    bitECLtoGDL |= boolEtot[0] ? bitEtot1 : 0;
    // bit 1
    bitECLtoGDL |= boolEtot[1] ? bitEtot2 : 0;
    // bit 2
    bitECLtoGDL |= boolEtot[2] ? bitEtot3 : 0;
    // bit 3
    bitECLtoGDL |= boolBhabha  ? bitBhabha : 0;
    // bit 4
    bitECLtoGDL |= boolPreBhabha   ? bitPreBhabha : 0;
    // bit 8
    bitECLtoGDL |= boolForwardICN  ? bitForwardICN : 0;
    // bit 9
    bitECLtoGDL |= boolBeamBkgVeto ? bitBeamBkgVeto : 0;
    // bit 10
    bitECLtoGDL |= bitTiming;
    //
    //  printf("bitECLtoGDL = %i \n", bitECLtoGDL);
    //----------------------
    //   if (0){ // check bit by "binary" output
    //     int xxx = bitECLtoGDL;
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
    int BeamBkgVeto = 0;
    if (boolBeamBkgVeto) {BeamBkgVeto = 1;}


    //----------------------
    //
    //-------------
    // Store
    //-------------
    int m_hitEneNum = 0;
    StoreArray<TRGECLTrg> trgEcltrgArray("TRGECLTrgs");
    trgEcltrgArray.appendNew();
    m_hitEneNum = trgEcltrgArray.getEntries() - 1;

    trgEcltrgArray[m_hitEneNum]->setEventId(m_nEvent);
    trgEcltrgArray[m_hitEneNum]->setPRS01(_PhiRingSum[0]);
    trgEcltrgArray[m_hitEneNum]->setPRS02(_PhiRingSum[1]);
    trgEcltrgArray[m_hitEneNum]->setPRS03(_PhiRingSum[2]);
    trgEcltrgArray[m_hitEneNum]->setPRS04(_PhiRingSum[3]);
    trgEcltrgArray[m_hitEneNum]->setPRS05(_PhiRingSum[4]);
    trgEcltrgArray[m_hitEneNum]->setPRS06(_PhiRingSum[5]);
    trgEcltrgArray[m_hitEneNum]->setPRS07(_PhiRingSum[6]);
    trgEcltrgArray[m_hitEneNum]->setPRS08(_PhiRingSum[7]);
    trgEcltrgArray[m_hitEneNum]->setPRS09(_PhiRingSum[8]);
    trgEcltrgArray[m_hitEneNum]->setPRS10(_PhiRingSum[9]);
    trgEcltrgArray[m_hitEneNum]->setPRS11(_PhiRingSum[10]);
    trgEcltrgArray[m_hitEneNum]->setPRS12(_PhiRingSum[11]);
    trgEcltrgArray[m_hitEneNum]->setPRS13(_PhiRingSum[12]);
    trgEcltrgArray[m_hitEneNum]->setPRS14(_PhiRingSum[13]);
    trgEcltrgArray[m_hitEneNum]->setPRS15(_PhiRingSum[14]);
    trgEcltrgArray[m_hitEneNum]->setPRS16(_PhiRingSum[15]);
    trgEcltrgArray[m_hitEneNum]->setPRS17(_PhiRingSum[16]);
    //
    trgEcltrgArray[m_hitEneNum]->setEtot(E_tot);
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
    //
    trgEcltrgArray[m_hitEneNum]->setICN(ICN);
    trgEcltrgArray[m_hitEneNum]->setICNFw(ICNForward);
    trgEcltrgArray[m_hitEneNum]->setICNBr(ICNBarrel);
    trgEcltrgArray[m_hitEneNum]->setICNBw(ICNBackward);
    //
    trgEcltrgArray[m_hitEneNum]->setECLtoGDL(bitECLtoGDL);
    trgEcltrgArray[m_hitEneNum]->setBeamBkgVeto(BeamBkgVeto);
    trgEcltrgArray[m_hitEneNum]->setEventTiming(EventTiming[iBin]);


  }
  return;
}
void TrgEcl::getEventTiming(int method)
{
  //-----------------------------------------------------------------------------
  //  Calculate Eventtiming
  //  1 : Belle methpd  Event-timing = Fastest TC timing
  //      trigger decision = within 500 ns from the event-timing
  //  2 : Energetic TC method : Event timing = most energetic TC in timewindow
  //      trigger decision =  tick by tick check trigger decision within timewindow
  //  3 : Energy weighted method : Even timing = Energy weighted timing of 5 energetic TC within timewindow
  //      trigger decision = same as 2
  //
  // Please see B2GM slide "Tsim ecl status(Insoo Lee)"  2014 JUN ECL or Trigger session, if you want to know in detail.
  //-------------------------------------------------------------------------------------------------------

  for (int iTCId = 0; iTCId < 576; iTCId++) {
    for (int iTimeindex = 0; iTimeindex < 64; iTimeindex++) {
      for (int ibin = 0; ibin < 64; ibin++) {
        HitTC[ibin][iTCId][iTimeindex] = 0;
      }
    }
  }


  double TimeWindow = 0;
  double OverlapWindow = 0;
  double WindowStart = 0;
  double WindowEnd = 0;
  double fluctuation = 0;

  int nBin = 0;
  double FastestTCHit = 0;
  double MaxE = 0;

//   double *Etop5 = new double [5];
//   double *Ttop5 = new double [5]



  if (method == 1) { //belle1 method
    nBin = 0;
    TimeWindow = 250;
    WindowStart = -4000;
    WindowEnd = WindowStart;// + TimeWindow;

    while (WindowEnd < 4000) {
      FastestTCHit = 9999;
      EventTiming[nBin] = 0;
      for (int iTCId = 0; iTCId < 576; iTCId++) {//Find fastest TC Hit
        for (int itime = 0 ; itime < 64 ; itime++) {
          if (Timing[iTCId][itime] == 0 && Energy[iTCId][itime] == 0) {continue;}
          if (Timing[iTCId][itime] > WindowEnd) {
            if (Timing[iTCId][itime] < FastestTCHit) {
              FastestTCHit =  Timing[iTCId][itime];
              EventTiming[nBin] = FastestTCHit;
            }
          }
        }
      }  // Find
      WindowStart = FastestTCHit;
      WindowEnd = WindowStart + TimeWindow;
      for (int iTCId = 0; iTCId < 576; iTCId++) {
        for (int itime = 0 ; itime < 64 ; itime++) {
          if (Timing[iTCId][itime] >= WindowStart &&  Timing[iTCId][itime] < WindowEnd) {
            if (Energy[iTCId][itime] > 0.1) {
              HitTC[nBin][iTCId][itime] = 1;
            }
          }
        }
      }
      if (EventTiming[nBin] != 0) {
        nBin ++;
      }
    }
  }


  if (method == 2) { // Energetic TC
    TimeWindow = 250;
    OverlapWindow = 125;
    nBin = 2 * 8000 / TimeWindow ;
    WindowStart = 0;
    WindowEnd = 0;
    fluctuation = ((gRandom ->Uniform(-1, 0))) * 100;


    for (int iTime = 0; iTime < nBin; iTime++) {
      MaxE = 0;
      WindowStart = iTime * TimeWindow - OverlapWindow + fluctuation - 4000;
      if (iTime == 0) {WindowStart = - 4000 + fluctuation;}
      WindowEnd  = WindowStart + TimeWindow;

      for (int iTCId = 0; iTCId < 576; iTCId++) {

        for (int itime = 0 ; itime < 64 ; itime++) {

          if (Timing[iTCId][itime] > (WindowStart) &&
              Timing[iTCId][itime] < (WindowEnd)) {


            HitTC[iTime][iTCId][itime] = 1;
            if (Energy[iTCId][itime] > MaxE) {
              MaxE = Energy[iTCId][itime];
              EventTiming[iTime] = Timing[iTCId][itime];
            }
          }
        }
      }

      if (iTime > 1) {
        if (EventTiming[iTime] < EventTiming[iTime - 1] + 5 && EventTiming[iTime] > EventTiming[iTime - 1] - 5
            && EventTiming[iTime] < EventTiming[iTime - 2] + 5 && EventTiming[iTime] > EventTiming[iTime - 2] - 5) {
          EventTiming[iTime - 1] = 0;
          EventTiming[iTime] = 0;
        }
      }
      if (EventTiming[iTime] == 0) {continue;}
    }
  }


  if (method == 3) { // Energy weighted
    TimeWindow = 250;
    OverlapWindow = 125;
    nBin = 2 * 8000 / TimeWindow ;
    WindowStart = 0;
    WindowEnd = 0;
    fluctuation = ((gRandom ->Uniform(-1, 0))) * 125;


    for (int iTime = 0; iTime < nBin; iTime++) {
      MaxE = 0;
      for (int i = 0; i < 5; i++) {
        Etop5[i] = 0;
        Ttop5[i] = 0;

      }

      WindowStart = iTime * TimeWindow - OverlapWindow + fluctuation - 4000;
      if (iTime == 0) {WindowStart = - 4000 + fluctuation;}
      WindowEnd  = WindowStart + TimeWindow;

      for (int iTCId = 0; iTCId < 576; iTCId++) {

        for (int itime = 0 ; itime < 64 ; itime++) {

          if (Timing[iTCId][itime] > (WindowStart) &&
              Timing[iTCId][itime] < (WindowEnd)) {
            if (Energy[iTCId][itime] < 0.1) {continue;}

            HitTC[iTime][iTCId][itime] = 1;
            //      cout << iTCId << endl;
            if (Energy[iTCId][itime] > Etop5[0]) {
              Etop5[0] = Energy[iTCId][itime];
              Ttop5[0] = Timing[iTCId][itime];
            } else if (Energy[iTCId][itime] > Etop5[1]) {
              Etop5[1] = Energy[iTCId][itime];
              Ttop5[1] = Timing[iTCId][itime];
            } else if (Energy[iTCId][itime] > Etop5[2]) {
              Etop5[2] = Energy[iTCId][itime];
              Ttop5[2] = Timing[iTCId][itime];
//             } else if (Energy[iTCId][itime] > Etop5[3]) {
//               Etop5[3] = Energy[iTCId][itime];
//               Ttop5[3] = Timing[iTCId][itime];
//             } else if (Energy[iTCId][itime] > Etop5[4]) {
//               Etop5[4] = Energy[iTCId][itime];
//               Ttop5[4] = Timing[iTCId][itime];
            }
          }
        }
      }
      for (int i = 0; i < 3; i++) {
        MaxE += Etop5[i];
        EventTiming[iTime] += Etop5[i] * Ttop5[i];
      }
      if (MaxE == 0) {continue;}
      EventTiming[iTime] = EventTiming[iTime] / MaxE;
      if (iTime > 1) {
        if (EventTiming[iTime] < EventTiming[iTime - 1] + 5 && EventTiming[iTime] > EventTiming[iTime - 1] - 5
            && EventTiming[iTime] < EventTiming[iTime - 2] + 5 && EventTiming[iTime] > EventTiming[iTime - 2] - 5) {
          EventTiming[iTime - 1] = 0;
          EventTiming[iTime] = 0;
        }
      }
      if (EventTiming[iTime] == 0) {continue;}
    }
  }

  return;
}









//
//
//
