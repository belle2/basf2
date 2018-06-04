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
#include <boost/format.hpp>

#include <TDatabasePDG.h>

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
  setDescription("Preselection based on generator truth information");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("nChargedMin", m_nChargedMin, "minimum number of charged particles", 0);
  addParam("nChargedMax", m_nChargedMax, "maximum number of charged particles", 999);
  addParam("MinChargedP", m_MinChargedP, "minimum charged momentum [GeV]", 0.25);
  addParam("MinChargedPt", m_MinChargedPt, "minimum charged transverse momentum (pt) [GeV]", 0.1);
  addParam("MinChargedTheta", m_MinChargedTheta, "minimum polar angle of charged particle [deg]", 17.);
  addParam("MaxChargedTheta", m_MaxChargedTheta, "maximum polar angle of charged particle [deg]", 150.);
  addParam("applyInCMS", m_applyInCMS, "if true apply the P,Pt,theta cuts in the center of mass frame", false);

  addParam("nPhotonMin", m_nPhotonMin, "minimum number of photons", 0);
  addParam("nPhotonMax", m_nPhotonMax, "maximum number of photons", 999);
  addParam("MinPhotonEnergy", m_MinPhotonEnergy, "minimum photon energy [GeV]", -1.);
  addParam("MinPhotonTheta", m_MinPhotonTheta, "minimum polar angle of photon [deg]", 15.);
  addParam("MaxPhotonTheta", m_MaxPhotonTheta, "maximum polar angle of photon [deg]", 165.);

  //initialize all member variables
  m_onlyPrimaries = false;
  m_maxLevel = -1;
  m_nCharged = 0;
  m_nPhoton = 0;
}

void GeneratorPreselectionModule::initialize()
{
  B2INFO("GeneratorPreselectionModule initialize");

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

  m_seen.clear();
  m_seen.resize(m_mcparticles.getEntries() + 1, false);


  for (int i = 0; i < m_mcparticles.getEntries(); i++) {
    MCParticle& mc = *m_mcparticles[i];
    if (mc.getMother() != NULL) continue;
    checkParticle(mc, 0);
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
}

void GeneratorPreselectionModule::checkParticle(const MCParticle& mc, int level)
{
  if (m_onlyPrimaries && !mc.hasStatus(MCParticle::c_PrimaryParticle)) return;
  if (mc.hasStatus(MCParticle::c_Initial) || mc.hasStatus(MCParticle::c_IsVirtual)) return;

  //Only use up to max level
  if (m_maxLevel >= 0 && level > m_maxLevel) return;
  ++level;
  string indent = "";

  for (int i = 0; i < level; i++) indent += "  ";

  TDatabasePDG* pdb = TDatabasePDG::Instance();
  TParticlePDG* pdef = pdb->GetParticle(mc.getPDG());
  string name = pdef ? (string(" (") + pdef->GetTitle() + ")") : "";

  if (m_seen[mc.getIndex()]) {
    B2INFO(boost::format("%4d %s%10d%s*") % mc.getIndex() % indent % mc.getPDG() % name);
    return;
  }
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

  const vector<MCParticle*> daughters = mc.getDaughters();
  for (MCParticle* daughter : daughters) {
    checkParticle(*daughter, level);
  }
  m_seen[mc.getIndex()] = true;

}
