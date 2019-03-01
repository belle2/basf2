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

  int vertSectors         = 1;
  int angleSectors        = 1;
  int minimumClusterSize  = 1;
  int maximumClusterSize  = 1000;
  int actualPositionX     = 0;
  int actualPositionY     = 0;
  double unitX = 0., unitY = 0.;
  double angleRange = 0., vertRange = 0.;
  double left = 0., right = 0., up = 0., down = 0.;
  vector<DATCONHoughSpaceClusterCand> HoughSpaceClusterCandCopy;
  vector<unsigned int> idList;
  vector<unsigned int> mergedList;
  TVector2 CandidateCoordinates;

  int** ArrayOfActiveHoughSpaceSectors;

  if (m_usePhase2Simulation) {
    B2WARNING("This mode is not yet implemented, nothing will happen! Return...");
    return;
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

  m_clusterCount = 1;

  for (auto it = HoughSpaceClusterCandCopy.begin(); it != HoughSpaceClusterCandCopy.end(); it++) {
    idList = it->getIdList();
    mergedList = idList;
    CandidateCoordinates = it->getCoord();
    actualPositionX = (int)CandidateCoordinates.X();
    actualPositionY = (int)CandidateCoordinates.Y();
    if (ArrayOfActiveHoughSpaceSectors[actualPositionY][actualPositionX] == -1) {
      m_clusterInitialPosition.Set((double)actualPositionX, (double)actualPositionY);
      m_clusterCenterOfGravity.Set((double)actualPositionX, (double)actualPositionY);
      m_clusterSize = 1;
      DepthFirstSearch(uSide, ArrayOfActiveHoughSpaceSectors, angleSectors, vertSectors,
                       actualPositionX, actualPositionY, mergedList);
      m_clusterCount++;
      if (m_clusterSize >= minimumClusterSize && m_clusterSize <= maximumClusterSize) {
        double CoGX = left + unitX * (((double)m_clusterCenterOfGravity.X() / (double)(m_clusterSize)) + 0.5);
        double CoGY = up   - unitY * (((double)m_clusterCenterOfGravity.Y() / (double)(m_clusterSize)) + 0.5);
        if (uSide) {
          uTrackCand.push_back(DATCONTrackCand(mergedList, TVector2(CoGX, CoGY)));
        } else {
          vTrackCand.push_back(DATCONTrackCand(mergedList, TVector2(CoGX, CoGY)));
        }
      }
    }
  }
}

void
DATCONTrackingModule::DepthFirstSearch(bool uSide, int** ArrayOfActiveHoughSpaceSectors, int angleSectors, int vertSectors,
                                       int actualPositionX, int actualPositionY, vector<unsigned int>& mergedList)
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

  ArrayOfActiveHoughSpaceSectors[actualPositionY][actualPositionX] = m_clusterCount;
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

          m_clusterCenterOfGravity += TVector2(l, k);
          m_clusterSize++;
          if (m_clusterSize >= maximumClusterSize || abs((int)m_clusterInitialPosition.X() - actualPositionX) >= maximumClusterSizeX
              || abs((int)m_clusterInitialPosition.Y() - actualPositionY) >= maximumClusterSizeY) {
            return;
          }
          DepthFirstSearch(uSide, ArrayOfActiveHoughSpaceSectors, angleSectors, vertSectors, l, k, mergedList);
        }
      }
    }
  }
}
