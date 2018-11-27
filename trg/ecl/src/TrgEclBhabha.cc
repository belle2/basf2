//---------------------------------------------------------------
// $Id$
//---------------------------------------------------------------
// Filename : TrgEclBhabha.cc
// Section  : TRG ECL
// Owner    : InSoo Lee/Yuuji Unno
// Email    : islee@hep.hanyang.ac.kr / yunno@post.kek.jp
//---------------------------------------------------------------
// Description : A class to tag Bhabha Veto
//
//    Belle type Bhabha (Bhbha00())
//
//    Theta Id combination
// bit id | combination(FWD|BWD) | Energy Threshold (FWD|BWD) GeV
//      0 | 1+2+3    |   16+17   |    4.0   |   2.5
//      1 |     3    |      15   |    4.0   |   2.5
//      2 |     4    |   14+15   |    4.0   |   2.5
//      3 |     5    |   14+15   |    4.0   |   2.5
//      4 |   4+5    |      14   |    4.0   |   2.5
//      5 |     5    |   13+14   |    4.0   |   2.5
//      6 |     5    |   12+13   |    4.0   |   2.5
//      7 |   5+6    |      13   |    4.0   |   2.5
//      8 |   5+6    |      12   |    4.0   |   2.5
//      9 |   6+7    |      12   |    4.0   |   2.5
//     10 |   6+7    |      11   |    4.0   |   2.5
//     11 |   7+8    |      11   |    4.0   |   2.5
//     12 |     8    |   10+11   |    3.0   |   3.0
//     13 |   8+9    |    9+10   |    3.5   |   3.0
//
//    Belle 2 3D Bhabha (Bhabha01)
//
// - Cluster base Logic
// - Compare all clusters satisfying following conditions
//  160 degree < (CM Phi_Cluster 1 - CM Phi_Cluster 2) < 200 degree
//  165 degree < (CM Theta Cluster 1 + CM Theta Cluster 2 ) < 190 degree
//  Boths Cluster CM E > 3 GeV  and One of cluster CM E > 4.5 GeV
//
//
//---------------------------------------------------------------
// $Log$ 2018-08-24 First Version
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

  _database = new TrgEclDataBase();

}

