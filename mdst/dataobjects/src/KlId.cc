/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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

