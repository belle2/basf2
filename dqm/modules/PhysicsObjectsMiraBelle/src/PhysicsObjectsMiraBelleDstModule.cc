/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <dqm/modules/PhysicsObjectsMiraBelle/PhysicsObjectsMiraBelleDstModule.h>
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

REG_MODULE(PhysicsObjectsMiraBelleDst)

PhysicsObjectsMiraBelleDstModule::PhysicsObjectsMiraBelleDstModule() : HistoModule()
{
  setDescription("Monitor Physics Objects Quality");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("TriggerIdentifier", m_triggerIdentifier,
           "Trigger identifier string used to select events for the histograms", std::string("software_trigger_cut&skim&accept_hadron"));
  addParam("DstListName", m_dstListName, "Name of the D*+ particle list", std::string("D*+:physMiraBelle"));
}

void PhysicsObjectsMiraBelleDstModule::defineHisto()
{
  TDirectory* oldDir = gDirectory;
  oldDir->mkdir("PhysicsObjectsMiraBelleDst")->cd();
  // Mass distributions
  m_h_D0_InvM = new TH1F("hist_D0_InvM", "Signal enhanced;m_h_D0_InvM;", 50, 1.81, 1.95);
  m_h_delta_m = new TH1F("hist_delta_m", "Signal enhanced;delta_m;", 50, 0.14, 0.16);
  // Soft pion
  m_h_D0_softpi_PID_ALL_pion = new TH1F("hist_D0_softpi_PID_ALL_pion", "D0_softpi_PID_ALL_pion;D0_softpi_PID_ALL_pion;", 50, 0, 1);
  m_h_D0_softpi_PID_SVD_pion = new TH1F("hist_D0_softpi_PID_SVD_pion", "D0_softpi_PID_SVD_pion;D0_softpi_PID_SVD_pion;", 50, 0, 1);
  m_h_D0_softpi_PID_CDC_pion = new TH1F("hist_D0_softpi_PID_CDC_pion", "D0_softpi_PID_CDC_pion;D0_softpi_PID_CDC_pion;", 50, 0, 1);
  m_h_D0_softpi_PID_TOP_pion = new TH1F("hist_D0_softpi_PID_TOP_pion", "D0_softpi_PID_TOP_pion;D0_softpi_PID_TOP_pion;", 50, 0, 1);
  m_h_D0_softpi_PID_ARICH_pion = new TH1F("hist_D0_softpi_PID_ARICH_pion", "D0_softpi_PID_ARICH_pion;D0_softpi_PID_ARICH_pion;", 50,
                                          0,
                                          1);
  m_h_D0_softpi_PID_ECL_pion = new TH1F("hist_D0_softpi_PID_ECL_pion", "D0_softpi_PID_ECL_pion;D0_softpi_PID_ECL_pion;", 50, 0, 1);
  m_h_D0_softpi_PID_KLM_pion = new TH1F("hist_D0_softpi_PID_KLM_pion", "D0_softpi_PID_KLM_pion;D0_softpi_PID_KLM_pion;", 50, 0, 1);
  m_h_sideband_D0_softpi_PID_ALL_pion = new TH1F("hist_sideband_D0_softpi_PID_ALL_pion",
                                                 "D0_softpi_PID_ALL_pion;D0_softpi_PID_ALL_pion;", 50, 0, 1);
  m_h_sideband_D0_softpi_PID_SVD_pion = new TH1F("hist_sideband_D0_softpi_PID_SVD_pion",
                                                 "D0_softpi_PID_SVD_pion;D0_softpi_PID_SVD_pion;", 50, 0, 1);
  m_h_sideband_D0_softpi_PID_CDC_pion = new TH1F("hist_sideband_D0_softpi_PID_CDC_pion",
                                                 "D0_softpi_PID_CDC_pion;D0_softpi_PID_CDC_pion;", 50, 0, 1);
  m_h_sideband_D0_softpi_PID_TOP_pion = new TH1F("hist_sideband_D0_softpi_PID_TOP_pion",
                                                 "D0_softpi_PID_TOP_pion;D0_softpi_PID_TOP_pion;", 50, 0, 1);
  m_h_sideband_D0_softpi_PID_ARICH_pion = new TH1F("hist_sideband_D0_softpi_PID_ARICH_pion",
                                                   "D0_softpi_PID_ARICH_pion;D0_softpi_PID_ARICH_pion;", 50, 0, 1);
  m_h_sideband_D0_softpi_PID_ECL_pion = new TH1F("hist_sideband_D0_softpi_PID_ECL_pion",
                                                 "D0_softpi_PID_ECL_pion;D0_softpi_PID_ECL_pion;", 50, 0, 1);
  m_h_sideband_D0_softpi_PID_KLM_pion = new TH1F("hist_sideband_D0_softpi_PID_KLM_pion",
                                                 "D0_softpi_PID_KLM_pion;D0_softpi_PID_KLM_pion;", 50, 0, 1);
  // pi from D0
  m_h_D0_pi_PID_ALL_pion = new TH1F("hist_D0_pi_PID_ALL_pion", "D0_pi_PID_ALL_pion;D0_pi_PID_ALL_pion;", 50, 0, 1);
  m_h_D0_pi_PID_SVD_pion = new TH1F("hist_D0_pi_PID_SVD_pion", "D0_pi_PID_SVD_pion;D0_pi_PID_SVD_pion;", 50, 0, 1);
  m_h_D0_pi_PID_CDC_pion = new TH1F("hist_D0_pi_PID_CDC_pion", "D0_pi_PID_CDC_pion;D0_pi_PID_CDC_pion;", 50, 0, 1);
  m_h_D0_pi_PID_TOP_pion = new TH1F("hist_D0_pi_PID_TOP_pion", "D0_pi_PID_TOP_pion;D0_pi_PID_TOP_pion;", 50, 0, 1);
  m_h_D0_pi_PID_ARICH_pion = new TH1F("hist_D0_pi_PID_ARICH_pion", "D0_pi_PID_ARICH_pion;D0_pi_PID_ARICH_pion;", 50, 0, 1);
  m_h_D0_pi_PID_ECL_pion = new TH1F("hist_D0_pi_PID_ECL_pion", "D0_pi_PID_ECL_pion;D0_pi_PID_ECL_pion;", 50, 0, 1);
  m_h_D0_pi_PID_KLM_pion = new TH1F("hist_D0_pi_PID_KLM_pion", "D0_pi_PID_KLM_pion;D0_pi_PID_KLM_pion;", 50, 0, 1);
  m_h_sideband_D0_pi_PID_ALL_pion = new TH1F("hist_sideband_D0_pi_PID_ALL_pion", "D0_pi_PID_ALL_pion;D0_pi_PID_ALL_pion;", 50, 0, 1);
  m_h_sideband_D0_pi_PID_SVD_pion = new TH1F("hist_sideband_D0_pi_PID_SVD_pion", "D0_pi_PID_SVD_pion;D0_pi_PID_SVD_pion;", 50, 0, 1);
  m_h_sideband_D0_pi_PID_CDC_pion = new TH1F("hist_sideband_D0_pi_PID_CDC_pion", "D0_pi_PID_CDC_pion;D0_pi_PID_CDC_pion;", 50, 0, 1);
  m_h_sideband_D0_pi_PID_TOP_pion = new TH1F("hist_sideband_D0_pi_PID_TOP_pion", "D0_pi_PID_TOP_pion;D0_pi_PID_TOP_pion;", 50, 0, 1);
  m_h_sideband_D0_pi_PID_ARICH_pion = new TH1F("hist_sideband_D0_pi_PID_ARICH_pion", "D0_pi_PID_ARICH_pion;D0_pi_PID_ARICH_pion;", 50,
                                               0, 1);
  m_h_sideband_D0_pi_PID_ECL_pion = new TH1F("hist_sideband_D0_pi_PID_ECL_pion", "D0_pi_PID_ECL_pion;D0_pi_PID_ECL_pion;", 50, 0, 1);
  m_h_sideband_D0_pi_PID_KLM_pion = new TH1F("hist_sideband_D0_pi_PID_KLM_pion", "D0_pi_PID_KLM_pion;D0_pi_PID_KLM_pion;", 50, 0, 1);
  // K from D0
  m_h_D0_K_PID_ALL_kaon = new TH1F("hist_D0_K_PID_ALL_kaon", "D0_K_PID_ALL_kaon;D0_K_PID_ALL_kaon;", 50, 0, 1);
  m_h_D0_K_PID_SVD_kaon = new TH1F("hist_D0_K_PID_SVD_kaon", "D0_K_PID_SVD_kaon;D0_K_PID_SVD_kaon;", 50, 0, 1);
  m_h_D0_K_PID_CDC_kaon = new TH1F("hist_D0_K_PID_CDC_kaon", "D0_K_PID_CDC_kaon;D0_K_PID_CDC_kaon;", 50, 0, 1);
  m_h_D0_K_PID_TOP_kaon = new TH1F("hist_D0_K_PID_TOP_kaon", "D0_K_PID_TOP_kaon;D0_K_PID_TOP_kaon;", 50, 0, 1);
  m_h_D0_K_PID_ARICH_kaon = new TH1F("hist_D0_K_PID_ARICH_kaon", "D0_K_PID_ARICH_kaon;D0_K_PID_ARICH_kaon;", 50, 0, 1);
  m_h_D0_K_PID_ECL_kaon = new TH1F("hist_D0_K_PID_ECL_kaon", "D0_K_PID_ECL_kaon;D0_K_PID_ECL_kaon;", 50, 0, 1);
  m_h_D0_K_PID_KLM_kaon = new TH1F("hist_D0_K_PID_KLM_kaon", "D0_K_PID_KLM_kaon;D0_K_PID_KLM_kaon;", 50, 0, 1);
  m_h_sideband_D0_K_PID_ALL_kaon = new TH1F("hist_sideband_D0_K_PID_ALL_kaon", "D0_K_PID_ALL_kaon;D0_K_PID_ALL_kaon;", 50, 0, 1);
  m_h_sideband_D0_K_PID_SVD_kaon = new TH1F("hist_sideband_D0_K_PID_SVD_kaon", "D0_K_PID_SVD_kaon;D0_K_PID_SVD_kaon;", 50, 0, 1);
  m_h_sideband_D0_K_PID_CDC_kaon = new TH1F("hist_sideband_D0_K_PID_CDC_kaon", "D0_K_PID_CDC_kaon;D0_K_PID_CDC_kaon;", 50, 0, 1);
  m_h_sideband_D0_K_PID_TOP_kaon = new TH1F("hist_sideband_D0_K_PID_TOP_kaon", "D0_K_PID_TOP_kaon;D0_K_PID_TOP_kaon;", 50, 0, 1);
  m_h_sideband_D0_K_PID_ARICH_kaon = new TH1F("hist_sideband_D0_K_PID_ARICH_kaon", "D0_K_PID_ARICH_kaon;D0_K_PID_ARICH_kaon;", 50, 0,
                                              1);
  m_h_sideband_D0_K_PID_ECL_kaon = new TH1F("hist_sideband_D0_K_PID_ECL_kaon", "D0_K_PID_ECL_kaon;D0_K_PID_ECL_kaon;", 50, 0, 1);
  m_h_sideband_D0_K_PID_KLM_kaon = new TH1F("hist_sideband_D0_K_PID_KLM_kaon", "D0_K_PID_KLM_kaon;D0_K_PID_KLM_kaon;", 50, 0, 1);

  oldDir->cd();
}


