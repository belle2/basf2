/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

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
//  @ Bhabha for veto
//  160 degree < (CM Phi_Cluster 1 - CM Phi_Cluster 2) < 200 degree
//  165 degree < (CM Theta Cluster 1 + CM Theta Cluster 2 ) < 190 degree
//  Both Cluster CM E > 3 GeV  and One of cluster CM E > 4.5 GeV
//  @ Bhabha for calibration  (selection bhabha)
//  140 degree < (CM Phi_Cluster 1 - CM Phi_Cluster 2) < 220 degree
//  160 degree < (CM Theta Cluster 1 + CM Theta Cluster 2 ) < 200 degree
//  Both Cluster CM E > 2.5 GeV  and One of cluster CM E > 4.0 GeV
//
//    ee->mumu selection
//  160 degree < (CM Phi_Cluster 1 - CM Phi_Cluster 2) < 200 degree
//  165 degree < (CM Theta Cluster 1 + CM Theta Cluster 2 ) < 190 degree
//  Both Cluster CM E < 2 GeV  and One of cluster CM E < 2 GeV
//
//---------------------------------------------------------------

#define TRG_SHORT_NAMES
#define TRGECLCLUSTER_SHORT_NAMES
#include <framework/gearbox/Unit.h>
#include "framework/datastore/StoreArray.h"

#include <trg/ecl/TrgEclBhabha.h>

#include "trg/ecl/dataobjects/TRGECLCluster.h"

#include <analysis/utility/PCmsLabTransform.h>

