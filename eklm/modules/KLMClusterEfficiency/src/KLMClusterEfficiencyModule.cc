/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* C++ headers. */
#include <set>

/* Belle2 headers. */
#include <eklm/modules/KLMClusterEfficiency/KLMClusterEfficiencyModule.h>
#include <framework/core/ModuleManager.h>
#include <framework/datastore/RelationArray.h>
#include <framework/datastore/StoreArray.h>
#include <mdst/dataobjects/KLMCluster.h>
#include <mdst/dataobjects/MCParticle.h>

using namespace Belle2;

REG_MODULE(KLMClusterEfficiency)

KLMClusterEfficiencyModule::KLMClusterEfficiencyModule() : Module()
{
  setDescription("Module for KLM cluster reconstruction efficiency studies.");
  setPropertyFlags(c_ParallelProcessingCertified);
  m_KL0Clusters = 0;
  m_PartlyKL0Clusters = 0;
  m_OtherClusters = 0;
  m_ReconstructedKL0Clusters[0] = 0;
  m_ReconstructedKL0Clusters[1] = 0;
  m_ReconstructedKL0Clusters[2] = 0;
}

KLMClusterEfficiencyModule::~KLMClusterEfficiencyModule()
{
}

void KLMClusterEfficiencyModule::initialize()
{
}

void KLMClusterEfficiencyModule::beginRun()
{
}

void KLMClusterEfficiencyModule::event()
{
  StoreArray<KLMCluster> klmClusters;
  StoreArray<MCParticle> mcParticles;
  int i1, i2, n1, n2;
  bool haveKL0;
  n1 = klmClusters.getEntries();
  for (i1 = 0; i1 < n1; i1++) {
    haveKL0 = false;
    RelationVector<MCParticle> clusterMCParticles =
      klmClusters[i1]->getRelationsTo<MCParticle>();
    n2 = clusterMCParticles.size();
    for (i2 = 0; i2 < n2; i2++) {
      if (clusterMCParticles[i2]->getPDG() == 130)
        haveKL0 = true;
    }
    if (haveKL0) {
      if (n2 == 1)
        m_KL0Clusters++;
      else
        m_PartlyKL0Clusters++;
    } else
      m_OtherClusters++;
  }
  n1 = mcParticles.getEntries();
  for (i1 = 0; i1 < n1; i1++) {
    if (mcParticles[i1]->getPDG() != 130)
      continue;
    RelationVector<KLMCluster> kl0Clusters =
      mcParticles[i1]->getRelationsFrom<KLMCluster>();
    n2 = kl0Clusters.size();
    if (n2 == 0)
      m_ReconstructedKL0Clusters[0]++;
    else if (n2 == 1)
      m_ReconstructedKL0Clusters[1]++;
    else if (n2 >= 2)
      m_ReconstructedKL0Clusters[2]++;
  }
}

void KLMClusterEfficiencyModule::endRun()
{
}

void KLMClusterEfficiencyModule::terminate()
{
  B2INFO("Total number of KLM clusters: " << m_KL0Clusters +
         m_PartlyKL0Clusters + m_OtherClusters);
  B2INFO("K_L0 clusters: " << m_KL0Clusters);
  B2INFO("(K_L0+other) clusters: " << m_PartlyKL0Clusters);
  B2INFO("Other clusters: " << m_OtherClusters);
  B2INFO("Total number of generated K_L0: " << m_ReconstructedKL0Clusters[0] +
         m_ReconstructedKL0Clusters[1] + m_ReconstructedKL0Clusters[2]);
  B2INFO("Nonreconstructed K_L0: " << m_ReconstructedKL0Clusters[0]);
  B2INFO("K_L0 reconstructed as 1 cluster: " << m_ReconstructedKL0Clusters[1]);
  B2INFO("K_L0 reconstructed as 2 or more clusters: " <<
         m_ReconstructedKL0Clusters[2]);
}

