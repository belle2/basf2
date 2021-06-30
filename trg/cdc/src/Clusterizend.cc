/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2021 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Sebastian Skambraks                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "framework/logging/Logger.h"
#include "trg/cdc/NDFinderDefs.h"
#include "trg/cdc/Clusterizend.h"

using namespace Belle2;
using namespace std;


bool Clusterizend::has_before(cell_index entry, ushort dim)
{
  if (entry[dim] > 0 || m_params.var_cyclic[dim]) {
    return true;
  }
  return false;
}
cell_index Clusterizend::before(cell_index entry, ushort dim)
{
  if (entry[dim] > 0) {
    entry[dim] -= 1;
    return entry;
  } else if (m_params.var_cyclic[dim]) {
    entry[dim] = m_valmax[dim];
    return entry;
  } else {
    B2ERROR("no before(), check with has_before");
    return entry;
  }
}

bool Clusterizend::has_after(cell_index entry, ushort dim)
{
  if (entry[dim] < m_valmax[dim] || m_params.var_cyclic[dim]) {
    return true;
  }
  return false;
}
cell_index Clusterizend::after(cell_index entry, ushort dim)
{
  if (entry[dim] < m_valmax[dim]) {
    entry[dim] += 1;
    return entry;
  } else if (m_params.var_cyclic[dim]) {
    entry[dim] = 0;
    return entry;
  } else {
    B2ERROR("no after(), check with has_before()");
    return entry;
  }
}


void Clusterizend::blockcheck(vector<cell_index>* neighbors, cell_index elem, ushort dim)
{
  if (dim > 10) {
    B2ERROR("dim too large, dim=" << dim);
  }
  if (has_before(elem, dim)) {
    cell_index ind = before(elem, dim);
    ushort leftwe = (*m_houghVals)[ind[0]][ind[1]][ind[2]];
    ushort leftvi = m_houghVisit[ind[0]][ind[1]][ind[2]];
    if (leftwe > m_params.minweight && leftvi == 0) {
      neighbors->push_back(ind);
    }
    if (m_params.diagonal && dim > 0) {
      blockcheck(neighbors, ind, dim - 1);
    }
  }
  if (has_after(elem, dim)) {
    cell_index ind = after(elem, dim);
    ushort rightwe = (*m_houghVals)[ind[0]][ind[1]][ind[2]];
    ushort rightvi = m_houghVisit[ind[0]][ind[1]][ind[2]];
    if (rightwe > m_params.minweight  && rightvi == 0) {
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

vector<cell_index>
Clusterizend::regionQuery(cell_index entry)
{
  vector<cell_index> neighbours;
  blockcheck(&neighbours, entry, m_dimsize - 1);
  return neighbours;
}

vector<SimpleCluster>
Clusterizend::dbscan()
{
  vector<SimpleCluster> C;
  vector<cell_index> candidates = getCandidates();
  for (unsigned long icand = 0; icand < candidates.size(); icand++) {
    cell_index entry = candidates[icand];
    c3index iom = entry[0];
    c3index iph = entry[1];
    c3index ith = entry[2];

    if (m_houghVisit[iom][iph][ith] == 0) {
      //B2DEBUG(19, "dbscan: unvisited cell");
      m_houghVisit[iom][iph][ith] = 1;
      vector<cell_index> N = regionQuery(entry);
      if (N.size() >= m_params.minpts) {
        //B2DEBUG(19, "dbscan: starting cluster, neightbors = " << N.size());
        SimpleCluster newcluster(entry);
        expandCluster(N, newcluster);
        C.push_back(newcluster);
      }
    }
  }
  return C;
}

void
Clusterizend::expandCluster(vector<cell_index>& N, SimpleCluster& C)
{
  while (N.size() > 0) {
    cell_index nextP = N.back();
    N.pop_back();
    ushort iom = nextP[0];
    ushort iph = nextP[1];
    ushort ith = nextP[2];
    if (m_houghVisit[iom][iph][ith] == 0) {
      m_houghVisit[iom][iph][ith] = 1;
      if ((*m_houghVals)[iom][iph][ith] < m_params.minweight) {
        continue;
      }
      vector<cell_index> nextN = regionQuery(nextP);
      if (nextN.size() >= m_params.minpts) {
        N.insert(N.end(), nextN.begin(), nextN.end());
      }
      C.append(nextP);
    }
  }
}


vector<cell_index>
Clusterizend::getCandidates()
{
  vector<cell_index> candidates;
  /** all candidiates TODO: select */
  for (c3index iom = 0; iom < 40; iom++) {
    for (c3index iph = 0; iph < 384; iph++) {
      for (c3index ith = 0; ith < 9; ith++) {
        if ((*m_houghVals)[iom][iph][ith] > m_params.minweight) {
          cell_index elem = {iom, iph, ith};
          candidates.push_back(elem);
        }
      }
    }
  }
  return candidates;
}
