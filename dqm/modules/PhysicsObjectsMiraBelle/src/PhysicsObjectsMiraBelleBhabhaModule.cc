/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <dqm/modules/PhysicsObjectsMiraBelle/PhysicsObjectsMiraBelleBhabhaModule.h>
#include <analysis/dataobjects/ParticleList.h>
#include <analysis/variables/TrackVariables.h>
#include <analysis/utility/PCmsLabTransform.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/TrackFitResult.h>
#include <mdst/dataobjects/EventLevelTrackingInfo.h>
#include <mdst/dataobjects/PIDLikelihood.h>
#include <top/variables/TOPDigitVariables.h>
#include <arich/dataobjects/ARICHLikelihood.h>
#include <mdst/dataobjects/SoftwareTriggerResult.h>
#include <TDirectory.h>
#include <TMath.h>
#include <map>

using namespace Belle2;

REG_MODULE(PhysicsObjectsMiraBelleBhabha);

PhysicsObjectsMiraBelleBhabhaModule::PhysicsObjectsMiraBelleBhabhaModule() : HistoModule()
{
  setDescription("Monitor Physics Objects Quality");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("TriggerIdentifier", m_triggerIdentifier,
           "Trigger identifier string used to select events for the histograms", std::string("software_trigger_cut&skim&accept_bhabha_all"));
  addParam("ePListName", m_ePListName, "Name of the electron particle list", std::string("e+:physMiraBelle"));
  addParam("bhabhaPListName", m_bhabhaPListName, "Name of the bhabha events particle list", std::string("Upsilon:ephysMiraBelle"));

}

void PhysicsObjectsMiraBelleBhabhaModule::defineHisto()
{
  TDirectory* oldDir = gDirectory;
  oldDir->mkdir("PhysicsObjectsMiraBelleBhabha");
  oldDir->cd("PhysicsObjectsMiraBelleBhabha");

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
  m_h_Pval = new TH1F("hist_Pval", "hist_Pval", 100, 0, 1);
  m_h_Pval->SetXTitle("hist_Pval");
  m_h_dD0 = new TH1F("hist_dD0", "hist_dD0", 100, -0.02, 0.02);
  m_h_dD0->SetXTitle("hist_dD0");
  m_h_dZ0 = new TH1F("hist_dZ0", "hist_dZ0", 100, -0.05, 0.05);
  m_h_dZ0->SetXTitle("hist_dZ0");
  m_h_dPtcms = new TH1F("hist_dPtcms", "hist_dPtcms", 100, -0.5, 0.5);
  m_h_dPtcms->SetXTitle("hist_dPtcms");
  m_h_nExtraCDCHits = new TH1F("hist_nExtraCDCHits", "hist_nExtraCDCHits", 500, 0, 5000);
  m_h_nExtraCDCHits->SetXTitle("hist_nExtraCDCHits");
  m_h_nECLClusters = new TH1F("hist_nECLClusters", "hist_nECLClusters", 100, 0, 60);
  m_h_nECLClusters->SetXTitle("hist_nECLClusters");
  m_h_electronid = new TH1F("hist_electronid", "hist_electronid", 20, 0, 1);
  m_h_electronid->SetXTitle("hist_electronid");
  m_h_inv_p = new TH1F("hist_inv_p", "hist_inv_p", 400, 0, 12);
  m_h_inv_p->SetXTitle("hist_inv_p");
  m_h_ndf = new TH1F("hist_ndf", "hist_ndf", 100, 0, 80);
  m_h_ndf->SetXTitle("hist_ndf");
  m_h_D0 = new TH1F("hist_D0", "hist_D0", 100, -0.1, 0.1);
  m_h_D0->SetXTitle("hist_D0");
  m_h_Z0 = new TH1F("hist_Z0", "hist_Z0", 100, -0.3, 0.3);
  m_h_Z0->SetXTitle("hist_Z0");
  m_h_theta = new TH1F("hist_theta", "hist_theta in CMS", 32, 10, 170);
  m_h_theta->SetXTitle("hist_theta");
  m_h_theta_lab = new TH1F("hist_theta_lab", "hist_theta in lab frame", 180, 0, 180);
  m_h_theta_lab->SetXTitle("hist_theta_lab");
  m_h_Phi0 = new TH1F("hist_Phi0", "hist_Phi0 in lab frame", 72, -180, 180);
  m_h_Phi0->SetXTitle("hist_Phi0");
  m_h_Pt = new TH1F("hist_Pt", "hist_Pt", 100, 0, 10);
  m_h_Pt->SetXTitle("hist_Pt");
  m_h_Mom = new TH1F("hist_Mom", "hist_Mom", 100, 0, 10);
  m_h_Mom->SetXTitle("hist_Mom");
  m_h_dPhicms = new TH1F("hist_dPhicms", "hist_dPhicms: 180#circ - |#phi_{1} - #phi_{2}|", 100, -10, 10);
  m_h_dPhicms->SetXTitle("hist_dPhicms");
  m_h_dThetacms = new TH1F("hist_dThetacms", "hist_dThetacms: |#theta_{1} + #theta_{2}| - 180#circ", 100, -10, 10);
  m_h_dThetacms->SetXTitle("hist_dThetacms");

  oldDir->cd();
}


void PhysicsObjectsMiraBelleBhabhaModule::initialize()
{
  REG_HISTOGRAM

  StoreObjPtr<SoftwareTriggerResult> result;
  result.isOptional();
}