TrgEclBhabha::~TrgEclBhabha()
{
  delete _TCMap;
}
bool TrgEclBhabha::GetBhabha00(std::vector<double> PhiRingSum)  //Belle 2D Bhabha veto method
{
  bool BtoBflag = false;


  vector<int> k011 = {3, 1, 2, 3 };    // (1)  F1+F2 + F3 + B1+B2
  vector<int> k012 = {2, 16, 17};    // (1)  F1+F2 + F3 + B1+B2

  vector<int> k021 = {1,  3}; // (2)  F3 + C12
  vector<int> k022 = {1, 15}; // (2)  F3 + C12

  vector<int> k03 = {2,  2, 3};                // (3)  F2 + F3

  vector<int> k04 = {1,  4};    // (4)  C1 + backward gap

  vector<int> k051 = {1,  4 };        // (5)  C1+C11+C12
  vector<int> k052 = {2, 14, 15};        // (5)  C1+C11+C12

  vector<int> k061 = {1,  5};         // (6)  C2+C11+C12
  vector<int> k062 = {2,  14, 15};         // (6)  C2+C11+C12

  vector<int> k071 = {2,  4, 5};        // (7)  C1+C2+C11
  vector<int> k072 = {1,  14};        // (7)  C1+C2+C11

  vector<int> k081 = {1,  5};        // (8)  C2+C10+C11
  vector<int> k082 = {2, 13, 14};        // (8)  C2+C10+C11

  vector<int> k091 = {1,  5 };         // (9)  C2+C9+C10
  vector<int> k092 = {2, 12, 13};         // (9)  C2+C9+C10

  vector<int> k101 = {2,  5, 6};             // (10) C2+C3+C10
  vector<int> k102 = {1,  13};             // (10) C2+C3+C10

  vector<int> k111 = {2,  5, 6};                // (11) C2+C3+C9
  vector<int> k112 = {1,  12};                // (11) C2+C3+C9

  vector<int> k121 = {2,  6, 7};        // (9)  C3+C4+C9
  vector<int> k122 = {1,  12};         // (9)  C3+C4+C9

  vector<int> k131 = {2,  6, 7};             // (10) C3+C4+C8
  vector<int> k132 = {1,  11};             // (10) C3+C4+C8

  vector<int> k141 = {2,  7, 8};                // (11) C4+C5+C8
  vector<int> k142 = {1,  11};                // (11) C4+C5+C8


  vector<int> k151 = {1,  8};         // (9)  C5+C7+C8
  vector<int> k152 = {2, 10, 11};         // (9)  C5+C7+C8

  vector<int> k161 = {2,  8, 9};             // (10) C5+C6+C7
  vector<int> k162 = {2,  9, 10};             // (10) C5+C6+C7

  vector<int> k17 = {2, 14, 15};                // (11) C11+C12 +forward gap

  vector<int> k18 = {1, 16};                // (11) B1 + forward gap

  vector<int> kLOM1 = {2,  2, 3 };    // (1)  F1+F2 + F3 + B1+B2
  vector<int> kLOM2 = {2, 16, 17};    // (1)  F1+F2 + F3 + B1+B2

  BhabhaComb.clear();
  BhabhaComb.resize(32, 0.0);
  for (int iii = 1; iii <= k011[0]; iii++) { BhabhaComb[0]  += PhiRingSum[k011[iii] - 1];}
  for (int iii = 1; iii <= k012[0]; iii++) { BhabhaComb[1]  += PhiRingSum[k012[iii] - 1];}
  for (int iii = 1; iii <= k021[0]; iii++) { BhabhaComb[2]  += PhiRingSum[k021[iii] - 1]; }
  for (int iii = 1; iii <= k022[0]; iii++) { BhabhaComb[3]  += PhiRingSum[k022[iii] - 1]; }
  for (int iii = 1; iii <= k03[0]; iii++) { BhabhaComb[4]  += PhiRingSum[k03[iii] - 1]; }
  for (int iii = 1; iii <= k04[0]; iii++) { BhabhaComb[5]  += PhiRingSum[k04[iii] - 1]; }
  for (int iii = 1; iii <= k051[0]; iii++) { BhabhaComb[6]  += PhiRingSum[k051[iii] - 1]; }
  for (int iii = 1; iii <= k052[0]; iii++) { BhabhaComb[7]  += PhiRingSum[k052[iii] - 1]; }
  for (int iii = 1; iii <= k061[0]; iii++) { BhabhaComb[8]  += PhiRingSum[k061[iii] - 1]; }
  for (int iii = 1; iii <= k062[0]; iii++) { BhabhaComb[9]  += PhiRingSum[k062[iii] - 1]; }
  for (int iii = 1; iii <= k071[0]; iii++) { BhabhaComb[10]  += PhiRingSum[k071[iii] - 1]; }
  for (int iii = 1; iii <= k072[0]; iii++) { BhabhaComb[11]  += PhiRingSum[k072[iii] - 1]; }
  for (int iii = 1; iii <= k081[0]; iii++) { BhabhaComb[12]  += PhiRingSum[k081[iii] - 1]; }
  for (int iii = 1; iii <= k082[0]; iii++) { BhabhaComb[13]  += PhiRingSum[k082[iii] - 1]; }
  for (int iii = 1; iii <= k091[0]; iii++) { BhabhaComb[14]  += PhiRingSum[k091[iii] - 1]; }
  for (int iii = 1; iii <= k092[0]; iii++) { BhabhaComb[15]  += PhiRingSum[k092[iii] - 1]; }
  for (int iii = 1; iii <= k101[0]; iii++) { BhabhaComb[16]  += PhiRingSum[k101[iii] - 1]; }
  for (int iii = 1; iii <= k102[0]; iii++) { BhabhaComb[17]  += PhiRingSum[k102[iii] - 1]; }
  for (int iii = 1; iii <= k111[0]; iii++) { BhabhaComb[18] += PhiRingSum[k111[iii] - 1]; }
  for (int iii = 1; iii <= k112[0]; iii++) { BhabhaComb[19] += PhiRingSum[k112[iii] - 1]; }
  for (int iii = 1; iii <= k121[0]; iii++) { BhabhaComb[20] += PhiRingSum[k121[iii] - 1]; }
  for (int iii = 1; iii <= k122[0]; iii++) { BhabhaComb[21] += PhiRingSum[k122[iii] - 1]; }
  for (int iii = 1; iii <= k131[0]; iii++) { BhabhaComb[22] += PhiRingSum[k131[iii] - 1]; }
  for (int iii = 1; iii <= k132[0]; iii++) { BhabhaComb[23] += PhiRingSum[k132[iii] - 1]; }
  for (int iii = 1; iii <= k141[0]; iii++) { BhabhaComb[24] += PhiRingSum[k141[iii] - 1]; }
  for (int iii = 1; iii <= k142[0]; iii++) { BhabhaComb[25] += PhiRingSum[k142[iii] - 1]; }
  for (int iii = 1; iii <= k151[0]; iii++) { BhabhaComb[26] += PhiRingSum[k151[iii] - 1]; }
  for (int iii = 1; iii <= k152[0]; iii++) { BhabhaComb[27] += PhiRingSum[k152[iii] - 1]; }
  for (int iii = 1; iii <= k161[0]; iii++) { BhabhaComb[28] += PhiRingSum[k161[iii] - 1]; }
  for (int iii = 1; iii <= k162[0]; iii++) { BhabhaComb[29] += PhiRingSum[k162[iii] - 1]; }
  for (int iii = 1; iii <= kLOM1[0]; iii++) { BhabhaComb[30]  += PhiRingSum[kLOM1[iii] - 1];}
  for (int iii = 1; iii <= kLOM2[0]; iii++) { BhabhaComb[31]  += PhiRingSum[kLOM2[iii] - 1];}


  BtoBflag  =
    ((BhabhaComb[0]  > 4 && BhabhaComb[1]  > 2.5) ||
     (BhabhaComb[2]  > 4 && BhabhaComb[3]  > 2.5) ||
     (BhabhaComb[6]  > 4 &&  BhabhaComb[7]  > 2.5) ||
     (BhabhaComb[8]  > 4 &&  BhabhaComb[9]  > 2.5) ||
     (BhabhaComb[10]  > 4 &&  BhabhaComb[11]  > 2.5) ||
     (BhabhaComb[12]  > 4 &&  BhabhaComb[13]  > 2.5) ||
     (BhabhaComb[14]  > 4 &&  BhabhaComb[15]  > 2.5) ||
     (BhabhaComb[16]  > 4 &&  BhabhaComb[17]  > 2.5) ||
     (BhabhaComb[18]  > 4 &&  BhabhaComb[19]  > 2.5) ||
     (BhabhaComb[20]  > 4 &&  BhabhaComb[21]  > 2.5) ||
     (BhabhaComb[22]  > 4 &&  BhabhaComb[23]  > 2.5) ||
     (BhabhaComb[24]  > 4 &&  BhabhaComb[25]  > 2.5) ||
     (BhabhaComb[26]  > 3 &&  BhabhaComb[27]  > 3) ||
     (BhabhaComb[28]  > 3.5 &&  BhabhaComb[29]  > 3));

  int bhabha01 = 0;
  int bhabha02 = 0;
  int bhabha03 = 0;
  int bhabha04 = 0;
  int bhabha05 = 0;
  int bhabha06 = 0;
  int bhabha07 = 0;
  int bhabha08 = 0;
  int bhabha09 = 0;
  int bhabha10 = 0;
  int bhabha11 = 0;
  int bhabha12 = 0;
  int bhabha13 = 0;
  int bhabha14 = 0;

  if ((BhabhaComb[0]  > 4.0 && BhabhaComb[1]  > 2.500)) {bhabha01 = 1;}
  if ((BhabhaComb[2]  > 4.0 && BhabhaComb[3]  > 2.500)) {bhabha02 = 1;}
  if ((BhabhaComb[6]  > 4.0 && BhabhaComb[7]  > 2.500)) {bhabha03 = 1;}
  if ((BhabhaComb[8]  > 4.0 && BhabhaComb[9]  > 2.500)) {bhabha04 = 1;}
  if ((BhabhaComb[10]  > 4.0 && BhabhaComb[11]  > 2.500)) {bhabha05 = 1;}
  if ((BhabhaComb[12]  > 4.0 && BhabhaComb[13]  > 2.500)) {bhabha06 = 1;}
  if ((BhabhaComb[14]  > 4.0 && BhabhaComb[15]  > 2.500)) {bhabha07 = 1;}
  if ((BhabhaComb[16]  > 4.0 && BhabhaComb[17]  > 2.500)) {bhabha08 = 1;}
  if ((BhabhaComb[18]  > 4.0 && BhabhaComb[19]  > 2.500)) {bhabha09 = 1;}
  if ((BhabhaComb[20]  > 4.0 && BhabhaComb[21]  > 2.500)) {bhabha10 = 1;}
  if ((BhabhaComb[22]  > 4.0 && BhabhaComb[23]  > 2.500)) {bhabha11 = 1;}
  if ((BhabhaComb[24]  > 4.0 && BhabhaComb[25]  > 2.500)) {bhabha12 = 1;}
  if ((BhabhaComb[26]  > 3.000 && BhabhaComb[27]  > 3.000)) {bhabha13 = 1;}
  if ((BhabhaComb[28]  > 3.500 && BhabhaComb[29]  > 3.000)) {bhabha14 = 1;}


  BhabhaComb.clear();
  BhabhaComb.push_back(bhabha01);
  BhabhaComb.push_back(bhabha02);
  BhabhaComb.push_back(bhabha03);
  BhabhaComb.push_back(bhabha04);
  BhabhaComb.push_back(bhabha05);
  BhabhaComb.push_back(bhabha06);
  BhabhaComb.push_back(bhabha07);
  BhabhaComb.push_back(bhabha08);
  BhabhaComb.push_back(bhabha09);
  BhabhaComb.push_back(bhabha10);
  BhabhaComb.push_back(bhabha11);
  BhabhaComb.push_back(bhabha12);
  BhabhaComb.push_back(bhabha13);
  BhabhaComb.push_back(bhabha14);
  BhabhaComb.push_back(0);
  BhabhaComb.push_back(0);
  BhabhaComb.push_back(0);
  BhabhaComb.push_back(0);

  return  BtoBflag;
}

