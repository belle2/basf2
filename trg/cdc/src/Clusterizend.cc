/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include "framework/logging/Logger.h"
#include "trg/cdc/NDFinderDefs.h"
#include "trg/cdc/Clusterizend.h"
using namespace Belle2;


bool Clusterizend::hasBefore(cell_index entry, ushort dim)
{
  if (entry[dim] > 0 || m_params.varCyclic[dim]) {
    return true;
  }
  return false;
}
cell_index Clusterizend::before(cell_index entry, ushort dim)
{
  if (entry[dim] > 0) {
    entry[dim] -= 1;
    return entry;
  } else if (m_params.varCyclic[dim]) {
    entry[dim] = m_valMax[dim];
    return entry;
  } else {
    B2ERROR("no before(), check with hasBefore");
    return entry;
  }
}

bool Clusterizend::hasAfter(cell_index entry, ushort dim)
{
  if (entry[dim] < m_valMax[dim] || m_params.varCyclic[dim]) {
    return true;
  }
  return false;
}
cell_index Clusterizend::after(cell_index entry, ushort dim)
{
  if (entry[dim] < m_valMax[dim]) {
    entry[dim] += 1;
    return entry;
  } else if (m_params.varCyclic[dim]) {
    entry[dim] = 0;
    return entry;
  } else {
    B2ERROR("no after(), check with hasBefore()");
    return entry;
  }
}


void Clusterizend::blockcheck(std::vector<cell_index>* neighbors, cell_index elem, ushort dim)
{
  if (dim > 10) {
    B2ERROR("dim too large, dim=" << dim);
  }
  if (hasBefore(elem, dim)) {
    cell_index ind = before(elem, dim);
    ushort leftwe = (*m_houghVals)[ind[0]][ind[1]][ind[2]];
    ushort leftvi = m_houghVisit[ind[0]][ind[1]][ind[2]];
    if (leftwe > m_params.minWeight && leftvi == 0) {
      neighbors->push_back(ind);
    }
    if (m_params.diagonal && dim > 0) {
      blockcheck(neighbors, ind, dim - 1);
    }
  }
  if (hasAfter(elem, dim)) {
    cell_index ind = after(elem, dim);
    ushort rightwe = (*m_houghVals)[ind[0]][ind[1]][ind[2]];
    ushort rightvi = m_houghVisit[ind[0]][ind[1]][ind[2]];
    if (rightwe > m_params.minWeight  && rightvi == 0) {
      neighbors->push_back(ind);
    }
    if (m_params.diagonal && dim > 0) {
      blockcheck(neighbors, ind, dim - 1);
    }
  }

  if (dim > 0) {
    blockcheck(neighbors, elem, dim - 1);
  }
}

std::vector<cell_index>
Clusterizend::regionQuery(cell_index entry)
{
  std::vector<cell_index> neighbours;
  blockcheck(&neighbours, entry, m_dimSize - 1);
  return neighbours;
}

std::vector<SimpleCluster>
Clusterizend::dbscan()
{
  std::vector<SimpleCluster> C;
  std::vector<cell_index> candidates = getCandidates();
  for (unsigned long icand = 0; icand < candidates.size(); icand++) {
    cell_index entry = candidates[icand];
    c3index iom = entry[0];
    c3index iph = entry[1];
    c3index ith = entry[2];

    if (m_houghVisit[iom][iph][ith] == 0) {
      //B2DEBUG(25, "dbscan: unvisited cell");
      m_houghVisit[iom][iph][ith] = 1;
      std::vector<cell_index> N = regionQuery(entry);
      if (N.size() >= m_params.minPts) {
        //B2DEBUG(25, "dbscan: starting cluster, neighbors = " << N.size());
        SimpleCluster newCluster(entry);
        expandCluster(N, newCluster);
        C.push_back(newCluster);
      }
    }
  }
  return C;
}

void
Clusterizend::expandCluster(std::vector<cell_index>& N, SimpleCluster& C)
{
  while (N.size() > 0) {
    cell_index nextP = N.back();
    N.pop_back();
    ushort iom = nextP[0];
    ushort iph = nextP[1];
    ushort ith = nextP[2];
    if (m_houghVisit[iom][iph][ith] == 0) {
      m_houghVisit[iom][iph][ith] = 1;
      if ((*m_houghVals)[iom][iph][ith] < m_params.minWeight) {
        continue;
      }
      std::vector<cell_index> nextN = regionQuery(nextP);
      if (nextN.size() >= m_params.minPts) {
        N.insert(N.end(), nextN.begin(), nextN.end());
      }
      C.append(nextP);
    }
  }
}

std::vector<cell_index>
Clusterizend::getCandidates()
{
  std::vector<cell_index> candidates;
  /** all candidates TODO: select */
  for (c3index iom = 0; iom < 40; iom++) {
    for (c3index iph = 0; iph < 384; iph++) {
      for (c3index ith = 0; ith < 9; ith++) {
        if ((*m_houghVals)[iom][iph][ith] > m_params.minWeight) {
          cell_index elem = {iom, iph, ith};
          candidates.push_back(elem);
        }
      }
    }
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
