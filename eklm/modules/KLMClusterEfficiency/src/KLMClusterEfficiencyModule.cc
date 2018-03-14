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

/* External headers. */
#include <TH2F.h>
#include <TMarker.h>
#include <TCanvas.h>
#include <TStyle.h>

/* Belle2 headers. */
#include <bklm/dataobjects/BKLMHit2d.h>
#include <eklm/dataobjects/EKLMHit2d.h>
#include <eklm/modules/KLMClusterEfficiency/KLMClusterEfficiencyModule.h>
#include <framework/datastore/RelationArray.h>

using namespace Belle2;

REG_MODULE(KLMClusterEfficiency)

KLMClusterEfficiencyModule::KLMClusterEfficiencyModule() : Module()
{
  int i;
  setDescription("Module for KLM cluster reconstruction efficiency studies.");
  addParam("SaveClusterData", m_SaveClusterData,
           "Whether to save cluster data or not.", false);
  addParam("SaveReconstructionData", m_SaveReconstructionData,
           "Whether to save reconstruction data or not.", true);
  addParam("OutputFile", m_OutputFileName, "Output file.",
           std::string("KLMClusterEfficiency.root"));
  addParam("EventsClusterHistograms", m_EventsClusterHistograms,
           "Draw cluster histograms for this number of events.", 0);
  m_OutputFile = NULL;
  m_ClusterTree = NULL;
  m_KL0Clusters = 0;
  m_PartlyKL0Clusters = 0;
  m_OtherClusters = 0;
  m_NonreconstructedKL0 = 0;
  for (i = 0; i < 3; i++)
    m_ReconstructedKL01Cluster[i] = 0;
  m_ExactlyReconstructedKL0 = 0;
  for (i = 0; i < 6; i++)
    m_ReconstructedKL02Clusters[i] = 0;
  m_ReconstructedKL03Clusters = 0;
}

KLMClusterEfficiencyModule::~KLMClusterEfficiencyModule()
{
}

void KLMClusterEfficiencyModule::initialize()
{
  if (m_SaveClusterData || m_SaveReconstructionData)
    m_OutputFile = new TFile(m_OutputFileName.c_str(), "recreate");
  if (m_SaveClusterData) {
    m_ClusterTree = new TTree("klm_cluster", "");
    m_ClusterTree->Branch("DecayVertexX", &m_DecayVertexX, "DecayVertexX/F");
    m_ClusterTree->Branch("DecayVertexY", &m_DecayVertexY, "DecayVertexY/F");
    m_ClusterTree->Branch("DecayVertexZ", &m_DecayVertexZ, "DecayVertexZ/F");
    m_ClusterTree->Branch("MaxDecayVertexHitAngle", &m_MaxDecayVertexHitAngle,
                          "MaxDecayVertexHitAngle/F");
    m_ClusterTree->Branch("ClusterX", &m_ClusterX, "ClusterX/F");
    m_ClusterTree->Branch("ClusterY", &m_ClusterY, "ClusterY/F");
    m_ClusterTree->Branch("ClusterZ", &m_ClusterZ, "ClusterZ/F");
    m_ClusterTree->Branch("MaxClusterHitAngle", &m_MaxClusterHitAngle,
                          "MaxClusterHitAngle/F");
  }
  if (m_SaveReconstructionData) {
    m_ReconstructionTree = new TTree("klm_reconstruction", "");
    m_ReconstructionTree->Branch("KL0Clusters", &m_KL0Clusters,
                                 "KL0Clusters/I");
    m_ReconstructionTree->Branch("PartlyKL0Clusters", &m_PartlyKL0Clusters,
                                 "PartlyKL0Clusters/I");
    m_ReconstructionTree->Branch("OtherClusters", &m_OtherClusters,
                                 "OtherClusters/I");
    m_ReconstructionTree->Branch("NonreconstructedKL0", &m_NonreconstructedKL0,
                                 "NonreconstructedKL0/I");
    m_ReconstructionTree->Branch("ReconstructedKL01Cluster",
                                 &m_ReconstructedKL01Cluster,
                                 "ReconstructedKL01Cluster[3]/I");
    m_ReconstructionTree->Branch("ReconstructedKL02Clusters",
                                 &m_ReconstructedKL02Clusters,
                                 "ReconstructedKL02Clusters[6]/I");
    m_ReconstructionTree->Branch("ReconstructedKL03Clusters",
                                 &m_ReconstructedKL03Clusters,
                                 "ReconstructedKL03Clusters/I");
  }
  gStyle->SetOptStat(0);
}

void KLMClusterEfficiencyModule::beginRun()
{
}

