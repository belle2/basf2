/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Torben Ferber                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// own include
#include <generators/modules/GeneratorPreselectionModule.h>
#include <framework/gearbox/Unit.h>
#include <numeric>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(GeneratorPreselection)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

GeneratorPreselectionModule::GeneratorPreselectionModule() : Module()
{
  // Set module properties
  setDescription("Preselection based on generator truth information. Returns 0 if no cut have been passed, 1 if only the charged cut has been passed, 10 if only the photon cut has been passed, and 11 if both charged and photon cuts have been passed.");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("nChargedMin", m_nChargedMin, "minimum number of charged particles", 0);
  addParam("nChargedMax", m_nChargedMax, "maximum number of charged particles", 999);
  addParam("MinChargedP", m_MinChargedP, "minimum charged momentum [GeV]", 0.25);
  addParam("MinChargedPt", m_MinChargedPt, "minimum charged transverse momentum (pt) [GeV]", 0.1);
  addParam("MinChargedTheta", m_MinChargedTheta, "minimum polar angle of charged particle [deg]", 17.);
  addParam("MaxChargedTheta", m_MaxChargedTheta, "maximum polar angle of charged particle [deg]", 150.);
  addParam("applyInCMS", m_applyInCMS, "if true apply the P,Pt,theta, and energy cuts in the center of mass frame", false);

  addParam("nPhotonMin", m_nPhotonMin, "minimum number of photons", 0);
  addParam("nPhotonMax", m_nPhotonMax, "maximum number of photons", 999);
  addParam("MinPhotonEnergy", m_MinPhotonEnergy, "minimum photon energy [GeV]", -1.);
  addParam("MinPhotonTheta", m_MinPhotonTheta, "minimum polar angle of photon [deg]", 15.);
  addParam("MaxPhotonTheta", m_MaxPhotonTheta, "maximum polar angle of photon [deg]", 165.);
}

void GeneratorPreselectionModule::initialize()
{
  B2DEBUG(29, "GeneratorPreselectionModule initialize");

  //convert limits to radian
  m_MinChargedTheta *= Unit::deg;
  m_MaxChargedTheta *= Unit::deg;
  m_MinPhotonTheta  *= Unit::deg;
  m_MaxPhotonTheta  *= Unit::deg;

  m_mcparticles.isRequired(m_particleList);
  if (m_applyInCMS) {
    m_initial.isRequired();
  }
}


void GeneratorPreselectionModule::event()
{
  m_nCharged = 0;
  m_nPhoton  = 0.;

  for (int i = 0; i < m_mcparticles.getEntries(); i++) {
    MCParticle& mc = *m_mcparticles[i];
    checkParticle(mc);
  }

  //check number of particles passing the cuts
  B2DEBUG(250, "number of charged passing cuts: " << m_nCharged);
  B2DEBUG(250, "number of photons passing cuts: " << m_nPhoton);

  //set return value
  int retvalue = 0;
  setReturnValue(retvalue);
  if (m_nCharged >= m_nChargedMin && m_nCharged <= m_nChargedMax) {
    retvalue = 1;
  }
  if (m_nPhoton >= m_nPhotonMin && m_nPhoton <= m_nPhotonMax) {
    retvalue += 10;
  }

  B2DEBUG(250, "return value: " << retvalue);
  setReturnValue(retvalue);

  if (m_resultCounter.find(retvalue) == m_resultCounter.end()) {
    m_resultCounter[retvalue] = 1;
  } else {
    m_resultCounter[retvalue] += 1;
  }
}

void GeneratorPreselectionModule::checkParticle(const MCParticle& mc)
{
  if (!mc.hasStatus(MCParticle::c_PrimaryParticle)) return;
  if (mc.hasStatus(MCParticle::c_Initial) or mc.hasStatus(MCParticle::c_IsVirtual)) return;

  const TVector3& p = mc.getMomentum();
  double energy     = mc.getEnergy();
  double mom        = p.Mag();
  double theta      = p.Theta();

  if (m_applyInCMS) {
    const TLorentzVector p_cms = m_initial->getLabToCMS() * mc.get4Vector();
    energy = p_cms.E();
    mom = p_cms.P();
    theta = p_cms.Theta();
  }

  if (mc.getPDG() == 22) {
    B2DEBUG(250, "E = " << energy << " theta=" << theta << " thetamin=" << m_MinPhotonTheta << " thetamax=" << m_MaxPhotonTheta);
    if (energy >= m_MinPhotonEnergy && theta >= m_MinPhotonTheta && theta <= m_MaxPhotonTheta) {
      m_nPhoton++;
    }
  }

  if (abs(mc.getCharge()) > 0.) {
    B2DEBUG(250, "pt = " << p.Pt() << " p=" << mom << " theta=" << theta << " thetamin=" << m_MinChargedTheta << " thetamax=" <<
            m_MaxChargedTheta);
    if (mom >= m_MinChargedP && p.Pt() >= m_MinChargedPt && theta >= m_MinChargedTheta && theta <= m_MaxChargedTheta) {
      m_nCharged++;
    }
  }

}

void GeneratorPreselectionModule::terminate()
{
  B2DEBUG(29, "Final results of the preselection module:");
  for (const auto& finalResult : m_resultCounter) {
    B2DEBUG(29, "\tPreselection with result " << finalResult.first << ": " << finalResult.second << " times.");
  }
  const unsigned int sumCounters = std::accumulate(m_resultCounter.begin(), m_resultCounter.end(), 0, [](auto lhs, auto rhs) {
    return lhs + rhs.second;
  });

  B2DEBUG(29, "Total number of tested events: " << sumCounters);
}

