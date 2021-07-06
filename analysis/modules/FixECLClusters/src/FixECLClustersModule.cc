/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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


