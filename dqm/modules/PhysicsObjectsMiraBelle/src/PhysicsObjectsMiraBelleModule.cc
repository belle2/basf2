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
//#include <tracking/dataobjects/Muid.h>
#include <mdst/dataobjects/SoftwareTriggerResult.h>
#include <TDirectory.h>
#include <map>

//avoid having to wrap everything in the namespace explicitly
//only permissible in .cc files!
using namespace Belle2;

//this line registers the module with the framework and actually makes it available
//in steering files or the the module list (basf2 -m).
//Note that the 'Module' part of the class name is missing, this is also the way it
//will be called in the module list.
REG_MODULE(PhysicsObjectsMiraBelle)

PhysicsObjectsMiraBelleModule::PhysicsObjectsMiraBelleModule() : HistoModule()
{
  setDescription("Monitor Physics Objects Quality");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("TriggerIdentifier", m_triggerIdentifier,
           //"Trigger identifier string used to select events for the histograms", std::string("software_trigger_cut&skim&accept_hadron"));
           "Trigger identifier string used to select events for the histograms", std::string("software_trigger_cut&skim&accept_mumu_2trk"));
  addParam("MuPListName", m_muPListName, "Name of the muon particle list", std::string("mu+:physMiraBelle"));
  // addParam("vphoPListName", m_vphoPListName, "Name of the parent particle list", std::string("vpho:physMiraBelle"));
}

