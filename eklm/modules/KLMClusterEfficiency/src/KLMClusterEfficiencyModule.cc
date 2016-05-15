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
  addParam("EventsClusterHistograms", m_EventsClusterHistograms,
           "Draw cluster histograms for this number of events.", 0);
  m_OutputFile = NULL;
  m_OutputTree = NULL;
  m_KL0Clusters = 0;
  m_PartlyKL0Clusters = 0;
  m_OtherClusters = 0;
  m_NonreconstructedKL0 = 0;
  for (i = 0; i < 2; i++) {
    m_ReconstructedKL0EKLM[i] = 0;
    m_ReconstructedKL0BKLM[i] = 0;
  }
  m_ReconstructedKL0EKLMBKLM = 0;
  m_ReconstructedKL03Clusters = 0;
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
  gStyle->SetOptStat(0);
}

void KLMClusterEfficiencyModule::beginRun()
{
}

void KLMClusterEfficiencyModule::event()
{
  StoreArray<KLMCluster> klmClusters;
  StoreArray<MCParticle> mcParticles;
  static int nevent = 0;
  char str[128];
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
      clusterPosition = klmClusters[i1]->getClusterPosition();
      clusterMarker->DrawMarker(clusterPosition.Z(), clusterPosition.X());
      RelationVector<BKLMHit2d> bklmHit2ds =
        klmClusters[i1]->getRelationsTo<BKLMHit2d>();
      n2 = bklmHit2ds.size();
      for (i2 = 0; i2 < n2; i2++) {
        hitPosition = bklmHit2ds[i2]->getGlobalPosition();
        hitMarker->DrawMarker(hitPosition.Z(), hitPosition.X());
      }
      RelationVector<EKLMHit2d> eklmHit2ds =
        klmClusters[i1]->getRelationsTo<EKLMHit2d>();
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
      clusterPosition = klmClusters[i1]->getClusterPosition();
      clusterMarker->DrawMarker(clusterPosition.Z(), clusterPosition.Y());
      RelationVector<BKLMHit2d> bklmHit2ds =
        klmClusters[i1]->getRelationsTo<BKLMHit2d>();
      n2 = bklmHit2ds.size();
      for (i2 = 0; i2 < n2; i2++) {
        hitPosition = bklmHit2ds[i2]->getGlobalPosition();
        hitMarker->DrawMarker(hitPosition.Z(), hitPosition.Y());
      }
      RelationVector<EKLMHit2d> eklmHit2ds =
        klmClusters[i1]->getRelationsTo<EKLMHit2d>();
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
      clusterPosition = klmClusters[i1]->getClusterPosition();
      clusterMarker->DrawMarker(clusterPosition.X(), clusterPosition.Y());
      RelationVector<BKLMHit2d> bklmHit2ds =
        klmClusters[i1]->getRelationsTo<BKLMHit2d>();
      n2 = bklmHit2ds.size();
      for (i2 = 0; i2 < n2; i2++) {
        hitPosition = bklmHit2ds[i2]->getGlobalPosition();
        hitMarker->DrawMarker(hitPosition.X(), hitPosition.Y());
      }
      RelationVector<EKLMHit2d> eklmHit2ds =
        klmClusters[i1]->getRelationsTo<EKLMHit2d>();
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
    RelationVector<KLMCluster> kl0Clusters =
      mcParticles[i1]->getRelationsFrom<KLMCluster>();
    n2 = kl0Clusters.size();
    if (n2 == 0)
      m_NonreconstructedKL0++;
    else if (n2 == 1) {
      RelationVector<BKLMHit2d> bklmHit2ds =
        kl0Clusters[0]->getRelationsTo<BKLMHit2d>();
      RelationVector<EKLMHit2d> eklmHit2ds =
        kl0Clusters[0]->getRelationsTo<EKLMHit2d>();
      if (bklmHit2ds.size() > 0)
        m_ReconstructedKL0BKLM[0]++;
      else if (eklmHit2ds.size() > 0)
        m_ReconstructedKL0EKLM[0]++;
    } else if (n2 == 2) {
      RelationVector<BKLMHit2d> bklmHit2ds1 =
        kl0Clusters[0]->getRelationsTo<BKLMHit2d>();
      RelationVector<BKLMHit2d> bklmHit2ds2 =
        kl0Clusters[1]->getRelationsTo<BKLMHit2d>();
      RelationVector<EKLMHit2d> eklmHit2ds1 =
        kl0Clusters[0]->getRelationsTo<EKLMHit2d>();
      RelationVector<EKLMHit2d> eklmHit2ds2 =
        kl0Clusters[1]->getRelationsTo<EKLMHit2d>();
      if (bklmHit2ds1.size() > 0 && bklmHit2ds2.size() > 0)
        m_ReconstructedKL0BKLM[1]++;
      else if ((bklmHit2ds1.size() > 0 && eklmHit2ds2.size() > 0) ||
               (eklmHit2ds1.size() > 0 && bklmHit2ds2.size() > 0))
        m_ReconstructedKL0EKLMBKLM++;
      else if (eklmHit2ds1.size() > 0 && eklmHit2ds2.size() > 0)
        m_ReconstructedKL0EKLM[1]++;
    } else if (n2 >= 3)
      m_ReconstructedKL03Clusters++;
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
  B2INFO("Total number of generated K_L0: " << m_NonreconstructedKL0 +
         m_ReconstructedKL0BKLM[0] + m_ReconstructedKL0EKLM[0] +
         m_ReconstructedKL0BKLM[1] + m_ReconstructedKL0EKLM[1] +
         m_ReconstructedKL0EKLMBKLM + m_ReconstructedKL03Clusters);
  B2INFO("Nonreconstructed K_L0: " << m_NonreconstructedKL0);
  B2INFO("K_L0 reconstructed as 1 cluster (total): " <<
         m_ReconstructedKL0BKLM[0] + m_ReconstructedKL0EKLM[0]);
  B2INFO("K_L0 reconstructed as 1 cluster (BKLM): " <<
         m_ReconstructedKL0BKLM[0]);
  B2INFO("K_L0 reconstructed as 1 cluster (EKLM): " <<
         m_ReconstructedKL0EKLM[0]);
  B2INFO("K_L0 reconstructed as 2 clusters (total): " <<
         m_ReconstructedKL0BKLM[1] + m_ReconstructedKL0EKLM[1] +
         m_ReconstructedKL0EKLMBKLM);
  B2INFO("K_L0 reconstructed as 2 clusters (BKLM): " <<
         m_ReconstructedKL0BKLM[1]);
  B2INFO("K_L0 reconstructed as 2 clusters (BKLM + EKLM): " <<
         m_ReconstructedKL0EKLMBKLM);
  B2INFO("K_L0 reconstructed as 2 clusters (EKLM): " <<
         m_ReconstructedKL0EKLM[1]);
  B2INFO("K_L0 reconstructed as 3 or more clusters: " <<
         m_ReconstructedKL03Clusters);
  m_OutputFile->cd();
  m_OutputTree->Write();
  delete m_OutputTree;
  delete m_OutputFile;
}

