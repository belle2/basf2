/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jo-Frederik Krohn                                        *
 *                                                                        *
 *                                                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* Belle2 headers. */
#include <mdst/dataobjects/KlId.h>
#include <mdst/dataobjects/KLMCluster.h>
#include <mdst/dataobjects/ECLCluster.h>

using namespace Belle2;


bool KlId::isKLM() const
{
  return getRelatedFrom<KLMCluster>();
}

bool KlId::isECL() const
{
  return getRelatedFrom<ECLCluster>();
}

double KlId::getKlId() const
{
  auto klmClusterWeight = getRelatedFromWithWeight<KLMCluster>();
  if (klmClusterWeight.first) return klmClusterWeight.second;
  auto eclClusterWeight = getRelatedFromWithWeight<ECLCluster>();
  if (eclClusterWeight.first) return eclClusterWeight.second;
  return nan("");
}

