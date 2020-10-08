/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2020  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Shun Watanuki                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <dqm/modules/PhysicsObjectsMiraBelle/PhysicsObjectsMiraBelleDst2Module.h>
#include <analysis/dataobjects/ParticleList.h>
#include <analysis/variables/ContinuumSuppressionVariables.h>
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
#include <TDirectory.h>
#include <map>

using namespace Belle2;

REG_MODULE(PhysicsObjectsMiraBelleDst2)

PhysicsObjectsMiraBelleDst2Module::PhysicsObjectsMiraBelleDst2Module() : HistoModule()
{
  setDescription("Monitor Physics Objects Quality");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("TriggerIdentifier", m_triggerIdentifier,
           "Trigger identifier string used to select events for the histograms", std::string("software_trigger_cut&skim&accept_hadron"));
  addParam("DstListName", m_dstListName, "Name of the D*+ particle list", std::string("D*+:physMiraBelle"));
}

void PhysicsObjectsMiraBelleDst2Module::defineHisto()
{
  TDirectory* oldDir = gDirectory;
  oldDir->mkdir("PhysicsObjectsMiraBelleDst2")->cd();
  // Mass distributions
  m_h_D0_pi0_InvM = new TH1F("hist_D0_pi0_InvM", "#pi^{0} mass of D^{*}#rightarrowK#pi#pi^{0};D0_pi0_InvM;", 50, 0.09, 0.17);

  oldDir->cd();
}


void PhysicsObjectsMiraBelleDst2Module::initialize()
{
  REG_HISTOGRAM

  StoreObjPtr<SoftwareTriggerResult> result;
  result.isOptional();
}

void PhysicsObjectsMiraBelleDst2Module::beginRun()
{
  m_h_D0_pi0_InvM->Reset();
}

void PhysicsObjectsMiraBelleDst2Module::event()
{

  StoreObjPtr<SoftwareTriggerResult> result;
  if (!result.isValid()) {
    B2WARNING("SoftwareTriggerResult object not available but needed to select events for the histograms.");
    return;
  }

  const std::map<std::string, int>& results = result->getResults();
  if (results.find(m_triggerIdentifier) == results.end()) {
    B2WARNING("PhysicsObjectsMiraBelleDst2: Can't find trigger identifier: " << m_triggerIdentifier);
    return;
  }

  // apply software trigger
  const bool accepted = (result->getResult(m_triggerIdentifier) == SoftwareTriggerCutResult::c_accept);
  if (accepted == false) return;

  // get D* candidates
  StoreObjPtr<ParticleList> dstParticles(m_dstListName);

  for (unsigned int i = 0; i < dstParticles->getListSize(); i++) {
    const Particle* dst = dstParticles->getParticle(i);
    const Particle* d0 = dst->getDaughter(0);
    float dst_mass = dst->getMass();
    float d0_mass = d0->getMass();
    float delta_m = dst_mass - d0_mass;
    // Mass of Pi0
    if (1.83 < d0_mass && d0_mass < 1.89 && 0.142 < delta_m && delta_m < 0.148) {
      m_h_D0_pi0_InvM->Fill(Belle2::Variable::particleInvariantMassFromDaughters(d0->getDaughter(2)));
    }
  }
}

// void PhysicsObjectsMiraBelleDst2Module::getPIDInformationMiraBelle(Particle* part, float pid[3][7])
// {
//   PIDLikelihood* I_like_coffee = part->getPIDLikelihood();
//   pid[0][0] = I_like_coffee->getProbability();
// }

void PhysicsObjectsMiraBelleDst2Module::endRun()
{
}

void PhysicsObjectsMiraBelleDst2Module::terminate()
{
}