void PhysicsObjectsMiraBelleDstModule::initialize()
{
  REG_HISTOGRAM

  StoreObjPtr<SoftwareTriggerResult> result;
  result.isOptional();
}

void PhysicsObjectsMiraBelleDstModule::beginRun()
{
  m_h_D0_InvM->Reset();
  m_h_delta_m->Reset();
  m_h_D0_softpi_PID_ALL_pion->Reset();
  m_h_D0_softpi_PID_SVD_pion->Reset();
  m_h_D0_softpi_PID_CDC_pion->Reset();
  m_h_D0_softpi_PID_TOP_pion->Reset();
  m_h_D0_softpi_PID_ARICH_pion->Reset();
  m_h_D0_softpi_PID_ECL_pion->Reset();
  m_h_D0_softpi_PID_KLM_pion->Reset();
  m_h_D0_pi_PID_ALL_pion->Reset();
  m_h_D0_pi_PID_SVD_pion->Reset();
  m_h_D0_pi_PID_CDC_pion->Reset();
  m_h_D0_pi_PID_TOP_pion->Reset();
  m_h_D0_pi_PID_ARICH_pion->Reset();
  m_h_D0_pi_PID_ECL_pion->Reset();
  m_h_D0_pi_PID_KLM_pion->Reset();
  m_h_D0_K_PID_ALL_kaon->Reset();
  m_h_D0_K_PID_SVD_kaon->Reset();
  m_h_D0_K_PID_CDC_kaon->Reset();
  m_h_D0_K_PID_TOP_kaon->Reset();
  m_h_D0_K_PID_ARICH_kaon->Reset();
  m_h_D0_K_PID_ECL_kaon->Reset();
  m_h_D0_K_PID_KLM_kaon->Reset();
  m_h_sideband_D0_softpi_PID_ALL_pion->Reset();
  m_h_sideband_D0_softpi_PID_SVD_pion->Reset();
  m_h_sideband_D0_softpi_PID_CDC_pion->Reset();
  m_h_sideband_D0_softpi_PID_TOP_pion->Reset();
  m_h_sideband_D0_softpi_PID_ARICH_pion->Reset();
  m_h_sideband_D0_softpi_PID_ECL_pion->Reset();
  m_h_sideband_D0_softpi_PID_KLM_pion->Reset();
  m_h_sideband_D0_pi_PID_ALL_pion->Reset();
  m_h_sideband_D0_pi_PID_SVD_pion->Reset();
  m_h_sideband_D0_pi_PID_CDC_pion->Reset();
  m_h_sideband_D0_pi_PID_TOP_pion->Reset();
  m_h_sideband_D0_pi_PID_ARICH_pion->Reset();
  m_h_sideband_D0_pi_PID_ECL_pion->Reset();
  m_h_sideband_D0_pi_PID_KLM_pion->Reset();
  m_h_sideband_D0_K_PID_ALL_kaon->Reset();
  m_h_sideband_D0_K_PID_SVD_kaon->Reset();
  m_h_sideband_D0_K_PID_CDC_kaon->Reset();
  m_h_sideband_D0_K_PID_TOP_kaon->Reset();
  m_h_sideband_D0_K_PID_ARICH_kaon->Reset();
  m_h_sideband_D0_K_PID_ECL_kaon->Reset();
  m_h_sideband_D0_K_PID_KLM_kaon->Reset();
}