using namespace std;
using namespace Belle2;
//
//
//
TrgEclBhabha::TrgEclBhabha()
{

  m_BhabhaComb.clear();
  m_MaxTCId.clear();
  m_ClusterEnergyLab.clear();
  m_ClusterTiming.clear();

  m_TCMap = new TrgEclMapping();

  m_database = new TrgEclDataBase();

  m_2DBhabhaThresholdFWD.clear();
  m_2DBhabhaThresholdBWD.clear();
  m_3DBhabhaSelectionThreshold.clear();
  m_3DBhabhaVetoThreshold.clear();
  m_3DBhabhaSelectionAngle.clear();
  m_3DBhabhaVetoAngle.clear();
  m_mumuAngle.clear();
  m_hie12BhabhaVetoAngle.clear();
  m_3DBhabhaVetoClusterTCIds.clear();
  m_3DBhabhaVetoClusterEnergies.clear();
  m_3DBhabhaVetoClusterTimings.clear();
  m_3DBhabhaVetoClusterThetaIds.clear();
  m_3DBhabhaVetoInTrackThetaRegion.clear();

  m_2DBhabhaThresholdFWD =
  {4.0, 4.0, 4.0, 4.0, 4.0, 4.0, 4.0, 4.0, 4.0, 4.0, 4.0, 4.0, 3.0, 3.5}; // GeV
  m_2DBhabhaThresholdBWD =
  {2.5, 2.5, 2.5, 2.5, 2.5, 2.5, 2.5, 2.5, 2.5, 2.5, 2.5, 2.5, 3.0, 3.0}; // GeV
  m_3DBhabhaVetoThreshold      = {3.0, 4.5}; // GeV
  m_3DBhabhaVetoAngle          = {160, 200, 165, 190}; // (phi low, high, theta low, high) (degree)
  m_3DBhabhaSelectionThreshold = {2.5, 4.0}; // GeV
  m_3DBhabhaSelectionAngle     = {140, 220, 160, 200}; // (phi low, high, theta low, high) (degree)
  m_3DBhabhaSelectionPreScale  = {1, 1, 1}; // (fw, br, bw)

  m_mumuThreshold          = 2.0; // GeV
  m_mumuAngle              = {160, 200, 165, 190}; // (phi low, high, theta low, high) (degree)

  m_3DBhabhaVetoInTrackThetaRegion = {3, 15}; // theta ID

  m_taub2bAngleCut  = {110, 250, 130, 230}; // degree
  m_taub2bCLELabCut = 1.9; // GeV
  m_taub2bEtotCut   = 7.0; // GeV

  m_hie12BhabhaVetoAngle   = {150, 210, 160, 200}; // (phi low, high, theta low, high) (degree)

  m_taub2b2AngleCut  = {120, 240, 140, 220}; // degree
  m_taub2b2CLELabCut = {3.0, 0.162}; // GeV
  m_taub2b2EtotCut   = 7.0; // GeV

  m_taub2b3AngleCut     = {120, 240, 140, 220}; // degree
  m_taub2b3EtotCut      = 7.0; // GeV
  m_taub2b3CLEb2bLabCut = 0.14; // GeV
  m_taub2b3CLELabCut    = {0.12, 4.5}; // GeV

  m_hie4LowCLELabCut = 0.5; //GeV
  //
  m_3DBhabhaVetoInTrackFlag = -10;

  m_taub2bAngleFlag = 0;
  m_taub2bEtotFlag  = 0;
  m_taub2bClusterEFlag = 0;

}
//
//
//
TrgEclBhabha::~TrgEclBhabha()
{
  delete m_TCMap;
  delete m_database;
}
//
//
//
bool TrgEclBhabha::getBhabha00(std::vector<double> PhiRingSum)  //Belle 2D Bhabha veto method
{
  bool BtoBflag = false;

  vector<int> k011 = {3, 1, 2, 3 };   // (1)  F1+F2 + F3 + B1+B2
  vector<int> k012 = {2, 16, 17};     // (1)  F1+F2 + F3 + B1+B2

  vector<int> k021 = {1,  3};         // (2)  F3 + C12
  vector<int> k022 = {1, 15};         // (2)  F3 + C12

  vector<int> k03 = {2,  2, 3};       // (3)  F2 + F3

  vector<int> k04 = {1,  4};          // (4)  C1 + backward gap

  vector<int> k051 = {1,  4 };        // (5)  C1+C11+C12
  vector<int> k052 = {2, 14, 15};     // (5)  C1+C11+C12

  vector<int> k061 = {1,  5};         // (6)  C2+C11+C12
  vector<int> k062 = {2,  14, 15};    // (6)  C2+C11+C12

  vector<int> k071 = {2,  4, 5};      // (7)  C1+C2+C11
  vector<int> k072 = {1,  14};        // (7)  C1+C2+C11

  vector<int> k081 = {1,  5};         // (8)  C2+C10+C11
  vector<int> k082 = {2, 13, 14};     // (8)  C2+C10+C11

  vector<int> k091 = {1,  5 };        // (9)  C2+C9+C10
  vector<int> k092 = {2, 12, 13};     // (9)  C2+C9+C10

  vector<int> k101 = {2,  5, 6};      // (10) C2+C3+C10
  vector<int> k102 = {1,  13};        // (10) C2+C3+C10

  vector<int> k111 = {2,  5, 6};      // (11) C2+C3+C9
  vector<int> k112 = {1,  12};        // (11) C2+C3+C9

  vector<int> k121 = {2,  6, 7};      // (9)  C3+C4+C9
  vector<int> k122 = {1,  12};        // (9)  C3+C4+C9

  vector<int> k131 = {2,  6, 7};      // (10) C3+C4+C8
  vector<int> k132 = {1,  11};        // (10) C3+C4+C8

  vector<int> k141 = {2,  7, 8};      // (11) C4+C5+C8
  vector<int> k142 = {1,  11};        // (11) C4+C5+C8


  vector<int> k151 = {1,  8};         // (9)  C5+C7+C8
  vector<int> k152 = {2, 10, 11};     // (9)  C5+C7+C8

  vector<int> k161 = {2,  8, 9};      // (10) C5+C6+C7
  vector<int> k162 = {2,  9, 10};     // (10) C5+C6+C7

  //vector<int> k17 = {2, 14, 15};      // (11) C11+C12 +forward gap

  //vector<int> k18 = {1, 16};          // (11) B1 + forward gap

  vector<int> kLOM1 = {2,  2, 3 };    // (1)  F1+F2 + F3 + B1+B2
  vector<int> kLOM2 = {2, 16, 17};    // (1)  F1+F2 + F3 + B1+B2

  m_BhabhaComb.clear();
  m_BhabhaComb.resize(32, 0.0);
  for (int iii = 1; iii <= k011[0];  iii++) { m_BhabhaComb[0]  += PhiRingSum[k011[iii]  - 1];}
  for (int iii = 1; iii <= k012[0];  iii++) { m_BhabhaComb[1]  += PhiRingSum[k012[iii]  - 1];}
  for (int iii = 1; iii <= k021[0];  iii++) { m_BhabhaComb[2]  += PhiRingSum[k021[iii]  - 1];}
  for (int iii = 1; iii <= k022[0];  iii++) { m_BhabhaComb[3]  += PhiRingSum[k022[iii]  - 1];}
  for (int iii = 1; iii <= k03[0];   iii++) { m_BhabhaComb[4]  += PhiRingSum[k03[iii]   - 1];}
  for (int iii = 1; iii <= k04[0];   iii++) { m_BhabhaComb[5]  += PhiRingSum[k04[iii]   - 1];}
  for (int iii = 1; iii <= k051[0];  iii++) { m_BhabhaComb[6]  += PhiRingSum[k051[iii]  - 1];}
  for (int iii = 1; iii <= k052[0];  iii++) { m_BhabhaComb[7]  += PhiRingSum[k052[iii]  - 1];}
  for (int iii = 1; iii <= k061[0];  iii++) { m_BhabhaComb[8]  += PhiRingSum[k061[iii]  - 1];}
  for (int iii = 1; iii <= k062[0];  iii++) { m_BhabhaComb[9]  += PhiRingSum[k062[iii]  - 1];}
  for (int iii = 1; iii <= k071[0];  iii++) { m_BhabhaComb[10] += PhiRingSum[k071[iii]  - 1];}
  for (int iii = 1; iii <= k072[0];  iii++) { m_BhabhaComb[11] += PhiRingSum[k072[iii]  - 1];}
  for (int iii = 1; iii <= k081[0];  iii++) { m_BhabhaComb[12] += PhiRingSum[k081[iii]  - 1];}
  for (int iii = 1; iii <= k082[0];  iii++) { m_BhabhaComb[13] += PhiRingSum[k082[iii]  - 1];}
  for (int iii = 1; iii <= k091[0];  iii++) { m_BhabhaComb[14] += PhiRingSum[k091[iii]  - 1];}
  for (int iii = 1; iii <= k092[0];  iii++) { m_BhabhaComb[15] += PhiRingSum[k092[iii]  - 1];}
  for (int iii = 1; iii <= k101[0];  iii++) { m_BhabhaComb[16] += PhiRingSum[k101[iii]  - 1];}
  for (int iii = 1; iii <= k102[0];  iii++) { m_BhabhaComb[17] += PhiRingSum[k102[iii]  - 1];}
  for (int iii = 1; iii <= k111[0];  iii++) { m_BhabhaComb[18] += PhiRingSum[k111[iii]  - 1];}
  for (int iii = 1; iii <= k112[0];  iii++) { m_BhabhaComb[19] += PhiRingSum[k112[iii]  - 1];}
  for (int iii = 1; iii <= k121[0];  iii++) { m_BhabhaComb[20] += PhiRingSum[k121[iii]  - 1];}
  for (int iii = 1; iii <= k122[0];  iii++) { m_BhabhaComb[21] += PhiRingSum[k122[iii]  - 1];}
  for (int iii = 1; iii <= k131[0];  iii++) { m_BhabhaComb[22] += PhiRingSum[k131[iii]  - 1];}
  for (int iii = 1; iii <= k132[0];  iii++) { m_BhabhaComb[23] += PhiRingSum[k132[iii]  - 1];}
  for (int iii = 1; iii <= k141[0];  iii++) { m_BhabhaComb[24] += PhiRingSum[k141[iii]  - 1];}
  for (int iii = 1; iii <= k142[0];  iii++) { m_BhabhaComb[25] += PhiRingSum[k142[iii]  - 1];}
  for (int iii = 1; iii <= k151[0];  iii++) { m_BhabhaComb[26] += PhiRingSum[k151[iii]  - 1];}
  for (int iii = 1; iii <= k152[0];  iii++) { m_BhabhaComb[27] += PhiRingSum[k152[iii]  - 1];}
  for (int iii = 1; iii <= k161[0];  iii++) { m_BhabhaComb[28] += PhiRingSum[k161[iii]  - 1];}
  for (int iii = 1; iii <= k162[0];  iii++) { m_BhabhaComb[29] += PhiRingSum[k162[iii]  - 1];}
  for (int iii = 1; iii <= kLOM1[0]; iii++) { m_BhabhaComb[30] += PhiRingSum[kLOM1[iii] - 1];}
  for (int iii = 1; iii <= kLOM2[0]; iii++) { m_BhabhaComb[31] += PhiRingSum[kLOM2[iii] - 1];}


  BtoBflag  =
    ((m_BhabhaComb[0]  >= m_2DBhabhaThresholdFWD[0] &&
      m_BhabhaComb[1]  >= m_2DBhabhaThresholdBWD[0]) ||
     (m_BhabhaComb[2]  >= m_2DBhabhaThresholdFWD[1] &&
      m_BhabhaComb[3]  >= m_2DBhabhaThresholdBWD[1]) ||
     (m_BhabhaComb[6]  >= m_2DBhabhaThresholdFWD[2] &&
      m_BhabhaComb[7]  >= m_2DBhabhaThresholdBWD[2]) ||
     (m_BhabhaComb[8]  >= m_2DBhabhaThresholdFWD[3] &&
      m_BhabhaComb[9]  >= m_2DBhabhaThresholdBWD[3]) ||
     (m_BhabhaComb[10] >= m_2DBhabhaThresholdFWD[4] &&
      m_BhabhaComb[11] >= m_2DBhabhaThresholdBWD[4]) ||
     (m_BhabhaComb[12] >= m_2DBhabhaThresholdFWD[5] &&
      m_BhabhaComb[13] >= m_2DBhabhaThresholdBWD[5]) ||
     (m_BhabhaComb[14] >= m_2DBhabhaThresholdFWD[6] &&
      m_BhabhaComb[15] >= m_2DBhabhaThresholdBWD[6]) ||
     (m_BhabhaComb[16] >= m_2DBhabhaThresholdFWD[7] &&
      m_BhabhaComb[17] >= m_2DBhabhaThresholdBWD[7]) ||
     (m_BhabhaComb[18] >= m_2DBhabhaThresholdFWD[8] &&
      m_BhabhaComb[19] >= m_2DBhabhaThresholdBWD[8]) ||
     (m_BhabhaComb[20] >= m_2DBhabhaThresholdFWD[9] &&
      m_BhabhaComb[21] >= m_2DBhabhaThresholdBWD[9]) ||
     (m_BhabhaComb[22] >= m_2DBhabhaThresholdFWD[10] &&
      m_BhabhaComb[23] >= m_2DBhabhaThresholdBWD[10]) ||
     (m_BhabhaComb[24] >= m_2DBhabhaThresholdFWD[11] &&
      m_BhabhaComb[25] >= m_2DBhabhaThresholdBWD[11]) ||
     (m_BhabhaComb[26] >= m_2DBhabhaThresholdFWD[12] &&
      m_BhabhaComb[27] >= m_2DBhabhaThresholdBWD[12]) ||
     (m_BhabhaComb[28] >= m_2DBhabhaThresholdFWD[13] &&
      m_BhabhaComb[29] >= m_2DBhabhaThresholdBWD[13]));

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

  if ((m_BhabhaComb[0]  >= m_2DBhabhaThresholdFWD[0] &&
       m_BhabhaComb[1]  >= m_2DBhabhaThresholdBWD[0]))  {bhabha01 = 1;}
  if ((m_BhabhaComb[2]  >= m_2DBhabhaThresholdFWD[1] &&
       m_BhabhaComb[3]  >= m_2DBhabhaThresholdBWD[1]))  {bhabha02 = 1;}
  if ((m_BhabhaComb[6]  >= m_2DBhabhaThresholdFWD[2] &&
       m_BhabhaComb[7]  >= m_2DBhabhaThresholdBWD[2]))  {bhabha03 = 1;}
  if ((m_BhabhaComb[8]  >= m_2DBhabhaThresholdFWD[3] &&
       m_BhabhaComb[9]  >= m_2DBhabhaThresholdBWD[3]))  {bhabha04 = 1;}
  if ((m_BhabhaComb[10] >= m_2DBhabhaThresholdFWD[4] &&
       m_BhabhaComb[11] >= m_2DBhabhaThresholdBWD[4]))  {bhabha05 = 1;}
  if ((m_BhabhaComb[12] >= m_2DBhabhaThresholdFWD[5] &&
       m_BhabhaComb[13] >= m_2DBhabhaThresholdBWD[5]))  {bhabha06 = 1;}
  if ((m_BhabhaComb[14] >= m_2DBhabhaThresholdFWD[6] &&
       m_BhabhaComb[15] >= m_2DBhabhaThresholdBWD[6]))  {bhabha07 = 1;}
  if ((m_BhabhaComb[16] >= m_2DBhabhaThresholdFWD[7] &&
       m_BhabhaComb[17] >= m_2DBhabhaThresholdBWD[7]))  {bhabha08 = 1;}
  if ((m_BhabhaComb[18] >= m_2DBhabhaThresholdFWD[8] &&
       m_BhabhaComb[19] >= m_2DBhabhaThresholdBWD[8]))  {bhabha09 = 1;}
  if ((m_BhabhaComb[20] >= m_2DBhabhaThresholdFWD[9] &&
       m_BhabhaComb[21] >= m_2DBhabhaThresholdBWD[9]))  {bhabha10 = 1;}
  if ((m_BhabhaComb[22] >= m_2DBhabhaThresholdFWD[10] &&
       m_BhabhaComb[23] >= m_2DBhabhaThresholdBWD[10])) {bhabha11 = 1;}
  if ((m_BhabhaComb[24] >= m_2DBhabhaThresholdFWD[11] &&
       m_BhabhaComb[25] >= m_2DBhabhaThresholdBWD[11])) {bhabha12 = 1;}
  if ((m_BhabhaComb[26] >= m_2DBhabhaThresholdFWD[12] &&
       m_BhabhaComb[27] >= m_2DBhabhaThresholdBWD[12])) {bhabha13 = 1;}
  if ((m_BhabhaComb[28] >= m_2DBhabhaThresholdFWD[13] &&
       m_BhabhaComb[29] >= m_2DBhabhaThresholdBWD[13])) {bhabha14 = 1;}

  m_BhabhaComb.clear();
  m_BhabhaComb.push_back(bhabha01);
  m_BhabhaComb.push_back(bhabha02);
  m_BhabhaComb.push_back(bhabha03);
  m_BhabhaComb.push_back(bhabha04);
  m_BhabhaComb.push_back(bhabha05);
  m_BhabhaComb.push_back(bhabha06);
  m_BhabhaComb.push_back(bhabha07);
  m_BhabhaComb.push_back(bhabha08);
  m_BhabhaComb.push_back(bhabha09);
  m_BhabhaComb.push_back(bhabha10);
  m_BhabhaComb.push_back(bhabha11);
  m_BhabhaComb.push_back(bhabha12);
  m_BhabhaComb.push_back(bhabha13);
  m_BhabhaComb.push_back(bhabha14);
  m_BhabhaComb.push_back(0);
  m_BhabhaComb.push_back(0);
  m_BhabhaComb.push_back(0);
  m_BhabhaComb.push_back(0);

  return  BtoBflag;
}
//========================================================
// 3D Bhabha veto
//========================================================
bool TrgEclBhabha::getBhabha01()
{
  //
  // Read Cluster Table
  //
  m_MaxTCId.clear();
  m_ClusterEnergyLab.clear();
  m_ClusterTiming.clear();
  m_3DBhabhaVetoInTrackFlag = -1;
  m_3DBhabhaVetoClusterTCIds.clear();
  m_3DBhabhaVetoClusterEnergies.clear();
  m_3DBhabhaVetoClusterTimings.clear();
  m_3DBhabhaVetoClusterThetaIds.clear();
  StoreArray<TRGECLCluster> trgeclClusterArray;
  for (int ii = 0; ii < trgeclClusterArray.getEntries(); ii++) {
    TRGECLCluster* aTRGECLCluster = trgeclClusterArray[ii];
    int maxTCId    = aTRGECLCluster->getMaxTCId();
    double clusterenergy  = aTRGECLCluster->getEnergyDep();
    double clustertiming  =  aTRGECLCluster->getTimeAve();
    m_ClusterTiming.push_back(clustertiming);
    m_ClusterEnergyLab.push_back(clusterenergy);
    m_MaxTCId.push_back(maxTCId);
  }
  const int ncluster = m_ClusterEnergyLab.size();

  int cl_idx1 = -1;
  int cl_idx2 = -1;
  bool BhabhaFlag = false;
  for (int icluster = 0; icluster < ncluster ; icluster++) {
    for (int jcluster = icluster + 1; jcluster < ncluster; jcluster ++) {
      bool BtoBFlag = false;

      if (icluster == jcluster) {continue;}

      int energy1 = 0;
      int energy2 = 0;
      int dphi = 0;
      int thetaSum = 0;
      get2CLETP(m_MaxTCId[icluster],
                m_MaxTCId[jcluster],
                energy1,
                energy2,
                dphi,
                thetaSum);

      if (dphi     > m_3DBhabhaVetoAngle[0] &&
          dphi     < m_3DBhabhaVetoAngle[1] &&
          thetaSum > m_3DBhabhaVetoAngle[2] &&
          thetaSum < m_3DBhabhaVetoAngle[3]) {
        BtoBFlag = true;
      }

      // CL E threshold cut
      if ((m_ClusterEnergyLab[icluster] > m_3DBhabhaVetoThreshold[0] * energy1 * 0.1 &&
           m_ClusterEnergyLab[jcluster] > m_3DBhabhaVetoThreshold[0] * energy2 * 0.1) &&
          (m_ClusterEnergyLab[icluster] > m_3DBhabhaVetoThreshold[1] * energy1 * 0.1 ||
           m_ClusterEnergyLab[jcluster] > m_3DBhabhaVetoThreshold[1] * energy2 * 0.1)) {
        if (BtoBFlag) {
          BhabhaFlag = true;
          cl_idx1 = icluster;
          cl_idx2 = jcluster;
        }
      }
    }
  }

  if (BhabhaFlag) {
    m_3DBhabhaVetoClusterTCIds.push_back(m_MaxTCId[cl_idx1]);
    m_3DBhabhaVetoClusterTCIds.push_back(m_MaxTCId[cl_idx2]);
    m_3DBhabhaVetoClusterEnergies.push_back(m_ClusterEnergyLab[cl_idx1]);
    m_3DBhabhaVetoClusterEnergies.push_back(m_ClusterEnergyLab[cl_idx2]);
    m_3DBhabhaVetoClusterTimings.push_back(m_ClusterTiming[cl_idx1]);
    m_3DBhabhaVetoClusterTimings.push_back(m_ClusterTiming[cl_idx2]);
    int cl_thetaid1 = m_TCMap->getTCThetaIdFromTCId(m_MaxTCId[cl_idx1]);
    int cl_thetaid2 = m_TCMap->getTCThetaIdFromTCId(m_MaxTCId[cl_idx2]);
    m_3DBhabhaVetoClusterThetaIds.push_back(cl_thetaid1);
    m_3DBhabhaVetoClusterThetaIds.push_back(cl_thetaid2);
    // set InTrack flag
    if ((cl_thetaid1 >= m_3DBhabhaVetoInTrackThetaRegion[0] &&
         cl_thetaid1 <= m_3DBhabhaVetoInTrackThetaRegion[1]) ||
        (cl_thetaid2 >= m_3DBhabhaVetoInTrackThetaRegion[0] &&
         cl_thetaid2 <= m_3DBhabhaVetoInTrackThetaRegion[1])) {
      m_3DBhabhaVetoInTrackFlag = 1;
    } else {
      m_3DBhabhaVetoInTrackFlag = 0;
    }
  }

  return BhabhaFlag;
}
//========================================================
// 3D Bhabha Selection
//========================================================
bool TrgEclBhabha::getBhabha02()
{
  //
  // Read Cluster Table
  //
  m_MaxTCId.clear();
  m_ClusterEnergyLab.clear();
  m_ClusterTiming.clear();
  m_3DBhabhaSelectionThetaFlag = -1;
  StoreArray<TRGECLCluster> trgeclClusterArray;
  for (int ii = 0; ii < trgeclClusterArray.getEntries(); ii++) {
    TRGECLCluster* aTRGECLCluster = trgeclClusterArray[ii];
    int maxTCId    = aTRGECLCluster->getMaxTCId();
    double clusterenergy  = aTRGECLCluster->getEnergyDep();
    double clustertiming  =  aTRGECLCluster->getTimeAve();
    m_ClusterTiming.push_back(clustertiming);
    m_ClusterEnergyLab.push_back(clusterenergy);
    m_MaxTCId.push_back(maxTCId);
  }
  const int ncluster = m_ClusterEnergyLab.size();

  int cl_idx1 = -1;
  int cl_idx2 = -1;
  bool BhabhaFlag = false;
  for (int icluster = 0; icluster < ncluster ; icluster++) {
    for (int jcluster = icluster + 1; jcluster < ncluster; ++jcluster) {
      bool BtoBFlag = false;

      if (icluster == jcluster) {continue;}

      int energy1 = 0;
      int energy2 = 0;
      int dphi = 0;
      int thetaSum = 0;
      get2CLETP(m_MaxTCId[icluster],
                m_MaxTCId[jcluster],
                energy1,
                energy2,
                dphi,
                thetaSum);

      if (dphi     > m_3DBhabhaSelectionAngle[0] &&
          dphi     < m_3DBhabhaSelectionAngle[1] &&
          thetaSum > m_3DBhabhaSelectionAngle[2] &&
          thetaSum < m_3DBhabhaSelectionAngle[3]) {BtoBFlag = true;}

      if ((m_ClusterEnergyLab[icluster] > m_3DBhabhaSelectionThreshold[0] * energy1 * 0.1 &&
           m_ClusterEnergyLab[jcluster] > m_3DBhabhaSelectionThreshold[0] * energy2 * 0.1) &&
          (m_ClusterEnergyLab[icluster] > m_3DBhabhaSelectionThreshold[1] * energy1 * 0.1 ||
           m_ClusterEnergyLab[jcluster] > m_3DBhabhaSelectionThreshold[1] * energy2 * 0.1)) {
        if (BtoBFlag) {
          BhabhaFlag = true;
          cl_idx1 = icluster;
          cl_idx2 = jcluster;
        }
      }
    }
  }
  if (BhabhaFlag) {
    m_3DBhabhaSelectionClusterTCIds.push_back(m_MaxTCId[cl_idx1]);
    m_3DBhabhaSelectionClusterTCIds.push_back(m_MaxTCId[cl_idx2]);
    m_3DBhabhaSelectionClusterEnergies.push_back(m_ClusterEnergyLab[cl_idx1]);
    m_3DBhabhaSelectionClusterEnergies.push_back(m_ClusterEnergyLab[cl_idx2]);
    m_3DBhabhaSelectionClusterTimings.push_back(m_ClusterTiming[cl_idx1]);
    m_3DBhabhaSelectionClusterTimings.push_back(m_ClusterTiming[cl_idx2]);
    int cl_thetaid1 = m_TCMap->getTCThetaIdFromTCId(m_MaxTCId[cl_idx1]);
    int cl_thetaid2 = m_TCMap->getTCThetaIdFromTCId(m_MaxTCId[cl_idx2]);
    m_3DBhabhaSelectionClusterThetaIds.push_back(cl_thetaid1);
    m_3DBhabhaSelectionClusterThetaIds.push_back(cl_thetaid2);
    // set theta flag(2bits) for prescale in GDL
    int cl_thetaid0 = (cl_thetaid1 < cl_thetaid2) ? cl_thetaid1 : cl_thetaid2;
    if (cl_thetaid0 <= 0 || cl_thetaid0 >= 18) {
      m_3DBhabhaSelectionThetaFlag = -2;
    } else {
      if (cl_thetaid0 == 1) { m_3DBhabhaSelectionThetaFlag = 0; }
      else if (cl_thetaid0 == 2) { m_3DBhabhaSelectionThetaFlag = 1; }
      else if (cl_thetaid0 == 3) { m_3DBhabhaSelectionThetaFlag = 2; }
      else                       { m_3DBhabhaSelectionThetaFlag = 3; }
    }
  }

  return BhabhaFlag;
}
//========================================================
// mu pair trigger
//========================================================
bool TrgEclBhabha::getmumu()
{
  bool MumuFlag = false;
  //
  // Read Cluster Table
  //
  m_MaxTCId.clear();
  m_ClusterEnergyLab.clear();
  m_ClusterTiming.clear();
  StoreArray<TRGECLCluster> trgeclClusterArray;
  for (int ii = 0; ii < trgeclClusterArray.getEntries(); ii++) {
    TRGECLCluster* aTRGECLCluster = trgeclClusterArray[ii];
    int maxTCId    = aTRGECLCluster->getMaxTCId();
    double clusterenergy  = aTRGECLCluster->getEnergyDep();
    double clustertiming  = aTRGECLCluster->getTimeAve();
    m_ClusterTiming.push_back(clustertiming);
    m_ClusterEnergyLab.push_back(clusterenergy);
    m_MaxTCId.push_back(maxTCId);
  }
  const int ncluster = m_ClusterEnergyLab.size();

  for (int icluster = 0; icluster < ncluster ; icluster++) {
    for (int jcluster = icluster + 1; jcluster < ncluster; jcluster ++) {

      bool BtoBFlag = false;
      if (icluster == jcluster) {continue;}

      int energy1 = 0;
      int energy2 = 0;
      int dphi = 0;
      int thetaSum = 0;
      get2CLETP(m_MaxTCId[icluster],
                m_MaxTCId[jcluster],
                energy1,
                energy2,
                dphi,
                thetaSum);

      if (dphi     > m_mumuAngle[0] &&
          dphi     < m_mumuAngle[1] &&
          thetaSum > m_mumuAngle[2] &&
          thetaSum < m_mumuAngle[3]) {
        BtoBFlag = true;
      }

      double mumuEThresholdLab_1 = m_mumuThreshold * energy1 * 0.1; // GeV
      double mumuEThresholdLab_2 = m_mumuThreshold * energy2 * 0.1; // GeV
      if (m_ClusterEnergyLab[icluster] < mumuEThresholdLab_1 &&
          m_ClusterEnergyLab[jcluster] < mumuEThresholdLab_2) {
        if (BtoBFlag) {
          MumuFlag = true;
        }
      }
    }
  }

  return MumuFlag;
}
//========================================================
// taub2b selection for tau 1x1 process
//========================================================
bool TrgEclBhabha::getTaub2b(double E_total1to17)
{
  //
  // Read Cluster Table
  //
  m_MaxTCId.clear();
  m_ClusterEnergyLab.clear();
  m_ClusterTiming.clear();

  m_taub2bAngleFlag    = 0;
  m_taub2bEtotFlag     = 0;
  m_taub2bClusterEFlag = 0;

  if (E_total1to17 < m_taub2bEtotCut) {
    m_taub2bEtotFlag = 1;
  }

  StoreArray<TRGECLCluster> trgeclClusterArray;
  for (int ii = 0; ii < trgeclClusterArray.getEntries(); ii++) {
    TRGECLCluster* aTRGECLCluster = trgeclClusterArray[ii];
    int maxTCId    = aTRGECLCluster->getMaxTCId();
    double clusterenergy  = aTRGECLCluster->getEnergyDep();
    m_ClusterEnergyLab.push_back(clusterenergy);
    m_MaxTCId.push_back(maxTCId);
  }

  const int ncluster = m_ClusterEnergyLab.size();

  for (int icluster = 0; icluster < ncluster ; icluster++) {
    for (int jcluster = icluster + 1; jcluster < ncluster; jcluster ++) {

      if (icluster == jcluster) {continue;}

      int energy1 = 0;
      int energy2 = 0;
      int dphi = 0;
      int thetaSum = 0;
      get2CLETP(m_MaxTCId[icluster],
                m_MaxTCId[jcluster],
                energy1,
                energy2,
                dphi,
                thetaSum);

      if (dphi     > m_taub2bAngleCut[0] &&
          dphi     < m_taub2bAngleCut[1] &&
          thetaSum > m_taub2bAngleCut[2] &&
          thetaSum < m_taub2bAngleCut[3]) {
        m_taub2bAngleFlag++;
        //
        if (m_ClusterEnergyLab[icluster] < m_taub2bCLELabCut ||
            m_ClusterEnergyLab[jcluster] < m_taub2bCLELabCut) {
          m_taub2bClusterEFlag++;
        }
      }
    }
  }

  bool Taub2bFlag = false;
  if (m_taub2bAngleFlag    > 0 &&
      m_taub2bEtotFlag     > 0 &&
      m_taub2bClusterEFlag > 0) {
    Taub2bFlag = true;
  }

  return Taub2bFlag;
}
//========================================================
// taub2b selection for tau 1x1 process (tigher selection than taub2b
//========================================================
bool TrgEclBhabha::getTaub2b2(double E_total1to17)
{
  //
  // Read Cluster Table
  //
  m_MaxTCId.clear();
  m_ClusterEnergyLab.clear();
  m_ClusterTiming.clear();

  int taub2b2EtotFlag  = 0;
  int taub2b2AngleFlag = 0;
  int taub2b2NCLEndcapFlag = 0;
  int taub2b2AngleCLEFlag = 0;

  if (E_total1to17 < m_taub2b2EtotCut) {
    taub2b2EtotFlag = 1;
  }

  // counter of cluster for E(cluster)>threshold in endcap
  int ncl_clecut_endcap = 0;
  // cluster array loop
  StoreArray<TRGECLCluster> trgeclClusterArray;
  for (int ii = 0; ii < trgeclClusterArray.getEntries(); ii++) {
    TRGECLCluster* aTRGECLCluster = trgeclClusterArray[ii];
    int maxTCId           = aTRGECLCluster->getMaxTCId();
    double clusterenergy  = aTRGECLCluster->getEnergyDep();
    double clusterthetaid = aTRGECLCluster->getMaxThetaId();
    m_ClusterEnergyLab.push_back(clusterenergy);
    m_MaxTCId.push_back(maxTCId);

    // count N(cluster) of E(cluster) > threshold in endcap
    if ((clusterthetaid <=  3 ||
         clusterthetaid >= 16) &&
        clusterenergy > m_taub2b2CLELabCut[0]) {
      ncl_clecut_endcap++;
    }
  }

  // bool for N(cluster) of E(cluster)>threshold in endcap
  if (ncl_clecut_endcap < 2) {
    taub2b2NCLEndcapFlag = 1;
  }

  // total number of cluster
  const int ncluster = m_ClusterEnergyLab.size();

  // 2 cluster combination
  for (int icluster = 0; icluster < ncluster ; icluster++) {
    for (int jcluster = icluster + 1; jcluster < ncluster; jcluster ++) {
      if (icluster == jcluster) {continue;}
      int energy1 = 0;
      int energy2 = 0;
      int dphi = 0;
      int thetaSum = 0;
      get2CLETP(m_MaxTCId[icluster],
                m_MaxTCId[jcluster],
                energy1,
                energy2,
                dphi,
                thetaSum);
      // delta phi and theta sum selection
      if (dphi     > m_taub2b2AngleCut[0] &&
          dphi     < m_taub2b2AngleCut[1] &&
          thetaSum > m_taub2b2AngleCut[2] &&
          thetaSum < m_taub2b2AngleCut[3]) {
        taub2b2AngleFlag++;
        if (m_ClusterEnergyLab[icluster] > m_taub2b2CLELabCut[1] &&
            m_ClusterEnergyLab[jcluster] > m_taub2b2CLELabCut[1]) {
          taub2b2AngleCLEFlag++;
        }
      }
    }
  }
  //
  bool taub2b2Flag = false;
  if (taub2b2EtotFlag      > 0 &&
      taub2b2AngleFlag     > 0 &&
      taub2b2NCLEndcapFlag > 0 &&
      taub2b2AngleCLEFlag  > 0) {
    taub2b2Flag = true;
  }

  return taub2b2Flag;
}