void PhysicsObjectsMiraBelleModule::defineHisto()
{
  TDirectory* oldDir = gDirectory;
  oldDir->mkdir("PhysicsObjectsMiraBelle")->cd();
  //oldDir->cd("PhysicsObjects");

  m_h_npxd = new TH1F("m_h_npxd", "m_h_npxd", 100, 0, 5);
  m_h_npxd->SetXTitle("m_h_npxd");
  m_h_nsvd = new TH1F("m_h_nsvd", "m_h_nsvd", 100, 0, 16);
  m_h_nsvd->SetXTitle("m_h_nsvd");
  m_h_ncdc = new TH1F("m_h_ncdc", "m_h_ncdc", 100, 0, 80);
  m_h_ncdc->SetXTitle("m_h_ncdc");
  m_h_topdig = new TH1F("m_h_topdig", "m_h_topdig", 120, 0, 120);
  m_h_topdig->SetXTitle("m_h_topdig");
  m_h_DetPhotonARICH = new TH1F("m_h_DetPhotonARICH", "m_h_DetPhotonARICH", 70, 0, 70);
  m_h_DetPhotonARICH->SetXTitle("m_h_DetPhotonARICH");
  m_h_klmTotalHits = new TH1F("m_h_klmTotalHits", "m_h_klmTotalHits", 15, 0, 15);
  m_h_klmTotalHits->SetXTitle("m_h_klmTotalHits");
  m_h_Pval = new TH1F("m_h_Pval", "m_h_Pval", 100, 0, 1);
  m_h_Pval->SetXTitle("m_h_Pval");
  m_h_dD0 = new TH1F("m_h_dD0", "m_h_dD0", 100, -0.02, 0.02);
  m_h_dD0->SetXTitle("m_h_dD0");
  m_h_dZ0 = new TH1F("m_h_dZ0", "m_h_dZ0", 100, -0.05, 0.05);
  m_h_dZ0->SetXTitle("m_h_dZ0");
  m_h_dPtcms = new TH1F("m_h_dPtcms", "m_h_dPtcms", 100, -0.5, 0.5);
  m_h_dPtcms->SetXTitle("m_h_dPtcms");
  m_h_nExtraCDCHits = new TH1F("m_h_nExtraCDCHits", "m_h_nExtraCDCHits", 100, 0, 1600);
  m_h_nExtraCDCHits->SetXTitle("m_h_nExtraCDCHits");
  m_h_nECLClusters = new TH1F("m_h_nECLClusters", "m_h_nECLClusters", 100, 0, 60);
  m_h_nECLClusters->SetXTitle("m_h_nECLClusters");
  // m_h_SVD_offline_occupancy_L3u = new TH1F("m_h_SVD_offline_occupancy_L3u", "m_h_SVD_offline_occupancy_L3u", 100, 0, 3);
  // m_h_SVD_offline_occupancy_L3u->SetXTitle("m_h_SVD_offline_occupancy_L3u");
  // m_h_SVD_offline_occupancy_L4u = new TH1F("m_h_SVD_offline_occupancy_L4u", "m_h_SVD_offline_occupancy_L4u", 100, 0, 3);
  // m_h_SVD_offline_occupancy_L4u->SetXTitle("m_h_SVD_offline_occupancy_L4u");
  // m_h_SVD_offline_occupancy_L5u = new TH1F("m_h_SVD_offline_occupancy_L5u", "m_h_SVD_offline_occupancy_L5u", 100, 0, 3);
  // m_h_SVD_offline_occupancy_L5u->SetXTitle("m_h_SVD_offline_occupancy_L5u");
  // m_h_SVD_offline_occupancy_L6u = new TH1F("m_h_SVD_offline_occupancy_L6u", "m_h_SVD_offline_occupancy_L6u", 100, 0, 3);
  // m_h_SVD_offline_occupancy_L6u->SetXTitle("m_h_SVD_offline_occupancy_L6u");
  // m_h_SVD_offline_occupancy_L3v = new TH1F("m_h_SVD_offline_occupancy_L3v", "m_h_SVD_offline_occupancy_L3v", 100, 0, 3);
  // m_h_SVD_offline_occupancy_L3v->SetXTitle("m_h_SVD_offline_occupancy_L3v");
  // m_h_SVD_offline_occupancy_L4v = new TH1F("m_h_SVD_offline_occupancy_L4v", "m_h_SVD_offline_occupancy_L4v", 100, 0, 3);
  // m_h_SVD_offline_occupancy_L4v->SetXTitle("m_h_SVD_offline_occupancy_L4v");
  // m_h_SVD_offline_occupancy_L5v = new TH1F("m_h_SVD_offline_occupancy_L5v", "m_h_SVD_offline_occupancy_L5v", 100, 0, 3);
  // m_h_SVD_offline_occupancy_L5v->SetXTitle("m_h_SVD_offline_occupancy_L5v");
  // m_h_SVD_offline_occupancy_L6v = new TH1F("m_h_SVD_offline_occupancy_L6v", "m_h_SVD_offline_occupancy_L6v", 100, 0, 3);
  // m_h_SVD_offline_occupancy_L6v->SetXTitle("m_h_SVD_offline_occupancy_L6v");
  m_h_muid = new TH1F("m_h_muid", "m_h_muid", 20, 0, 1);
  m_h_muid->SetXTitle("m_h_muid");
  m_h_inv_p = new TH1F("m_h_inv_p", "m_h_inv_p", 100, 8, 12);
  m_h_inv_p->SetXTitle("m_h_inv_p");
  m_h_ndf = new TH1F("m_h_ndf", "m_h_ndf", 100, 0, 80);
  m_h_ndf->SetXTitle("m_h_ndf");
  m_h_D0 = new TH1F("m_h_D0", "m_h_D0", 100, -0.1, 0.1);
  m_h_D0->SetXTitle("m_h_D0");
  m_h_Z0 = new TH1F("m_h_Z0", "m_h_Z0", 100, -0.3, 0.3);
  m_h_Z0->SetXTitle("m_h_Z0");
  m_h_theta = new TH1F("m_h_theta", "m_h_theta", 36, 10, 170);
  m_h_theta->SetXTitle("m_h_theta");
  m_h_Phi0 = new TH1F("m_h_Phi0", "m_h_Phi0", 72, -180, 180);
  m_h_Phi0->SetXTitle("m_h_Phi0");
  m_h_Pt = new TH1F("m_h_Pt", "m_h_Pt", 100, 0, 10);
  m_h_Pt->SetXTitle("m_h_Pt");
  m_h_Mom = new TH1F("m_h_Mom", "m_h_Mom", 100, 0, 10);
  m_h_Mom->SetXTitle("m_h_Mom");
  m_h_klmClusterLayers = new TH1F("m_h_klmClusterLayers", "m_h_klmClusterLayers", 16, 0, 16);
  m_h_klmClusterLayers->SetXTitle("m_h_klmClusterLayers");
  m_h_klmTotalBarrelHits = new TH1F("m_h_klmTotalBarrelHits", "m_h_klmTotalBarrelHits", 16, 0, 16);
  m_h_klmTotalBarrelHits->SetXTitle("m_h_klmTotalBarrelHits");
  m_h_klmTotalEndcapHits = new TH1F("m_h_klmTotalEndcapHits", "m_h_klmTotalEndcapHits", 16, 0, 16);
  m_h_klmTotalEndcapHits->SetXTitle("m_h_klmTotalEndcapHits");
  m_h_dPhicms = new TH1F("m_h_dPhicms", "m_h_dPhicms", 100, -1, 1);
  m_h_dPhicms->SetXTitle("m_h_dPhicms");

  oldDir->cd();
}


// PhysicsObjectsMiraBelleModule::~PhysicsObjectsMiraBelleModule()
// {
// }


