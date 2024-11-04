/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//+
// File : PhysicsObjectsDQMModule.cc
// Description : Module to monitor physics objects on HLT
//-

#include <dqm/modules/PhysicsObjectsDQM/PhysicsObjectsDQMModule.h>
#include <analysis/dataobjects/ParticleList.h>
#include <analysis/variables/EventShapeVariables.h>
#include <analysis/ContinuumSuppression/FoxWolfram.h>
#include <analysis/utility/PCmsLabTransform.h>
#include <mdst/dataobjects/Track.h>
#include <framework/datastore/StoreArray.h>
#include <analysis/ClusterUtility/ClusterUtils.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/gearbox/Const.h>
#include <mdst/dataobjects/SoftwareTriggerResult.h>
#include <hlt/softwaretrigger/calculations/utilities.h>
#include <TDirectory.h>
#include <map>

using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(PhysicsObjectsDQM);

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

PhysicsObjectsDQMModule::PhysicsObjectsDQMModule() : HistoModule()
{
  //Set module properties

  setDescription("Monitor Physics Objects Quality");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("TriggerIdentifier", m_triggerIdentifier,
           "Trigger identifier string used to select events for the histograms", std::string("software_trigger_cut&skim&accept_hadron"));
  addParam("TriggerIdentifierMuMu", m_triggerIdentifierMuMu,
           "Trigger identifier string used to select events for the mumu histograms",
           std::string("software_trigger_cut&skim&accept_mumutight"));
  addParam("TriggerIdentifierBhabha", m_triggerIdentifierBhabha,
           "Trigger identifier string used to select events for the bhabha histograms",
           std::string("software_trigger_cut&skim&accept_bhabha_all"));
  addParam("TriggerIdentifierHadronb2", m_triggerIdentifierHadronb2,
           "Trigger identifier string used to select events for the hadronb2 histograms",
           std::string("software_trigger_cut&skim&accept_hadronb2"));
  addParam("PI0PListName", m_pi0PListName, "Name of the pi0 particle list", std::string("pi0:physDQM"));
  addParam("KS0PListName", m_ks0PListName, "Name of the KS0 particle list", std::string("K_S0:physDQM"));
  addParam("UpsPListName", m_upsPListName, "Name of the Ups particle list", std::string("Upsilon:physDQM"));

  addParam("UpsBhabhaPListName", m_upsBhabhaPListName, "Name of the Ups bhabha particle list", std::string("Upsilon:ephysDQM"));
  addParam("UpsHadPListName", m_hadbphysDQM, "Name of the pi hadron particle list", std::string("pi+:hadbphysDQM"));
}

void PhysicsObjectsDQMModule::defineHisto()
{
  TDirectory* oldDir = gDirectory;
  oldDir->mkdir("PhysicsObjects");
  oldDir->cd("PhysicsObjects");

  m_h_mKS0 = new TH1F("mKS0", "KS0 Invariant Mass", 20, 0.48, 0.52);
  m_h_mKS0->SetXTitle("M(K_{S}^{0}) [GeV]");

  m_h_mPI0 = new TH1F("mPI0", "pi0 Invariant Mass", 25, 0.10, 0.15);
  m_h_mPI0->SetXTitle("M(#pi^{0}) [GeV]");

  m_h_mUPS = new TH1F("mUPS", "Ups Invariant Mass", 500, 9, 12);
  m_h_mUPS->SetXTitle("M(#mu#mu) [GeV]");

  m_h_R2 = new TH1F("R2", "Event Level R2", 36, 0, 1.2);
  m_h_R2->SetXTitle("R2");

  m_h_mUPSe = new TH1F("mUPSe", "Ups ee Invariant Mass", 500, 9, 12);
  m_h_mUPSe->SetXTitle("M(ee) [GeV]");

  m_h_physicsresults = new TH1F("physicsresults", "Event physics results", 10, 0, 10);
  m_h_physicsresults->SetXTitle("Event physics results");
  m_h_physicsresults->GetXaxis()->SetBinLabel(2, "Hadron");
  m_h_physicsresults->GetXaxis()->SetBinLabel(3, "Hadronb2");
  m_h_physicsresults->GetXaxis()->SetBinLabel(4, "Hadronb2_tight");
  m_h_physicsresults->GetXaxis()->SetBinLabel(5, "mumu_tight");
  m_h_physicsresults->GetXaxis()->SetBinLabel(6, "bhabha_all");
  oldDir->cd();
}


void PhysicsObjectsDQMModule::initialize()
{
  REG_HISTOGRAM

  StoreObjPtr<SoftwareTriggerResult> result;
  result.isOptional();
}


void PhysicsObjectsDQMModule::beginRun()
{
  m_h_mKS0->Reset();
  m_h_mPI0->Reset();
  m_h_mUPS->Reset();
  m_h_R2->Reset();
  m_h_physicsresults->Reset();
}


void PhysicsObjectsDQMModule::endRun()
{
}


void PhysicsObjectsDQMModule::terminate()
{
}