//========================================================
// taub2b3
//========================================================
bool TrgEclBhabha::getTaub2b3(double E_total1to17)
{
  //
  // Read Cluster Table
  //
  m_MaxTCId.clear();
  m_MaxTCThetaId.clear();
  m_ClusterEnergyLab.clear();
  m_ClusterTiming.clear();

  int taub2b3EtotFlag = 0;
  int taub2b3AngleCLEThetaIdFlag = 0;
  int taub2b3CLELowCutFlag  = 1;
  int taub2b3CLEHighCutFlag = 1;

  // Total Energy Etot < 7 GeV in lab
  if (E_total1to17 < m_taub2b3EtotCut) {
    taub2b3EtotFlag = 1;
  }

  // cluster array loop
  StoreArray<TRGECLCluster> trgeclClusterArray;
  for (int ii = 0; ii < trgeclClusterArray.getEntries(); ii++) {
    TRGECLCluster* aTRGECLCluster = trgeclClusterArray[ii];
    int maxTCId           = aTRGECLCluster->getMaxTCId();
    double clusterenergy  = aTRGECLCluster->getEnergyDep();
    m_ClusterEnergyLab.push_back(clusterenergy);
    m_MaxTCId.push_back(maxTCId);
    m_MaxTCThetaId.push_back(aTRGECLCluster->getMaxThetaId());

    // All clusters in the event should be E > 0.12 GeV in lab.
    if (clusterenergy <= m_taub2b3CLELabCut[0]) {
      taub2b3CLELowCutFlag = 0;
    }
    // The number of clusters with E > 4.5 GeV in lab should be 0.
    if (clusterenergy > m_taub2b3CLELabCut[1]) {
      taub2b3CLEHighCutFlag = 0;
    }
  }

  // total number of cluster
  const int ncluster = m_ClusterEnergyLab.size();

  // 2 cluster combination
  for (int icluster = 0; icluster < ncluster ; icluster++) {
    for (int jcluster = icluster + 1; jcluster < ncluster; jcluster ++) {
      if (icluster == jcluster) {continue;}
      int energy1 = 0;
      int energy2 = 0;
      int dphi = 0;
      int thetaSum = 0;
      get2CLETP(m_MaxTCId[icluster],
                m_MaxTCId[jcluster],
                energy1,
                energy2,
                dphi,
                thetaSum);

      // delta phi and theta sum selection in cms
      if (dphi     > m_taub2b3AngleCut[0] &&
          dphi     < m_taub2b3AngleCut[1] &&
          thetaSum > m_taub2b3AngleCut[2] &&
          thetaSum < m_taub2b3AngleCut[3]) {
        // Cluster ThetaID selection
        if (m_MaxTCThetaId[icluster] >=  2 &&
            m_MaxTCThetaId[icluster] <= 16 &&
            m_MaxTCThetaId[jcluster] >=  2 &&
            m_MaxTCThetaId[jcluster] <= 16) {
          // Cluster energy selection in lab
          if (m_ClusterEnergyLab[icluster] > m_taub2b3CLEb2bLabCut ||
              m_ClusterEnergyLab[jcluster] > m_taub2b3CLEb2bLabCut) {
            taub2b3AngleCLEThetaIdFlag++;
          }
        }
      }
    }
  }
  // all selections
  bool taub2b3Flag = false;
  if (taub2b3EtotFlag            > 0 &&
      taub2b3AngleCLEThetaIdFlag > 0 &&
      taub2b3CLELowCutFlag       > 0 &&
      taub2b3CLEHighCutFlag      > 0) {
    taub2b3Flag = true;
  }

  return taub2b3Flag;
}

