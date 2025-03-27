/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <vector>
#include <utility>
#include <cmath>
#include "trg/cdc/NDFinderDefs.h"
#include "trg/cdc/Clusterizend.h"

using namespace Belle2;

// Main function to find the clusters in the Hough space
std::vector<SimpleCluster> Clusterizend::makeClusters()
{
  std::vector<SimpleCluster> candidateClusters;
  c3array houghSpaceBackup = *m_houghSpace;
  for (unsigned short quadrant = 0; quadrant < 4; ++quadrant) {
    for (unsigned short _ = 0; _ < m_clustererParams.iterations; ++_) {
      auto [globalMax, peakWeight] = getGlobalMax(quadrant);
      if (peakWeight < m_clustererParams.minPeakWeight || peakWeight == 0) {
        *m_houghSpace = houghSpaceBackup;
        break;
      }
      auto [newCluster, totalWeight] = createCluster(globalMax);
      if (totalWeight >= m_clustererParams.minTotalWeight) {
        candidateClusters.push_back(newCluster);
      }
      deleteGlobalMax(globalMax);
    }
    *m_houghSpace = houghSpaceBackup;
  }
  return candidateClusters;
}

// Returns the global maximum index and weight for one quadrant
std::pair<cell_index, unsigned long> Clusterizend::getGlobalMax(const unsigned short quadrant)
{
  unsigned long maxValue = 0;
  cell_index maxIndex = {0, 0, 0};
  c3index quadrantStart = m_clustererParams.nPhi / 8 + quadrant * (m_clustererParams.nPhi / 4);
  c3index quadrantEnd = m_clustererParams.nPhi / 8 + (quadrant + 1) * (m_clustererParams.nPhi / 4);
  for (c3index omegaIdx = 0; omegaIdx < m_clustererParams.nOmega; ++omegaIdx) {
    for (c3index phiIdx = quadrantStart; phiIdx < quadrantEnd; ++phiIdx) {
      c3index phiIdxMod = phiIdx % m_clustererParams.nPhi;
      for (c3index thetaIdx = 0; thetaIdx < m_clustererParams.nTheta; ++thetaIdx) {
        if ((*m_houghSpace)[omegaIdx][phiIdxMod][thetaIdx] > maxValue) {
          maxValue = (*m_houghSpace)[omegaIdx][phiIdxMod][thetaIdx];
          maxIndex = {omegaIdx, phiIdxMod, thetaIdx};
        }
      }
    }
  }
  return {maxIndex, maxValue};
}

// Creates the surrounding cluster (fixed shape) around the global maximum index
std::pair<SimpleCluster, unsigned long> Clusterizend::createCluster(const cell_index& maxIndex)
{
  SimpleCluster fixedCluster;
  unsigned long totalClusterWeight = 0;

  c3index omegaMax = maxIndex[0];
  c3index phiMax = maxIndex[1];
  c3index thetaMax = maxIndex[2];

  c3index thetaLower = std::max<short>(0, thetaMax - 1);
  c3index thetaUpper = std::min<short>(m_clustererParams.nTheta, thetaMax + 2);

  // First 3x3: omegaMax
  ClusterBounds firstBounds = {
    thetaLower,
    thetaUpper,
    phiMax - 1,
    phiMax + 2,
    omegaMax
  };
  addClusterCells(firstBounds, fixedCluster, totalClusterWeight);

  // Second 3x3: omegaMax - 1 (if valid)
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

  // Third 3x3: omegaMax + 1 (if valid)
  if (omegaMax + 1 < m_clustererParams.nOmega) {
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

// Adds the 3x3 cluster cells (fixed shape) for each of the three omega bins)
void Clusterizend::addClusterCells(const ClusterBounds& bounds, SimpleCluster& fixedCluster, unsigned long& totalClusterWeight)
{
  for (c3index thetaIdx = bounds.thetaLowerBound; thetaIdx < bounds.thetaUpperBound; ++thetaIdx) {
    for (c3index phiIdx = bounds.phiLowerBound; phiIdx < bounds.phiUpperBound; ++phiIdx) {
      c3index phiIdxMod = (phiIdx + m_clustererParams.nPhi) % m_clustererParams.nPhi;
      cell_index newMemberIndex = {bounds.omega, phiIdxMod, thetaIdx};
      fixedCluster.appendCell(newMemberIndex);
      totalClusterWeight += (*m_houghSpace)[bounds.omega][phiIdxMod][thetaIdx];
    }
  }
}

// Deletes the surroundings of the global maximum with a butterfly cutout
void Clusterizend::deleteGlobalMax(const cell_index& maxIndex)
{
  c3index omegaMax = maxIndex[0];
  c3index phiMax = maxIndex[1];

  c3index omegaLowerBound = std::max<unsigned short>(0, omegaMax - m_clustererParams.omegaTrim);
  c3index omegaUpperBound = std::min<unsigned short>(m_clustererParams.nOmega, omegaMax + m_clustererParams.omegaTrim + 1);

  for (c3index thetaIdx = 0; thetaIdx < m_clustererParams.nTheta; ++thetaIdx) {
    for (c3index omegaIdx = omegaLowerBound; omegaIdx < omegaUpperBound; ++omegaIdx) {
      c3index phiIndex = phiMax + omegaMax - omegaIdx;
      c3index relativePhi = phiIndex - phiMax;

      if (relativePhi > 0) {
        DeletionBounds firstBounds = {
          phiIndex - m_clustererParams.phiTrim,
          static_cast<c3index>(phiIndex + m_clustererParams.phiTrim + std::floor(2.4 * relativePhi)),
          omegaIdx,
          thetaIdx
        };
        clearHoughSpaceRow(firstBounds);
      } else if (relativePhi < 0) {
        DeletionBounds secondBounds = {
          static_cast<c3index>(phiIndex - m_clustererParams.phiTrim + std::ceil(2.4 * relativePhi)),
          phiIndex + m_clustererParams.phiTrim + 1,
          omegaIdx,
          thetaIdx
        };
        clearHoughSpaceRow(secondBounds);
      } else {
        DeletionBounds thirdBounds = {
          phiIndex - m_clustererParams.phiTrim,
          phiIndex + m_clustererParams.phiTrim + 1,
          omegaIdx,
          thetaIdx
        };
        clearHoughSpaceRow(thirdBounds);
      }
    }
  }
}

// Clears a single omega row
void Clusterizend::clearHoughSpaceRow(const DeletionBounds& bounds)
{
  for (c3index phiIdx = bounds.phiLowerBound; phiIdx < bounds.phiUpperBound; ++phiIdx) {
    c3index phiIdxMod = (phiIdx + m_clustererParams.nPhi) % m_clustererParams.nPhi;
    (*m_houghSpace)[bounds.omega][phiIdxMod][bounds.theta] = 0;
  }
}
