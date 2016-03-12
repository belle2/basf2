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
  int i1, i2, n1, n2;
  bool haveKL0;
  n1 = klmClusters.getEntries();
  for (i1 = 0; i1 < n1; i1++) {
    haveKL0 = false;
    RelationVector<MCParticle> mcParticles =
      klmClusters[i1]->getRelationsTo<MCParticle>();
    n2 = mcParticles.size();
    for (i2 = 0; i2 < n2; i2++) {
      if (mcParticles[i2]->getPDG() == 130)
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
}

void KLMClusterEfficiencyModule::endRun()
{
}

void KLMClusterEfficiencyModule::terminate()
{
  printf("K_L0 clusters: %d\n(K_L0+other) clusters: %d\nOther clusters: %d\n",
         m_KL0Clusters, m_PartlyKL0Clusters, m_OtherClusters);
}

