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


template<class T> bool KlId::hasRelationFromClusterType() const
{
  if (this->getRelatedFrom<T>()) { return true; }
  return false;
}

bool KlId::isKLM() const
{
  return hasRelationFromClusterType<KLMCluster>();
}

bool KlId::isECL() const
{
  return hasRelationFromClusterType<ECLCluster>();
}

double KlId::getKlId() const
{
  if (isKLM()) {
    return this->getRelatedFromWithWeight<KLMCluster>().second;
  } else if (isECL()) {
    return this->getRelatedFromWithWeight<ECLCluster>().second;
  }
  return -999;
}

