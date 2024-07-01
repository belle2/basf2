/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

// own include
#include <generators/modules/GeneratorPreselectionModule.h>
#include <analysis/utility/PCmsLabTransform.h>
#include <framework/gearbox/Unit.h>
#include <numeric>
#include <Math/LorentzRotation.h>
#include <Math/Boost.h>
using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(GeneratorPreselection);

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
  addParam("applyToPhotonInMother", m_applyToPhotonInMother, "if true apply energy cut to photon in the rest frame of mother", false);
  addParam("stableParticles", m_stableParticles, "if true apply the selection criteria for stable particles in the generator", false);

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
  if (m_stableParticles)
    if (!mc.hasStatus(MCParticle::c_StableInGenerator)) return;

  const ROOT::Math::XYZVector& p = mc.getMomentum();
  double energy     = mc.getEnergy();
  double mom        = p.R();
  double theta      = p.Theta();

  if (m_applyInCMS) {
    const ROOT::Math::PxPyPzEVector p_cms = PCmsLabTransform::labToCms(mc.get4Vector());
    energy = p_cms.E();
    mom = p_cms.P();
    theta = p_cms.Theta();
  }

  if (m_applyToPhotonInMother && mc.getPDG() == 22) {
    energy = 0;
    if (mc.getMother() && !(mc.getMother()->isInitial()) && !(mc.getMother()->getPDG() == 111)
        && !(mc.getMother()->getPDG() == 221)) { // mother is not the incoming beams, pi0 or eta
      //
      ROOT::Math::PxPyPzEVector p_this = mc.get4Vector();
      ROOT::Math::PxPyPzEVector p_moth = mc.getMother()->get4Vector();
      // Create a Lorentz boost to the rest frame of the mother particle
      ROOT::Math::Boost boost_to_mother_rest_frame(p_moth.BoostToCM());
      ROOT::Math::PxPyPzEVector p_moth_rest = boost_to_mother_rest_frame * p_moth;
      ROOT::Math::PxPyPzEVector p_this_rest = boost_to_mother_rest_frame * p_this;
      double check_this_e =  p_this_rest.E();
      double check_this_p =  p_this_rest.P();
      //
      energy = check_this_e;
      mom =  check_this_p;
      theta = (m_MinPhotonTheta + m_MaxPhotonTheta) / 2; // always pass
      //
      B2DEBUG(250, "GeneratorPresel: energy, mom = " << p_this.E() << " " << p_this.P() << " mother id, energy, mom = " <<
              mc.getMother()->getPDG() << " " << p_moth.E() << " " << p_moth.P() << "energy/mom in mother's rest frame " <<  check_this_e << " "
              << check_this_p << " energy/mom of mother in rest frame " << p_moth_rest.E() << " " << p_moth_rest.P());
    }
  }

  if (mc.getPDG() == 22) {
    B2DEBUG(250, "E = " << energy << " theta=" << theta << " thetamin=" << m_MinPhotonTheta << " thetamax=" << m_MaxPhotonTheta);
    if (energy >= m_MinPhotonEnergy && theta >= m_MinPhotonTheta && theta <= m_MaxPhotonTheta) {
      m_nPhoton++;
    }
  }

  if (abs(mc.getCharge()) > 0.) {
    B2DEBUG(250, "pt = " << p.Rho() << " p=" << mom << " theta=" << theta << " thetamin=" << m_MinChargedTheta << " thetamax=" <<
            m_MaxChargedTheta);
    if (mom >= m_MinChargedP && p.Rho() >= m_MinChargedPt && theta >= m_MinChargedTheta && theta <= m_MaxChargedTheta) {
      m_nCharged++;
    }
  }

}

void GeneratorPreselectionModule::terminate()
{
  B2RESULT("Final results of the preselection module:");
  for (const auto& finalResult : m_resultCounter) {
    B2RESULT("\tPreselection with result " << finalResult.first << ": " << finalResult.second << " times.");
  }
  const unsigned int sumCounters = std::accumulate(m_resultCounter.begin(), m_resultCounter.end(), 0, [](auto lhs, auto rhs) {
    return lhs + rhs.second;
  });

  B2RESULT("Total number of tested events: " << sumCounters);
}

