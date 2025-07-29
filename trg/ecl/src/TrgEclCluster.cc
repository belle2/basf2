/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

//---------------------------------------------------------------
// Description :
// -A class to make cluster from TC Hit
// -ICN Logic ( function name : setICN(tcid) )
//    Isolate cluster counting logic used in Belle for 3x3 TCs
//
//    |5|1|6|
//    |2|0|3|     ICN = [ 0 and !(1 or 2) and !(4 and 7) ]
//    |7|4|8|
//
//    3x3 TC
//
// -Clustering Logic (Function name : setICN(tcid, tcenergy, tctiming) )
// 1.Find TC Hit (0) satisfying ICN logic
// 2.Find the most energetic TC Hit in the neghbor TCs with 0 (9 TCs)
// 3.Bind neighbor TC hits with the most energetic TC (Center TC) as a Cluster
// 4.Cluster energy : Total energy in Cluster
// 5.Timing : Timing of the most energetic TC in a cluster or Energy weighted timing in a cluster
//
// -The limit number of Cluster is 6 due to hardware limitation (ETM)
// -The cluster information (energy, timing) is saved in the order of barrel, forward endcap and backwrd endcap
//---------------------------------------------------------------

#define TRG_SHORT_NAMES
#define TRGECLCLUSTER_SHORT_NAMES

#include <framework/datastore/StoreArray.h>
#include <framework/logging/Logger.h>

#include <trg/ecl/TrgEclCluster.h>
#include "trg/ecl/dataobjects/TRGECLCluster.h"

