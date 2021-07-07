/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <dqm/modules/PhysicsObjectsMiraBelle/PhysicsObjectsMiraBelleModule.h>
#include <analysis/dataobjects/ParticleList.h>
#include <analysis/variables/ContinuumSuppressionVariables.h>
#include <analysis/variables/TrackVariables.h>
#include <analysis/utility/PCmsLabTransform.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/TrackFitResult.h>
#include <mdst/dataobjects/KLMCluster.h>
#include <mdst/dataobjects/HitPatternCDC.h>
#include <mdst/dataobjects/HitPatternVXD.h>
#include <mdst/dataobjects/EventLevelTrackingInfo.h>
#include <mdst/dataobjects/PIDLikelihood.h>
#include <top/variables/TOPDigitVariables.h>
#include <arich/modules/arichDQM/ARICHDQMModule.h>
#include <arich/dataobjects/ARICHLikelihood.h>
#include <klm/dataobjects/KLMMuidLikelihood.h>
#include <mdst/dataobjects/SoftwareTriggerResult.h>
#include <TDirectory.h>
#include <map>

using namespace Belle2;

REG_MODULE(PhysicsObjectsMiraBelle)

PhysicsObjectsMiraBelleModule::PhysicsObjectsMiraBelleModule() : HistoModule()
{
  setDescription("Monitor Physics Objects Quality");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("TriggerIdentifier", m_triggerIdentifier,
           "Trigger identifier string used to select events for the histograms", std::string("software_trigger_cut&skim&accept_mumutight"));
  addParam("MuPListName", m_muPListName, "Name of the muon particle list", std::string("mu+:physMiraBelle"));
}

void PhysicsObjectsMiraBelleModule::defineHisto()
{
  TDirectory* oldDir = gDirectory;
  oldDir->mkdir("PhysicsObjectsMiraBelle")->cd();

  m_h_npxd = new TH1F("hist_npxd", "hist_npxd", 100, 0, 5);
  m_h_npxd->SetXTitle("hist_npxd");
  m_h_nsvd = new TH1F("hist_nsvd", "hist_nsvd", 100, 0, 16);
  m_h_nsvd->SetXTitle("hist_nsvd");
  m_h_ncdc = new TH1F("hist_ncdc", "hist_ncdc", 100, 0, 80);
  m_h_ncdc->SetXTitle("hist_ncdc");
  m_h_topdig = new TH1F("hist_topdig", "hist_topdig", 120, 0, 120);
  m_h_topdig->SetXTitle("hist_topdig");
  m_h_DetPhotonARICH = new TH1F("hist_DetPhotonARICH", "hist_DetPhotonARICH", 70, 0, 70);
  m_h_DetPhotonARICH->SetXTitle("hist_DetPhotonARICH");
  m_h_klmTotalHits = new TH1F("hist_klmTotalHits", "hist_klmTotalHits", 15, 0, 15);
  m_h_klmTotalHits->SetXTitle("hist_klmTotalHits");
  m_h_Pval = new TH1F("hist_Pval", "hist_Pval", 100, 0, 1);
  m_h_Pval->SetXTitle("hist_Pval");
  m_h_dD0 = new TH1F("hist_dD0", "hist_dD0", 100, -0.02, 0.02);
  m_h_dD0->SetXTitle("hist_dD0");
  m_h_dZ0 = new TH1F("hist_dZ0", "hist_dZ0", 100, -0.05, 0.05);
  m_h_dZ0->SetXTitle("hist_dZ0");
  m_h_dPtcms = new TH1F("hist_dPtcms", "hist_dPtcms", 100, -0.5, 0.5);
  m_h_dPtcms->SetXTitle("hist_dPtcms");
  m_h_nExtraCDCHits = new TH1F("hist_nExtraCDCHits", "hist_nExtraCDCHits", 100, 0, 1600);
  m_h_nExtraCDCHits->SetXTitle("hist_nExtraCDCHits");
  m_h_nECLClusters = new TH1F("hist_nECLClusters", "hist_nECLClusters", 100, 0, 60);
  m_h_nECLClusters->SetXTitle("hist_nECLClusters");
  m_h_muid = new TH1F("hist_muid", "hist_muid", 20, 0, 1);
  m_h_muid->SetXTitle("hist_muid");
  m_h_inv_p = new TH1F("hist_inv_p", "hist_inv_p", 100, 8, 12);
  m_h_inv_p->SetXTitle("hist_inv_p");
  m_h_ndf = new TH1F("hist_ndf", "hist_ndf", 100, 0, 80);
  m_h_ndf->SetXTitle("hist_ndf");
  m_h_D0 = new TH1F("hist_D0", "hist_D0", 100, -0.1, 0.1);
  m_h_D0->SetXTitle("hist_D0");
  m_h_Z0 = new TH1F("hist_Z0", "hist_Z0", 100, -0.3, 0.3);
  m_h_Z0->SetXTitle("hist_Z0");
  m_h_theta = new TH1F("hist_theta", "hist_theta", 36, 10, 170);
  m_h_theta->SetXTitle("hist_theta");
  m_h_Phi0 = new TH1F("hist_Phi0", "hist_Phi0", 72, -180, 180);
  m_h_Phi0->SetXTitle("hist_Phi0");
  m_h_Pt = new TH1F("hist_Pt", "hist_Pt", 100, 0, 10);
  m_h_Pt->SetXTitle("hist_Pt");
  m_h_Mom = new TH1F("hist_Mom", "hist_Mom", 100, 0, 10);
  m_h_Mom->SetXTitle("hist_Mom");
  m_h_klmClusterLayers = new TH1F("hist_klmClusterLayers", "hist_klmClusterLayers", 16, 0, 16);
  m_h_klmClusterLayers->SetXTitle("hist_klmClusterLayers");
  m_h_klmTotalBarrelHits = new TH1F("hist_klmTotalBarrelHits", "hist_klmTotalBarrelHits", 16, 0, 16);
  m_h_klmTotalBarrelHits->SetXTitle("hist_klmTotalBarrelHits");
  m_h_klmTotalEndcapHits = new TH1F("hist_klmTotalEndcapHits", "hist_klmTotalEndcapHits", 16, 0, 16);
  m_h_klmTotalEndcapHits->SetXTitle("hist_klmTotalEndcapHits");
  m_h_dPhicms = new TH1F("hist_dPhicms", "hist_dPhicms", 100, -1, 1);
  m_h_dPhicms->SetXTitle("hist_dPhicms");

  oldDir->cd();
}


