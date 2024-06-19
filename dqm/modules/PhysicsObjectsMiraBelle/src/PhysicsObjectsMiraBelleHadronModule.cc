/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <dqm/modules/PhysicsObjectsMiraBelle/PhysicsObjectsMiraBelleHadronModule.h>
#include <hlt/softwaretrigger/calculations/utilities.h>
#include <analysis/dataobjects/ParticleList.h>
#include <analysis/variables/ContinuumSuppressionVariables.h>
#include <analysis/ContinuumSuppression/FoxWolfram.h>
#include <analysis/variables/TrackVariables.h>
#include <analysis/utility/PCmsLabTransform.h>
#include <analysis/variables/Variables.h>
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
#include <mdst/dataobjects/SoftwareTriggerResult.h>
#include <analysis/ClusterUtility/ClusterUtils.h>
#include <TDirectory.h>
#include <map>

using namespace Belle2;

REG_MODULE(PhysicsObjectsMiraBelleHadron);

PhysicsObjectsMiraBelleHadronModule::PhysicsObjectsMiraBelleHadronModule() : HistoModule()
{
  setDescription("Monitor Physics Objects Quality");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("TriggerIdentifier", m_triggerIdentifier,
           "Trigger identifier string used to select events for the histograms", std::string("software_trigger_cut&skim&accept_hadronb2"));
  addParam("hadronb2piPListName", m_hadpiPListName, "Name of the pi+ particle list", std::string("pi+:hadb2physMiraBelle"));
}

void PhysicsObjectsMiraBelleHadronModule::defineHisto()
{
  TDirectory* oldDir = gDirectory;
  oldDir->mkdir("PhysicsObjectsMiraBelleHadron");
  oldDir->cd("PhysicsObjectsMiraBelleHadron");

  // Mass distributions
  m_h_nECLClusters = new TH1F("hist_nECLClusters", "hist_nECLClusters", 100, 0, 60);
  m_h_nECLClusters->SetXTitle("hist_nECLClusters");
  m_h_visibleEnergyCMSnorm = new TH1F("hist_visibleEnergyCMSnorm", "hist_visibleEnergyCMSnorm", 100, 0, 2);
  m_h_visibleEnergyCMSnorm->SetXTitle("hist_visibleEnergyCMSnorm");
  m_h_EsumCMSnorm = new TH1F("hist_EsumCMSnorm", "hist_EsumCMSnorm", 100, 0, 2);
  m_h_EsumCMSnorm->SetXTitle("hist_EsumCMSnorm");
  m_h_R2 = new TH1F("hist_R2", "hist_R2", 100, 0, 1);
  m_h_R2->SetXTitle("hist_R2");
  m_h_physicsresultsH = new TH1F("hist_physicsresultsH", "hist_physicsresultsH", 10, 0, 10);
  m_h_physicsresultsH->SetXTitle("hist_physicsresultsH");
  m_h_physicsresultsH->GetXaxis()->SetBinLabel(2, "Hadronb2");
  m_h_physicsresultsH->GetXaxis()->SetBinLabel(3, "Hadronb2_tight");

  oldDir->cd();
}


void PhysicsObjectsMiraBelleHadronModule::initialize()
{
  REG_HISTOGRAM

  StoreObjPtr<SoftwareTriggerResult> result;
  result.isOptional();
}

void PhysicsObjectsMiraBelleHadronModule::beginRun()
{

  m_h_nECLClusters->Reset();
  m_h_visibleEnergyCMSnorm->Reset();
  m_h_EsumCMSnorm->Reset();
  m_h_R2->Reset();
  m_h_physicsresultsH->Reset();
}

void PhysicsObjectsMiraBelleHadronModule::event()
{

  StoreObjPtr<SoftwareTriggerResult> result;
  if (!result.isValid()) {
    B2WARNING("SoftwareTriggerResult object not available but needed to select events for the histograms.");
    return;
  }

  const std::map<std::string, int>& results = result->getResults();
  if (results.find(m_triggerIdentifier) == results.end()) {
    B2WARNING("PhysicsObjectsMiraBelleHadron: Can't find trigger identifier: " << m_triggerIdentifier);
    return;
  }

  // apply software trigger
  const bool accepted = (result->getResult(m_triggerIdentifier) == SoftwareTriggerCutResult::c_accept);
  if (accepted == false) return;
  m_h_physicsresultsH->Fill(1);
  // get pi list
  StoreObjPtr<ParticleList> hadpiParticles(m_hadpiPListName);
  std::vector<ROOT::Math::XYZVector> m_pionHadv3;

  double EsumPiHad = 0.;

  for (unsigned int i = 0; i < hadpiParticles->getListSize(); i++) {
    const Particle* parPiHad = hadpiParticles->getParticle(i);
    ROOT::Math::PxPyPzEVector V4PiHad = PCmsLabTransform::labToCms(parPiHad->get4Vector());
    m_pionHadv3.push_back(parPiHad->getMomentum());
    EsumPiHad += V4PiHad.E();

  }
  //nECLClustersLE
  double neclClusters = -1.;
  double eneclClusters = 0.;
  StoreArray<ECLCluster> eclClusters;
  ClusterUtils Cl;
  double EsumGamma = 0.;

  if (eclClusters.isValid()) {
    const unsigned int numberOfECLClusters = std::count_if(eclClusters.begin(), eclClusters.end(),
    [](const ECLCluster & eclcluster) {
      return (eclcluster.hasHypothesis(ECLCluster::EHypothesisBit::c_nPhotons)
              and eclcluster.getEnergy(ECLCluster::EHypothesisBit::c_nPhotons) > 0.1);
    });
    neclClusters = numberOfECLClusters;
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

  m_h_nECLClusters->Fill(neclClusters);
  m_h_visibleEnergyCMSnorm->Fill(visibleEnergyCMSnorm);
  m_h_EsumCMSnorm->Fill(EsumCMSnorm);
  m_h_R2->Fill(R2);
  bool hadronb_tag = visibleEnergyCMSnorm > 0.4 && EsumCMSnorm > 0.2 && R2 < 0.2;
  if (hadronb_tag) {
    m_h_physicsresultsH->Fill(2);
  }

}



void PhysicsObjectsMiraBelleHadronModule::endRun()
{
}

void PhysicsObjectsMiraBelleHadronModule::terminate()
{
}

