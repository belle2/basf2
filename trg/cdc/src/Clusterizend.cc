/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <vector>
#include <utility>
#include <array>
#include <cmath>
#include "trg/cdc/NDFinderDefs.h"
#include "trg/cdc/Clusterizend.h"

using namespace Belle2;

// Create all the clusters in the Hough space
std::vector<SimpleCluster> Clusterizend::makeClusters()
{
  std::vector<SimpleCluster> candidateClusters;
  c3array houghSpaceBackup;
  if (m_clustererParams.iterations > 1) houghSpaceBackup = *m_houghSpace;
  for (unsigned short quadrant = 0; quadrant < 4; ++quadrant) {
    for (unsigned short section = 0; section < 4; ++section) {
      std::array<c3index, 2> sectionBounds = getSectionBounds(quadrant, section);
      iterateOverSection(sectionBounds, candidateClusters);
      if (m_clustererParams.iterations > 1) *m_houghSpace = houghSpaceBackup;
    }
  }
  return candidateClusters;
}

// Get the phi bounds of one quadrant section
std::array<c3index, 2> Clusterizend::getSectionBounds(const unsigned short quadrant, const unsigned section)
{
  c3index quadrantSize = m_clustererParams.nPhi / 4;
  c3index quadrantOffset = m_clustererParams.nPhi / 8;
  c3index sectionSize = quadrantSize / 4;

  c3index quadrantStart = quadrant * quadrantSize + quadrantOffset;
  c3index sectionStart = quadrantStart + section * sectionSize;
  c3index sectionEnd = sectionStart + sectionSize;

  return {sectionStart, sectionEnd};
}

// Iterate m_clustererParams.iterations times over one section
void Clusterizend::iterateOverSection(const std::array<c3index, 2>& sectionBounds, std::vector<SimpleCluster>& candidateClusters)
{
  for (unsigned short _ = 0; _ < m_clustererParams.iterations; ++_) {
    auto [sectionPeak, peakWeight] = getSectionPeak(sectionBounds);
    if (peakWeight < m_clustererParams.minPeakWeight || peakWeight == 0) {
      break;
    }
    SimpleCluster newCluster = createCluster(sectionPeak);
    unsigned int totalWeight = calculateTotalWeight(newCluster);
    newCluster.setPeakCell(sectionPeak);
    newCluster.setPeakWeight(peakWeight);
    newCluster.setTotalWeight(totalWeight);
    if (totalWeight >= m_clustererParams.minTotalWeight) {
      candidateClusters.push_back(newCluster);
    }
    if (m_clustererParams.iterations > 1) deletePeakSurroundings(sectionPeak);
  }
}

// Returns the global section peak index and weight
std::pair<cell_index, unsigned int> Clusterizend::getSectionPeak(const std::array<c3index, 2>& sectionBounds)
{
  unsigned int peakValue = 0;
  cell_index peakCell = {0, 0, 0};
  for (c3index omegaIdx = 0; omegaIdx < m_clustererParams.nOmega; ++omegaIdx) {
    for (c3index phiIdx = sectionBounds[0]; phiIdx < sectionBounds[1]; ++phiIdx) {
      c3index phiIdxMod = phiIdx % m_clustererParams.nPhi;
      for (c3index cotIdx = 0; cotIdx < m_clustererParams.nCot; ++cotIdx) {
        if ((*m_houghSpace)[omegaIdx][phiIdxMod][cotIdx] > peakValue) {
          peakValue = (*m_houghSpace)[omegaIdx][phiIdxMod][cotIdx];
          peakCell = {omegaIdx, phiIdxMod, cotIdx};
        }
      }
    }
  }
  return {peakCell, peakValue};
}

// Creates the surrounding cluster (fixed shape) around the section peak index
SimpleCluster Clusterizend::createCluster(const cell_index& peakCell)
{
  c3index omegaPeak = peakCell[0];
  c3index phiPeak = peakCell[1];
  c3index cotPeak = peakCell[2];

  c3index cotLower = cotPeak - 1;
  c3index cotUpper = cotPeak + 1;

  c3index phiLeft = (phiPeak - 1 + m_clustererParams.nPhi) % m_clustererParams.nPhi;
  c3index phiRight = (phiPeak + 1 + m_clustererParams.nPhi) % m_clustererParams.nPhi;

  c3index omegaUpperRight = omegaPeak - 1;
  c3index omegaLowerLeft = omegaPeak + 1;

  // Add in-bound cells to the cluster:
  SimpleCluster fixedCluster;
  fixedCluster.appendCell(peakCell);
  fixedCluster.appendCell({omegaPeak, phiLeft, cotPeak});
  fixedCluster.appendCell({omegaPeak, phiRight, cotPeak});
  if (cotLower >= 0) {
    fixedCluster.appendCell({omegaPeak, phiPeak, cotLower});
  }
  if (cotUpper < m_clustererParams.nCot) {
    fixedCluster.appendCell({omegaPeak, phiPeak, cotUpper});
  }
  if (omegaUpperRight >= 0) {
    fixedCluster.appendCell({omegaUpperRight, phiRight, cotPeak});
  }
  if (omegaLowerLeft < m_clustererParams.nOmega) {
    fixedCluster.appendCell({omegaLowerLeft, phiLeft, cotPeak});
  }
  return fixedCluster;
}

// Adds the weight of all cluster cells together
unsigned int Clusterizend::calculateTotalWeight(const SimpleCluster& cluster)
{
  unsigned int totalClusterWeight = 0;
  std::vector<cell_index> clusterCells = cluster.getCells();
  for (const cell_index& cell : clusterCells) {
    totalClusterWeight += (*m_houghSpace)[cell[0]][cell[1]][cell[2]];
  }
  return totalClusterWeight;
}

// Deletes the surroundings of the peak index with a butterfly cutout
void Clusterizend::deletePeakSurroundings(const cell_index& peakCell)
{
  c3index omegaPeak = peakCell[0];
  c3index phiPeak = peakCell[1];

  c3index omegaLowerBound = std::max<unsigned short>(0, omegaPeak - m_clustererParams.omegaTrim);
  c3index omegaUpperBound = std::min<unsigned short>(m_clustererParams.nOmega, omegaPeak + m_clustererParams.omegaTrim + 1);

  for (c3index cotIdx = 0; cotIdx < m_clustererParams.nCot; ++cotIdx) {
    for (c3index omegaIdx = omegaLowerBound; omegaIdx < omegaUpperBound; ++omegaIdx) {
      c3index phiCell = phiPeak + omegaPeak - omegaIdx;
      c3index relativePhi = phiCell - phiPeak;

      if (relativePhi > 0) {
        DeletionBounds firstBounds = {
          phiCell - m_clustererParams.phiTrim,
          static_cast<c3index>(phiCell + m_clustererParams.phiTrim + std::floor(2.4 * relativePhi)),
          omegaIdx,
          cotIdx
        };
        clearHoughSpaceRow(firstBounds);
      } else if (relativePhi < 0) {
        DeletionBounds secondBounds = {
          static_cast<c3index>(phiCell - m_clustererParams.phiTrim + std::ceil(2.4 * relativePhi)),
          phiCell + m_clustererParams.phiTrim + 1,
          omegaIdx,
          cotIdx
        };
        clearHoughSpaceRow(secondBounds);
      } else {
        DeletionBounds thirdBounds = {
          phiCell - m_clustererParams.phiTrim,
          phiCell + m_clustererParams.phiTrim + 1,
          omegaIdx,
          cotIdx
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
    (*m_houghSpace)[bounds.omega][phiIdxMod][bounds.cot] = 0;
  }
}