void PhysicsObjectsMiraBelleModule::initialize()
{
  REG_HISTOGRAM

  StoreObjPtr<SoftwareTriggerResult> result;
  result.isRequired();
}

void PhysicsObjectsMiraBelleModule::beginRun()
{
  m_h_npxd->Reset();// ok
  m_h_nsvd->Reset();// ok
  m_h_ncdc->Reset();// ok
  m_h_topdig->Reset();// ok
  m_h_DetPhotonARICH->Reset();// ok
  m_h_klmTotalHits->Reset();// ok
  m_h_Pval->Reset();// ok
  m_h_dD0->Reset();// ok
  m_h_dZ0->Reset();// ok
  m_h_dPtcms->Reset();// ok
  m_h_nExtraCDCHits->Reset();// ok
  m_h_nECLClusters->Reset();// ok (?)
  // m_h_SVD_offline_occupancy_L3u->Reset();
  // m_h_SVD_offline_occupancy_L4u->Reset();
  // m_h_SVD_offline_occupancy_L5u->Reset();
  // m_h_SVD_offline_occupancy_L6u->Reset();
  // m_h_SVD_offline_occupancy_L3v->Reset();
  // m_h_SVD_offline_occupancy_L4v->Reset();
  // m_h_SVD_offline_occupancy_L5v->Reset();
  // m_h_SVD_offline_occupancy_L6v->Reset();
  m_h_muid->Reset();// ok
  m_h_inv_p->Reset();
  m_h_ndf->Reset();
  m_h_D0->Reset();// ok
  m_h_Z0->Reset();// ok
  m_h_theta->Reset();// ok
  m_h_Phi0->Reset();// ok
  m_h_Pt->Reset();// ok
  m_h_Mom->Reset();// ok
  m_h_klmClusterLayers->Reset();// ok
  m_h_klmTotalBarrelHits->Reset();// ok
  m_h_klmTotalEndcapHits->Reset();// ok
  m_h_dPhicms->Reset();// ok
}