void KLMClusterEfficiencyModule::event()
{
  static int nevent = 0;
  char str[128];
  int i1, i2, i3, n1, n2, n3;
  int bs1, bs2, es1, es2;
  TVector3 decayVertex, clusterPosition, hitPosition;
  float angle;
  bool haveKL0;
  n1 = m_KLMClusters.getEntries();
  for (i1 = 0; i1 < n1; i1++) {
    haveKL0 = false;
    RelationVector<MCParticle> clusterMCParticles =
      m_KLMClusters[i1]->getRelationsTo<MCParticle>();
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
  if (nevent < m_EventsClusterHistograms) {
    static TH2F* hzx = new TH2F("hzx", "", 100, -300, 420, 100, -320, 320);
    static TH2F* hzy = new TH2F("hzy", "", 100, -300, 420, 100, -320, 320);
    static TH2F* hxy = new TH2F("hxy", "", 100, -320, 320, 100, -320, 320);
    static TCanvas* c1 = new TCanvas();
    static TMarker* clusterMarker = new TMarker(0, 0, 20);
    static TMarker* hitMarker = new TMarker(0, 0, 21);
    if (nevent == 0) {
      hzx->GetXaxis()->SetTitle("z, cm");
      hzx->GetYaxis()->SetTitle("x, cm");
      hzy->GetXaxis()->SetTitle("z, cm");
      hzy->GetYaxis()->SetTitle("y, cm");
      hxy->GetXaxis()->SetTitle("x, cm");
      hxy->GetYaxis()->SetTitle("y, cm");
    }
    hzx->Draw();
    for (i1 = 0; i1 < n1; i1++) {
      clusterMarker->SetMarkerColor(i1 + 1);
      hitMarker->SetMarkerColor(i1 + 1);
      clusterPosition = m_KLMClusters[i1]->getClusterPosition();
      clusterMarker->DrawMarker(clusterPosition.Z(), clusterPosition.X());
      RelationVector<BKLMHit2d> bklmHit2ds =
        m_KLMClusters[i1]->getRelationsTo<BKLMHit2d>();
      n2 = bklmHit2ds.size();
      for (i2 = 0; i2 < n2; i2++) {
        hitPosition = bklmHit2ds[i2]->getGlobalPosition();
        hitMarker->DrawMarker(hitPosition.Z(), hitPosition.X());
      }
      RelationVector<EKLMHit2d> eklmHit2ds =
        m_KLMClusters[i1]->getRelationsTo<EKLMHit2d>();
      n2 = eklmHit2ds.size();
      for (i2 = 0; i2 < n2; i2++) {
        hitPosition = eklmHit2ds[i2]->getPosition();
        hitMarker->DrawMarker(hitPosition.Z(), hitPosition.X());
      }
    }
    snprintf(str, 128, "clusters%dzx.eps", nevent);
    c1->Print(str);
    hzy->Draw();
    for (i1 = 0; i1 < n1; i1++) {
      clusterMarker->SetMarkerColor(i1 + 1);
      hitMarker->SetMarkerColor(i1 + 1);
      clusterPosition = m_KLMClusters[i1]->getClusterPosition();
      clusterMarker->DrawMarker(clusterPosition.Z(), clusterPosition.Y());
      RelationVector<BKLMHit2d> bklmHit2ds =
        m_KLMClusters[i1]->getRelationsTo<BKLMHit2d>();
      n2 = bklmHit2ds.size();
      for (i2 = 0; i2 < n2; i2++) {
        hitPosition = bklmHit2ds[i2]->getGlobalPosition();
        hitMarker->DrawMarker(hitPosition.Z(), hitPosition.Y());
      }
      RelationVector<EKLMHit2d> eklmHit2ds =
        m_KLMClusters[i1]->getRelationsTo<EKLMHit2d>();
      n2 = eklmHit2ds.size();
      for (i2 = 0; i2 < n2; i2++) {
        hitPosition = eklmHit2ds[i2]->getPosition();
        hitMarker->DrawMarker(hitPosition.Z(), hitPosition.Y());
      }
    }
    snprintf(str, 128, "clusters%dzy.eps", nevent);
    c1->Print(str);
    hxy->Draw();
    for (i1 = 0; i1 < n1; i1++) {
      clusterMarker->SetMarkerColor(i1 + 1);
      hitMarker->SetMarkerColor(i1 + 1);
      clusterPosition = m_KLMClusters[i1]->getClusterPosition();
      clusterMarker->DrawMarker(clusterPosition.X(), clusterPosition.Y());
      RelationVector<BKLMHit2d> bklmHit2ds =
        m_KLMClusters[i1]->getRelationsTo<BKLMHit2d>();
      n2 = bklmHit2ds.size();
      for (i2 = 0; i2 < n2; i2++) {
        hitPosition = bklmHit2ds[i2]->getGlobalPosition();
        hitMarker->DrawMarker(hitPosition.X(), hitPosition.Y());
      }
      RelationVector<EKLMHit2d> eklmHit2ds =
        m_KLMClusters[i1]->getRelationsTo<EKLMHit2d>();
      n2 = eklmHit2ds.size();
      for (i2 = 0; i2 < n2; i2++) {
        hitPosition = eklmHit2ds[i2]->getPosition();
        hitMarker->DrawMarker(hitPosition.X(), hitPosition.Y());
      }
    }
    snprintf(str, 128, "clusters%dxy.eps", nevent);
    c1->Print(str);
    nevent++;
  }
  n1 = m_MCParticles.getEntries();
  for (i1 = 0; i1 < n1; i1++) {
    if (m_MCParticles[i1]->getPDG() != 130)
      continue;
    decayVertex = m_MCParticles[i1]->getDecayVertex();
    m_DecayVertexX = decayVertex.X();
    m_DecayVertexY = decayVertex.Y();
    m_DecayVertexZ = decayVertex.Z();
    m_MaxDecayVertexHitAngle = 0;
    RelationVector<BKLMHit2d> mcBKLMHit2ds =
      m_MCParticles[i1]->getRelationsFrom<BKLMHit2d>();
    n2 = mcBKLMHit2ds.size();
    for (i2 = 0; i2 < n2; i2++) {
      hitPosition = mcBKLMHit2ds[i2]->getGlobalPosition();
      angle = decayVertex.Angle(hitPosition);
      if (angle > m_MaxDecayVertexHitAngle)
        m_MaxDecayVertexHitAngle = angle;
    }
    RelationVector<EKLMHit2d> mcEKLMHit2ds =
      m_MCParticles[i1]->getRelationsFrom<EKLMHit2d>();
    n2 = mcEKLMHit2ds.size();
    for (i2 = 0; i2 < n2; i2++) {
      hitPosition = mcEKLMHit2ds[i2]->getPosition();
      angle = decayVertex.Angle(hitPosition);
      if (angle > m_MaxDecayVertexHitAngle)
        m_MaxDecayVertexHitAngle = angle;
    }
    RelationVector<KLMCluster> kl0Clusters =
      m_MCParticles[i1]->getRelationsFrom<KLMCluster>();
    n2 = kl0Clusters.size();
    if (n2 == 0)
      m_NonreconstructedKL0++;
    else if (n2 == 1) {
      RelationVector<BKLMHit2d> bklmHit2ds =
        kl0Clusters[0]->getRelationsTo<BKLMHit2d>();
      RelationVector<EKLMHit2d> eklmHit2ds =
        kl0Clusters[0]->getRelationsTo<EKLMHit2d>();
      bs1 = bklmHit2ds.size();
      es1 = eklmHit2ds.size();
      if (bs1 > 0) {
        if (es1 > 0)
          m_ReconstructedKL01Cluster[1]++;
        else
          m_ReconstructedKL01Cluster[0]++;
      } else if (es1 > 0)
        m_ReconstructedKL01Cluster[2]++;
      RelationVector<MCParticle> m_MCParticles2 =
        kl0Clusters[0]->getRelationsTo<MCParticle>();
      if (m_MCParticles2.size() == 1) {
        if (m_MCParticles2.weight(0) == 1)
          m_ExactlyReconstructedKL0++;
      }
    } else if (n2 == 2) {
      RelationVector<BKLMHit2d> bklmHit2ds1 =
        kl0Clusters[0]->getRelationsTo<BKLMHit2d>();
      RelationVector<BKLMHit2d> bklmHit2ds2 =
        kl0Clusters[1]->getRelationsTo<BKLMHit2d>();
      RelationVector<EKLMHit2d> eklmHit2ds1 =
        kl0Clusters[0]->getRelationsTo<EKLMHit2d>();
      RelationVector<EKLMHit2d> eklmHit2ds2 =
        kl0Clusters[1]->getRelationsTo<EKLMHit2d>();
      bs1 = bklmHit2ds1.size();
      bs2 = bklmHit2ds2.size();
      es1 = eklmHit2ds1.size();
      es2 = eklmHit2ds2.size();
      if (bs1 > 0 && bs2 > 0) {
        if (es1 > 0 && es2 > 0) {
          m_ReconstructedKL02Clusters[3]++;
        } else if ((es1 > 0 && es2 == 0) || (es1 == 0 && es2 > 0)) {
          m_ReconstructedKL02Clusters[1]++;
        } else if (es1 == 0 && es2 == 0) {
          m_ReconstructedKL02Clusters[0]++;
        }
      } else if (bs1 > 0 && bs2 == 0) {
        if (es1 > 0 && es2 > 0) {
          m_ReconstructedKL02Clusters[4]++;
        } else if (es1 == 0 && es2 > 0) {
          m_ReconstructedKL02Clusters[2]++;
        }
      } else if (bs1 == 0 && bs2 > 0) {
        if (es1 > 0 && es2 > 0) {
          m_ReconstructedKL02Clusters[4]++;
        } else if (es1 > 0 && es2 == 0) {
          m_ReconstructedKL02Clusters[2]++;
        }
      } else if (bs1 == 0 && bs2 == 0) {
        if (es1 > 0 && es2 > 0) {
          m_ReconstructedKL02Clusters[5]++;
        }
      }
    } else if (n2 >= 3)
      m_ReconstructedKL03Clusters++;
    if (m_SaveClusterData) {
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
        m_ClusterTree->Fill();
      }
    }
  }
}