void PhysicsObjectsMiraBelleModule::initialize()
{
  REG_HISTOGRAM

  StoreObjPtr<SoftwareTriggerResult> result;
  result.isOptional();
}

void PhysicsObjectsMiraBelleModule::beginRun()
{
  m_h_npxd->Reset();
  m_h_nsvd->Reset();
  m_h_ncdc->Reset();
  m_h_topdig->Reset();
  m_h_DetPhotonARICH->Reset();
  m_h_klmTotalHits->Reset();
  m_h_Pval->Reset();
  m_h_dD0->Reset();
  m_h_dZ0->Reset();
  m_h_dPtcms->Reset();
  m_h_nExtraCDCHits->Reset();
  m_h_nECLClusters->Reset();
  m_h_muid->Reset();
  m_h_inv_p->Reset();
  m_h_ndf->Reset();
  m_h_D0->Reset();
  m_h_Z0->Reset();
  m_h_theta->Reset();
  m_h_Phi0->Reset();
  m_h_Pt->Reset();
  m_h_Mom->Reset();
  m_h_klmClusterLayers->Reset();
  m_h_klmTotalBarrelHits->Reset();
  m_h_klmTotalEndcapHits->Reset();
  m_h_dPhicms->Reset();
}

void PhysicsObjectsMiraBelleModule::event()
{

  StoreObjPtr<SoftwareTriggerResult> result;
  if (!result.isValid()) {
    B2WARNING("SoftwareTriggerResult object not available but needed to select events for the histograms.");
    return;
  }

  const std::map<std::string, int>& results = result->getResults();
  if (results.find(m_triggerIdentifier) == results.end()) {
    B2WARNING("PhysicsObjectsMiraBelle: Can't find trigger identifier: " << m_triggerIdentifier);
    return;
  }

  // apply software trigger
  const bool accepted = (result->getResult(m_triggerIdentifier) == SoftwareTriggerCutResult::c_accept);
  if (accepted == false) return;

  // for resolution (difference b/w 2 tracks)
  double d0[2] = {};
  double z0[2] = {};
  double ptcms[2] = {};
  double phicms[2] = {};

  // get muons
  StoreObjPtr<ParticleList> muParticles(m_muPListName);
  for (unsigned int i = 0; i < muParticles->getListSize(); i++) {
    Particle* mu = muParticles->getParticle(i);
    const Belle2::Track* track = mu->getTrack();
    if (!track) {
      continue;
    }

    // Detector hits
    m_h_npxd->Fill(Belle2::Variable::trackNPXDHits(mu));
    m_h_nsvd->Fill(Belle2::Variable::trackNSVDHits(mu));
    m_h_ncdc->Fill(Belle2::Variable::trackNCDCHits(mu));
    m_h_topdig->Fill(Belle2::Variable::TOPVariable::topDigitCount(mu));
    ARICHLikelihood* lkh = track->getRelated<ARICHLikelihood>();
    if (lkh) {
      m_h_DetPhotonARICH->Fill(lkh->getDetPhot());
    }

    // KLM total hits
    KLMMuidLikelihood* muid = track->getRelatedTo<KLMMuidLikelihood>();
    if (muid) {
      unsigned int bklm_hit = muid->getTotalBarrelHits();
      unsigned int eklm_hit = muid->getTotalEndcapHits();
      m_h_klmTotalBarrelHits->Fill(bklm_hit);
      m_h_klmTotalEndcapHits->Fill(eklm_hit);
      m_h_klmTotalHits->Fill(bklm_hit + eklm_hit);
    }

    // KLM Cluster layers
    KLMCluster* klmc = track->getRelated<KLMCluster>();
    if (klmc) {
      m_h_klmClusterLayers->Fill(klmc->getLayers());
    }

    // muon ID
    PIDLikelihood* pid_lkh = track->getRelated<PIDLikelihood>();
    if (pid_lkh) {
      m_h_muid->Fill(pid_lkh->getProbability(Belle2::Const::muon));
    }

    // Track variables
    const TrackFitResult* fitresult = track->getTrackFitResult(Belle2::Const::pion);
    if (fitresult) {
      // Pvalue
      double pval = fitresult->getPValue();
      m_h_Pval->Fill(pval);
      // separate mu+ and mu-
      int index = fitresult->getChargeSign() > 0 ? 0 : 1;
      d0[index] = fitresult->getD0();
      z0[index] = fitresult->getZ0();
      m_h_D0->Fill(d0[index]);
      m_h_Z0->Fill(z0[index]);
      // Momentum
      ptcms[index] = Belle2::PCmsLabTransform::labToCms(fitresult->get4Momentum()).Pt();//CMS
      phicms[index] = Belle2::PCmsLabTransform::labToCms(fitresult->get4Momentum()).Phi();
      m_h_Pt->Fill(fitresult->get4Momentum().Pt());//Lab
      m_h_theta->Fill(Belle2::PCmsLabTransform::labToCms(fitresult->get4Momentum()).Theta());//CMS
      m_h_Phi0->Fill(fitresult->get4Momentum().Phi());//Lab
      m_h_Mom->Fill(fitresult->get4Momentum().P());//Lab
    }
  }
  // Resolution
  m_h_dD0->Fill((d0[0] + d0[1]) / sqrt(2));
  m_h_dZ0->Fill((z0[0] - z0[1]) / sqrt(2));
  m_h_dPtcms->Fill((ptcms[0] - ptcms[1]) / sqrt(2));
  m_h_dPhicms->Fill(180 - abs(phicms[0] - phicms[1]));
  // Event level information
  StoreObjPtr<EventLevelTrackingInfo> elti;
  if (elti) {
    m_h_nExtraCDCHits->Fill(elti->getNCDCHitsNotAssigned());
  }
  //nECLClustersLE
  double neclClusters = -1.;
  StoreArray<ECLCluster> eclClusters;
  if (eclClusters.isValid()) {
    const unsigned int numberOfECLClusters = std::count_if(eclClusters.begin(), eclClusters.end(),
    [](const ECLCluster & eclcluster) {
      return (eclcluster.hasHypothesis(ECLCluster::EHypothesisBit::c_nPhotons)
              and eclcluster.getEnergy(ECLCluster::EHypothesisBit::c_nPhotons) > 0.1);
    });
    neclClusters = numberOfECLClusters;
  }
  m_h_nECLClusters->Fill(neclClusters);
}

void PhysicsObjectsMiraBelleModule::endRun()
{
}

void PhysicsObjectsMiraBelleModule::terminate()
{
}

