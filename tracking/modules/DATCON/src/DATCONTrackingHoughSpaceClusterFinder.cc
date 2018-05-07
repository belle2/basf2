/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Michael Schnell, Christian Wessel                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/modules/DATCON/DATCONTrackingModule.h>

using namespace std;
using namespace Belle2;

void
DATCONTrackingModule::FindHoughSpaceCluster(bool uSide)
{

  int vertSectors         =  0;
  int angleSectors        =  0;
  int* clusterCount = new int(1);
  int* clusterSize  = new int(1);
  int minimumClusterSize = 1;
  int maximumClusterSize = 1000;
  int actualPositionX     =   0;
  int actualPositionY     =   0;
  int* initialPositionX = new int(0);
  int* initialPositionY = new int(0);
  double unitX, unitY;
  double angleRange, vertRange;
  double left, right, up, down;
  vector<DATCONHoughSpaceClusterCand> HoughSpaceClusterCandCopy;
  vector<unsigned int> idList;
  vector<unsigned int> mergedList;
  TVector2 CandidateCoordinates;

  TVector2* CenterOfGravity = new TVector2(0, 0);

  vector<int> clusterSizes;
  vector<TVector2> CoG;

  int** ArrayOfActiveHoughSpaceSectors;

  if (m_usePhase2Simulation) {
    if (uSide) {
      angleSectors = m_nPhase2PhiSectors;
      vertSectors  = m_nPhase2PhiVerticalSectors;
      left         = -m_Phase2PhiRange;
      right        = m_Phase2PhiRange;
      up           = m_Phase2PhiVerticalRange;
      down         = -m_Phase2PhiVerticalRange;
    } else {
      angleSectors = m_nPhase2ThetaSectors;
      vertSectors  = m_nPhase2ThetaVerticalSectors;
      left         = -m_Phase2ThetaRange;
      right        = m_Phase2ThetaRange;
      up           = m_Phase2ThetaVerticalRange;
      down         = -m_Phase2ThetaVerticalRange;
    }
  } else {
    if (uSide) {
      angleSectors = m_nAngleSectorsU;
      vertSectors  = m_nVertSectorsU;
      HoughSpaceClusterCandCopy = uHoughSpaceClusterCand;
      ArrayOfActiveHoughSpaceSectors = ArrayOfActiveSectorsPhiHS;
      minimumClusterSize  = m_MinimumPhiHSClusterSize;
      maximumClusterSize  = m_MaximumPhiHSClusterSize;
      left         = -M_PI;
      right        = M_PI;
      up           = m_rectSizeU;
      down         = -m_rectSizeU;
    } else {
      angleSectors = m_nAngleSectorsV;
      vertSectors  = m_nVertSectorsV;
      HoughSpaceClusterCandCopy = vHoughSpaceClusterCand;
      ArrayOfActiveHoughSpaceSectors = ArrayOfActiveSectorsThetaHS;
      minimumClusterSize  = m_MinimumThetaHSClusterSize;
      maximumClusterSize  = m_MaximumThetaHSClusterSize;
      left         = -M_PI;
      right        = 0.;
      up           = m_rectSizeV;
      down         = -m_rectSizeV;
    }
  }

  angleRange = (right - left);
  vertRange  = (up - down);
  unitX      = angleRange / (double)(angleSectors);
  unitY      = vertRange / (double)(vertSectors);

  // cell content meanings:
  // -1      : active sector, not yet visited
  // 0       : non-active sector (will never be visited, only checked)
  // 1,2,3...: index of the clusters

  for (auto it = HoughSpaceClusterCandCopy.begin(); it != HoughSpaceClusterCandCopy.end(); it++) {
    idList = it->getIdList();
    mergedList = idList;
    CandidateCoordinates = it->getCoord();
    int j = (int)CandidateCoordinates.X();
    int i = (int)CandidateCoordinates.Y();
    if (ArrayOfActiveHoughSpaceSectors[i][j] == -1) {
      actualPositionX = j;
      actualPositionY = i;
      (*initialPositionX) = actualPositionX;
      (*initialPositionY) = actualPositionY;
      CenterOfGravity->Set((actualPositionX + 0.5), (actualPositionY + 0.5));
      (*clusterSize) = 1;
      DepthFirstSearch(uSide, ArrayOfActiveHoughSpaceSectors, angleSectors, vertSectors, initialPositionX, initialPositionY,
                       actualPositionX, actualPositionY, clusterCount, clusterSize, CenterOfGravity, mergedList);
      (*clusterCount)++;
      if ((*clusterSize) >= minimumClusterSize && (*clusterSize) <= maximumClusterSize) {
        double CoGX = left + unitX * CenterOfGravity->X() / (double)(*clusterSize);
//         double CoGY = down + unitY * CenterOfGravity->Y() / (double)(*clusterSize);
        double CoGY = up - unitY * CenterOfGravity->Y() / (double)(*clusterSize);
        CenterOfGravity->Set(CoGX, CoGY);
        if (uSide) {
          uTrackCand.push_back(DATCONTrackCand(mergedList, (*CenterOfGravity)));
        } else {
          vTrackCand.push_back(DATCONTrackCand(mergedList, (*CenterOfGravity)));
        }
      }
    }
  }

  delete[] clusterCount;
  delete[] clusterSize;
  delete[] initialPositionX;
  delete[] initialPositionY;

}

