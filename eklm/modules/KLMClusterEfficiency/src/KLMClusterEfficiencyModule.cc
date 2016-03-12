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
#include <bklm/dataobjects/BKLMHit2d.h>
#include <eklm/dataobjects/EKLMHit2d.h>
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
  int i;
  setDescription("Module for KLM cluster reconstruction efficiency studies.");
  addParam("OutputFile", m_OutputFileName, "Output file.",
           std::string("KLMClusterEfficiency.root"));
  m_OutputFile = NULL;
  m_OutputTree = NULL;
  m_KL0Clusters = 0;
  m_PartlyKL0Clusters = 0;
  m_OtherClusters = 0;
  for (i = 0; i < 4; i++)
    m_ReconstructedKL0Clusters[i] = 0;
  m_ReconstructedKL0ClustersEKLMBKLM = 0;
}

KLMClusterEfficiencyModule::~KLMClusterEfficiencyModule()
{
}

void KLMClusterEfficiencyModule::initialize()
{
  m_OutputFile = new TFile(m_OutputFileName.c_str(), "recreate");
  m_OutputTree = new TTree("klm_cluster", "");
  m_OutputTree->Branch("DecayVertexX", &m_DecayVertexX, "DecayVertexX/F");
  m_OutputTree->Branch("DecayVertexY", &m_DecayVertexY, "DecayVertexY/F");
  m_OutputTree->Branch("DecayVertexZ", &m_DecayVertexZ, "DecayVertexZ/F");
  m_OutputTree->Branch("MaxDecayVertexHitAngle", &m_MaxDecayVertexHitAngle,
                       "MaxDecayVertexHitAngle/F");
  m_OutputTree->Branch("ClusterX", &m_ClusterX, "ClusterX/F");
  m_OutputTree->Branch("ClusterY", &m_ClusterY, "ClusterY/F");
  m_OutputTree->Branch("ClusterZ", &m_ClusterZ, "ClusterZ/F");
  m_OutputTree->Branch("MaxClusterHitAngle", &m_MaxClusterHitAngle,
                       "MaxClusterHitAngle/F");
}

void KLMClusterEfficiencyModule::beginRun()
{
}

void KLMClusterEfficiencyModule::event()
{
  StoreArray<KLMCluster> klmClusters;
  StoreArray<MCParticle> mcParticles;
  int i1, i2, i3, n1, n2, n3;
  TVector3 decayVertex, clusterPosition, hitPosition;
  float angle;
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
    decayVertex = mcParticles[i1]->getDecayVertex();
    m_DecayVertexX = decayVertex.X();
    m_DecayVertexY = decayVertex.Y();
    m_DecayVertexZ = decayVertex.Z();
    m_MaxDecayVertexHitAngle = 0;
    RelationVector<BKLMHit2d> mcBKLMHit2ds =
      mcParticles[i1]->getRelationsFrom<BKLMHit2d>();
    n2 = mcBKLMHit2ds.size();
    for (i2 = 0; i2 < n2; i2++) {
      hitPosition = mcBKLMHit2ds[i2]->getGlobalPosition();
      angle = decayVertex.Angle(hitPosition);
      if (angle > m_MaxDecayVertexHitAngle)
        m_MaxDecayVertexHitAngle = angle;
    }
    RelationVector<EKLMHit2d> mcEKLMHit2ds =
      mcParticles[i1]->getRelationsFrom<EKLMHit2d>();
    n2 = mcEKLMHit2ds.size();
    for (i2 = 0; i2 < n2; i2++) {
      hitPosition = mcEKLMHit2ds[i2]->getPosition();
      angle = decayVertex.Angle(hitPosition);
      if (angle > m_MaxDecayVertexHitAngle)
        m_MaxDecayVertexHitAngle = angle;
    }
    n2 = mcEKLMHit2ds.size();
    RelationVector<KLMCluster> kl0Clusters =
      mcParticles[i1]->getRelationsFrom<KLMCluster>();
    n2 = kl0Clusters.size();
    if (n2 == 0)
      m_ReconstructedKL0Clusters[0]++;
    else if (n2 == 1)
      m_ReconstructedKL0Clusters[1]++;
    else if (n2 == 2) {
      m_ReconstructedKL0Clusters[2]++;
      RelationVector<BKLMHit2d> bklmHit2ds1 =
        kl0Clusters[0]->getRelationsTo<BKLMHit2d>();
      RelationVector<BKLMHit2d> bklmHit2ds2 =
        kl0Clusters[1]->getRelationsTo<BKLMHit2d>();
      RelationVector<EKLMHit2d> eklmHit2ds1 =
        kl0Clusters[0]->getRelationsTo<EKLMHit2d>();
      RelationVector<EKLMHit2d> eklmHit2ds2 =
        kl0Clusters[1]->getRelationsTo<EKLMHit2d>();
      if ((bklmHit2ds1.size() > 0 && eklmHit2ds1.size() == 0 &&
           bklmHit2ds2.size() == 0 && eklmHit2ds2.size() > 0) ||
          (bklmHit2ds1.size() == 0 && eklmHit2ds1.size() > 0 &&
           bklmHit2ds2.size() > 0 && eklmHit2ds2.size() == 0))
        m_ReconstructedKL0ClustersEKLMBKLM++;
    } else if (n2 >= 3)
      m_ReconstructedKL0Clusters[3]++;
    for (i2 = 0; i2 < n2; i2++) {
      clusterPosition = kl0Clusters[i2]->getClusterPosition();
      m_ClusterX = clusterPosition.X();
      m_ClusterY = clusterPosition.Y();
      m_ClusterZ = clusterPosition.Z();
      m_MaxClusterHitAngle = 0;
      RelationVector<BKLMHit2d> bklmHit2ds =
        kl0Clusters[i2]->getRelationsTo<BKLMHit2d>();
      n3 = bklmHit2ds.size();
      for (i3 = 0; i3 < n3; i3++) {
        hitPosition = bklmHit2ds[i3]->getGlobalPosition();
        angle = clusterPosition.Angle(hitPosition);
        if (angle > m_MaxClusterHitAngle)
          m_MaxClusterHitAngle = angle;
      }
      RelationVector<EKLMHit2d> eklmHit2ds =
        kl0Clusters[i2]->getRelationsTo<EKLMHit2d>();
      n3 = eklmHit2ds.size();
      for (i3 = 0; i3 < n3; i3++) {
        hitPosition = eklmHit2ds[i3]->getPosition();
        angle = clusterPosition.Angle(hitPosition);
        if (angle > m_MaxClusterHitAngle)
          m_MaxClusterHitAngle = angle;
      }
      m_OutputTree->Fill();
    }
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
         m_ReconstructedKL0Clusters[1] + m_ReconstructedKL0Clusters[2] +
         m_ReconstructedKL0Clusters[3]);
  B2INFO("Nonreconstructed K_L0: " << m_ReconstructedKL0Clusters[0]);
  B2INFO("K_L0 reconstructed as 1 cluster: " << m_ReconstructedKL0Clusters[1]);
  B2INFO("K_L0 reconstructed as 2 clusters: " << m_ReconstructedKL0Clusters[2]);
  B2INFO("Including K_L0 reconstructed as 2 clusters (EKLM + BKLM): " <<
         m_ReconstructedKL0ClustersEKLMBKLM);
  B2INFO("K_L0 reconstructed as 3 or more clusters: " <<
         m_ReconstructedKL0Clusters[3]);
  m_OutputFile->cd();
  m_OutputTree->Write();
  delete m_OutputTree;
  delete m_OutputFile;
}