void PhysicsObjectsMiraBelleDstModule::event()
{

  StoreObjPtr<SoftwareTriggerResult> result;
  if (!result.isValid()) {
    B2WARNING("SoftwareTriggerResult object not available but needed to select events for the histograms.");
    return;
  }

  const std::map<std::string, int>& results = result->getResults();
  if (results.find(m_triggerIdentifier) == results.end()) {
    B2WARNING("PhysicsObjectsMiraBelleDst: Can't find trigger identifier: " << m_triggerIdentifier);
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
    if (delta_m < 0.14 || 0.16 < delta_m || d0_mass < 1.81 || 1.95 < d0_mass) continue;
    // True if the event is in signal region
    bool isSignal_D0_InvM(false), isSignal_delta_m(false);
    // D0 and D*+ mass
    if (0.143 < delta_m && delta_m < 0.147) {
      m_h_D0_InvM->Fill(d0_mass);
      isSignal_D0_InvM = true;
    }
    if (1.83 < d0_mass && d0_mass < 1.89) {
      m_h_delta_m->Fill(delta_m);
      isSignal_delta_m = true;
    }
    const PIDLikelihood* pid_K = d0->getDaughter(0)->getPIDLikelihood();
    const PIDLikelihood* pid_Pi = d0->getDaughter(1)->getPIDLikelihood();
    const PIDLikelihood* pid_softPi = dst->getDaughter(1)->getPIDLikelihood();

    if (pid_K != NULL && pid_Pi != NULL && pid_softPi != NULL) {
      if (isSignal_D0_InvM && isSignal_delta_m) {
        // Signal region
        // PID of K
        m_h_D0_K_PID_ALL_kaon->Fill(pid_K->getProbability(Belle2::Const::kaon, Belle2::Const::pion, 1.));
        if (pid_K->isAvailable(Const::SVD))   m_h_D0_K_PID_SVD_kaon   ->Fill(pid_K->getProbability(Belle2::Const::kaon, Belle2::Const::pion,
              1., Const::SVD));
        if (pid_K->isAvailable(Const::CDC))   m_h_D0_K_PID_CDC_kaon   ->Fill(pid_K->getProbability(Belle2::Const::kaon, Belle2::Const::pion,
              1., Const::CDC));
        if (pid_K->isAvailable(Const::TOP))   m_h_D0_K_PID_TOP_kaon   ->Fill(pid_K->getProbability(Belle2::Const::kaon, Belle2::Const::pion,
              1., Const::TOP));
        if (pid_K->isAvailable(Const::ARICH)) m_h_D0_K_PID_ARICH_kaon ->Fill(pid_K->getProbability(Belle2::Const::kaon, Belle2::Const::pion,
              1., Const::ARICH));
        if (pid_K->isAvailable(Const::ECL))   m_h_D0_K_PID_ECL_kaon   ->Fill(pid_K->getProbability(Belle2::Const::kaon, Belle2::Const::pion,
              1., Const::ECL));
        if (pid_K->isAvailable(Const::KLM))   m_h_D0_K_PID_KLM_kaon   ->Fill(pid_K->getProbability(Belle2::Const::kaon, Belle2::Const::pion,
              1., Const::KLM));
        // PID of pi
        m_h_D0_pi_PID_ALL_pion->Fill(pid_Pi->getProbability(Belle2::Const::pion, Belle2::Const::kaon, 1.));
        if (pid_Pi->isAvailable(Const::SVD))   m_h_D0_pi_PID_SVD_pion   ->Fill(pid_Pi->getProbability(Belle2::Const::pion,
              Belle2::Const::kaon, 1., Const::SVD));
        if (pid_Pi->isAvailable(Const::CDC))   m_h_D0_pi_PID_CDC_pion   ->Fill(pid_Pi->getProbability(Belle2::Const::pion,
              Belle2::Const::kaon, 1., Const::CDC));
        if (pid_Pi->isAvailable(Const::TOP))   m_h_D0_pi_PID_TOP_pion   ->Fill(pid_Pi->getProbability(Belle2::Const::pion,
              Belle2::Const::kaon, 1., Const::TOP));
        if (pid_Pi->isAvailable(Const::ARICH)) m_h_D0_pi_PID_ARICH_pion ->Fill(pid_Pi->getProbability(Belle2::Const::pion,
              Belle2::Const::kaon, 1., Const::ARICH));
        if (pid_Pi->isAvailable(Const::ECL))   m_h_D0_pi_PID_ECL_pion   ->Fill(pid_Pi->getProbability(Belle2::Const::pion,
              Belle2::Const::kaon, 1., Const::ECL));
        if (pid_Pi->isAvailable(Const::KLM))   m_h_D0_pi_PID_KLM_pion   ->Fill(pid_Pi->getProbability(Belle2::Const::pion,
              Belle2::Const::kaon, 1., Const::KLM));
        // PID of soft pi
        m_h_D0_softpi_PID_ALL_pion->Fill(pid_softPi->getProbability(Belle2::Const::pion, Belle2::Const::kaon, 1.));
        if (pid_softPi->isAvailable(Const::SVD))   m_h_D0_softpi_PID_SVD_pion   ->Fill(pid_softPi->getProbability(Belle2::Const::pion,
              Belle2::Const::kaon, 1., Const::SVD));
        if (pid_softPi->isAvailable(Const::CDC))   m_h_D0_softpi_PID_CDC_pion   ->Fill(pid_softPi->getProbability(Belle2::Const::pion,
              Belle2::Const::kaon, 1., Const::CDC));
        if (pid_softPi->isAvailable(Const::TOP))   m_h_D0_softpi_PID_TOP_pion   ->Fill(pid_softPi->getProbability(Belle2::Const::pion,
              Belle2::Const::kaon, 1., Const::TOP));
        if (pid_softPi->isAvailable(Const::ARICH)) m_h_D0_softpi_PID_ARICH_pion ->Fill(pid_softPi->getProbability(Belle2::Const::pion,
              Belle2::Const::kaon, 1., Const::ARICH));
        if (pid_softPi->isAvailable(Const::ECL))   m_h_D0_softpi_PID_ECL_pion   ->Fill(pid_softPi->getProbability(Belle2::Const::pion,
              Belle2::Const::kaon, 1., Const::ECL));
        if (pid_softPi->isAvailable(Const::KLM))   m_h_D0_softpi_PID_KLM_pion   ->Fill(pid_softPi->getProbability(Belle2::Const::pion,
              Belle2::Const::kaon, 1., Const::KLM));
      } else {
        // Sideband region for BG subtraction
        // PID of K
        m_h_sideband_D0_K_PID_ALL_kaon->Fill(pid_K->getProbability(Belle2::Const::kaon, Belle2::Const::pion, 1.));
        if (pid_K->isAvailable(Const::SVD))   m_h_sideband_D0_K_PID_SVD_kaon   ->Fill(pid_K->getProbability(Belle2::Const::kaon,
              Belle2::Const::pion, 1., Const::SVD));
        if (pid_K->isAvailable(Const::CDC))   m_h_sideband_D0_K_PID_CDC_kaon   ->Fill(pid_K->getProbability(Belle2::Const::kaon,
              Belle2::Const::pion, 1., Const::CDC));
        if (pid_K->isAvailable(Const::TOP))   m_h_sideband_D0_K_PID_TOP_kaon   ->Fill(pid_K->getProbability(Belle2::Const::kaon,
              Belle2::Const::pion, 1., Const::TOP));
        if (pid_K->isAvailable(Const::ARICH)) m_h_sideband_D0_K_PID_ARICH_kaon ->Fill(pid_K->getProbability(Belle2::Const::kaon,
              Belle2::Const::pion, 1., Const::ARICH));
        if (pid_K->isAvailable(Const::ECL))   m_h_sideband_D0_K_PID_ECL_kaon   ->Fill(pid_K->getProbability(Belle2::Const::kaon,
              Belle2::Const::pion, 1., Const::ECL));
        if (pid_K->isAvailable(Const::KLM))   m_h_sideband_D0_K_PID_KLM_kaon   ->Fill(pid_K->getProbability(Belle2::Const::kaon,
              Belle2::Const::pion, 1., Const::KLM));
        // PID of pi
        m_h_sideband_D0_pi_PID_ALL_pion->Fill(pid_Pi->getProbability(Belle2::Const::pion, Belle2::Const::kaon, 1.));
        if (pid_Pi->isAvailable(Const::SVD))   m_h_sideband_D0_pi_PID_SVD_pion   ->Fill(pid_Pi->getProbability(Belle2::Const::pion,
              Belle2::Const::kaon, 1., Const::SVD));
        if (pid_Pi->isAvailable(Const::CDC))   m_h_sideband_D0_pi_PID_CDC_pion   ->Fill(pid_Pi->getProbability(Belle2::Const::pion,
              Belle2::Const::kaon, 1., Const::CDC));
        if (pid_Pi->isAvailable(Const::TOP))   m_h_sideband_D0_pi_PID_TOP_pion   ->Fill(pid_Pi->getProbability(Belle2::Const::pion,
              Belle2::Const::kaon, 1., Const::TOP));
        if (pid_Pi->isAvailable(Const::ARICH)) m_h_sideband_D0_pi_PID_ARICH_pion ->Fill(pid_Pi->getProbability(Belle2::Const::pion,
              Belle2::Const::kaon, 1., Const::ARICH));
        if (pid_Pi->isAvailable(Const::ECL))   m_h_sideband_D0_pi_PID_ECL_pion   ->Fill(pid_Pi->getProbability(Belle2::Const::pion,
              Belle2::Const::kaon, 1., Const::ECL));
        if (pid_Pi->isAvailable(Const::KLM))   m_h_sideband_D0_pi_PID_KLM_pion   ->Fill(pid_Pi->getProbability(Belle2::Const::pion,
              Belle2::Const::kaon, 1., Const::KLM));
        // PID of soft pi
        m_h_sideband_D0_softpi_PID_ALL_pion->Fill(pid_softPi->getProbability(Belle2::Const::pion, Belle2::Const::kaon, 1.));
        if (pid_softPi->isAvailable(Const::SVD))   m_h_sideband_D0_softpi_PID_SVD_pion   ->Fill(pid_softPi->getProbability(
                Belle2::Const::pion, Belle2::Const::kaon, 1., Const::SVD));
        if (pid_softPi->isAvailable(Const::CDC))   m_h_sideband_D0_softpi_PID_CDC_pion   ->Fill(pid_softPi->getProbability(
                Belle2::Const::pion, Belle2::Const::kaon, 1., Const::CDC));
        if (pid_softPi->isAvailable(Const::TOP))   m_h_sideband_D0_softpi_PID_TOP_pion   ->Fill(pid_softPi->getProbability(
                Belle2::Const::pion, Belle2::Const::kaon, 1., Const::TOP));
        if (pid_softPi->isAvailable(Const::ARICH)) m_h_sideband_D0_softpi_PID_ARICH_pion ->Fill(pid_softPi->getProbability(
                Belle2::Const::pion, Belle2::Const::kaon, 1., Const::ARICH));
        if (pid_softPi->isAvailable(Const::ECL))   m_h_sideband_D0_softpi_PID_ECL_pion   ->Fill(pid_softPi->getProbability(
                Belle2::Const::pion, Belle2::Const::kaon, 1., Const::ECL));
        if (pid_softPi->isAvailable(Const::KLM))   m_h_sideband_D0_softpi_PID_KLM_pion   ->Fill(pid_softPi->getProbability(
                Belle2::Const::pion, Belle2::Const::kaon, 1., Const::KLM));
      }
    }
  }
}

// void PhysicsObjectsMiraBelleDstModule::getPIDInformationMiraBelle(Particle* part, float pid[3][7])
// {
//   PIDLikelihood* I_like_coffee = part->getPIDLikelihood();
//   pid[0][0] = I_like_coffee->getProbability();
// }

void PhysicsObjectsMiraBelleDstModule::endRun()
{
}

void PhysicsObjectsMiraBelleDstModule::terminate()
{
}