void
DATCONTrackingModule::DepthFirstSearch(bool uSide, int** ArrayOfActiveHoughSpaceSectors, int angleSectors, int vertSectors,
                                       int* initialPositionX, int* initialPositionY, int actualPositionX, int actualPositionY, int* clusterCount, int* clusterSize,
                                       TVector2* CenterOfGravity, vector<unsigned int>& mergedList)
{
  vector<unsigned int> mergeMeIDList;
  int maximumClusterSize;
  int maximumClusterSizeX;
  int maximumClusterSizeY;
  vector<DATCONHoughSpaceClusterCand> HoughSpaceClusterCandCopy;
  TVector2 CandidateCoordinates;

  if (uSide) {
    HoughSpaceClusterCandCopy = uHoughSpaceClusterCand;
    maximumClusterSize  = m_MaximumPhiHSClusterSize;
    maximumClusterSizeX = m_MaximumPhiHSClusterSizeX;
    maximumClusterSizeY = m_MaximumPhiHSClusterSizeY;
  } else {
    HoughSpaceClusterCandCopy = vHoughSpaceClusterCand;
    maximumClusterSize  = m_MaximumThetaHSClusterSize;
    maximumClusterSizeX = m_MaximumThetaHSClusterSizeX;
    maximumClusterSizeY = m_MaximumThetaHSClusterSizeY;
  }

  ArrayOfActiveHoughSpaceSectors[actualPositionY][actualPositionX] = *clusterCount;
  for (int k = actualPositionY; k >= actualPositionY - 1; k--) {
    for (int l = actualPositionX; l <= actualPositionX + 1; l++) {
      if (k >= 0  && k < vertSectors && l >= 0 && l < angleSectors) {
        if (ArrayOfActiveHoughSpaceSectors[k][l] == -1) {
          for (auto it = HoughSpaceClusterCandCopy.begin(); it != HoughSpaceClusterCandCopy.end(); it++) {
            mergeMeIDList = it->getIdList();
            CandidateCoordinates = it->getCoord();
            int j = (int)CandidateCoordinates.X();
            int i = (int)CandidateCoordinates.Y();
            if (j == l && i == k) {
              break;
            }
          }
          mergeIdList(mergedList, mergeMeIDList);

          (*CenterOfGravity) += TVector2((l + 0.5), (k + 0.5));  // TODO: check whether k-0.5 would be better...
          (*clusterSize)++;
          if ((*clusterSize) >= maximumClusterSize || fabs((*initialPositionX) - actualPositionX) >= maximumClusterSizeX
              || fabs((*initialPositionY) - actualPositionY) >= maximumClusterSizeY) {
            return;
          }
          DepthFirstSearch(uSide, ArrayOfActiveHoughSpaceSectors, angleSectors, vertSectors, initialPositionX, initialPositionY, l, k,
                           clusterCount, clusterSize, CenterOfGravity, mergedList);

        }
      }
    }
  }
}