using namespace std;
using namespace Belle2;
//
//
//
TrgEclCluster::TrgEclCluster():
  m_BRICN(0), m_FWDICN(0), m_BWDICN(0),
  m_BRNofCluster(0), m_FWDNofCluster(0), m_BWDNofCluster(0),
  m_EventId(0), m_Method(1), m_LimitNCluster(6), m_Position(1)
{

  TrgEclCluster::initialize();
  m_TCMap = new TrgEclMapping();

}
//
//
//
TrgEclCluster::~TrgEclCluster()
{
  delete m_TCMap;
}
//
//
//
void TrgEclCluster::initialize(void)
{
  m_icnfwbrbw.clear();
  m_BrCluster.clear();
  m_FwCluster.clear();
  m_BwCluster.clear();
  m_Quadrant.clear();

  m_TCId.clear();
  m_Timing.clear();
  m_Energy.clear();

  m_ClusterTiming.clear();
  m_ClusterEnergy.clear();
  m_ClusterPositionX.clear();
  m_ClusterPositionY.clear();
  m_ClusterPositionZ.clear();
  m_NofTCinCluster.clear();
  m_MaxTCId.clear();
  m_TempCluster.clear();

  m_icnfwbrbw.resize(3, 0);
  m_TempCluster.resize(9);
  m_Quadrant.resize(3, std::vector<int>(4, 0.0));

  m_ClusterTiming.resize(3);
  m_ClusterEnergy.resize(3);
  m_ClusterPositionX.resize(3);
  m_ClusterPositionY.resize(3);
  m_ClusterPositionZ.resize(3);
  m_NofTCinCluster.resize(3);
  m_MaxTCId.resize(3);
}
//
//
//
int
TrgEclCluster::getICNFwBr(void)
{
  //  get ICN in Barrel and Forward Endcap except for the most inner (=Physics region : Theta Id 2 ~ 15)
  return m_icnfwbrbw[0] + m_icnfwbrbw[1];

}
//
//
//
int
TrgEclCluster::getICNSub(int FwBrBw)
{
  //  get ICN in Barrel and Backward endcap, Forward Endcap except for the most inner (=Physics region : Theta Id 2 ~ 15)

  return m_icnfwbrbw[FwBrBw];

}
//
//
//
int
TrgEclCluster::getBrICNCluster(int ICNId, int location)
{
  // get the # of Cluster in Barrel.
  return m_BrCluster[ICNId][location];

}
//
//
//
int
TrgEclCluster::getBwICNCluster(int ICNId, int location)
{

  return m_BwCluster[ICNId][location];

}
//
//
//
int
TrgEclCluster::getFwICNCluster(int ICNId, int location)
{

  return m_FwCluster[ICNId][location];

}
//
//
//
void
TrgEclCluster::setICN(const std::vector<int>& tcid)
{
  m_TCId = tcid ;
  m_Quadrant.clear();
  m_Quadrant.resize(3, std::vector<int>(4, 0.0));
  m_icnfwbrbw.clear();
  m_icnfwbrbw.resize(3, 0);
  m_BRICN = 0 ;
  m_FWDICN = 0;
  m_BWDICN = 0;

  m_icnfwbrbw[1] = setBarrelICN();
  m_icnfwbrbw[0] = setForwardICN();
  m_icnfwbrbw[2] = setBackwardICN();

  return;
}
//
//
//
void
TrgEclCluster::setICN(const std::vector<int>& tcid,
                      const std::vector<double>& tcenergy,
                      const std::vector<double>& tctiming)
{
  m_TCId = tcid ;
  m_Energy = tcenergy;
  m_Timing = tctiming;

  setBarrelICN(m_Method);
  setForwardICN(m_Method);
  setBackwardICN(m_Method);

  save(m_EventId);

  return;
}
//
//
//
void
TrgEclCluster::save(int m_nEvent)
{
  // Save Cluster
  // Save order : Barrel, Forward, Backward

  int m_hitNum = 0;
  int clusterId = 0;
  int counter = 0;
  for (int iposition = 0; iposition < 3 ; iposition ++) {
    const int Ncluster = m_ClusterEnergy[iposition].size();
    for (int icluster = 0; icluster < Ncluster; icluster++) {
      if (m_ClusterEnergy[iposition][icluster] <= 0) {continue;}

      if (counter >= m_LimitNCluster) {
        continue;
      } else {
        counter ++;
      }

      StoreArray<TRGECLCluster> ClusterArray;
      ClusterArray.appendNew();
      m_hitNum = ClusterArray.getEntries() - 1;
      ClusterArray[m_hitNum]->setEventId(m_nEvent);
      ClusterArray[m_hitNum]->setClusterId(clusterId);
      ClusterArray[m_hitNum]->setMaxTCId(m_MaxTCId[iposition][icluster]);
      ClusterArray[m_hitNum]->setMaxThetaId(m_TCMap->getTCThetaIdFromTCId(m_MaxTCId[iposition][icluster]));
      ClusterArray[m_hitNum]->setMaxPhiId(m_TCMap->getTCPhiIdFromTCId(m_MaxTCId[iposition][icluster]));
      ClusterArray[m_hitNum]->setNofTCinCluster(m_NofTCinCluster[iposition][icluster]);
      ClusterArray[m_hitNum]->setEnergyDep(m_ClusterEnergy[iposition][icluster]);
      ClusterArray[m_hitNum]->setTimeAve(m_ClusterTiming[iposition][icluster]);

      ClusterArray[m_hitNum]->setPositionX(m_ClusterPositionX[iposition][icluster]);
      ClusterArray[m_hitNum]->setPositionY(m_ClusterPositionY[iposition][icluster]);
      ClusterArray[m_hitNum]->setPositionZ(m_ClusterPositionZ[iposition][icluster]);
      clusterId ++;
    }
  }


}
//
//
//
void TrgEclCluster::setBarrelICN(int Method)
{

  std::vector<int> TCFire;
  std::vector<double> TCFireEnergy;
  std::vector<double> TCFireTiming;
  std::vector<std::vector<double>> TCFirePosition;

  TCFire.clear();
  TCFireEnergy.clear();
  TCFireTiming.clear();
  TCFirePosition.clear();

  TCFire.resize(432, 0);
  TCFireEnergy.resize(432, 0.);
  TCFireTiming.resize(432, 0.);
  TCFirePosition.resize(432, std::vector<double>(3, 0.));

  const int  hit_size  = m_TCId.size();
  for (int ihit = 0 ; ihit < hit_size ; ihit++) {
    if (m_TCId[ihit] >= 81 && m_TCId[ihit] <= 512) {
      TCFire[m_TCId[ihit] - 81] = m_TCId[ihit];
      TCFireEnergy[m_TCId[ihit] - 81] = m_Energy[ihit];
      TCFireTiming[m_TCId[ihit] - 81] = m_Timing[ihit];
      TCFirePosition[m_TCId[ihit] - 81][0] = (m_TCMap->getTCPosition(m_TCId[ihit])).X();
      TCFirePosition[m_TCId[ihit] - 81][1] = (m_TCMap->getTCPosition(m_TCId[ihit])).Y();
      TCFirePosition[m_TCId[ihit] - 81][2] = (m_TCMap->getTCPosition(m_TCId[ihit])).Z();
    }
  }
  //
  //
  //
  int tc_upper = 0; // check upper TC
  int tc_upper_right = 0; // check right TC
  int tc_right = 0; // check right TC
  int tc_lower_right = 0;
  int tc_lower = 0; // check lower TC
  int tc_lower_left = 0; // check lower TC
  int tc_left = 0;
  int tc_upper_left = 0;

  for (int iii = 0 ; iii < 432 ; iii++) {

    if (TCFire[iii] == 0) { continue; }

    setBarrelICNsub(iii,
                    TCFire,
                    tc_upper,
                    tc_upper_right,
                    tc_right,
                    tc_lower_right,
                    tc_lower,
                    tc_lower_left,
                    tc_left,
                    tc_upper_left);

    if (!(tc_upper != 0 || tc_left != 0)) {
      if (!(tc_lower != 0 && tc_lower_left != 0)) {

        if (Method == 1) {
          // for cluster method2(Consider TC energy in oderto find the center of cluster)
          int maxTCid = 0;
          double maxTCEnergy = 0;
          for (int iTC = 0; iTC < 9; iTC++) { //find center of Cluster
            if (m_TempCluster[iTC] == 0) {continue;}
            if (maxTCEnergy < TCFireEnergy[m_TempCluster[iTC] - 81]) {
              maxTCEnergy = TCFireEnergy[m_TempCluster[iTC] - 81];
              maxTCid = m_TempCluster[iTC];
            }
          }
          // maxTCid is in barral
          if (maxTCid <  81 ||
              maxTCid > 512) {
            B2ERROR("maxTCID is out of barrel region : maxTCID = " << maxTCid);
          }

          m_TempCluster[0] = maxTCid;

          // 8 TCs surrounding maxTC
          int indexNeighboringTC[8] = {0};
          // set neighboring TC ID of maxTC by taking boundary condtion into account
          if (maxTCid < 93) {
            indexNeighboringTC[0] = maxTCid + 420 - 81;
            indexNeighboringTC[1] = maxTCid + 419 - 81;
            indexNeighboringTC[2] = maxTCid -   1 - 81;
            indexNeighboringTC[3] = maxTCid +  11 - 81;
            indexNeighboringTC[4] = maxTCid +  12 - 81;
            indexNeighboringTC[5] = maxTCid +  13 - 81;
            indexNeighboringTC[6] = maxTCid +   1 - 81;
            indexNeighboringTC[7] = maxTCid + 421 - 81;
          } else if (maxTCid < 501) {
            indexNeighboringTC[0] = maxTCid -  12 - 81;
            indexNeighboringTC[1] = maxTCid -  13 - 81;
            indexNeighboringTC[2] = maxTCid -   1 - 81;
            indexNeighboringTC[3] = maxTCid +  11 - 81;
            indexNeighboringTC[4] = maxTCid +  12 - 81;
            indexNeighboringTC[5] = maxTCid +  13 - 81;
            indexNeighboringTC[6] = maxTCid +   1 - 81;
            indexNeighboringTC[7] = maxTCid -  11 - 81;
          } else {
            indexNeighboringTC[0] = maxTCid -  12 - 81;
            indexNeighboringTC[1] = maxTCid -  13 - 81;
            indexNeighboringTC[2] = maxTCid -   1 - 81;
            indexNeighboringTC[3] = maxTCid - 421 - 81;
            indexNeighboringTC[4] = maxTCid - 420 - 81;
            indexNeighboringTC[5] = maxTCid - 419 - 81;
            indexNeighboringTC[6] = maxTCid +   1 - 81;
            indexNeighboringTC[7] = maxTCid -  11 - 81;
          }
          // check out of boundary(0-431) of TCFire array
          for (int jjj = 0; jjj < 8; jjj++) {
            if (indexNeighboringTC[jjj] <=  -1 ||
                indexNeighboringTC[jjj] >= (int) TCFire.size()) {
              indexNeighboringTC[jjj] = -1;
            }
          }
          // check forward and backward side boundary in barrel
          if ((maxTCid - 81) % 12 == 0) {
            // forward side in barral
            indexNeighboringTC[1] = -1;
            indexNeighboringTC[2] = -1;
            indexNeighboringTC[3] = -1;
          } else if ((maxTCid - 81) % 12 == 11) {
            // backward side in barral
            indexNeighboringTC[5] = -1;
            indexNeighboringTC[6] = -1;
            indexNeighboringTC[7] = -1;
          }
          // set TC hit flag for 8 TCs surrounding maxTC
          for (int jjj = 0; jjj < 8; jjj++) {
            if (indexNeighboringTC[jjj] >= 0) {
              m_TempCluster[jjj + 1] = TCFire.at(indexNeighboringTC[jjj]);
            } else {
              m_TempCluster[jjj + 1] = 0;
            }
          }
        }

        for (int iNearTC = 1; iNearTC < 9; iNearTC ++) {
          for (int jNearTC = 1; jNearTC < 9; jNearTC ++) {
            if (m_TempCluster[iNearTC] == 0) {continue;}
            if (iNearTC == jNearTC) {continue;}
            if (m_TempCluster[iNearTC] == m_TempCluster[jNearTC]) {
              m_TempCluster[jNearTC] = 0;
            }
          }
        }

        int maxTCId = 999;
        double clusterenergy = 0;
        double clustertiming = 0;
        double clusterpositionX = 0;
        double clusterpositionY = 0;
        double clusterpositionZ = 0;
        int noftcincluster = 0;
        for (int iNearTC = 0; iNearTC < 9; iNearTC ++) {
          if (m_TempCluster[iNearTC] == 0) {continue;}
          else {noftcincluster++;}
          clusterenergy +=   TCFireEnergy[m_TempCluster[iNearTC] - 81];
          clustertiming +=   TCFireEnergy[m_TempCluster[iNearTC] - 81] * TCFireTiming[m_TempCluster[iNearTC] - 81];
          clusterpositionX += TCFireEnergy[m_TempCluster[iNearTC] - 81] * TCFirePosition[m_TempCluster[iNearTC] - 81][0];
          clusterpositionY += TCFireEnergy[m_TempCluster[iNearTC] - 81] * TCFirePosition[m_TempCluster[iNearTC] - 81][1];
          clusterpositionZ += TCFireEnergy[m_TempCluster[iNearTC] - 81] * TCFirePosition[m_TempCluster[iNearTC] - 81][2];
        }
        //
        maxTCId = m_TempCluster[0];
        //
        clustertiming /= clusterenergy;
        if (m_Position == 1) {
          clusterpositionX /= clusterenergy;
          clusterpositionY /= clusterenergy;
          clusterpositionZ /= clusterenergy;
        } else if (m_Position == 0) {
          clustertiming =  TCFireTiming[maxTCId - 81];
          clusterpositionX = TCFirePosition[maxTCId - 81][0];
          clusterpositionY = TCFirePosition[maxTCId - 81][1];
          clusterpositionZ = TCFirePosition[maxTCId - 81][2];
        }
        if (clustertiming == 0 && clusterenergy == 0) {continue;}
        m_BrCluster.push_back(m_TempCluster);
        m_ClusterEnergy[0].push_back(clusterenergy);
        m_ClusterTiming[0].push_back(clustertiming);
        m_ClusterPositionX[0].push_back(clusterpositionX);
        m_ClusterPositionY[0].push_back(clusterpositionY);
        m_ClusterPositionZ[0].push_back(clusterpositionZ);
        m_NofTCinCluster[0].push_back(noftcincluster);
        m_MaxTCId[0].push_back(maxTCId);
      }
    }
  }// iii loop
  m_BRNofCluster = m_MaxTCId[0].size();

}
//
//
//
void
TrgEclCluster::setForwardICN(int Method)
{

  std::vector<int> TCFire;
  std::vector<double> TCFireEnergy;
  std::vector<double> TCFireTiming;
  std::vector<std::vector<double>> TCFirePosition;

  std::vector<double> TempClusterEnergy;
  std::vector<double> TempClusterTiming;
  std::vector<double> TempClusterPositionX;
  std::vector<double> TempClusterPositionY;
  std::vector<double> TempClusterPositionZ;
  std::vector<double> Sort1D;
  std::vector<std::vector<double>> Sort2D;

  std::vector<int> TempNofTCinCluster;
  std::vector<int> TempMaxTCId;

  int TempICNTCId = 0;;

  TempClusterEnergy.clear();
  TempClusterTiming.clear();
  TempClusterPositionX.clear();
  TempClusterPositionY.clear();
  TempClusterPositionZ.clear();
  TempNofTCinCluster.clear();
  TempMaxTCId.clear();

  Sort2D.clear();
  Sort1D.clear();

  TCFire.clear();
  TCFireEnergy.clear();
  TCFireTiming.clear();
  TCFirePosition.clear();

  TCFire.resize(96, 0);
  TCFireEnergy.resize(96, 0.);
  TCFireTiming.resize(96, 0.);

  TCFirePosition.resize(96, std::vector<double>(3, 0.));


  const int  hit_size  = m_TCId.size();
  for (int ihit = 0 ; ihit < hit_size ; ihit++) {
    if (m_TCId[ihit] > 80) {continue;}

    TCFireEnergy[m_TCId[ihit] - 1] = m_Energy[ihit];
    TCFireTiming[m_TCId[ihit] - 1] = m_Timing[ihit];
    TCFirePosition[m_TCId[ihit] - 1][0] = (m_TCMap->getTCPosition(m_TCId[ihit])).X();
    TCFirePosition[m_TCId[ihit] - 1][1] = (m_TCMap->getTCPosition(m_TCId[ihit])).Y();
    TCFirePosition[m_TCId[ihit] - 1][2] = (m_TCMap->getTCPosition(m_TCId[ihit])).Z();

    //------------------------------------
    // To rearrange the hit map
    //
    //   original       converted
    //  (<- Theta)    (<- Theta)
    //
    //   3  2  1       64 32  0
    //   4  5  -       65 33  1
    //   8  7  6  =>   66 34  2
    //   9 10  -       67 35  3
    //  ....           ...
    //  78 77 76       94 62 30
    //  79 80  -       95 63 31
    //
    // Here, TCId-1 becomes TCId=0 and 1.
    //------------------------------------
    int iTCId0 = m_TCId[ihit] - 1;
    int kkk = 0;
    if (iTCId0 % 5 == 0) {
      kkk = (iTCId0 / 5) * 2;
      TCFire[kkk]   = m_TCId[ihit];
      TCFire[kkk + 1] = m_TCId[ihit];
    } else {
      kkk = iTCId0 / 5;
      switch (iTCId0 % 5) {
        case 1 :
          TCFire[32 + 2 * kkk]   = m_TCId[ihit]; break;
        case 2 :
          TCFire[64 + 2 * kkk]   = m_TCId[ihit]; break;
        case 3 :
          TCFire[64 + 2 * kkk + 1] = m_TCId[ihit]; break;
        case 4 :
          TCFire[32 + 2 * kkk + 1] = m_TCId[ihit]; break;
        default:
          break;
      }
    }
  }

  for (int iii = 0 ; iii < 96 ; iii++) {
    int icn_flag = 0;
    if (iii < 32) {
      if (iii % 2 == 1) {
        continue;
      }
    }
    for (int iinit = 0; iinit < 9; iinit ++)
      m_TempCluster[iinit] = 0;
    if (TCFire[iii] == 0)
      continue;
    // cppcheck-suppress knownConditionTrueFalse
    if (iii < 32) { // most inner
      if (iii == 0) {
        m_TempCluster[0] = TCFire[iii];
        m_TempCluster[1] = TCFire[30]; // top
        m_TempCluster[2] = TCFire[63]; // left up
        m_TempCluster[3] = TCFire[iii + 32] ; // left 1
        m_TempCluster[4] = TCFire[iii + 33]; // left 2
        m_TempCluster[5] = TCFire[iii + 34]; // left bottom
        m_TempCluster[6] = TCFire[iii + 2]; // bottom
        m_TempCluster[7] = 0;
        m_TempCluster[8] = 0;

      } else if (iii == 30) {
        m_TempCluster[0] = TCFire[iii];
        m_TempCluster[1] = TCFire[iii - 2]; // top
        m_TempCluster[2] = TCFire[iii + 31]; // left up
        m_TempCluster[3] = TCFire[iii + 32] ; // left 1
        m_TempCluster[4] = TCFire[iii + 33]; // left 2
        m_TempCluster[5] = TCFire[32]; // left bottom
        m_TempCluster[6] = TCFire[0]; // bottom
        m_TempCluster[7] = 0;
        m_TempCluster[8] = 0;

      } else {
        m_TempCluster[0] = TCFire[iii];
        m_TempCluster[1] = TCFire[iii - 2]; // top
        m_TempCluster[2] = TCFire[iii + 31]; // left up
        m_TempCluster[3] = TCFire[iii + 32] ; // left 1
        m_TempCluster[4] = TCFire[iii + 33]; // left 2
        m_TempCluster[5] = TCFire[iii + 34]; // left bottom
        m_TempCluster[6] = TCFire[iii + 2]; // bottom
        m_TempCluster[7] = 0;
        m_TempCluster[8] = 0;

      }
      if (!(m_TempCluster[1] != 0 || (m_TempCluster[3] != 0 || m_TempCluster[4] != 0))) {
        if (!(m_TempCluster[5] != 0 && m_TempCluster[6] != 0)) {
          icn_flag = 1;


        }
      }

    } else if (iii < 64) {
      if (iii == 32) {
        m_TempCluster[0] = TCFire[iii];
        m_TempCluster[1] = TCFire[63]; // up
        m_TempCluster[2] = TCFire[iii - 2]; // up right
        m_TempCluster[3] = TCFire[iii - 32]; // right
        m_TempCluster[4] = 0; //right bottom
        m_TempCluster[5] = TCFire[iii + 1]; // bottom
        m_TempCluster[6] = TCFire[iii + 33]; // left bottom
        m_TempCluster[7] = TCFire[iii + 32]; // left
        m_TempCluster[8] = TCFire[95]; // up left

      } else if (iii == 33) {

        m_TempCluster[0] = TCFire[iii];
        m_TempCluster[1] = TCFire[iii - 1]; // up
        m_TempCluster[2] = 0; // up right
        m_TempCluster[3] = TCFire[iii - 32]; // right
        m_TempCluster[4] = TCFire[iii - 30]; //right bottom
        m_TempCluster[5] = TCFire[iii + 1]; // bottom
        m_TempCluster[6] = TCFire[iii + 33]; // left bottom
        m_TempCluster[7] = TCFire[iii + 32]; // left
        m_TempCluster[8] = TCFire[iii + 31]; // up left

      }

      else if (iii == 62) {
        m_TempCluster[0] = TCFire[iii];
        m_TempCluster[1] = TCFire[iii - 1]; // up
        m_TempCluster[2] = TCFire[iii - 34]; // up right
        m_TempCluster[3] = TCFire[iii - 32]; // right
        m_TempCluster[4] = 0; //right bottom
        m_TempCluster[5] = TCFire[iii + 1]; // bottom
        m_TempCluster[6] = TCFire[iii + 33]; // left bottom
        m_TempCluster[7] = TCFire[iii + 32]; // left
        m_TempCluster[8] = TCFire[iii + 31]; // up left


      } else if (iii == 63) {
        m_TempCluster[0] = TCFire[iii];
        m_TempCluster[1] = TCFire[iii - 1]; // up
        m_TempCluster[2] = 0; // up right
        m_TempCluster[3] = TCFire[iii - 32]; // right
        m_TempCluster[4] = TCFire[1]; //right bottom
        m_TempCluster[5] = TCFire[32]; // bottom
        m_TempCluster[6] = TCFire[64]; // left bottom
        m_TempCluster[7] = TCFire[iii + 32]; // left
        m_TempCluster[8] = TCFire[iii + 31]; // up left

      }

      else {

        m_TempCluster[0] = TCFire[iii];
        m_TempCluster[1] = TCFire[iii - 1]; // up
        m_TempCluster[2] = TCFire[iii - 34]; // up right
        m_TempCluster[3] = TCFire[iii - 32]; // right
        m_TempCluster[4] = TCFire[iii - 30]; //right bottom
        m_TempCluster[5] = TCFire[iii + 1]; // bottom
        m_TempCluster[6] = TCFire[iii + 33]; // left bottom
        m_TempCluster[7] = TCFire[iii + 32]; // left
        m_TempCluster[8] = TCFire[iii + 31]; // up left
        if (iii % 2 == 0) {
          m_TempCluster[4] = 0;
        } else {
          m_TempCluster[2] = 0;
        }
      }
      if (!(m_TempCluster[1] != 0 || m_TempCluster[7] != 0)) {
        if (!(m_TempCluster[5] != 0 && m_TempCluster[6] != 0)) {
          icn_flag = 1;
        }
      }

    } else {
      if (iii == 64) {
        m_TempCluster[0] = TCFire[iii];
        m_TempCluster[1] = TCFire[95]; // top
        m_TempCluster[2] = TCFire[63];// right top
        m_TempCluster[3] = TCFire[iii - 32]; //right
        m_TempCluster[4] = TCFire[iii - 31]; //right bottom
        m_TempCluster[5] = TCFire[iii + 1]; //bottom
        m_TempCluster[6] = 0; //bottom left
        m_TempCluster[7] = 0; // left
        m_TempCluster[8] = 0; //top left
      } else if (iii == 95) {
        m_TempCluster[0] = TCFire[iii];
        m_TempCluster[1] = TCFire[iii - 1]; // top
        m_TempCluster[2] = TCFire[iii - 33]; // right top
        m_TempCluster[3] = TCFire[iii - 32]; //right
        m_TempCluster[4] = TCFire[32]; //right bottom
        m_TempCluster[5] = TCFire[64]; //bottom
        m_TempCluster[6] = 0; //bottom left
        m_TempCluster[7] = 0; // left
        m_TempCluster[8] = 0; //top left

      } else {
        m_TempCluster[0] = TCFire[iii];
        m_TempCluster[1] = TCFire[iii - 1]; // top
        m_TempCluster[2] = TCFire[iii - 33]; // right top
        m_TempCluster[3] = TCFire[iii - 32]; //right
        m_TempCluster[4] = TCFire[iii - 31]; //right bottom
        m_TempCluster[5] = TCFire[iii + 1]; //bottom
        m_TempCluster[6] = 0; //bottom left
        m_TempCluster[7] = 0; // left
        m_TempCluster[8] = 0; //top left
      }
      if (!(m_TempCluster[1] != 0 || m_TempCluster[7] != 0)) {
        if (!(m_TempCluster[5] != 0 && m_TempCluster[6] != 0)) {
          icn_flag = 1;

        }
      }


    }

    if (icn_flag == 1) {
      TempICNTCId = TCFire[iii];

      if (Method == 1) { //for cluster method2
        int maxTCid = 0;
        double maxTCEnergy = 0;
        for (int iTC = 0; iTC < 9; iTC++) { //find center of Cluster
          if (m_TempCluster[iTC] == 0) {continue;}
          if (maxTCEnergy <  TCFireEnergy[m_TempCluster[iTC] - 1]) {
            maxTCEnergy = TCFireEnergy[m_TempCluster[iTC] - 1];
            maxTCid = m_TempCluster[iTC];
          }
        }
        m_TempCluster.resize(9, 0);
        m_TempCluster[0] = maxTCid;
        int kkk = (m_TempCluster[0] - 1) / 5;
        if ((m_TempCluster[0] - 1) % 5 == 0) {
          kkk = kkk * 2;
        } else if ((m_TempCluster[0] - 1) % 5 == 1) {
          kkk = 32 +  2 * kkk;
        } else if ((m_TempCluster[0] - 1) % 5 == 2) {
          kkk = 64 +  2 * kkk;
        } else if ((m_TempCluster[0] - 1) % 5 == 3) {
          kkk = 64 + 1 +  2 * kkk;
        } else if ((m_TempCluster[0] - 1) % 5 == 4) {
          kkk = 32 + 1 +  2 * kkk;
        }

        if (kkk < 32) { // most inner
          if (kkk == 0) {
            m_TempCluster[0] = TCFire[kkk];
            m_TempCluster[1] = TCFire[30]; // top
            m_TempCluster[2] = TCFire[63]; // left up
            m_TempCluster[3] = TCFire[kkk + 32] ; // left 1
            m_TempCluster[4] = TCFire[kkk + 33]; // left 2
            m_TempCluster[5] = TCFire[kkk + 34]; // left bottom
            m_TempCluster[6] = TCFire[kkk + 2]; // bottom
            m_TempCluster[7] = 0;
            m_TempCluster[8] = 0;

          } else if (kkk == 30) {
            m_TempCluster[0] = TCFire[kkk];
            m_TempCluster[1] = TCFire[kkk - 2]; // top
            m_TempCluster[2] = TCFire[kkk + 31]; // left up
            m_TempCluster[3] = TCFire[kkk + 32] ; // left 1
            m_TempCluster[4] = TCFire[kkk + 33]; // left 2
            m_TempCluster[5] = TCFire[32]; // left bottom
            m_TempCluster[6] = TCFire[0]; // bottom
            m_TempCluster[7] = 0;
            m_TempCluster[8] = 0;

          } else {
            m_TempCluster[0] = TCFire[kkk];
            m_TempCluster[1] = TCFire[kkk - 2]; // top
            m_TempCluster[2] = TCFire[kkk + 31]; // left up
            m_TempCluster[3] = TCFire[kkk + 32] ; // left 1
            m_TempCluster[4] = TCFire[kkk + 33]; // left 2
            m_TempCluster[5] = TCFire[kkk + 34]; // left bottom
            m_TempCluster[6] = TCFire[kkk + 2]; // bottom
            m_TempCluster[7] = 0;
            m_TempCluster[8] = 0;

          }

        } else if (kkk < 64) {
          if (kkk == 32) {
            m_TempCluster[0] = TCFire[kkk];
            m_TempCluster[1] = TCFire[63]; // up
            m_TempCluster[2] = TCFire[kkk - 2]; // up right
            m_TempCluster[3] = TCFire[kkk - 32]; // right
            m_TempCluster[4] = 0; //right bottom
            m_TempCluster[5] = TCFire[kkk + 1]; // bottom
            m_TempCluster[6] = TCFire[kkk + 33]; // left bottom
            m_TempCluster[7] = TCFire[kkk + 32]; // left
            m_TempCluster[8] = TCFire[95]; // up left

          } else if (kkk == 33) {

            m_TempCluster[0] = TCFire[kkk];
            m_TempCluster[1] = TCFire[kkk - 1]; // up
            m_TempCluster[2] = 0; // up right
            m_TempCluster[3] = TCFire[kkk - 32]; // right
            m_TempCluster[4] = TCFire[kkk - 30]; //right bottom
            m_TempCluster[5] = TCFire[kkk + 1]; // bottom
            m_TempCluster[6] = TCFire[kkk + 33]; // left bottom
            m_TempCluster[7] = TCFire[kkk + 32]; // left
            m_TempCluster[8] = TCFire[kkk + 31]; // up left

          }

          else if (kkk == 62) {
            m_TempCluster[0] = TCFire[kkk];
            m_TempCluster[1] = TCFire[kkk - 1]; // up
            m_TempCluster[2] = TCFire[kkk - 34]; // up right
            m_TempCluster[3] = TCFire[kkk - 32]; // right
            m_TempCluster[4] = 0; //right bottom
            m_TempCluster[5] = TCFire[kkk + 1]; // bottom
            m_TempCluster[6] = TCFire[kkk + 33]; // left bottom
            m_TempCluster[7] = TCFire[kkk + 32]; // left
            m_TempCluster[8] = TCFire[kkk + 31]; // up left


          } else if (kkk == 63) {
            m_TempCluster[0] = TCFire[kkk];
            m_TempCluster[1] = TCFire[kkk - 1]; // up
            m_TempCluster[2] = 0; // up right
            m_TempCluster[3] = TCFire[kkk - 32]; // right
            m_TempCluster[4] = TCFire[1]; //right bottom
            m_TempCluster[5] = TCFire[32]; // bottom
            m_TempCluster[6] = TCFire[64]; // left bottom
            m_TempCluster[7] = TCFire[kkk + 32]; // left
            m_TempCluster[8] = TCFire[kkk + 31]; // up left

          }

          else {
            m_TempCluster[0] = TCFire[kkk];
            m_TempCluster[1] = TCFire[kkk - 1]; // up
            m_TempCluster[2] = TCFire[kkk - 34]; // up right
            m_TempCluster[3] = TCFire[kkk - 32]; // right
            m_TempCluster[4] = TCFire[kkk - 30]; //right bottom
            m_TempCluster[5] = TCFire[kkk + 1]; // bottom
            m_TempCluster[6] = TCFire[kkk + 33]; // left bottom
            m_TempCluster[7] = TCFire[kkk + 32]; // left
            m_TempCluster[8] = TCFire[kkk + 31]; // up left
            if (kkk % 2 == 0) {
              m_TempCluster[4] = 0;
            } else {
              m_TempCluster[2] = 0;
            }

          }



        } else {
          if (kkk == 64) {
            m_TempCluster[0] = TCFire[kkk];
            m_TempCluster[1] = TCFire[95]; // top
            m_TempCluster[2] = TCFire[63];// right top
            m_TempCluster[3] = TCFire[kkk - 32]; //right
            m_TempCluster[4] = TCFire[kkk - 31]; //right bottom
            m_TempCluster[5] = TCFire[kkk + 1]; //bottom
            m_TempCluster[6] = 0; //bottom left
            m_TempCluster[7] = 0; // left
            m_TempCluster[8] = 0; //top left
          } else if (kkk == 95) {
            m_TempCluster[0] = TCFire[kkk];
            m_TempCluster[1] = TCFire[kkk - 1]; // top
            m_TempCluster[2] = TCFire[kkk - 33]; // right top
            m_TempCluster[3] = TCFire[kkk - 32]; //right
            m_TempCluster[4] = TCFire[32]; //right bottom
            m_TempCluster[5] = TCFire[64]; //bottom
            m_TempCluster[6] = 0; //bottom left
            m_TempCluster[7] = 0; // left
            m_TempCluster[8] = 0; //top left

          } else {
            m_TempCluster[0] = TCFire[kkk];
            m_TempCluster[1] = TCFire[kkk - 1]; // top
            m_TempCluster[2] = TCFire[kkk - 33]; // right top
            m_TempCluster[3] = TCFire[kkk - 32]; //right
            m_TempCluster[4] = TCFire[kkk - 31]; //right bottom
            m_TempCluster[5] = TCFire[kkk + 1]; //bottom
            m_TempCluster[6] = 0; //bottom left
            m_TempCluster[7] = 0; // left
            m_TempCluster[8] = 0; //top left
          }

        }
      }
      for (int iNearTC = 1; iNearTC < 9; iNearTC ++) {
        for (int jNearTC = 1; jNearTC < 9; jNearTC ++) {
          if (m_TempCluster[iNearTC] == 0) {continue;}
          if (iNearTC == jNearTC)continue;
          if (m_TempCluster[iNearTC] == m_TempCluster[jNearTC]) {
            m_TempCluster[jNearTC] = 0;
          }
        }
      }

      int maxTCId = 999;
      double clusterenergy = 0;
      double clustertiming = 0;
      double clusterpositionX = 0;
      double clusterpositionY = 0;
      double clusterpositionZ = 0;
      int noftcincluster = 0;
      for (int iNearTC = 0; iNearTC < 9; iNearTC ++) {
        if (m_TempCluster[iNearTC] == 0) {continue;}
        else {noftcincluster++;}
        clusterenergy +=   TCFireEnergy[m_TempCluster[iNearTC] - 1];
        clustertiming +=   TCFireEnergy[m_TempCluster[iNearTC] - 1] * TCFireTiming[m_TempCluster[iNearTC] - 1];
        clusterpositionX += TCFireEnergy[m_TempCluster[iNearTC] - 1] * TCFirePosition[m_TempCluster[iNearTC] - 1][0];
        clusterpositionY += TCFireEnergy[m_TempCluster[iNearTC] - 1] * TCFirePosition[m_TempCluster[iNearTC] - 1][1];
        clusterpositionZ += TCFireEnergy[m_TempCluster[iNearTC] - 1] * TCFirePosition[m_TempCluster[iNearTC] - 1][2];

      }
      //
      maxTCId = m_TempCluster[0];
      //

      clustertiming /= clusterenergy;
      if (m_Position == 1) {
        clusterpositionX /= clusterenergy;
        clusterpositionY /= clusterenergy;
        clusterpositionZ /= clusterenergy;
      } else if (m_Position == 0) {
        clustertiming =  TCFireTiming[maxTCId - 1];
        clusterpositionX = TCFirePosition[maxTCId - 1][0];
        clusterpositionY = TCFirePosition[maxTCId - 1][1];
        clusterpositionZ = TCFirePosition[maxTCId - 1][2];
      }
      if (clustertiming == 0 && clusterenergy == 0) {continue;}

      TempClusterEnergy.push_back(clusterenergy);
      TempClusterTiming.push_back(clustertiming);
      TempClusterPositionX.push_back(clusterpositionX);
      TempClusterPositionY.push_back(clusterpositionY);
      TempClusterPositionZ.push_back(clusterpositionZ);
      TempNofTCinCluster.push_back(noftcincluster);
      TempMaxTCId.push_back(maxTCId);

      Sort1D.push_back(TempICNTCId);
      Sort1D.push_back(clusterenergy);
      Sort1D.push_back(clustertiming);
      Sort1D.push_back(clusterpositionX);
      Sort1D.push_back(clusterpositionY);
      Sort1D.push_back(clusterpositionZ);
      Sort1D.push_back(noftcincluster);
      Sort1D.push_back(maxTCId);

      Sort2D.push_back(Sort1D);
      Sort1D.clear();

    }

  }

  // Sorting in the order of TC Id

  sort(Sort2D.begin(), Sort2D.end(),
  [](const vector<double>& aa1, const vector<double>& aa2) {return aa1[0] < aa2[0];});

  const int clustersize = Sort2D.size();
  for (int jtc = 0; jtc < clustersize; jtc++) {

    m_ClusterEnergy[1].push_back(Sort2D[jtc][1]);
    m_ClusterTiming[1].push_back(Sort2D[jtc][2]);
    m_ClusterPositionX[1].push_back(Sort2D[jtc][3]);
    m_ClusterPositionY[1].push_back(Sort2D[jtc][4]);
    m_ClusterPositionZ[1].push_back(Sort2D[jtc][5]);
    m_NofTCinCluster[1].push_back(Sort2D[jtc][6]);
    m_MaxTCId[1].push_back(Sort2D[jtc][7]);
  }

  m_FWDNofCluster = m_MaxTCId[1].size();
}
//
//
//
void TrgEclCluster::setBackwardICN(int Method)
{
  std::vector<int> TCFire;
  std::vector<double> TCFireEnergy;
  std::vector<double> TCFireTiming;
  std::vector<std::vector<double>> TCFirePosition;


  std::vector<double> TempClusterEnergy;
  std::vector<double> TempClusterTiming;
  std::vector<double> TempClusterPositionX;
  std::vector<double> TempClusterPositionY;
  std::vector<double> TempClusterPositionZ;
  std::vector<int> TempNofTCinCluster;
  std::vector<int> TempMaxTCId;

  std::vector<double> Sort1D;
  std::vector<std::vector<double>> Sort2D;

  int TempICNTCId = 0;

  TempClusterEnergy.clear();
  TempClusterTiming.clear();
  TempClusterPositionX.clear();
  TempClusterPositionY.clear();
  TempClusterPositionZ.clear();
  TempNofTCinCluster.clear();
  TempMaxTCId.clear();
  Sort1D.clear();
  Sort2D.clear();

  TCFire.clear();
  TCFireEnergy.clear();
  TCFireTiming.clear();
  TCFirePosition.clear();

  TCFire.resize(64, 0);
  TCFireEnergy.resize(64, 0.);
  TCFireTiming.resize(64, 0.);
  TCFirePosition.resize(64, std::vector<double>(3, 0.));

  const int  hit_size  = m_TCId.size();
  for (int ihit = 0 ; ihit < hit_size ; ihit++) {
    if (m_TCId[ihit] < 513) {continue;}
    TCFireEnergy[m_TCId[ihit] - 513] = m_Energy[ihit];
    TCFireTiming[m_TCId[ihit] - 513] = m_Timing[ihit];
    TCFirePosition[m_TCId[ihit] - 513][0] = (m_TCMap->getTCPosition(m_TCId[ihit])).X();
    TCFirePosition[m_TCId[ihit] - 513][1] = (m_TCMap->getTCPosition(m_TCId[ihit])).Y();
    TCFirePosition[m_TCId[ihit] - 513][2] = (m_TCMap->getTCPosition(m_TCId[ihit])).Z();

    //------------------------------------
    // To rearrange the hit map
    //
    //   original       converted
    //  (<- Theta)    (<- Theta)
    //
    //     516  515       32  0
    //     513  514       33  1
    //     520  519  =>   34  2
    //     517  518       35  3
    //     ...            .
    //     576  575       62 30
    //     573  574       63 31
    //
    // Here, TCId-1 becomes TCId=0 and 1.
    //------------------------------------
    int iTCId0 = m_TCId[ihit] - 1;
    int kkk = 0;
    if ((iTCId0 - 512) % 4 == 2) {
      kkk = (iTCId0 - 512) / 2 - 1;
    }
    if ((iTCId0 - 512) % 4 == 1) {
      kkk = ((iTCId0 - 512) + 1) / 2;
    }
    if ((iTCId0 - 512) % 4 == 3) {
      kkk =  32 + ((iTCId0 - 512) - 3) / 2;
    }
    if ((iTCId0 - 512) % 4 == 0) {
      kkk =  33 + ((iTCId0 - 512)) / 2;
    }

    TCFire[kkk] = iTCId0 + 1;

  }


  for (int iii = 0 ; iii < 64 ; iii ++) {

    if (TCFire[iii] == 0) { continue; }

    for (int iinit = 0; iinit < 9; iinit ++) {m_TempCluster[iinit] = 0;}
    if (iii < 32) {
      if (iii == 0) {
        m_TempCluster[0] = TCFire[ 0];
        m_TempCluster[1] = TCFire[31]; // top
        m_TempCluster[2] = 0;// right top
        m_TempCluster[3] = 0; //right
        m_TempCluster[4] = 0; //right bottom
        m_TempCluster[5] = TCFire[ 1]; // bottom
        m_TempCluster[6] = TCFire[33]; // bottom left
        m_TempCluster[7] = TCFire[32]; // left
        m_TempCluster[8] = TCFire[63]; // top left
      } else if (iii == 31) {
        m_TempCluster[0] = TCFire[31];
        m_TempCluster[1] = TCFire[30];
        m_TempCluster[2] = 0;// right top
        m_TempCluster[3] = 0; //right
        m_TempCluster[4] = 0; //right bottom
        m_TempCluster[5] = TCFire[0]; //bottom
        m_TempCluster[6] = TCFire[32]; // bottom left
        m_TempCluster[7] = TCFire[63]; // left
        m_TempCluster[8] = TCFire[62]; //top left
      } else {
        m_TempCluster[0] = TCFire[iii];
        m_TempCluster[1] = TCFire[iii - 1]; // top
        m_TempCluster[2] = 0;// right top
        m_TempCluster[3] = 0; //right
        m_TempCluster[4] = 0; //right bottom
        m_TempCluster[5] = TCFire[iii + 1]; //bottom
        m_TempCluster[6] = TCFire[iii + 33]; //bottom left
        m_TempCluster[7] = TCFire[iii + 32]; // left
        m_TempCluster[8] = TCFire[iii + 31]; //top left
      }
    } else {
      if (iii == 32) {
        m_TempCluster[0] = TCFire[32];
        m_TempCluster[1] = TCFire[63]; // top
        m_TempCluster[2] = TCFire[31];// right top
        m_TempCluster[3] = TCFire[0]; //right
        m_TempCluster[4] = TCFire[1]; //right bottom
        m_TempCluster[5] = TCFire[33]; //bottom
        m_TempCluster[6] = 0; //bottom left
        m_TempCluster[7] = 0; // left
        m_TempCluster[8] = 0; //top left
      } else if (iii == 63) {
        m_TempCluster[0] = TCFire[63];
        m_TempCluster[1] = TCFire[62]; // top
        m_TempCluster[2] = TCFire[30];// right top
        m_TempCluster[3] = TCFire[31]; //right
        m_TempCluster[4] = TCFire[0]; //right bottom
        m_TempCluster[5] = TCFire[32]; //bottom
        m_TempCluster[6] = 0; //bottom left
        m_TempCluster[7] = 0; // left
        m_TempCluster[8] = 0; //top left
      } else {
        m_TempCluster[0] = TCFire[iii];
        m_TempCluster[1] = TCFire[iii - 1]; // top
        m_TempCluster[2] = TCFire[iii - 33]; // right top
        m_TempCluster[3] = TCFire[iii - 32]; //right
        m_TempCluster[4] = TCFire[iii - 31]; //right bottom
        m_TempCluster[5] = TCFire[iii + 1]; //bottom
        m_TempCluster[6] = 0; //bottom left
        m_TempCluster[7] = 0; // left
        m_TempCluster[8] = 0; //top left
      }

    }

    if (!(m_TempCluster[1] != 0 || m_TempCluster[7] != 0)) {
      if (!(m_TempCluster[5] != 0 && m_TempCluster[6] != 0)) {
        TempICNTCId = TCFire[iii];

        if (Method == 1) {
          int maxTCid = 0;
          double maxTCEnergy = 0;
          for (int iTC = 0; iTC < 9; iTC++) { //find center of Cluster
            if (m_TempCluster[iTC] == 0) {continue;}
            if (maxTCEnergy <       TCFireEnergy[m_TempCluster[iTC] - 513]) {
              maxTCEnergy = TCFireEnergy[m_TempCluster[iTC] - 513];
              maxTCid = m_TempCluster[iTC];
            }
          }
          int kkk = 0;
          m_TempCluster[0] = maxTCid;
          if ((m_TempCluster[0] - 513) % 4 == 2) {
            kkk = (m_TempCluster[0] - 513) / 2 - 1;
          }
          if ((m_TempCluster[0] - 513) % 4 == 1) {
            kkk = ((m_TempCluster[0] - 513) + 1) / 2;
          }
          if ((m_TempCluster[0] - 513) % 4 == 3) {
            kkk =  32 + ((m_TempCluster[0] - 513) - 3) / 2;
          }
          if ((m_TempCluster[0] - 513) % 4 == 0) {
            kkk =  33 + ((m_TempCluster[0] - 513)) / 2;
          }

          if (kkk < 32) {
            if (kkk == 0) {
              m_TempCluster[0] = TCFire[ 0];
              m_TempCluster[1] = TCFire[31]; // top
              m_TempCluster[2] = 0;// right top
              m_TempCluster[3] = 0; //right
              m_TempCluster[4] = 0; //right bottom
              m_TempCluster[5] = TCFire[ 1]; // bottom
              m_TempCluster[6] = TCFire[33]; // bottom left
              m_TempCluster[7] = TCFire[32]; // left
              m_TempCluster[8] = TCFire[63]; // top left
            } else if (kkk == 31) {
              m_TempCluster[0] = TCFire[31];
              m_TempCluster[1] = TCFire[30];
              m_TempCluster[2] = 0;// right top
              m_TempCluster[3] = 0; //right
              m_TempCluster[4] = 0; //right bottom
              m_TempCluster[5] = TCFire[0]; //bottom
              m_TempCluster[6] = TCFire[32]; // bottom left
              m_TempCluster[7] = TCFire[63]; // left
              m_TempCluster[8] = TCFire[62]; //top left
            } else {
              m_TempCluster[0] = TCFire[kkk];
              m_TempCluster[1] = TCFire[kkk - 1]; // top
              m_TempCluster[2] = 0;// right top
              m_TempCluster[3] = 0; //right
              m_TempCluster[4] = 0; //right bottom
              m_TempCluster[5] = TCFire[kkk + 1]; //bottom
              m_TempCluster[6] = TCFire[kkk + 33]; //bottom left
              m_TempCluster[7] = TCFire[kkk + 32]; // left
              m_TempCluster[8] = TCFire[kkk + 31]; //top left
            }
          } else {
            if (kkk == 32) {
              m_TempCluster[0] = TCFire[32];
              m_TempCluster[1] = TCFire[63]; // top
              m_TempCluster[2] = TCFire[31];// right top
              m_TempCluster[3] = TCFire[0]; //right
              m_TempCluster[4] = TCFire[1]; //right bottom
              m_TempCluster[5] = TCFire[33]; //bottom
              m_TempCluster[6] = 0; //bottom left
              m_TempCluster[7] = 0; // left
              m_TempCluster[8] = 0; //top left
            } else if (kkk == 63) {
              m_TempCluster[0] = TCFire[63];
              m_TempCluster[1] = TCFire[62]; // top
              m_TempCluster[2] = TCFire[30];// right top
              m_TempCluster[3] = TCFire[31]; //right
              m_TempCluster[4] = TCFire[0]; //right bottom
              m_TempCluster[5] = TCFire[32]; //bottom
              m_TempCluster[6] = 0; //bottom left
              m_TempCluster[7] = 0; // left
              m_TempCluster[8] = 0; //top left
            } else {
              m_TempCluster[0] = TCFire[kkk];
              m_TempCluster[1] = TCFire[kkk - 1]; // top
              m_TempCluster[2] = TCFire[kkk - 33]; // right top
              m_TempCluster[3] = TCFire[kkk - 32]; //right
              m_TempCluster[4] = TCFire[kkk - 31]; //right bottom
              m_TempCluster[5] = TCFire[kkk + 1]; //bottom
              m_TempCluster[6] = 0; //bottom left
              m_TempCluster[7] = 0; // left
              m_TempCluster[8] = 0; //top left
            }

          }
        }

        for (int iNearTC = 1; iNearTC < 9; iNearTC ++) {
          for (int jNearTC = 1; jNearTC < 9; jNearTC ++) {
            if (m_TempCluster[iNearTC] == 0) {continue;}
            if (iNearTC == jNearTC) {continue;}
            if (m_TempCluster[iNearTC] == m_TempCluster[jNearTC]) {
              m_TempCluster[jNearTC] = 0;
            }
          }
        }

        int maxTCId = 999;
        double clusterenergy = 0;
        double clustertiming = 0;
        double clusterpositionX = 0;
        double clusterpositionY = 0;
        double clusterpositionZ = 0;
        int noftcincluster = 0;
        for (int iNearTC = 0; iNearTC < 9; iNearTC ++) {
          if (m_TempCluster[iNearTC] == 0) {continue;}
          else {noftcincluster++;}

          clusterenergy +=   TCFireEnergy[m_TempCluster[iNearTC] - 513];
          clustertiming +=   TCFireEnergy[m_TempCluster[iNearTC] - 513] * TCFireTiming[m_TempCluster[iNearTC] - 513];
          clusterpositionX += TCFireEnergy[m_TempCluster[iNearTC] - 513] * TCFirePosition[m_TempCluster[iNearTC] - 513][0];
          clusterpositionY += TCFireEnergy[m_TempCluster[iNearTC] - 513] * TCFirePosition[m_TempCluster[iNearTC] - 513][1];
          clusterpositionZ += TCFireEnergy[m_TempCluster[iNearTC] - 513] * TCFirePosition[m_TempCluster[iNearTC] - 513][2];


        }
        //
        maxTCId = m_TempCluster[0];
        //

        clustertiming /= clusterenergy;
        if (m_Position == 1) {
          clusterpositionX /= clusterenergy;
          clusterpositionY /= clusterenergy;
          clusterpositionZ /= clusterenergy;
        } else if (m_Position == 0) {
          clustertiming =  TCFireTiming[maxTCId - 513];
          clusterpositionX = TCFirePosition[maxTCId - 513][0];
          clusterpositionY = TCFirePosition[maxTCId - 513][1];
          clusterpositionZ = TCFirePosition[maxTCId - 513][2];
        }

        if (clustertiming == 0 && clusterenergy == 0) {continue;}
        TempClusterEnergy.push_back(clusterenergy);
        TempClusterTiming.push_back(clustertiming);
        TempClusterPositionX.push_back(clusterpositionX);
        TempClusterPositionY.push_back(clusterpositionY);
        TempClusterPositionZ.push_back(clusterpositionZ);
        TempNofTCinCluster.push_back(noftcincluster);
        TempMaxTCId.push_back(maxTCId);


        Sort1D.push_back(TempICNTCId);
        Sort1D.push_back(clusterenergy);
        Sort1D.push_back(clustertiming);
        Sort1D.push_back(clusterpositionX);
        Sort1D.push_back(clusterpositionY);
        Sort1D.push_back(clusterpositionZ);
        Sort1D.push_back(noftcincluster);
        Sort1D.push_back(maxTCId);

        Sort2D.push_back(Sort1D);
        Sort1D.clear();


      }
    }
  }

  sort(Sort2D.begin(), Sort2D.end(),
  [](const vector<double>& aa1, const vector<double>& aa2) {return aa1[0] < aa2[0];});

  const int clustersize = Sort2D.size();
  for (int jtc = 0; jtc < clustersize; jtc++) {
    m_ClusterEnergy[2].push_back(Sort2D[jtc][1]);
    m_ClusterTiming[2].push_back(Sort2D[jtc][2]);
    m_ClusterPositionX[2].push_back(Sort2D[jtc][3]);
    m_ClusterPositionY[2].push_back(Sort2D[jtc][4]);
    m_ClusterPositionZ[2].push_back(Sort2D[jtc][5]);
    m_NofTCinCluster[2].push_back(Sort2D[jtc][6]);
    m_MaxTCId[2].push_back(Sort2D[jtc][7]);
  }

  m_BWDNofCluster = m_MaxTCId[2].size();
}
//
//
//
int
TrgEclCluster::setBarrelICN()
{

  std::vector<int> TCFire;

  TCFire.clear();

  TCFire.resize(432, 0);

  const int  hit_size  = m_TCId.size();
  for (int ihit = 0 ; ihit < hit_size ; ihit++) {
    if (m_TCId[ihit] >= 81 && m_TCId[ihit] <= 512) {
      TCFire[m_TCId[ihit] - 81] = m_TCId[ihit];
    }
  }
  //
  //
  //
  int tc_upper = 0; // check upper TC
  int tc_upper_right = 0; // check right TC
  int tc_right = 0; // check right TC
  int tc_lower_right = 0;
  int tc_lower = 0; // check lower TC
  int tc_lower_left = 0; // check lower TC
  int tc_left = 0;
  int tc_upper_left = 0;

  for (int iii = 0 ; iii < 432 ; iii++) {
    if (TCFire[iii] == 0) { continue; }

    setBarrelICNsub(iii,
                    TCFire,
                    tc_upper,
                    tc_upper_right,
                    tc_right,
                    tc_lower_right,
                    tc_lower,
                    tc_lower_left,
                    tc_left,
                    tc_upper_left);

    if (!(tc_upper != 0 || tc_left != 0)) {
      if (!(tc_lower != 0 && tc_lower_left != 0)) {
        m_BRICN++;
        int phiid = m_TCMap->getTCPhiIdFromTCId(iii + 80 + 1);

        if (phiid == 36 || (phiid > 0 && phiid < 11)) {
          m_Quadrant[1][0]++;
        }
        if (phiid > 8 && phiid < 20) {
          m_Quadrant[1][1]++;
        }
        if (phiid > 17 && phiid < 29) {
          m_Quadrant[1][2]++;
        }
        if ((phiid > 26 && phiid < 37) || phiid == 1) {
          m_Quadrant[1][3]++;
        }

      }
    }// iii loop
  }
  return m_BRICN;

}
//
//
//
void
TrgEclCluster::setBarrelICNsub(int iii,
                               std::vector<int> TCFire,
                               int& tc_upper,
                               int& tc_upper_right,
                               int& tc_right,
                               int& tc_lower_right,
                               int& tc_lower,
                               int& tc_lower_left,
                               int& tc_left,
                               int& tc_upper_left)
{
  // take into account TCId jump at boundary between FAM-9 and FAM-44
  int offset_upper = 0;
  int offset_lower = 0;
  if (iii <= 11) {
    // FAM-9
    offset_upper = 432;
    offset_lower =   0;
  } else if (iii <= 419) {
    // FAM-10 to FAM-43
    offset_upper = 0;
    offset_lower = 0;
  } else {
    // FAM-44
    offset_upper = 0;
    offset_lower = -432;
  }

  // take into account TC at boundary near endcaps
  if (iii % 12 == 0) {
    // TC at most forward side
    tc_upper       = TCFire[iii -  12 + offset_upper];
    tc_upper_right = 0;
    tc_right       = 0;
    tc_lower_right = 0;
    tc_lower       = TCFire[iii +  12 + offset_lower];
    tc_lower_left  = TCFire[iii +  13 + offset_lower];
    tc_left        = TCFire[iii +   1];
    tc_upper_left  = TCFire[iii -  11 + offset_upper];
  } else if (iii % 12 == 11) {
    // TC at most backward side
    tc_upper       = TCFire[iii -  12 + offset_upper];
    tc_upper_right = TCFire[iii -  13 + offset_upper];
    tc_right       = TCFire[iii -   1];
    tc_lower_right = TCFire[iii +  11 + offset_lower];
    tc_lower       = TCFire[iii +  12 + offset_lower];
    tc_lower_left  = 0;
    tc_left        = 0;
    tc_upper_left  = 0;
  } else {
    tc_upper       = TCFire[iii -  12 + offset_upper];
    tc_upper_right = TCFire[iii -  13 + offset_upper];
    tc_right       = TCFire[iii -   1];
    tc_lower_right = TCFire[iii +  11 + offset_lower];
    tc_lower       = TCFire[iii +  12 + offset_lower];
    tc_lower_left  = TCFire[iii +  13 + offset_lower];
    tc_left        = TCFire[iii +   1];
    tc_upper_left  = TCFire[iii -  11 + offset_upper];
  }

  m_TempCluster[0] = iii + 80 + 1; //middle of ICN
  m_TempCluster[1] = tc_upper; // upper
  m_TempCluster[2] = tc_upper_right; //right
  m_TempCluster[3] = tc_right; //right
  m_TempCluster[4] = tc_lower_right; //lower
  m_TempCluster[5] = tc_lower; //lower
  m_TempCluster[6] = tc_lower_left; //lower
  m_TempCluster[7] = tc_left; //lower
  m_TempCluster[8] = tc_upper_left; //lower right;
}
//
//
//
int
TrgEclCluster::setForwardICN()
{

  std::vector<int> TCFire;


  TCFire.clear();

  TCFire.resize(96, 0);


  const int  hit_size  = m_TCId.size();
  for (int ihit = 0 ; ihit < hit_size ; ihit++) {
    if (m_TCId[ihit] > 80) {continue;}


    //------------------------------------
    // To rearrange the hit map
    //
    //   original       converted
    //  (<- Theta)    (<- Theta)
    //
    //   3  2  1       64 32  0
    //   4  5  -       65 33  1
    //   8  7  6  =>   66 34  2
    //   9 10  -       67 35  3
    //  ....           ...
    //  78 77 76       94 62 30
    //  79 80  -       95 63 31
    //
    // Here, TCId-1 becomes TCId=0 and 1.
    //------------------------------------
    int iTCId0 = m_TCId[ihit] - 1;
    int kkk = 0;
    if (iTCId0 % 5 == 0) {
      kkk = (iTCId0 / 5) * 2;
      TCFire[kkk]   = m_TCId[ihit];
      TCFire[kkk + 1] = m_TCId[ihit];
    } else {
      kkk = iTCId0 / 5;
      switch (iTCId0 % 5) {
        case 1 :
          TCFire[32 + 2 * kkk]   = m_TCId[ihit]; break;
        case 2 :
          TCFire[64 + 2 * kkk]   = m_TCId[ihit]; break;
        case 3 :
          TCFire[64 + 2 * kkk + 1] = m_TCId[ihit]; break;
        case 4 :
          TCFire[32 + 2 * kkk + 1] = m_TCId[ihit]; break;
        default:
          break;
      }
    }
  }


  for (int iii = 32 ; iii < 96 ; iii++) {
    for (int iinit = 0; iinit < 9; iinit ++) {m_TempCluster[iinit] = 0;}
    if (TCFire[iii] == 0) { continue; }
    if (iii < 64) {
      if (iii == 32) {
        m_TempCluster[0] = TCFire[iii];
        m_TempCluster[1] = TCFire[63]; // top
        m_TempCluster[2] = 0;// right top
        m_TempCluster[3] = 0; //right
        m_TempCluster[4] = 0; //right bottom
        m_TempCluster[5] = TCFire[iii + 1]; //bottom
        m_TempCluster[6] = TCFire[iii + 33]; //bottom left
        m_TempCluster[7] = TCFire[iii + 32]; // left
        m_TempCluster[8] = TCFire[95]; //top left
      } else if (iii == 63) {
        m_TempCluster[0] = TCFire[iii];
        m_TempCluster[1] = TCFire[iii - 1]; // top
        m_TempCluster[2] = 0;                         // right top
        m_TempCluster[3] = 0;                         //right
        m_TempCluster[4] = 0; //right bottom
        m_TempCluster[5] =  TCFire[32]; //bottom
        m_TempCluster[6] = TCFire[64]; //bottom left
        m_TempCluster[7] = TCFire[iii +        32]; // left
        m_TempCluster[8] = TCFire[iii +    31]; //top left
      } else {
        m_TempCluster[0] = TCFire[iii];
        m_TempCluster[1] = TCFire[iii - 1]; // top
        m_TempCluster[2] = 0; // right top
        m_TempCluster[3] = 0; //right
        m_TempCluster[4] = 0; //right bottom
        m_TempCluster[5] = TCFire[iii + 1]; //bottom
        m_TempCluster[6] = TCFire[iii + 33]; //bottom left
        m_TempCluster[7] = TCFire[iii + 32]; // left
        m_TempCluster[8] = TCFire[iii + 31]; //top left
      }
    } else {
      if (iii == 64) {
        m_TempCluster[0] = TCFire[iii];
        m_TempCluster[1] = TCFire[95]; // top
        m_TempCluster[2] = TCFire[63];// right top
        m_TempCluster[3] = TCFire[iii - 32]; //right
        m_TempCluster[4] = TCFire[iii - 31]; //right bottom
        m_TempCluster[5] = TCFire[iii + 1]; //bottom
        m_TempCluster[6] = 0; //bottom left
        m_TempCluster[7] = 0; // left
        m_TempCluster[8] = 0; //top left
      } else if (iii == 95) {
        m_TempCluster[0] = TCFire[iii];
        m_TempCluster[1] = TCFire[iii - 1]; // top
        m_TempCluster[2] = TCFire[iii - 33]; // right top
        m_TempCluster[3] = TCFire[iii - 32]; //right
        m_TempCluster[4] = TCFire[32]; //right bottom
        m_TempCluster[5] = TCFire[64]; //bottom
        m_TempCluster[6] = 0; //bottom left
        m_TempCluster[7] = 0; // left
        m_TempCluster[8] = 0; //top left

      } else {
        m_TempCluster[0] = TCFire[iii];
        m_TempCluster[1] = TCFire[iii - 1]; // top
        m_TempCluster[2] = TCFire[iii - 33]; // right top
        m_TempCluster[3] = TCFire[iii - 32]; //right
        m_TempCluster[4] = TCFire[iii - 31]; //right bottom
        m_TempCluster[5] = TCFire[iii + 1]; //bottom
        m_TempCluster[6] = 0; //bottom left
        m_TempCluster[7] = 0; // left
        m_TempCluster[8] = 0; //top left
      }

    }
    if (!(m_TempCluster[1] != 0 || m_TempCluster[7] != 0)) {
      if (!(m_TempCluster[5] != 0 && m_TempCluster[6] != 0)) {
        m_FWDICN++;
        int phiid = m_TCMap->getTCPhiIdFromTCId(TCFire[iii]);

        if (phiid == 32 || (phiid > 0 && phiid < 10)) {
          m_Quadrant[0][0]++;
        }
        if (phiid > 7 && phiid < 18) {
          m_Quadrant[0][1]++;
        }
        if (phiid > 15 && phiid < 26) {
          m_Quadrant[0][2]++;
        }
        if ((phiid > 22 && phiid < 33) || phiid == 1) {
          m_Quadrant[0][3]++;
        }


      }
    }
  }
  return m_FWDICN;
}
//
//
//
int TrgEclCluster::setBackwardICN()
{
  std::vector<int> TCFire;


  TCFire.clear();

  TCFire.resize(64, 0);



  const int  hit_size  = m_TCId.size();
  for (int ihit = 0 ; ihit < hit_size ; ihit++) {
    if (m_TCId[ihit] < 513) {continue;}

    //    TCFire[m_TCId[ihit] - 513] = m_TCId[ihit];
    //------------------------------------
    // To rearrange the hit map
    //
    //   original       converted
    //  (<- Theta)    (<- Theta)
    //
    //     516  515       32  0
    //     513  514       33  1
    //     520  519  =>   34  2
    //     517  518       35  3
    //     ...            .
    //     576  575       62 30
    //     573  574       63 31
    //
    // Here, TCId-1 becomes TCId=0 and 1.
    //------------------------------------
    int iTCId0 = m_TCId[ihit] - 1;
    int kkk = 0;
    if ((iTCId0 - 512) % 4 == 2) {
      kkk = (iTCId0 - 512) / 2 - 1;
    }
    if ((iTCId0 - 512) % 4 == 1) {
      kkk = ((iTCId0 - 512) + 1) / 2;
    }
    if ((iTCId0 - 512) % 4 == 3) {
      kkk =  32 + ((iTCId0 - 512) - 3) / 2;
    }
    if ((iTCId0 - 512) % 4 == 0) {
      kkk =  33 + ((iTCId0 - 512)) / 2;
    }

    TCFire[kkk] = iTCId0 + 1;

  }


  for (int iii = 0 ; iii < 64 ; iii ++) {

    if (TCFire[iii] == 0) { continue; }

    for (int iinit = 0; iinit < 9; iinit ++) {m_TempCluster[iinit] = 0;}
    if (iii < 32) {
      if (iii == 0) {
        m_TempCluster[0] = TCFire[ 0];
        m_TempCluster[1] = TCFire[31]; // top
        m_TempCluster[2] = 0;// right top
        m_TempCluster[3] = 0; //right
        m_TempCluster[4] = 0; //right bottom
        m_TempCluster[5] = TCFire[ 1]; // bottom
        m_TempCluster[6] = TCFire[33]; // bottom left
        m_TempCluster[7] = TCFire[32]; // left
        m_TempCluster[8] = TCFire[63]; // top left
      } else if (iii == 31) {
        m_TempCluster[0] = TCFire[31];
        m_TempCluster[1] = TCFire[30];
        m_TempCluster[2] = 0;// right top
        m_TempCluster[3] = 0; //right
        m_TempCluster[4] = 0; //right bottom
        m_TempCluster[5] = TCFire[0]; //bottom
        m_TempCluster[6] = TCFire[32]; // bottom left
        m_TempCluster[7] = TCFire[63]; // left
        m_TempCluster[8] = TCFire[62]; //top left
      } else {
        m_TempCluster[0] = TCFire[iii];
        m_TempCluster[1] = TCFire[iii - 1]; // top
        m_TempCluster[2] = 0;// right top
        m_TempCluster[3] = 0; //right
        m_TempCluster[4] = 0; //right bottom
        m_TempCluster[5] = TCFire[iii + 1]; //bottom
        m_TempCluster[6] = TCFire[iii + 33]; //bottom left
        m_TempCluster[7] = TCFire[iii + 32]; // left
        m_TempCluster[8] = TCFire[iii + 31]; //top left
      }
    } else {
      if (iii == 32) {
        m_TempCluster[0] = TCFire[32];
        m_TempCluster[1] = TCFire[63]; // top
        m_TempCluster[2] = TCFire[31];// right top
        m_TempCluster[3] = TCFire[0]; //right
        m_TempCluster[4] = TCFire[1]; //right bottom
        m_TempCluster[5] = TCFire[33]; //bottom
        m_TempCluster[6] = 0; //bottom left
        m_TempCluster[7] = 0; // left
        m_TempCluster[8] = 0; //top left
      } else if (iii == 63) {
        m_TempCluster[0] = TCFire[63];
        m_TempCluster[1] = TCFire[62]; // top
        m_TempCluster[2] = TCFire[30];// right top
        m_TempCluster[3] = TCFire[31]; //right
        m_TempCluster[4] = TCFire[0]; //right bottom
        m_TempCluster[5] = TCFire[32]; //bottom
        m_TempCluster[6] = 0; //bottom left
        m_TempCluster[7] = 0; // left
        m_TempCluster[8] = 0; //top left
      } else {
        m_TempCluster[0] = TCFire[iii];
        m_TempCluster[1] = TCFire[iii - 1]; // top
        m_TempCluster[2] = TCFire[iii - 33]; // right top
        m_TempCluster[3] = TCFire[iii - 32]; //right
        m_TempCluster[4] = TCFire[iii - 31]; //right bottom
        m_TempCluster[5] = TCFire[iii + 1]; //bottom
        m_TempCluster[6] = 0; //bottom left
        m_TempCluster[7] = 0; // left
        m_TempCluster[8] = 0; //top left
      }

    }

    if (!(m_TempCluster[1] != 0 || m_TempCluster[7] != 0)) {
      if (!(m_TempCluster[5] != 0 && m_TempCluster[6] != 0)) {
        m_BWDICN ++;
        int phiid = m_TCMap->getTCPhiIdFromTCId(TCFire[iii]);

        if (phiid == 32 || (phiid > 0 && phiid < 10)) {
          m_Quadrant[2][0]++;
        }
        if (phiid > 7 && phiid < 18) {
          m_Quadrant[2][1]++;
        }
        if (phiid > 15 && phiid < 26) {
          m_Quadrant[2][2]++;
        }
        if ((phiid > 22 && phiid < 33) || phiid == 1) {
          m_Quadrant[2][3]++;
        }


      }
    }
  }

  return m_BWDICN;
}
//
//
//
int TrgEclCluster::getNofExceedCluster()
{
  int ncluster =
    m_ClusterEnergy[0].size() +
    m_ClusterEnergy[1].size() +
    m_ClusterEnergy[2].size();
  if (ncluster > m_LimitNCluster) {
    return ncluster;
  } else {
    return 0;
  }
}
//
//===<END>
//
