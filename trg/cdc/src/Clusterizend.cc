/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include "trg/cdc/NDFinderDefs.h"
#include "trg/cdc/Clusterizend.h"
using namespace Belle2;

std::vector<SimpleCluster> Clusterizend::makeClusters()
{
  std::vector<SimpleCluster> candidateClusters;
  c3array houghSpaceBackup = *m_houghSpace;
  for (unsigned char quadrant = 0; quadrant < 4; ++quadrant) {
    for (unsigned char _ = 0; _ < m_params.iterations; ++_) {
      auto [globalMax, peakWeight] = getGlobalMax(quadrant);
      if (peakWeight < m_params.minPeakWeight || peakWeight == 0) {
        *m_houghSpace = houghSpaceBackup;
        break;
      }
      auto [newCluster, totalWeight] = createCluster(globalMax);
      if (totalWeight >= m_params.minTotalWeight) {
        candidateClusters.push_back(newCluster);
      }
      deleteMax(globalMax);
    }
    *m_houghSpace = houghSpaceBackup;
  }
  return candidateClusters;
}

std::pair<cell_index, unsigned long> Clusterizend::getGlobalMax(const unsigned char quadrant)
{
  unsigned long maxValue = 0;
  cell_index maxIndex = {0, 0, 0};
  c3index quadrantStart = m_params.nPhi / 8 + quadrant * (m_params.nPhi / 4);
  c3index quadrantEnd = m_params.nPhi / 8 + (quadrant + 1) * (m_params.nPhi / 4);
  for (c3index omegaIdx = 0; omegaIdx < m_params.nOmega; ++omegaIdx) {
    for (c3index phiIdx = quadrantStart; phiIdx < quadrantEnd; ++phiIdx) {
      c3index phiIdxMod = phiIdx % m_params.nPhi;
      for (c3index thetaIdx = 0; thetaIdx < m_params.nTheta; ++thetaIdx) {
        if ((*m_houghSpace)[omegaIdx][phiIdxMod][thetaIdx] > maxValue) {
          maxValue = (*m_houghSpace)[omegaIdx][phiIdxMod][thetaIdx];
          maxIndex = {omegaIdx, phiIdxMod, thetaIdx};
        }
      }
    }
  }
  return {maxIndex, maxValue};
}


std::pair<SimpleCluster, unsigned long> Clusterizend::createCluster(const cell_index& maxIndex)
{
  SimpleCluster fixedCluster;
  unsigned long totalClusterWeight = 0;

  c3index omegaMax = maxIndex[0];
  c3index phiMax = maxIndex[1];
  c3index thetaMax = maxIndex[2];

  c3index thetaLower = std::max<int>(0, thetaMax - 1);
  c3index thetaUpper = std::min<int>(m_params.nTheta, thetaMax + 2);

  /* First 3x3: omegaMax */
  ClusterBounds firstBounds = {
    thetaLower,
    thetaUpper,
    phiMax - 1,
    phiMax + 2,
    omegaMax
  };
  addClusterCells(firstBounds, fixedCluster, totalClusterWeight);

  /* Second 3x3: omegaMax - 1 (if valid) */
  if (omegaMax - 1 >= 0) {
    ClusterBounds secondBounds = {
      thetaLower,
      thetaUpper,
      phiMax + 1,
      phiMax + 4,
      omegaMax - 1
    };
    addClusterCells(secondBounds, fixedCluster, totalClusterWeight);
  }

  /* Third 3x3: omegaMax + 1 (if valid) */
  if (omegaMax + 1 < m_params.nOmega) {
    ClusterBounds thirdBounds = {
      thetaLower,
      thetaUpper,
      phiMax - 3,
      phiMax,
      omegaMax + 1
    };
    addClusterCells(thirdBounds, fixedCluster, totalClusterWeight);
  }
  return {fixedCluster, totalClusterWeight};
}


void Clusterizend::addClusterCells(const ClusterBounds& bounds, SimpleCluster& fixedCluster, unsigned long& totalClusterWeight)
{
  for (c3index thetaIdx = bounds.thetaLowerBound; thetaIdx < bounds.thetaUpperBound; ++thetaIdx) {
    for (c3index phiIdx = bounds.phiLowerBound; phiIdx < bounds.phiUpperBound; ++phiIdx) {
      c3index phiIdxMod = (phiIdx + m_params.nPhi) % m_params.nPhi;
      cell_index newMemberIndex = {bounds.omega, phiIdxMod, thetaIdx};
      fixedCluster.append(newMemberIndex);
      totalClusterWeight += (*m_houghSpace)[bounds.omega][phiIdxMod][thetaIdx];
    }
  }
}


void Clusterizend::deleteMax(const cell_index& maxIndex)
{
  c3index omegaMax = maxIndex[0];
  c3index phiMax = maxIndex[1];
  c3index thetaMax = maxIndex[2];

  c3index thetaLowerBound = std::max<int>(0, thetaMax - m_params.thetaTrim);
  c3index thetaUpperBound = std::min<int>(m_params.nTheta, thetaMax + m_params.thetaTrim + 1);
  c3index omegaLowerBound = std::max<int>(0, omegaMax - m_params.omegaTrim);
  c3index omegaUpperBound = std::min<int>(m_params.nOmega, omegaMax + m_params.omegaTrim + 1);

  for (c3index thetaIdx = thetaLowerBound; thetaIdx < thetaUpperBound; ++thetaIdx) {
    for (c3index omegaIdx = omegaLowerBound; omegaIdx < omegaUpperBound; ++omegaIdx) {
      c3index phiIndex = phiMax + omegaMax - omegaIdx;
      c3index relativePhi = phiIndex - phiMax;

      if (relativePhi > 0) {
        DeletionBounds firstBounds = {
          phiIndex - m_params.phiTrim,
          static_cast<c3index>(phiIndex + m_params.phiTrim + std::floor(2.4 * relativePhi)),
          omegaIdx,
          thetaIdx
        };
        clearHoughSpaceRow(firstBounds);
      } else if (relativePhi < 0) {
        DeletionBounds secondBounds = {
          static_cast<c3index>(phiIndex - m_params.phiTrim + std::ceil(2.4 * relativePhi)),
          phiIndex + m_params.phiTrim + 1,
          omegaIdx,
          thetaIdx
        };
        clearHoughSpaceRow(secondBounds);
      } else {
        DeletionBounds thirdBounds = {
          phiIndex - m_params.phiTrim,
          phiIndex + m_params.phiTrim + 1,
          omegaIdx,
          thetaIdx
        };
        clearHoughSpaceRow(thirdBounds);
      }
    }
  }
}


void Clusterizend::clearHoughSpaceRow(const DeletionBounds& bounds)
{
  for (c3index phiIdx = bounds.phiLowerBound; phiIdx < bounds.phiUpperBound; ++phiIdx) {
    c3index phiIdxMod = (phiIdx + m_params.nPhi) % m_params.nPhi;
    (*m_houghSpace)[bounds.omega][phiIdxMod][bounds.theta] = 0;
  }
}