bool TrgEclBhabha::GetBhabha01()
{
  //-----------------------
  // 3D Bhabha veto
  //------------------------
  bool BtoBFlag = false;
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
  const int ncluster = ClusterEnergy.size();
  //
  //
  //
  //
  BhabhaComb.clear();
  BhabhaComb.resize(18, 0);


  for (int icluster = 0; icluster < ncluster ; icluster++) {
    for (int jcluster = icluster + 1; jcluster < ncluster; jcluster ++) {
      BtoBFlag = false;
      BhabhaFlag = false;
      if (icluster == jcluster) {continue;}
      int lut1 = _database->Get3DBhabhaLUT(MaxTCId[icluster]);
      int lut2 = _database->Get3DBhabhaLUT(MaxTCId[jcluster]);
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


      int dphi = abs(phi1 - phi2);
      if (dphi > 180) {dphi = 360 - dphi;}
      int thetaSum = theta1 + theta2;


      if (dphi > 160 && thetaSum > 165 && thetaSum < 190) {BtoBFlag = true;}

      if ((ClusterEnergy[icluster] * 100.) > 30 * energy1 && (ClusterEnergy[jcluster] * 100.) > 30 * (energy2)
          && ((ClusterEnergy[icluster] * 100.) > 45 * energy1 || (ClusterEnergy[jcluster] * 100.) > 45 * (energy2))) {
        if (BtoBFlag) {BhabhaFlag = true;}
      }
      // if (BhabhaFlag && icluster == 0 && jcluster == 1) {BhabhaComb[0] = 1;}
      // else if (BhabhaFlag && icluster == 0 && jcluster == 2) {BhabhaComb[1] = 1;}
      // else if (BhabhaFlag && icluster == 0 && jcluster == 3) {BhabhaComb[2] = 1;}
      // else if (BhabhaFlag && icluster == 0 && jcluster == 4) {BhabhaComb[3] = 1;}
      // else if (BhabhaFlag && icluster == 0 && jcluster == 5) {BhabhaComb[4] = 1;}
      // else if (BhabhaFlag && icluster == 1 && jcluster == 2) {BhabhaComb[5] = 1;}
      // else if (BhabhaFlag && icluster == 1 && jcluster == 3) {BhabhaComb[6] = 1;}
      // else if (BhabhaFlag && icluster == 1 && jcluster == 4) {BhabhaComb[7] = 1;}
      // else if (BhabhaFlag && icluster == 1 && jcluster == 5) {BhabhaComb[8] = 1;}
      // else if (BhabhaFlag && icluster == 2 && jcluster == 3) {BhabhaComb[9] = 1;}
      // else if (BhabhaFlag && icluster == 2 && jcluster == 4) {BhabhaComb[10] = 1;}
      // else if (BhabhaFlag && icluster == 2 && jcluster == 5) {BhabhaComb[11] = 1;}
      // else if (BhabhaFlag && icluster == 3 && jcluster == 4) {BhabhaComb[12] = 1;}
      // else if (BhabhaFlag && icluster == 3 && jcluster == 5) {BhabhaComb[13] = 1;}
      // else if (BhabhaFlag && icluster == 4 && jcluster == 5) {BhabhaComb[14] = 1;}


    }





  }



  return BhabhaFlag;
}