void KLMClusterEfficiencyModule::endRun()
{
}

void KLMClusterEfficiencyModule::terminate()
{
  int i, rec1Cluster, rec2Clusters;
  rec1Cluster = 0;
  rec2Clusters = 0;
  for (i = 0; i < 3; i++)
    rec1Cluster = rec1Cluster + m_ReconstructedKL01Cluster[i];
  for (i = 0; i < 6; i++)
    rec2Clusters = rec2Clusters + m_ReconstructedKL02Clusters[i];
  B2INFO("Total number of KLM clusters: " << m_KL0Clusters +
         m_PartlyKL0Clusters + m_OtherClusters);
  B2INFO("K_L0 clusters: " << m_KL0Clusters);
  B2INFO("(K_L0+other) clusters: " << m_PartlyKL0Clusters);
  B2INFO("Other clusters: " << m_OtherClusters);
  B2INFO("Total number of generated K_L0: " << m_NonreconstructedKL0 +
         rec1Cluster + rec2Clusters + m_ReconstructedKL03Clusters);
  B2INFO("Nonreconstructed K_L0: " << m_NonreconstructedKL0);
  B2INFO("K_L0 reconstructed as 1 cluster (total): " << rec1Cluster);
  B2INFO("K_L0 reconstructed as 1 cluster (BKLM): " <<
         m_ReconstructedKL01Cluster[0]);
  B2INFO("K_L0 reconstructed as 1 cluster (BKLM/EKLM): " <<
         m_ReconstructedKL01Cluster[1]);
  B2INFO("K_L0 reconstructed as 1 cluster (EKLM): " <<
         m_ReconstructedKL01Cluster[2]);
  B2INFO("K_L0 reconstructed as 1 cluster (exact reconstruction): " <<
         m_ExactlyReconstructedKL0);
  B2INFO("K_L0 reconstructed as 2 clusters (total): " << rec2Clusters);
  B2INFO("K_L0 reconstructed as 2 clusters (2 * BKLM): " <<
         m_ReconstructedKL02Clusters[0]);
  B2INFO("K_L0 reconstructed as 2 clusters (BKLM + BKLM/EKLM): " <<
         m_ReconstructedKL02Clusters[1]);
  B2INFO("K_L0 reconstructed as 2 clusters (BKLM + EKLM): " <<
         m_ReconstructedKL02Clusters[2]);
  B2INFO("K_L0 reconstructed as 2 clusters (2 * BKLM/EKLM): " <<
         m_ReconstructedKL02Clusters[3]);
  B2INFO("K_L0 reconstructed as 2 clusters (BKLM/EKLM + EKLM): " <<
         m_ReconstructedKL02Clusters[4]);
  B2INFO("K_L0 reconstructed as 2 clusters (2 * EKLM): " <<
         m_ReconstructedKL02Clusters[5]);
  B2INFO("K_L0 reconstructed as 3 or more clusters: " <<
         m_ReconstructedKL03Clusters);
  if (m_SaveReconstructionData) {
    m_ReconstructionTree->Fill();
    m_OutputFile->cd();
    m_ReconstructionTree->Write();
    delete m_ReconstructionTree;
  }
  if (m_SaveClusterData) {
    m_OutputFile->cd();
    m_ClusterTree->Write();
    delete m_ClusterTree;
  }
  if (m_SaveClusterData || m_SaveReconstructionData)
    delete m_OutputFile;
}

