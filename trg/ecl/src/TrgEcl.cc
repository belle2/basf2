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

#include "trg/ecl/dataobjects/TRGECLDigi.h"
#include "trg/ecl/dataobjects/TRGECLTrg.h"
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
TrgEcl::setPRS(TrgEclFAM* obj_trgeclfam, int hitTC[][20])
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
    _PhiRingSum[iTCThetaId] = 0;
  }


  for (int iTCId = 1; iTCId <= 576; iTCId++) {


    //   cout <<"check_1"<< endl;
    int ntcoutput = obj_trgeclfam->getTCNoOutput(iTCId);
    //  cout << ntcoutput << endl;
    for (int iTCThetaId = 0; iTCThetaId < 17; iTCThetaId++) {
      if (iTCThetaId == obj_trgeclfam->getTCThetaId(iTCId)) {
        //    cout <<"check_2"<< endl;
        for (int intcoutput = 0 ; intcoutput < ntcoutput ; intcoutput++) {
          if (hitTC[iTCId][intcoutput] > 0) {
            _PhiRingSum[iTCThetaId] += obj_trgeclfam->getTCEnergy(iTCId, intcoutput);
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
  TrgEclFAM* obj_trgeclfam = new TrgEclFAM();
  obj_trgeclfam->setup(m_nEvent, 1);
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
  for (int iBin = 0; iBin < 160 ; iBin ++) {
    EventTiming[iBin] = 0;
  }
  getEventTiming(obj_trgeclfam, 3); //1: belle , 2: Energetic , 3 : Energy weighted


  for (int iBin = 0 ; iBin < 160; iBin ++) {

    if (EventTiming[iBin] == 0) {continue;}
    setPRS(obj_trgeclfam, HitTC[iBin]);

    //--------------------------------------------------

    double E_tot = 0;
    for (int iii = 1; iii <= 14; iii++) { E_tot += _PhiRingSum[iii]; }

    //----------
    // Bhabha*
    //----------
    //
    // // Threshold for each combination of PRS
    // //
    // //   5.0,  // (1)  F1 + F2 + B1 + B2
    // //   5.5,  // (2)  F2 + F3 + B1 + B2 + C11 + C12
    // //   5.0,  // (3)  F2 :: backward gap
    // //   5.0,  // (4)  F3 + C10 + C11 + C12
    // //   5.0,  // (5)  C1 + C9 + C10
    // //   5.0,  // (6)  C1 + C2 + C9
    // //   5.0,  // (7)  C2 + C8 + C9
    // //   5.0,  // (8)  C3 + C7 + C8
    // //   5.0,  // (9)  C4 + C6 + C7z
    // //   5.0,  // (10) C5 + C6
    // //   3.0   // (11) C10 :: forward gap
    //
    int k01[] = {4,  1, 2, 16, 17};     // (1)  F1+F2 + B1+B2
    int k02[] = {6,  2, 3, 14, 15, 16, 17}; // (2)  F2+F3 + C11+C12 + B1+B2
    int k03[] = {1,  2};                // (3)  F2
    int k04[] = {4,  3, 13, 14, 15};    // (4)  F3 + C10+C11+C12
    int k05[] = {3,  4, 12, 13};        // (5)  C1+C9+C10
    int k06[] = {3,  4, 5, 12};         // (6)  C1+C2+C9
    int k07[] = {3,  5, 11, 12};        // (7)  C2+C8+C9
    int k08[] = {3,  6, 10, 11};        // (8)  C3+C7+C8
    int k09[] = {3,  7, 9, 10};         // (9)  C4+C6+C7
    int k10[] = {2,  8, 9};             // (10) C5+C6
    int k11[] = {1, 13};                // (11) C10

    double vct_bhabha[11];
    for (int iii = 0; iii < 11; iii++) {vct_bhabha[iii] = 0.0;}
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
    bool boolBhabhaStar =
      (vct_bhabha[0]  > 5.0 ||
       vct_bhabha[1]  > 5.5 ||
       vct_bhabha[2]  > 5.0 ||
       vct_bhabha[3]  > 5.0 ||
       vct_bhabha[4]  > 5.0 ||
       vct_bhabha[5]  > 5.0 ||
       vct_bhabha[6]  > 5.0 ||
       vct_bhabha[7]  > 5.0 ||
       vct_bhabha[8]  > 5.0 ||
       vct_bhabha[9]  > 5.0 ||
       vct_bhabha[10] > 3.0);
    //----------
    // ICN
    //----------
    //
    TrgEclCluster objCluster;
    objCluster.setICN(obj_trgeclfam, HitTC[iBin]);
    //  int BarrelICN =
    //  int ForwardICN =
    //  int BackwardICN = objCluster.getBackwardICN(&obj_trgeclfam);
    //  std::cout << "ICN = "<< BarrelICN << " + "<< ForwardICN << std::endl;
    int ICN = objCluster.getICNFwBr();
    int ICNForward  = objCluster.getICNSub(0);
    int ICNBarrel   = objCluster.getICNSub(1);
    int ICNBackward = objCluster.getICNSub(2);
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
    //if (E_tot > 0.1){continue;}
    bool boolEtot[3] = {false};
    if (E_tot > 1.0) boolEtot[1] = true;
    bool boolBhabha = (boolBhabhaStar && ICN > 4);
    bool boolPreBhabha = false;
    bool boolForwardICN = objCluster.getICNSub(0);
    bool boolBeamBkgVeto = objCluster.getBeamBkgVeto();
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


    //cout << vct_bhabha[0] << "  " << vct_bhabha[1] << "  " << vct_bhabha[2] << "  " << vct_bhabha[3] << "  " << vct_bhabha[4] << "  " << vct_bhabha[5] << "  " << vct_bhabha[6] << "  " << vct_bhabha[7] << "  " << vct_bhabha[8] << "  " << vct_bhabha[9] << "  " << vct_bhabha[10] << endl;
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
    // cout << itimebin << endl;
    trgEcltrgArray[m_hitEneNum]->setECLtoGDL(bitECLtoGDL);
    trgEcltrgArray[m_hitEneNum]->setBeamBkgVeto(BeamBkgVeto);
    trgEcltrgArray[m_hitEneNum]->setEventTiming(EventTiming[iBin]);

  }
  return;
}
void TrgEcl::getEventTiming(TrgEclFAM* obj_trgeclfam, int method)
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

  double TimeWindow = 0;
  double OverlapWindow = 0;
  double WindowStart = 0;
  double WindowEnd = 0;
  double fluctuation = 0;

  int nBin = 0;
  double FastestTCHit = 0;
  double MaxE = 0;

  double Etop5[5] = {0};
  double Ttop5[5] = {0};

  if (method == 1) { //belle1 method
    nBin = 0;
    TimeWindow = 500;
    WindowStart = -4000;
    WindowEnd = 0;

    while (WindowEnd < 4000) {
      FastestTCHit = 9999;
      EventTiming[nBin] = 0;
      for (int iTCId = 1; iTCId <= 576; iTCId++) {//Find fastest TC Hit
        int nTCOutput = obj_trgeclfam->getTCNoOutput(iTCId);// the # of hit per TC
        for (int inTCOutput = 0 ; inTCOutput < nTCOutput ; inTCOutput++) {
          if ((obj_trgeclfam->getTCTiming(iTCId, inTCOutput)) > WindowEnd) {
            if ((obj_trgeclfam->getTCTiming(iTCId, inTCOutput)) < FastestTCHit) {
              //     cout << FastestTCHit << endl;
              FastestTCHit = obj_trgeclfam->getTCTiming(iTCId, inTCOutput);
              EventTiming[nBin] = FastestTCHit;
            }
          }
        }
      }  // Find

      WindowStart = FastestTCHit;
      WindowEnd = WindowStart + TimeWindow;
      for (int iTCId = 1; iTCId <= 576; iTCId++) {
        int nTCOutput = obj_trgeclfam->getTCNoOutput(iTCId);// the # of hit per TC
        for (int inTCOutput = 0 ; inTCOutput < nTCOutput ; inTCOutput++) {
          if ((obj_trgeclfam->getTCTiming(iTCId, inTCOutput)) >= WindowStart && (obj_trgeclfam->getTCTiming(iTCId, inTCOutput)) < WindowEnd) {
            if ((obj_trgeclfam->getTCEnergy(iTCId, inTCOutput)) > 0.1) {
              HitTC[nBin][iTCId][inTCOutput] = 1;
            }
          }
        }
      }
      if (EventTiming[nBin] != 0) { nBin ++;}
      //  cout << EventTiming[nBin] << endl;
    }
  }



  if (method == 2) { // Energetic TC
    TimeWindow = 200;
    OverlapWindow = 100;
    nBin = 2 * 8000 / TimeWindow ;
    WindowStart = 0;
    WindowEnd = 0;
    fluctuation = ((gRandom ->Uniform(-1, 0))) * 100;


    for (int iTime = 0; iTime < nBin; iTime++) {
      MaxE = 0;
      WindowStart = iTime * TimeWindow - OverlapWindow + fluctuation - 4000;
      if (iTime == 0) {WindowStart = - 4000 + fluctuation;}
      WindowEnd  = WindowStart + TimeWindow;

      for (int iTCId = 1; iTCId <= 576; iTCId++) {

        int ntcoutput = obj_trgeclfam->getTCNoOutput(iTCId);
        for (int intcoutput = 0 ; intcoutput < ntcoutput ; intcoutput++) {

          if ((obj_trgeclfam->getTCTiming(iTCId, intcoutput)) > (WindowStart) &&
              (obj_trgeclfam->getTCTiming(iTCId, intcoutput)) < (WindowEnd)) {


            HitTC[iTime][iTCId][intcoutput] = 1;
            if ((obj_trgeclfam->getTCEnergy(iTCId, intcoutput)) > MaxE) {
              MaxE = (obj_trgeclfam->getTCEnergy(iTCId, intcoutput));
              EventTiming[iTime] = obj_trgeclfam->getTCTiming(iTCId, intcoutput);
            }
          }
        }
      }

      if (iTime > 1) {
        if (EventTiming[iTime] < EventTiming[iTime - 1] + 5 && EventTiming[iTime] > EventTiming[iTime - 1] - 5 && EventTiming[iTime] < EventTiming[iTime - 2] + 5 && EventTiming[iTime] > EventTiming[iTime - 2] - 5) {
          EventTiming[iTime - 1] = 0;
          EventTiming[iTime] = 0;
        }
      }
      if (EventTiming[iTime] == 0) {continue;}
    }
  }


  if (method == 3) { // Energy weighted
    TimeWindow = 200;
    OverlapWindow = 100;
    nBin = 2 * 8000 / TimeWindow ;
    WindowStart = 0;
    WindowEnd = 0;
    fluctuation = ((gRandom ->Uniform(-1, 0))) * 100;


    for (int iTime = 0; iTime < nBin; iTime++) {
      MaxE = 0;
      for (int i = 0; i < 5; i++) {
        Etop5[i] = 0;
        Ttop5[i] = 0;

      }

      WindowStart = iTime * TimeWindow - OverlapWindow + fluctuation - 4000;
      if (iTime == 0) {WindowStart = - 4000 + fluctuation;}
      WindowEnd  = WindowStart + TimeWindow;

      for (int iTCId = 1; iTCId <= 576; iTCId++) {

        int ntcoutput = obj_trgeclfam->getTCNoOutput(iTCId);
        for (int intcoutput = 0 ; intcoutput < ntcoutput ; intcoutput++) {

          if ((obj_trgeclfam->getTCTiming(iTCId, intcoutput)) > (WindowStart) &&
              (obj_trgeclfam->getTCTiming(iTCId, intcoutput)) < (WindowEnd)) {


            HitTC[iTime][iTCId][intcoutput] = 1;
            if (obj_trgeclfam->getTCEnergy(iTCId, intcoutput) > Etop5[0]) {
              Etop5[0] = obj_trgeclfam->getTCEnergy(iTCId, intcoutput);
              Ttop5[0] = obj_trgeclfam->getTCTiming(iTCId, intcoutput);
            } else if (obj_trgeclfam->getTCEnergy(iTCId, intcoutput) > Etop5[1]) {
              Etop5[1] = obj_trgeclfam->getTCEnergy(iTCId, intcoutput);
              Ttop5[1] = obj_trgeclfam->getTCTiming(iTCId, intcoutput);
            } else if (obj_trgeclfam->getTCEnergy(iTCId, intcoutput) > Etop5[2]) {
              Etop5[2] = obj_trgeclfam->getTCEnergy(iTCId, intcoutput);
              Ttop5[2] = obj_trgeclfam->getTCTiming(iTCId, intcoutput);
            } else if (obj_trgeclfam->getTCEnergy(iTCId, intcoutput) > Etop5[3]) {
              Etop5[3] = obj_trgeclfam->getTCEnergy(iTCId, intcoutput);
              Ttop5[3] = obj_trgeclfam->getTCTiming(iTCId, intcoutput);
            } else if (obj_trgeclfam->getTCEnergy(iTCId, intcoutput) > Etop5[4]) {
              Etop5[4] = obj_trgeclfam->getTCEnergy(iTCId, intcoutput);
              Ttop5[4] = obj_trgeclfam->getTCTiming(iTCId, intcoutput);
            }
          }
        }
      }
      for (int i = 0; i < 5; i++) {
        MaxE += Etop5[i];
        EventTiming[iTime] += Etop5[i] * Ttop5[i];
      }
      if (MaxE == 0) {continue;}
      EventTiming[iTime] = EventTiming[iTime] / MaxE;
      if (iTime > 1) {
        if (EventTiming[iTime] < EventTiming[iTime - 1] + 5 && EventTiming[iTime] > EventTiming[iTime - 1] - 5 && EventTiming[iTime] < EventTiming[iTime - 2] + 5 && EventTiming[iTime] > EventTiming[iTime - 2] - 5) {
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