void PhysicsObjectsMiraBelleModule::event()
{

  StoreObjPtr<SoftwareTriggerResult> result;
  if (!result.isValid()) {
    B2ERROR("SoftwareTriggerResult object not available but needed to select events for the histograms.");
    return;
  }

  const std::map<std::string, int>& results = result->getResults();
  if (results.find(m_triggerIdentifier) == results.end()) {
    B2ERROR("PhysicsObjectsMiraBelle: Can't find trigger identifier: " << m_triggerIdentifier);
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
  // std::cout << " # of tracks  = " << muParticles->getListSize() << std::endl;
  for (unsigned int i = 0; i < muParticles->getListSize(); i++) {
    Particle* mu = muParticles->getParticle(i);
    const Belle2::Track* track = mu->getTrack();
    if (!track) {
      continue;
    }

    // Detector hits
    m_h_npxd->Fill(Belle2::Variable::trackNPXDHits(mu));
    // std::cout << "Belle2::Variable::trackNPXDHits(mu) = " << Belle2::Variable::trackNPXDHits(mu) << std::endl;
    m_h_nsvd->Fill(Belle2::Variable::trackNSVDHits(mu));
    // std::cout << "Belle2::Variable::trackNSVDHits(mu) = " << Belle2::Variable::trackNSVDHits(mu) << std::endl;
    m_h_ncdc->Fill(Belle2::Variable::trackNCDCHits(mu));
    // std::cout << "Belle2::Variable::trackNCDCHits(mu) = " << Belle2::Variable::trackNCDCHits(mu) << std::endl;
    m_h_topdig->Fill(Belle2::Variable::TOPVariable::topDigitCount(mu));
    // std::cout << "Belle2::Variable::TOPVariable::topDigitCount(mu) = " << Belle2::Variable::TOPVariable::topDigitCount(mu) << std::endl;
    ARICHLikelihood* lkh = track->getRelated<ARICHLikelihood>();
    if (lkh) {
      m_h_DetPhotonARICH->Fill(lkh->getDetPhot());
      // std::cout << "lkh->getDetPhot() = " << lkh->getDetPhot() << std::endl;
    }

    // KLM total hits
    KLMMuidLikelihood* muid = track->getRelatedTo<KLMMuidLikelihood>();
    if (muid) {
      unsigned int bklm_hit = muid->getTotalBarrelHits();
      unsigned int eklm_hit = muid->getTotalEndcapHits();
      m_h_klmTotalBarrelHits->Fill(bklm_hit);
      // std::cout << "bklm_hit = " << bklm_hit << std::endl;
      m_h_klmTotalEndcapHits->Fill(eklm_hit);
      // std::cout << "eklm_hit = " << eklm_hit << std::endl;
      m_h_klmTotalHits->Fill(bklm_hit + eklm_hit);
      // std::cout << "bklm_hit+eklm_hit = " << bklm_hit + eklm_hit << std::endl;
    }

    // KLM Cluster layers
    KLMCluster* klmc = track->getRelated<KLMCluster>();
    if (klmc) {
      m_h_klmClusterLayers->Fill(klmc->getLayers());
      // std::cout << "klmc->getLayers() = " << klmc->getLayers() << std::endl;
    }

    // muon ID
    PIDLikelihood* pid_lkh = track->getRelated<PIDLikelihood>();
    if (pid_lkh) {
      m_h_muid->Fill(pid_lkh->getProbability(Belle2::Const::muon));
      // std::cout << "muid = pid_lkh->getProbability(Belle2::Const::muon) = " << pid_lkh->getProbability(Belle2::Const::muon) << std::endl;
    }

    // Track variables
    const TrackFitResult* fitresult = track->getTrackFitResult(Belle2::Const::pion);
    if (fitresult) {
      // Pvalue
      double pval = fitresult->getPValue();
      m_h_Pval->Fill(pval);
      // std::cout << "pval = " << pval << std::endl;
      // separate mu+ and mu-
      int index = fitresult->getChargeSign() > 0 ? 0 : 1;
      d0[index] = fitresult->getD0();
      z0[index] = fitresult->getZ0();
      m_h_D0->Fill(d0[index]);
      // std::cout << "d0[" << index << "] = " << d0[index] << std::endl;
      m_h_Z0->Fill(z0[index]);
      // std::cout << "z0[" << index << "] = " << z0[index] << std::endl;
      // Momentum
      ptcms[index] = Belle2::PCmsLabTransform::labToCms(fitresult->get4Momentum()).Pt();//CMS
      phicms[index] = Belle2::PCmsLabTransform::labToCms(fitresult->get4Momentum()).Phi();
      m_h_Pt->Fill(fitresult->get4Momentum().Pt());//Lab
      // std::cout << "fitresult->get4Momentum().Pt() = " << fitresult->get4Momentum().Pt() << std::endl;
      m_h_theta->Fill(Belle2::PCmsLabTransform::labToCms(fitresult->get4Momentum()).Theta());//CMS
      // std::cout << "Belle2::PCmsLabTransform::labToCms(fitresult->get4Momentum()).Theta() = " << Belle2::PCmsLabTransform::labToCms(fitresult->get4Momentum()).Theta() << std::endl;
      m_h_Phi0->Fill(fitresult->get4Momentum().Phi());//Lab
      // std::cout << "fitresult->get4Momentum().Phi() = " << fitresult->get4Momentum().Phi() << std::endl;
      m_h_Mom->Fill(fitresult->get4Momentum().P());//Lab
      // std::cout << "fitresult->get4Momentum().P() = " << fitresult->get4Momentum().P() << std::endl;
    }
  }
  // Resolution
  m_h_dD0->Fill((d0[0] + d0[1]) / sqrt(2));
  // std::cout << "(d0[0]+d0[1])/sqrt(2) = " << (d0[0] + d0[1]) / sqrt(2) << std::endl;
  m_h_dZ0->Fill((z0[0] - z0[1]) / sqrt(2));
  // std::cout << "(z0[0]-z0[1])/sqrt(2) = " << (z0[0] - z0[1]) / sqrt(2) << std::endl;
  m_h_dPtcms->Fill((ptcms[0] - ptcms[1]) / sqrt(2));
  // std::cout << "(ptcms[0]-ptcms[1])/sqrt(2) = " << (ptcms[0] - ptcms[1]) / sqrt(2) << std::endl;
  m_h_dPhicms->Fill(180 - abs(phicms[0] - phicms[1]));
  // std::cout << "180-abs(phicms[0]-phicms[1]) = " << 180 - abs(phicms[0] - phicms[1]) << std::endl;
  // Event level information
  StoreObjPtr<EventLevelTrackingInfo> elti;
  if (elti) {
    m_h_nExtraCDCHits->Fill(elti->getNCDCHitsNotAssigned());
    // std::cout << "elti->getNCDCHitsNotAssigned() = " << elti->getNCDCHitsNotAssigned() << std::endl;
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
  // std::cout << "neclClusters = " << neclClusters << std::endl;
}

void PhysicsObjectsMiraBelleModule::endRun()
{
}

void PhysicsObjectsMiraBelleModule::terminate()
{
}