void PhysicsObjectsMiraBelleBhabhaModule::beginRun()
{
  m_h_npxd->Reset();
  m_h_nsvd->Reset();
  m_h_ncdc->Reset();
  m_h_topdig->Reset();
  m_h_DetPhotonARICH->Reset();
  m_h_Pval->Reset();
  m_h_dD0->Reset();
  m_h_dZ0->Reset();
  m_h_dPtcms->Reset();
  m_h_nExtraCDCHits->Reset();
  m_h_nECLClusters->Reset();
  m_h_electronid->Reset();
  m_h_inv_p->Reset();
  m_h_ndf->Reset();
  m_h_D0->Reset();
  m_h_Z0->Reset();
  m_h_theta->Reset();
  m_h_theta_lab->Reset();
  m_h_Phi0->Reset();
  m_h_Pt->Reset();
  m_h_Mom->Reset();
  m_h_dPhicms->Reset();
  m_h_dThetacms->Reset();
}

void PhysicsObjectsMiraBelleBhabhaModule::event()
{

  StoreObjPtr<SoftwareTriggerResult> result;
  if (!result.isValid()) {
    B2WARNING("SoftwareTriggerResult object not available but needed to select events for the histograms.");
    return;
  }

  const std::map<std::string, int>& results = result->getResults();
  if (results.find(m_triggerIdentifier) == results.end()) {
    B2WARNING("PhysicsObjectsMiraBelleBhabha: Can't find trigger identifier: " << m_triggerIdentifier);
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
  double thetacms[2] = {};

  //get the bhabha events for beam energy check
  StoreObjPtr<ParticleList> UpsParticles(m_bhabhaPListName);
  if (UpsParticles.isValid()) {
    for (unsigned int i = 0; i < UpsParticles->getListSize(); i++) {
      Particle* Ups = UpsParticles->getParticle(i);
      m_h_inv_p->Fill(Ups->getMass());
    }
  }

  // get electrons
  StoreObjPtr<ParticleList> electronParticles(m_ePListName);
  for (unsigned int i = 0; i < electronParticles->getListSize(); i++) {
    Particle* electron = electronParticles->getParticle(i);
    const Belle2::Track* track = electron->getTrack();
    if (!track) {
      continue;
    }

    // Detector hits
    m_h_npxd->Fill(Belle2::Variable::trackNPXDHits(electron));
    m_h_nsvd->Fill(Belle2::Variable::trackNSVDHits(electron));
    m_h_ncdc->Fill(Belle2::Variable::trackNCDCHits(electron));
    m_h_topdig->Fill(Belle2::Variable::TOPVariable::topDigitCount(electron));
    ARICHLikelihood* lkh = track->getRelated<ARICHLikelihood>();
    if (lkh) {
      m_h_DetPhotonARICH->Fill(lkh->getDetPhot());
    }

    // electron ID
    PIDLikelihood* pid_lkh = track->getRelated<PIDLikelihood>();
    if (pid_lkh) {
      m_h_electronid->Fill(pid_lkh->getProbability(Belle2::Const::electron));
    }

    // Track variables
    const TrackFitResult* fitresult = track->getTrackFitResult(Belle2::Const::pion);
    if (fitresult) {
      // Pvalue
      double pval = fitresult->getPValue();
      m_h_Pval->Fill(pval);
      // separate electron+ and electron-
      int index = fitresult->getChargeSign() > 0 ? 0 : 1;
      d0[index] = fitresult->getD0();
      z0[index] = fitresult->getZ0();
      m_h_D0->Fill(d0[index]);
      m_h_Z0->Fill(z0[index]);
      // Momentum
      ptcms[index] = Belle2::PCmsLabTransform::labToCms(fitresult->get4Momentum()).Pt();//CMS
      phicms[index] = Belle2::PCmsLabTransform::labToCms(fitresult->get4Momentum()).Phi() * TMath::RadToDeg();
      thetacms[index] = Belle2::PCmsLabTransform::labToCms(fitresult->get4Momentum()).Theta() * TMath::RadToDeg();
      m_h_Pt->Fill(fitresult->get4Momentum().Pt());//Lab
      m_h_theta->Fill(Belle2::PCmsLabTransform::labToCms(fitresult->get4Momentum()).Theta() * TMath::RadToDeg());//CMS
      m_h_theta_lab->Fill(fitresult->get4Momentum().Theta() * TMath::RadToDeg());//Lab
      m_h_Phi0->Fill(fitresult->get4Momentum().Phi() * TMath::RadToDeg());//Lab
      m_h_Mom->Fill(fitresult->get4Momentum().P());//Lab
    }
  }
  // Resolution
  m_h_dD0->Fill((d0[0] + d0[1]) / sqrt(2));
  m_h_dZ0->Fill((z0[0] - z0[1]) / sqrt(2));
  m_h_dPtcms->Fill((ptcms[0] - ptcms[1]) / sqrt(2));
  m_h_dPhicms->Fill(180 - abs(phicms[0] - phicms[1]));
  m_h_dThetacms->Fill(abs(thetacms[0] + thetacms[1]) - 180);
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

void PhysicsObjectsMiraBelleBhabhaModule::endRun()
{
}

void PhysicsObjectsMiraBelleBhabhaModule::terminate()
{
}

