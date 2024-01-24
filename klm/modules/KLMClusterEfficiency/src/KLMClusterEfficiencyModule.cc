/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

/* Own header. */
#include <klm/modules/KLMClusterEfficiency/KLMClusterEfficiencyModule.h>

/* KLM headers. */
#include <klm/dataobjects/KLMHit2d.h>

/* Basf2 headers. */
#include <framework/gearbox/Const.h>

/* ROOT headers. */
#include <Math/VectorUtil.h>
#include <TCanvas.h>
#include <TH2F.h>
#include <TMarker.h>
#include <TStyle.h>

using namespace Belle2;

REG_MODULE(KLMClusterEfficiency);

KLMClusterEfficiencyModule::KLMClusterEfficiencyModule() : Module(),
  m_OutputFile(nullptr), m_ClusterTree(nullptr), m_ReconstructionTree(nullptr),
  m_DecayVertexX(0), m_DecayVertexY(0), m_DecayVertexZ(0),
  m_MaxDecayVertexHitAngle(0), m_ClusterX(0), m_ClusterY(0), m_ClusterZ(0),
  m_MaxClusterHitAngle(0), m_KL0Clusters(0), m_PartlyKL0Clusters(0),
  m_OtherClusters(0), m_NonreconstructedKL0(0),
  m_ReconstructedKL01Cluster{0, 0, 0}, m_ExactlyReconstructedKL0(0),
  m_ReconstructedKL02Clusters{0, 0, 0, 0, 0, 0},
  m_ReconstructedKL03Clusters(0)
{
  setDescription("Module for KLM cluster reconstruction efficiency studies.");
  addParam("SaveClusterData", m_SaveClusterData,
           "Whether to save cluster data or not.", false);
  addParam("SaveReconstructionData", m_SaveReconstructionData,
           "Whether to save reconstruction data or not.", true);
  addParam("OutputFile", m_OutputFileName, "Output file.",
           std::string("KLMClusterEfficiency.root"));
  addParam("EventsClusterHistograms", m_EventsClusterHistograms,
           "Draw cluster histograms for this number of events.", 0);
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
  /* cppcheck-suppress variableScope */
  char str[128];
  int i1, i2, i3, n1, n2, n3;
  int bs1, bs2, es1, es2;
  ROOT::Math::XYZVector clusterPosition;
  ROOT::Math::XYZVector decayVertex, hitPosition;
  float angle;
  /* cppcheck-suppress variableScope */
  bool haveKL0;
  n1 = m_KLMClusters.getEntries();
  for (i1 = 0; i1 < n1; i1++) {
    haveKL0 = false;
    RelationVector<MCParticle> clusterMCParticles =
      m_KLMClusters[i1]->getRelationsTo<MCParticle>();
    n2 = clusterMCParticles.size();
    for (i2 = 0; i2 < n2; i2++) {
      if (clusterMCParticles[i2]->getPDG() == Const::Klong.getPDGCode())
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
      RelationVector<KLMHit2d> klmHit2ds =
        m_KLMClusters[i1]->getRelationsTo<KLMHit2d>();
      n2 = klmHit2ds.size();
      for (i2 = 0; i2 < n2; i2++) {
        hitPosition = klmHit2ds[i2]->getPosition();
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
      RelationVector<KLMHit2d> klmHit2ds =
        m_KLMClusters[i1]->getRelationsTo<KLMHit2d>();
      n2 = klmHit2ds.size();
      for (i2 = 0; i2 < n2; i2++) {
        hitPosition = klmHit2ds[i2]->getPosition();
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
      RelationVector<KLMHit2d> klmHit2ds =
        m_KLMClusters[i1]->getRelationsTo<KLMHit2d>();
      n2 = klmHit2ds.size();
      for (i2 = 0; i2 < n2; i2++) {
        hitPosition = klmHit2ds[i2]->getPosition();
        hitMarker->DrawMarker(hitPosition.X(), hitPosition.Y());
      }
    }
    snprintf(str, 128, "clusters%dxy.eps", nevent);
    c1->Print(str);
    nevent++;
  }
  n1 = m_MCParticles.getEntries();
  for (i1 = 0; i1 < n1; i1++) {
    if (m_MCParticles[i1]->getPDG() != Const::Klong.getPDGCode())
      continue;
    decayVertex = m_MCParticles[i1]->getDecayVertex();
    m_DecayVertexX = decayVertex.X();
    m_DecayVertexY = decayVertex.Y();
    m_DecayVertexZ = decayVertex.Z();
    m_MaxDecayVertexHitAngle = 0;
    RelationVector<KLMHit2d> mcKLMHit2ds =
      m_MCParticles[i1]->getRelationsFrom<KLMHit2d>();
    n2 = mcKLMHit2ds.size();
    for (i2 = 0; i2 < n2; i2++) {
      hitPosition = mcKLMHit2ds[i2]->getPosition();
      angle = ROOT::Math::VectorUtil::Angle(decayVertex, hitPosition);
      if (angle > m_MaxDecayVertexHitAngle)
        m_MaxDecayVertexHitAngle = angle;
    }
    RelationVector<KLMCluster> kl0Clusters =
      m_MCParticles[i1]->getRelationsFrom<KLMCluster>();
    n2 = kl0Clusters.size();
    if (n2 == 0)
      m_NonreconstructedKL0++;
    else if (n2 == 1) {
      RelationVector<KLMHit2d> klmHit2ds =
        kl0Clusters[0]->getRelationsTo<KLMHit2d>();
      bs1 = 0;
      es1 = 0;
      for (const KLMHit2d& hit2d : klmHit2ds) {
        if (hit2d.getSubdetector() == KLMElementNumbers::c_BKLM)
          bs1++;
        else
          es1++;
      }
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
      RelationVector<KLMHit2d> klmHit2ds1 =
        kl0Clusters[0]->getRelationsTo<KLMHit2d>();
      RelationVector<KLMHit2d> klmHit2ds2 =
        kl0Clusters[1]->getRelationsTo<KLMHit2d>();
      bs1 = 0;
      bs2 = 0;
      es1 = 0;
      es2 = 0;
      for (const KLMHit2d& hit2d : klmHit2ds1) {
        if (hit2d.getSubdetector() == KLMElementNumbers::c_BKLM)
          bs1++;
        else
          es1++;
      }
      for (const KLMHit2d& hit2d : klmHit2ds2) {
        if (hit2d.getSubdetector() == KLMElementNumbers::c_BKLM)
          bs2++;
        else
          es2++;
      }
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
        RelationVector<KLMHit2d> klmHit2ds =
          kl0Clusters[i2]->getRelationsTo<KLMHit2d>();
        n3 = klmHit2ds.size();
        for (i3 = 0; i3 < n3; i3++) {
          hitPosition = klmHit2ds[i3]->getPosition();
          angle = ROOT::Math::VectorUtil::Angle(clusterPosition, hitPosition);
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
  /* Always printed once, not necessary to use LogVar. */
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

