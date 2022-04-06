/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <analysis/utility/PCmsLabTransform.h>

#include <analysis/modules/EventKinematics/EventKinematicsModule.h>

#include <analysis/dataobjects/ParticleList.h>
#include <analysis/dataobjects/Particle.h>

#include <framework/logging/Logger.h>
#include <framework/gearbox/Const.h>

#include <iostream>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(EventKinematics);

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

EventKinematicsModule::EventKinematicsModule() : Module()
{
  // Set module properties
  setDescription("Module to compute global event kinematic attributes like missing momentum and energy.");

  // Parameter definitions
  addParam("particleLists", m_particleLists, "List of the ParticleLists", vector<string>());
  addParam("usingMC", m_usingMC, "is built using generated particles", false);

}

EventKinematicsModule::~EventKinematicsModule() = default;

void EventKinematicsModule::initialize()
{
  auto arrayName = (!m_usingMC) ? "EventKinematics" : "EventKinematicsFromMC";
  m_eventKinematics.registerInDataStore(arrayName);

}

void EventKinematicsModule::event()
{
  if (!m_eventKinematics) m_eventKinematics.construct(m_usingMC);
  EventKinematicsModule::getParticleMomentumLists(m_particleLists);

  ROOT::Math::XYZVector missingMomentum = EventKinematicsModule::getMissingMomentum();
  m_eventKinematics->addMissingMomentum(missingMomentum);

  ROOT::Math::XYZVector missingMomentumCMS = EventKinematicsModule::getMissingMomentumCMS();
  m_eventKinematics->addMissingMomentumCMS(missingMomentumCMS);

  float missingEnergyCMS = EventKinematicsModule::getMissingEnergyCMS();
  m_eventKinematics->addMissingEnergyCMS(missingEnergyCMS);

  float missingMass2 = missingEnergyCMS * missingEnergyCMS - missingMomentumCMS.R() * missingMomentumCMS.R();
  m_eventKinematics->addMissingMass2(missingMass2);

  float visibleEnergyCMS = EventKinematicsModule::getVisibleEnergyCMS();
  m_eventKinematics->addVisibleEnergyCMS(visibleEnergyCMS);

  float totalPhotonsEnergy = EventKinematicsModule::getTotalPhotonsEnergy();
  m_eventKinematics->addTotalPhotonsEnergy(totalPhotonsEnergy);
}

void EventKinematicsModule::terminate()
{
}

void EventKinematicsModule::getParticleMomentumLists(vector<string> particleLists)
{
  PCmsLabTransform T;

  m_particleMomentumList.clear();
  m_photonsMomentumList.clear();
  m_particleMomentumListCMS.clear();

  int nParticleLists = particleLists.size();
  B2DEBUG(10, "Number of ParticleLists to calculate Event Kinematics variables: " << nParticleLists);

  for (int i_pl = 0; i_pl != nParticleLists; ++i_pl) {
    string particleListName = particleLists[i_pl];
    B2DEBUG(10, "ParticleList: " << particleListName);
    StoreObjPtr<ParticleList> plist(particleListName);
    int m_part = plist->getListSize();
    for (int i = 0; i < m_part; i++) {
      const Particle* part = plist->getParticle(i);
      if (part->getParticleSource() == Particle::EParticleSourceObject::c_MCParticle and !m_usingMC) {
        B2FATAL("EventKinematics received MCParticles as an input, but usingMC flag is false");
      }
      if (part->getParticleSource() != Particle::EParticleSourceObject::c_MCParticle and m_usingMC) {
        B2FATAL("EventKinematics received reconstructed Particles as an input, but usingMC flag is true");
      }

      ROOT::Math::PxPyPzEVector p_lab = part->get4Vector();
      m_particleMomentumList.push_back(p_lab);

      if ((part->getParticleSource() == Particle::EParticleSourceObject::c_ECLCluster or
           part->getParticleSource() == Particle::EParticleSourceObject::c_MCParticle)
          and (part->getPDGCode() == Const::photon.getPDGCode()))
        m_photonsMomentumList.push_back(p_lab);

      ROOT::Math::PxPyPzEVector p_cms = T.rotateLabToCms() * p_lab;
      m_particleMomentumListCMS.push_back(p_cms);
    }
  }
  return;
}


ROOT::Math::XYZVector EventKinematicsModule::getMissingMomentum()
{
  PCmsLabTransform T;
  ROOT::Math::PxPyPzEVector beam = T.getBeamFourMomentum();
  ROOT::Math::XYZVector p = beam.Vect();
  int nParticles = m_particleMomentumList.size();
  for (int i = 0; i < nParticles; ++i) {
    p -= m_particleMomentumList.at(i).Vect();
  }
  return p;
}

ROOT::Math::XYZVector EventKinematicsModule::getMissingMomentumCMS()
{
  ROOT::Math::XYZVector p(0., 0., 0.);
  int nParticles = m_particleMomentumListCMS.size();
  for (int i = 0; i < nParticles; ++i) {
    p -= m_particleMomentumListCMS.at(i).Vect();
  }
  return p;
}

float EventKinematicsModule::getMissingEnergyCMS()
{
  PCmsLabTransform T;
  float ECMS = T.getCMSEnergy();
  int nParticles = m_particleMomentumListCMS.size();
  for (int i = 0; i < nParticles; ++i) {
    ECMS -= m_particleMomentumListCMS.at(i).E();
  }
  return ECMS;
}

float EventKinematicsModule::getVisibleEnergyCMS()
{
  float visibleE = 0.0;
  int nParticles = m_particleMomentumListCMS.size();
  for (int i = 0; i < nParticles; ++i) {
    visibleE += m_particleMomentumListCMS.at(i).E();
  }
  return visibleE;
}

float EventKinematicsModule::getTotalPhotonsEnergy()
{
  float photonsEnergy = 0.0;
  int nParticles = m_photonsMomentumList.size();
  for (int i = 0; i < nParticles; ++i) {
    photonsEnergy += m_photonsMomentumList.at(i).E();
  }
  return photonsEnergy;
}
