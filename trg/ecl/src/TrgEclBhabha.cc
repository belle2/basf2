//---------------------------------------------------------------
// $Id$
//---------------------------------------------------------------
// Filename : TrgEclBhabha.cc
// Section  : TRG ECL
// Owner    : InSoo Lee/Yuuji Unno
// Email    : islee@hep.hanyang.ac.kr / yunno@post.kek.jp
//---------------------------------------------------------------
// Description : A class to represent TRG ECL
//---------------------------------------------------------------
// $Log$
//---------------------------------------------------------------

#define TRG_SHORT_NAMES
#define TRGECLCLUSTER_SHORT_NAMES
#include <framework/gearbox/Unit.h>
#include "framework/datastore/StoreArray.h"


#include <trg/ecl/TrgEclBhabha.h>
#include <framework/logging/Logger.h>

#include "trg/ecl/dataobjects/TRGECLCluster.h"

using namespace std;
using namespace Belle2;
//
//
//
TrgEclBhabha::TrgEclBhabha()
{
  BhabhaComb.clear();
  MaxTCId.clear();
  ClusterEnergy.clear();
  ClusterTiming.clear();
  ClusterPosition.clear();

  _TCMap = new TrgEclMapping();



}

TrgEclBhabha::~TrgEclBhabha()
{
  delete _TCMap;
}
bool TrgEclBhabha::GetBhabha00(std::vector<double> PhiRingSum)  //Belle 2D Bhabha veto method
{
  bool BtoBflag = false;


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



  BhabhaComb.resize(18);
  for (int iii = 1; iii <= k01[0]; iii++) { BhabhaComb[0]  += PhiRingSum[k01[iii] - 1];}
  for (int iii = 1; iii <= k02[0]; iii++) { BhabhaComb[1]  += PhiRingSum[k02[iii] - 1]; }
  for (int iii = 1; iii <= k03[0]; iii++) { BhabhaComb[2]  += PhiRingSum[k03[iii] - 1]; }
  for (int iii = 1; iii <= k04[0]; iii++) { BhabhaComb[3]  += PhiRingSum[k04[iii] - 1]; }
  for (int iii = 1; iii <= k05[0]; iii++) { BhabhaComb[4]  += PhiRingSum[k05[iii] - 1]; }
  for (int iii = 1; iii <= k06[0]; iii++) { BhabhaComb[5]  += PhiRingSum[k06[iii] - 1]; }
  for (int iii = 1; iii <= k07[0]; iii++) { BhabhaComb[6]  += PhiRingSum[k07[iii] - 1]; }
  for (int iii = 1; iii <= k08[0]; iii++) { BhabhaComb[7]  += PhiRingSum[k08[iii] - 1]; }
  for (int iii = 1; iii <= k09[0]; iii++) { BhabhaComb[8]  += PhiRingSum[k09[iii] - 1]; }
  for (int iii = 1; iii <= k10[0]; iii++) { BhabhaComb[9]  += PhiRingSum[k10[iii] - 1]; }
  for (int iii = 1; iii <= k11[0]; iii++) { BhabhaComb[10] += PhiRingSum[k11[iii] - 1]; }
  for (int iii = 1; iii <= k12[0]; iii++) { BhabhaComb[11] += PhiRingSum[k12[iii] - 1]; }
  for (int iii = 1; iii <= k13[0]; iii++) { BhabhaComb[12] += PhiRingSum[k13[iii] - 1]; }
  for (int iii = 1; iii <= k14[0]; iii++) { BhabhaComb[13] += PhiRingSum[k14[iii] - 1]; }
  for (int iii = 1; iii <= k15[0]; iii++) { BhabhaComb[14] += PhiRingSum[k15[iii] - 1]; }
  for (int iii = 1; iii <= k16[0]; iii++) { BhabhaComb[15] += PhiRingSum[k16[iii] - 1]; }
  for (int iii = 1; iii <= k17[0]; iii++) { BhabhaComb[16] += PhiRingSum[k17[iii] - 1]; }
  for (int iii = 1; iii <= k18[0]; iii++) { BhabhaComb[17] += PhiRingSum[k18[iii] - 1]; }



  BtoBflag  =
    (BhabhaComb[0]  > 5.0 ||
     BhabhaComb[1]  > 3.0 ||
     BhabhaComb[2]  > 5.0 ||
     BhabhaComb[3]  > 4.0 ||
     BhabhaComb[4]  > 5.0 ||
     BhabhaComb[5]  > 5.0 ||
     BhabhaComb[6]  > 5.0 ||
     BhabhaComb[7]  > 5.0 ||
     BhabhaComb[8]  > 5.0 ||
     BhabhaComb[9]  > 5.0 ||
     BhabhaComb[10] > 5.0 ||
     BhabhaComb[11] > 5.0 ||
     BhabhaComb[12] > 5.0 ||
     BhabhaComb[13] > 5.0 ||
     BhabhaComb[14] > 5.0 ||
     BhabhaComb[15] > 5.0 ||
     BhabhaComb[16] > 3.0 ||
     BhabhaComb[17] > 3.0);
  //----------

  return  BtoBflag;
}
bool TrgEclBhabha::GetBhabha01()
{
  //-----------------------
  // 3D Bhabha veto
  //------------------------
  bool BtoBFlag = false;
  bool GapFlag = false;
  bool ThetaBtoBFlag = false;
  bool PhiBtoBFlag = false;
  bool BhabhaFlag = false;

  //
  //
  // Read Cluster Table
  //
  //
  ClusterEnergy.clear();
  ClusterTiming.clear();
  ClusterPosition.clear();
  //  int EventId = 0;
  StoreArray<TRGECLCluster> trgeclClusterArray;
  for (int ii = 0; ii < trgeclClusterArray.getEntries(); ii++) {
    TRGECLCluster* aTRGECLCluster = trgeclClusterArray[ii];
    //  EventId = aTRGECLCluster ->getEventId();
    int maxTCId    = aTRGECLCluster ->getMaxTCId();
    double clusterenergy  = aTRGECLCluster ->getEnergyDep();
    double clustertiming  =  aTRGECLCluster -> getTimeAve();
    TVector3 clusterposition(aTRGECLCluster ->getPositionX(), aTRGECLCluster ->getPositionY(), aTRGECLCluster ->getPositionZ());
    ClusterTiming.push_back(clustertiming);
    ClusterEnergy.push_back(clusterenergy);
    ClusterPosition.push_back(clusterposition);
    MaxTCId.push_back(maxTCId);
  }
  const int hit_size = ClusterEnergy.size();
  //
  //
  //
  //Find 2 energetic TCs in a event
  //

  std::vector<int> Max2TCId;
  std::vector<double> Max2Energy;
  std::vector<double> Max2Timing;
  std::vector<double> Max2PositionR;
  std::vector<double> Max2PositionTheta;
  std::vector<double> Max2PositionPhi;
  std::vector<int> Max2ThetaId;
  std::vector<int> Max2PhiId;


  Max2TCId.clear();
  Max2Energy.clear();
  Max2Timing.clear();
  Max2PositionR.clear();
  Max2PositionTheta.clear();
  Max2PositionPhi.clear();
  Max2ThetaId.clear();
  Max2PhiId.clear();

  Max2TCId.resize(2, 0);
  Max2ThetaId.resize(2, 0);
  Max2PhiId.resize(2, 0);
  Max2Energy.resize(2, 0.0);
  Max2Timing.resize(2, 0.0);
  Max2PositionR.resize(2, 0.0);
  Max2PositionTheta.resize(2, 0.0);
  Max2PositionPhi.resize(2, 0.0);

  for (int ihit = 0; ihit < hit_size; ihit ++) {
    if (_TCMap ->getTCThetaIdFromTCId(MaxTCId[ihit]) > 8) {continue;}
    if (Max2Energy[0] < ClusterEnergy[ihit]) {
      Max2TCId[0] = MaxTCId[ihit];
      Max2ThetaId[0] = _TCMap ->getTCThetaIdFromTCId(Max2TCId[0]) + 1;
      Max2PhiId[0] = _TCMap ->getTCPhiIdFromTCId(Max2TCId[0]) + 1;
      Max2Energy[0] = ClusterEnergy[ihit];
      Max2Timing[0] = ClusterTiming[ihit];
      Max2PositionR[0] = ClusterPosition[ihit].Mag();
      Max2PositionTheta[0] = ClusterPosition[ihit].Theta();
      Max2PositionPhi[0] = ClusterPosition[ihit].Phi();

    }
  }
  for (int ihit = 0; ihit < hit_size; ihit ++) {
    if (_TCMap ->getTCThetaIdFromTCId(MaxTCId[ihit]) < 7) {continue;}
    if (Max2Energy[1] < ClusterEnergy[ihit] && Max2TCId[0] != MaxTCId[ihit]) {
      Max2TCId[1] = MaxTCId[ihit];
      Max2Energy[1] = ClusterEnergy[ihit];
      Max2Timing[1] = ClusterTiming[ihit];
      Max2PositionR[1] = ClusterPosition[ihit].Mag();
      Max2PositionTheta[1] = ClusterPosition[ihit].Theta();
      Max2PositionPhi[1] = ClusterPosition[ihit].Phi();
      Max2ThetaId[1] = _TCMap ->getTCThetaIdFromTCId(Max2TCId[1]) + 1;
      Max2PhiId[1] = _TCMap ->getTCPhiIdFromTCId(Max2TCId[1]) + 1;

    }
  }
  //
  if (Max2Energy[0] == 0 && Max2Energy[1] == 0) {return BtoBFlag;}
  //
  //find Back to Back though theta direction
  //
  if (Max2Energy[0] != 0 && Max2Energy[0] != 0) {
    if ((Max2ThetaId[0] + Max2ThetaId[1]) > 14 && (Max2ThetaId[0] + Max2ThetaId[1]) < 24) {
      ThetaBtoBFlag = true;
    }
    // for(int iCluster= 1; iCluster<thetacomb[Max2ThetaId[0]-1][0]+1;iCluster++) {
    //  if (abs(Max2ThetaId[1]-thetacomb[Max2ThetaId[0]-1][iCluster])<3){
    //         ThetaBtoBFlag = true;
    //  }

    if (abs(Max2PhiId[0] - Max2PhiId[1]) > 12 && abs(Max2PhiId[0] - Max2PhiId[1]) < 22) {
      PhiBtoBFlag = true;
    }
  }

  //Check Gap Bhabha
  //
  // FWD Gap :  (E1<0.1GeV or E1==0(no hit) ) && (thetaId1==3 or ThetaId1 ==4 (TC near fwdGap))  && (ThetaId2 ==15 or ThetaId2 ==16 or ThetaId2 ==17)  && E2 > 3 GeV
  //
  // BWD Gap : ( E2<0.1GeV or E2 ==0 ) && (thetaId2==14 or ThetaId2 ==15 (TC near bwdGap) )  && ( ThetaId1 == 2 or ThetaId1 == 3 or ThetaId1 == 4)  && E1 > 4 GeV
  if ((Max2Energy[0] < 0.1) && (Max2ThetaId[0] == 3 || Max2ThetaId[0] == 4 || Max2Energy[0] == 0) && (Max2ThetaId[1] == 15
      || Max2ThetaId[1] == 16 || Max2ThetaId[1] == 17) && Max2Energy[1] > 1) {
    GapFlag = true;
  } else if ((Max2Energy[1] < 0.1) && (Max2ThetaId[1] == 14 || Max2ThetaId[1] == 15 || Max2Energy[1] == 0) && (Max2ThetaId[0] == 2
             || Max2ThetaId[0] == 3 || Max2ThetaId[0] == 4) && Max2Energy[0] > 1) {
    GapFlag = true;
  }



  if (ThetaBtoBFlag && PhiBtoBFlag) {
    BtoBFlag = true;
  }
  if ((BtoBFlag && (Max2Energy[0] + Max2Energy[1]) > 4 && (Max2Energy[0] > 1 && Max2Energy[1] > 1)) || GapFlag) {
    BhabhaFlag = true;

  }


  return BhabhaFlag;
}

//
//===<END>
//
