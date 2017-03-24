/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Anze Zupanc                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <analysis/modules/FixECLClusters/FixECLClustersModule.h>


using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(FixECLClusters)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

FixECLClustersModule::FixECLClustersModule() : Module()
{
  // Set module properties
  setDescription("Sets ConnectedRegionID and ClusterID to ECLClusters on MC samples with old ECLCluster design.");
}

void FixECLClustersModule::initialize()
{
  B2INFO("FixECLClustersModule will overwrite ConnectedRegionID with cluster's StoreArray index+1 and ClusterID with 1.");
  m_eclClusters.isRequired();
}

void FixECLClustersModule::event()
{

  for (int i = 0; i < m_eclClusters.getEntries(); i++) {
    ECLCluster* cluster      = m_eclClusters[i];
    cluster->setConnectedRegionId(i + 1);
    cluster->setClusterId(1);
  }
}