//========================================================
// additional Bhabha veto
//========================================================
int TrgEclBhabha::getBhabhaAddition(void)
{

  std::vector<int> MaxThetaId;
  MaxThetaId.clear();
  m_ClusterEnergyLab.clear();

  int bit_bhabha_addition = 0;

  StoreArray<TRGECLCluster> trgeclClusterArray;
  for (int ii = 0; ii < trgeclClusterArray.getEntries(); ii++) {
    TRGECLCluster* aTRGECLCluster = trgeclClusterArray[ii];
    int maxTCId = aTRGECLCluster->getMaxTCId();
    m_MaxTCId.push_back(maxTCId);
    int maxThetaId = aTRGECLCluster->getMaxThetaId();
    MaxThetaId.push_back(maxThetaId);
    double clusterenergy  = aTRGECLCluster->getEnergyDep();
    m_ClusterEnergyLab.push_back(clusterenergy);
  }
  int NofCluster1to17 = MaxThetaId.size();

  if (NofCluster1to17 == 1) {
    if (MaxThetaId[0] <= 3) {
      bit_bhabha_addition |= 0x01;
    }
  } else if (NofCluster1to17 == 2) {

    int energy1 = 0;
    int energy2 = 0;
    int dphi = 0;
    int thetaSum = 0;
    get2CLETP(m_MaxTCId[0],
              m_MaxTCId[1],
              energy1,
              energy2,
              dphi,
              thetaSum);

    // for hie1
    if ((dphi     > m_hie12BhabhaVetoAngle[0] &&
         dphi     < m_hie12BhabhaVetoAngle[1]) &&
        (thetaSum > m_hie12BhabhaVetoAngle[2] &&
         thetaSum < m_hie12BhabhaVetoAngle[3])) {
      bit_bhabha_addition |= 0x02;
    }
    // for hie2
    if ((dphi     > m_hie12BhabhaVetoAngle[0] &&
         dphi     < m_hie12BhabhaVetoAngle[1]) ||
        (thetaSum > m_hie12BhabhaVetoAngle[2] &&
         thetaSum < m_hie12BhabhaVetoAngle[3])) {
      bit_bhabha_addition |= 0x04;
    }

    // for hie3 and hie4
    // get thetaID of lower energy cluster
    int lowe_MaxThetaId = 0;
    if (m_ClusterEnergyLab[0] < m_ClusterEnergyLab[1]) {
      lowe_MaxThetaId = MaxThetaId[0];
    } else {
      lowe_MaxThetaId = MaxThetaId[1];
    }
    // select cluster in endcap region
    if (lowe_MaxThetaId <= 3 ||
        lowe_MaxThetaId >= 16) {
      // for hie3
      bit_bhabha_addition |= 0x08;
      // for hie4
      // require low energy bound for cluster
      if (m_ClusterEnergyLab[0] > m_hie4LowCLELabCut &&
          m_ClusterEnergyLab[1] > m_hie4LowCLELabCut) {
        bit_bhabha_addition |= 0x10;
      }
    }
  }

  return bit_bhabha_addition;
}
//========================================================
// get cluster energy and angles(dphi and theta_sum) from LUT
//========================================================
void TrgEclBhabha::get2CLETP(int TCId1,
                             int TCId2,
                             int& energy1,
                             int& energy2,
                             int& dphi,
                             int& thetaSum)
{
  int lut1 = m_database->get3DBhabhaLUT(TCId1);
  int lut2 = m_database->get3DBhabhaLUT(TCId2);
  energy1 = 15 & lut1;
  energy2 = 15 & lut2;
  lut1 >>= 4;
  lut2 >>= 4;
  int phi1 = 511 & lut1;
  int phi2 = 511 & lut2;
  lut1 >>= 9;
  lut2 >>= 9;
  int theta1 = lut1;
  int theta2 = lut2;

  dphi = abs(phi1 - phi2);
  thetaSum = theta1 + theta2;

  return;
}
//========================================================
//
//========================================================
