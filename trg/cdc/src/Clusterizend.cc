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
  std::vector<SimpleCluster> candidates;
  c3array houghValsBackup = *m_houghVals;
  for (unsigned char quadrant = 0; quadrant < 4; quadrant++) {
    for (unsigned char iter = 0; iter < m_params.iterations; iter++) {
      auto [globalMax, peakWeight] = getGlobalMax(quadrant);
      if (peakWeight < m_params.minPeakWeight || peakWeight == 0) {
        *m_houghVals = houghValsBackup;
        break;
      }
      auto [newCluster, totalWeight] = createCluster(globalMax);
      if (totalWeight >= m_params.minTotalWeight) {
        candidates.push_back(newCluster);
      }
      deleteMax(globalMax);
    }
    *m_houghVals = houghValsBackup;
  }
  return candidates;
}

std::pair<cell_index, unsigned long> Clusterizend::getGlobalMax(unsigned char quadrant)
{
  unsigned long maxValue = 0;
  cell_index maxIndex = {0, 0, 0};
  for (c3index iom = 0; iom < 40; iom++) {
    for (c3index iph = 48 + quadrant * 96; iph < 48 + (quadrant + 1) * 96; iph++) {
      c3index phiMod = iph % 384;
      for (c3index ith = 0; ith < 9; ith++) {
        if ((*m_houghVals)[iom][phiMod][ith] > maxValue) {
          maxValue = (*m_houghVals)[iom][phiMod][ith];
          maxIndex = {iom, phiMod, ith};
        }
      }
    }
  }
  return {maxIndex, maxValue};
}

std::pair<SimpleCluster, unsigned long> Clusterizend::createCluster(cell_index maxIndex)
{
  SimpleCluster fixedCluster;
  c3index omIndex = maxIndex[0];
  c3index phIndex = maxIndex[1];
  c3index thIndex = maxIndex[2];
  unsigned long totalClusterWeight = 0;
  for (c3index ith = std::max<int>(0, thIndex - 1); ith < std::min<int>(9, thIndex + 2); ith++) {
    for (c3index iph = phIndex - 1; iph < phIndex + 2; iph++) {
      c3index iphMod = (iph + 384) % 384;
      cell_index newMemberIndex = {omIndex, iphMod, ith};
      fixedCluster.append(newMemberIndex);
      totalClusterWeight += (*m_houghVals)[omIndex][iphMod][ith];
    }
  }
  if (omIndex - 1 >= 0) {
    for (c3index ith = std::max<int>(0, thIndex - 1); ith < std::min<int>(9, thIndex + 2); ith++) {
      for (c3index iph = phIndex + 1; iph < phIndex + 4; iph++) {
        c3index iphMod = (iph + 384) % 384;
        cell_index newMemberIndex = {omIndex - 1, iphMod, ith};
        fixedCluster.append(newMemberIndex);
        totalClusterWeight += (*m_houghVals)[omIndex - 1][iphMod][ith];
      }
    }
  }
  if (omIndex + 1 < 40) {
    for (c3index ith = std::max<int>(0, thIndex - 1); ith < std::min<int>(9, thIndex + 2); ith++) {
      for (c3index iph = phIndex - 3; iph < phIndex; iph++) {
        c3index iphMod = (iph + 384) % 384;
        cell_index newMemberIndex = {omIndex + 1, iphMod, ith};
        fixedCluster.append(newMemberIndex);
        totalClusterWeight += (*m_houghVals)[omIndex + 1][iphMod][ith];
      }
    }
  }
  return {fixedCluster, totalClusterWeight};
}

void Clusterizend::deleteMax(cell_index maxIndex)
{
  c3index omIndex = maxIndex[0];
  c3index phIndex = maxIndex[1];
  c3index thIndex = maxIndex[2];
  for (c3index ith = std::max<int>(0, thIndex - m_params.thetaTrim); ith < std::min<int>(9, thIndex + m_params.thetaTrim + 1);
       ith++) {
    for (c3index iom = std::max<int>(0, omIndex - m_params.omegaTrim); iom < std::min<int>(40, omIndex + m_params.omegaTrim + 1);
         iom++) {
      c3index phiIndex = phIndex + omIndex - iom;
      c3index relativePhi = phiIndex - phIndex;
      if (relativePhi > 0) {
        for (c3index iph = phiIndex - m_params.phiTrim; iph < phiIndex + m_params.phiTrim + std::floor(2.4 * relativePhi); iph++) {
          c3index iphMod = (iph + 384) % 384;
          (*m_houghVals)[iom][iphMod][ith] = 0;
        }
      } else if (relativePhi < 0) {
        for (c3index iph = phiIndex - m_params.phiTrim + std::ceil(2.4 * relativePhi); iph < phiIndex + m_params.phiTrim + 1; iph++) {
          c3index iphMod = (iph + 384) % 384;
          (*m_houghVals)[iom][iphMod][ith] = 0;
        }
      } else {
        for (c3index iph = phiIndex - m_params.phiTrim; iph < phiIndex + m_params.phiTrim + 1; iph++) {
          c3index iphMod = (iph + 384) % 384;
          (*m_houghVals)[iom][iphMod][ith] = 0;
        }
      }
    }
  }
}
