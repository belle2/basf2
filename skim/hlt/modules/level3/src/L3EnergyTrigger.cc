/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Hidekazu Kakuno                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "framework/datastore/StoreArray.h"
#include "skim/hlt/dataobjects/L3Tag.h"
//#include "skim/hlt/dataobjects/L3Cluster.h"
#include "skim/hlt/modules/level3/FCFinder.h"
#include "skim/hlt/modules/level3/FCCluster.h"
#include "skim/hlt/modules/level3/L3EnergyTrigger.h"
#include <cmath>

using namespace Belle2;

bool
L3EnergyTrigger::select(L3Tag* tag)
{
  double energySum(0.);
  //StoreArray<L3Cluster> clusters;
  FTList<FCCluster*>& clusters = FCFinder::instance().getClusters();
  //const int n = clusters.getEntries();
  const int n = clusters.length();
  for (int i = 0; i < n; i++) {
    FCCluster& c = *clusters[i];
    //L3Cluster& c = *clusters[i];
    energySum += c.getEnergy();
  }
  if (tag) tag->setEnergySum(energySum);

  return energySum > m_minEnergy;
}