void PhysicsObjectsDQMModule::event()
{
  StoreObjPtr<SoftwareTriggerResult> result;
  if (!result.isValid()) {
    B2WARNING("SoftwareTriggerResult object not available but needed to select events for the histograms.");
    return;
  }

  const std::map<std::string, int>& results = result->getResults();

  if (results.find(m_triggerIdentifier) == results.end()) {
    //Cannot find the m_triggerIdentifier, move on to mumu
    B2WARNING("PhysicsObjectsDQM: Can't find trigger identifier: " << m_triggerIdentifier);
  } else {
    const bool accepted = (result->getResult(m_triggerIdentifier) == SoftwareTriggerCutResult::c_accept);
    if (accepted != false) {

      m_h_physicsresults->Fill(1); //hadron events

      StoreObjPtr<ParticleList> pi0Particles(m_pi0PListName);
      StoreObjPtr<ParticleList> ks0Particles(m_ks0PListName);

      double R2 = Belle2::Variable::foxWolframR2(nullptr);
      m_h_R2->Fill(R2);

      if (pi0Particles.isValid() && abs(pi0Particles->getPDGCode()) == Const::pi0.getPDGCode()) {
        for (unsigned int i = 0; i < pi0Particles->getListSize(); i++) {
          Particle* pi0 = pi0Particles->getParticle(i);
          m_h_mPI0->Fill(pi0->getMass());
        }
      }
      if (ks0Particles.isValid() && abs(ks0Particles->getPDGCode()) == Const::Kshort.getPDGCode()) {
        for (unsigned int i = 0; i < ks0Particles->getListSize(); i++) {
          Particle* ks0 = ks0Particles->getParticle(i);
          m_h_mKS0->Fill(ks0->getMass());
        }
      }
    }
  }
  if (results.find(m_triggerIdentifierMuMu) == results.end()) {
    //Cannot find the m_triggerIdentifierMuMu, stop now
    B2WARNING("PhysicsObjectsDQM: Can't find trigger identifier: " << m_triggerIdentifierMuMu);
    return;
  } else {
    const bool accepted = (result->getResult(m_triggerIdentifierMuMu) == SoftwareTriggerCutResult::c_accept);
    if (accepted != false) {
      m_h_physicsresults->Fill(4); //mumu events
      StoreObjPtr<ParticleList> UpsParticles(m_upsPListName);
      if (UpsParticles.isValid()) {
        for (unsigned int i = 0; i < UpsParticles->getListSize(); i++) {
          Particle* Ups = UpsParticles->getParticle(i);
          m_h_mUPS->Fill(Ups->getMass());
        }
      }
    }
  }
  //bhabha, hadrons
  if (results.find(m_triggerIdentifierBhabha) == results.end()) {
    //Cannot find the m_triggerIdentifierBhahba, stop now
    B2WARNING("PhysicsObjectsDQM: Can't find trigger identifier: " << m_triggerIdentifierBhabha);
    return;
  } else {
    const bool accepted = (result->getResult(m_triggerIdentifierBhabha) == SoftwareTriggerCutResult::c_accept);
    if (accepted != false) {
      m_h_physicsresults->Fill(5); //bhabha events
      StoreObjPtr<ParticleList> UpsbhabhaParticles(m_upsBhabhaPListName);
      if (UpsbhabhaParticles.isValid()) {
        for (unsigned int i = 0; i < UpsbhabhaParticles->getListSize(); i++) {
          Particle* Upse = UpsbhabhaParticles->getParticle(i);
          m_h_mUPSe->Fill(Upse->getMass());
        }
      }
    }
  }

  if (results.find(m_triggerIdentifierHadronb2) == results.end()) {
    //Cannot find the m_triggerIdentifierHadronb2, stop now
    B2WARNING("PhysicsObjectsDQM: Can't find trigger identifier: " << m_triggerIdentifierHadronb2);
    return;
  } else {
    const bool accepted = (result->getResult(m_triggerIdentifierHadronb2) == SoftwareTriggerCutResult::c_accept);
    if (accepted != false) {

      m_h_physicsresults->Fill(2); //hadronb2 events

      //variables for hadronb2 tight
      StoreObjPtr<ParticleList> hadpiParticles(m_hadbphysDQM);
      std::vector<ROOT::Math::XYZVector> m_pionHadv3;
      double EsumPiHad = 0.;
      for (unsigned int i = 0; i < hadpiParticles->getListSize(); i++) {
        const Particle* parPiHad = hadpiParticles->getParticle(i);
        ROOT::Math::PxPyPzEVector V4PiHad = PCmsLabTransform::labToCms(parPiHad->get4Vector());
        m_pionHadv3.push_back(parPiHad->getMomentum());
        EsumPiHad += V4PiHad.E();
      }
      //EnECLClustersLE
      double eneclClusters = 0.;
      StoreArray<ECLCluster> eclClusters;
      ClusterUtils Cl;
      double EsumGamma = 0.;
      if (eclClusters.isValid()) {
        for (int ncl = 0; ncl < eclClusters.getEntries(); ncl++) {
          if (eclClusters[ncl]->hasHypothesis(ECLCluster::EHypothesisBit::c_nPhotons)
              && eclClusters[ncl]->getEnergy(ECLCluster::EHypothesisBit::c_nPhotons) > 0.1) {
            eneclClusters += eclClusters[ncl]->getEnergy(ECLCluster::EHypothesisBit::c_nPhotons);
            if (!eclClusters[ncl]->getRelatedFrom<Track>()) {
              ROOT::Math::PxPyPzEVector V4Gamma_CMS = PCmsLabTransform::labToCms(Cl.Get4MomentumFromCluster(eclClusters[ncl],
                                                      ECLCluster::EHypothesisBit::c_nPhotons));
              EsumGamma += V4Gamma_CMS.E();
            }
          }
        }
      }
      double visibleEnergyCMSnorm = (EsumPiHad + EsumGamma) / (Belle2::SoftwareTrigger::BeamEnergyCMS() * 2.0);
      double EsumCMSnorm = eneclClusters / (Belle2::SoftwareTrigger::BeamEnergyCMS() * 2.0);
      FoxWolfram fw(m_pionHadv3);
      fw.calculateBasicMoments();
      double R2 = fw.getR(2);
      bool hadronb_tag = visibleEnergyCMSnorm > 0.4 && EsumCMSnorm > 0.2 && R2 < 0.2;
      if (hadronb_tag) {
        m_h_physicsresults->Fill(3); //hadronb2 tight events
      }

    }
  }



}
