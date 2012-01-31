//---------------------------------------------------------------
// $Id$
//---------------------------------------------------------------
// Filename : TRGECL.cc
// Section  : TRG ECL
// Owner    : Yuuji Unno
// Email    : yunno@post.kek.jp
//---------------------------------------------------------------
// Description : A class to represent ECL
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

#include "trg/ecl/TRGECL.h"
#include "trg/ecl/TRGECLTCHit.h"
#include "trg/ecl/TRGECLEne.h"
#include "trg/ecl/TRGECLCluster.h"
//
#include <math.h>
//
//
using namespace std;
using namespace Belle2;
//
//
//
TRGECL::TRGECL():
  bitECLtoGDL(0)
{
}
//
//
//
std::string
TRGECL::name(void) const
{
  return "TRGECL";
}
//
//
//
std::string
TRGECL::version(void) const
{
  return std::string("TRGECL 1.00");
}
//
//
//
void
TRGECL::initialize(int m_nEvent)
{
}
//
//
//
TRGECL *TRGECL::_ecl = 0;
//
//
//
TRGECL *
TRGECL::getTRGECL(void) {

  //  if (! _ecl){
  //    std::cout << "TRGECL::getTRGECL !!! TRGECL is not created yet" << std::endl;
  //  }

  if (_ecl){ delete _ecl; }
  _ecl = new TRGECL();

  return _ecl;

}
//
//
//
void
TRGECL::setPRS(TCHit * obj_tchit){
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
  for (int iTCThetaId = 0; iTCThetaId < 17; iTCThetaId++ ){
    _PhiRingSum[iTCThetaId] = 0;
  }
  for (int iTCId = 1; iTCId <= 576; iTCId++) {
    for (int iTCThetaId = 0; iTCThetaId < 17; iTCThetaId++) {
      if (iTCThetaId == obj_tchit->getTCThetaId(iTCId)) {
        _PhiRingSum[iTCThetaId] += obj_tchit->getTCEnergy(iTCId);
      }
    }
  }
}
//
//
//
void
TRGECL::simulate(int m_nEvent)
{
  TCHit * obj_tchit = new TCHit();
  obj_tchit->setup();
  setPRS(obj_tchit);
  //
  //----------
  // TC Etot
  //----------
  //
  // Energy sum of forward and barrel except for extreme forward
  // so Etot is sum of "phi ring ID" = 1-14
  // Etot threshold are
  // 1.0 GeV for Etot01
  // 0.5 GeV for Etot02
  // 3.0 GeV for Etot03
  //
  // double TsimECL_parameters::Etot[3] = { 1.000, 0.500, 3.000 };
  double E_tot = 0;
  for (int iii = 1; iii <= 14; iii++) { E_tot += _PhiRingSum[iii]; }
  //
  //----------
  // Bhabha*
  //----------
  //
  // Threshold for each combination of PRS
  //
  //   5.0,  // (1)  F1 + F2 + B1 + B2
  //   5.5,  // (2)  F2 + F3 + B1 + B2 + C11 + C12
  //   5.0,  // (3)  F2 :: backward gap
  //   5.0,  // (4)  F3 + C10 + C11 + C12
  //   5.0,  // (5)  C1 + C9 + C10
  //   5.0,  // (6)  C1 + C2 + C9
  //   5.0,  // (7)  C2 + C8 + C9
  //   5.0,  // (8)  C3 + C7 + C8
  //   5.0,  // (9)  C4 + C6 + C7
  //   5.0,  // (10) C5 + C6
  //   3.0   // (11) C10 :: forward gap
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
  for (int iii = 1; iii <= k01[0]; iii++) { vct_bhabha[0]  += _PhiRingSum[k01[iii] - 1]; }
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
     vct_bhabha[1]  > 5.0 ||
     vct_bhabha[2]  > 5.0 ||
     vct_bhabha[3]  > 5.0 ||
     vct_bhabha[4]  > 5.0 ||
     vct_bhabha[5]  > 5.0 ||
     vct_bhabha[6]  > 5.0 ||
     vct_bhabha[7]  > 5.0 ||
     vct_bhabha[8]  > 5.0 ||
     vct_bhabha[9]  > 5.0 ||
     vct_bhabha[10] > 5.0);
  //
  //----------
  // ICN
  //----------
  //
  //  TRGECLCluster & aaa = TRGECLCluster::Instance();
  TRGECLCluster objCluster;
  objCluster.setICN(obj_tchit);
  //  int BarrelICN = 
  //  int ForwardICN = 
  //  int BackwardICN = objCluster.getBackwardICN(&obj_tchit);
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

  bool boolEtot[3] = {false};
  if (E_tot > 1.0) boolEtot[1] = true;
  bool boolBhabha = (boolBhabhaStar && ICN>4);
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
  //----------------------
  //
  //-------------
  // Store
  //-------------
  //
  // TC Level
  //
  int m_hitTCNum = 0;
  for (int iii = 0; iii < 576 ; iii++) {
    int TCId = iii+1;
    double tmp_tc_energy = obj_tchit->getTCEnergy(TCId);
    if ( tmp_tc_energy < 1.0e-9) {continue;}
    StoreArray<TRGECLTCHit> eclTCHitArray("TRGECLTCHit");
    m_hitTCNum = eclTCHitArray->GetLast() + 1;
    new(eclTCHitArray->AddrAt(m_hitTCNum)) TRGECLTCHit();
    eclTCHitArray[m_hitTCNum]->setEventId(m_nEvent);
    eclTCHitArray[m_hitTCNum]->setTCId(TCId);
    eclTCHitArray[m_hitTCNum]->setThetaId(obj_tchit->getTCThetaId(TCId));
    eclTCHitArray[m_hitTCNum]->setPhiId(obj_tchit->getTCPhiId(TCId));
    eclTCHitArray[m_hitTCNum]->setEnergyDep(tmp_tc_energy);
    eclTCHitArray[m_hitTCNum]->setTimeAve(0);
  }
  //
  // Total level
  //
  int m_hitEneNum = 0;
  StoreArray<TRGECLEne> eclEneArray("TRGECLEne");
  m_hitEneNum = eclEneArray->GetLast() + 1;
  new(eclEneArray->AddrAt(m_hitEneNum)) TRGECLEne();
  eclEneArray[m_hitEneNum]->setEventId(m_nEvent);
  eclEneArray[m_hitEneNum]->setPRS01(_PhiRingSum[0]);
  eclEneArray[m_hitEneNum]->setPRS02(_PhiRingSum[1]);
  eclEneArray[m_hitEneNum]->setPRS03(_PhiRingSum[2]);
  eclEneArray[m_hitEneNum]->setPRS04(_PhiRingSum[3]);
  eclEneArray[m_hitEneNum]->setPRS05(_PhiRingSum[4]);
  eclEneArray[m_hitEneNum]->setPRS06(_PhiRingSum[5]);
  eclEneArray[m_hitEneNum]->setPRS07(_PhiRingSum[6]);
  eclEneArray[m_hitEneNum]->setPRS08(_PhiRingSum[7]);
  eclEneArray[m_hitEneNum]->setPRS09(_PhiRingSum[8]);
  eclEneArray[m_hitEneNum]->setPRS10(_PhiRingSum[9]);
  eclEneArray[m_hitEneNum]->setPRS11(_PhiRingSum[10]);
  eclEneArray[m_hitEneNum]->setPRS12(_PhiRingSum[11]);
  eclEneArray[m_hitEneNum]->setPRS13(_PhiRingSum[12]);
  eclEneArray[m_hitEneNum]->setPRS14(_PhiRingSum[13]);
  eclEneArray[m_hitEneNum]->setPRS15(_PhiRingSum[14]);
  eclEneArray[m_hitEneNum]->setPRS16(_PhiRingSum[15]);
  eclEneArray[m_hitEneNum]->setPRS17(_PhiRingSum[16]);
  //
  eclEneArray[m_hitEneNum]->setEtot(E_tot);
  //
  eclEneArray[m_hitEneNum]->setBhabha01(vct_bhabha[0]);
  eclEneArray[m_hitEneNum]->setBhabha02(vct_bhabha[1]);
  eclEneArray[m_hitEneNum]->setBhabha03(vct_bhabha[2]);
  eclEneArray[m_hitEneNum]->setBhabha04(vct_bhabha[3]);
  eclEneArray[m_hitEneNum]->setBhabha05(vct_bhabha[4]);
  eclEneArray[m_hitEneNum]->setBhabha06(vct_bhabha[5]);
  eclEneArray[m_hitEneNum]->setBhabha07(vct_bhabha[6]);
  eclEneArray[m_hitEneNum]->setBhabha08(vct_bhabha[7]);
  eclEneArray[m_hitEneNum]->setBhabha09(vct_bhabha[8]);
  eclEneArray[m_hitEneNum]->setBhabha10(vct_bhabha[9]);
  eclEneArray[m_hitEneNum]->setBhabha11(vct_bhabha[10]);
  //
  eclEneArray[m_hitEneNum]->setICN(ICN);
  eclEneArray[m_hitEneNum]->setICNFw(ICNForward);
  eclEneArray[m_hitEneNum]->setICNBr(ICNBarrel);
  eclEneArray[m_hitEneNum]->setICNBw(ICNBackward);
  //
  eclEneArray[m_hitEneNum]->setECLtoGDL(bitECLtoGDL);
  //
  //
}
//
//
//
