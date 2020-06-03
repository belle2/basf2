/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Michel Villanueva, Ami Rostomyan, Jorge Martinez         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <analysis/utility/PCmsLabTransform.h>

#include <analysis/modules/EventKinematics/EventKinematicsModule.h>

#include <analysis/dataobjects/ParticleList.h>
#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/EventKinematics.h>

#include <framework/datastore/StoreObjPtr.h>

#include <framework/logging/Logger.h>
#include <framework/gearbox/Const.h>

#include <iostream>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(EventKinematics)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

EventKinematicsModule::EventKinematicsModule() : Module()
{
  // Set module properties
  setDescription("Module to compute global event kinematic attributes like missing momentum and energy.");

  // Parameter definitions
  addParam("particleLists", m_particleLists, "List of the ParticleLists", vector<string>());

}

EventKinematicsModule::~EventKinematicsModule() = default;

void EventKinematicsModule::initialize()
{
  StoreObjPtr<EventKinematics> evtKinematics;
  evtKinematics.registerInDataStore();

}

void EventKinematicsModule::beginRun()
{
}

void EventKinematicsModule::event()
{
  StoreObjPtr<EventKinematics> eventKinematics;
  if (!eventKinematics) eventKinematics.create();
  EventKinematicsModule::getParticleMomentumLists(m_particleLists);

  TVector3 missingMomentum = EventKinematicsModule::getMissingMomentum();
  eventKinematics->addMissingMomentum(missingMomentum);

  TVector3 missingMomentumCMS = EventKinematicsModule::getMissingMomentumCMS();
  eventKinematics->addMissingMomentumCMS(missingMomentumCMS);

  float missingEnergyCMS = EventKinematicsModule::getMissingEnergyCMS();
  eventKinematics->addMissingEnergyCMS(missingEnergyCMS);

  float missingMass2 = missingEnergyCMS * missingEnergyCMS - missingMomentumCMS.Mag() * missingMomentumCMS.Mag();
  eventKinematics->addMissingMass2(missingMass2);

  float visibleEnergyCMS = EventKinematicsModule::getVisibleEnergyCMS();
  eventKinematics->addVisibleEnergyCMS(visibleEnergyCMS);

  float totalPhotonsEnergy = EventKinematicsModule::getTotalPhotonsEnergy();
  eventKinematics->addTotalPhotonsEnergy(totalPhotonsEnergy);
}

void EventKinematicsModule::endRun()
{
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

      TLorentzVector p_lab = part->get4Vector();
      m_particleMomentumList.push_back(p_lab);

      if ((part->getParticleSource() == Particle::EParticleSourceObject::c_ECLCluster)
          and (part->getPDGCode() == Const::photon.getPDGCode()))
        m_photonsMomentumList.push_back(p_lab);

      TLorentzVector p_cms = T.rotateLabToCms() * p_lab;
      m_particleMomentumListCMS.push_back(p_cms);
    }
  }
  return;
}


TVector3 EventKinematicsModule::getMissingMomentum()
{
  PCmsLabTransform T;
  TLorentzVector beam = T.getBeamFourMomentum();
  TVector3 p = beam.Vect();
  int nParticles = m_particleMomentumList.size();
  for (int i = 0; i < nParticles; ++i) {
    p -= m_particleMomentumList.at(i).Vect();
  }
  return p;
}

TVector3 EventKinematicsModule::getMissingMomentumCMS()
{
  TVector3 p(0., 0., 0.);
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
