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
#include <analysis/utility/ReferenceFrame.h>

#include <analysis/modules/EventShape/EventShapeModule.h>

#include <analysis/dataobjects/ParticleList.h>
#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/EventShape.h>

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

#include <framework/logging/Logger.h>

#include <iostream>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(EventShape)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

EventShapeModule::EventShapeModule() : Module()
{
  // Set module properties
  setDescription("Module to compute event shape attributes like Thrust of event, missing momentum and energy.");

  // Parameter definitions
  addParam("particleLists", m_particleLists, "List of the ParticleLists", vector<string>());

}

EventShapeModule::~EventShapeModule()
{
}

void EventShapeModule::initialize()
{
  StoreObjPtr<EventShape> evtShape;
  evtShape.registerInDataStore();

}

void EventShapeModule::beginRun()
{
}

void EventShapeModule::event()
{
  StoreObjPtr<EventShape> eventShape;
  if (!eventShape) eventShape.create();
  EventShapeModule::getParticleMomentumLists(m_particleLists);

  TVector3 thrustAxis = EventShapeModule::getThrustOfEvent();
  eventShape->addThrustAxis(thrustAxis);

  TVector3 missingMomentum = EventShapeModule::getMissingMomentum();
  eventShape->addMissingMomentum(missingMomentum);

  TVector3 missingMomentumCMS = EventShapeModule::getMissingMomentumCMS();
  eventShape->addMissingMomentumCMS(missingMomentumCMS);

  float missingEnergyCMS = EventShapeModule::getMissingEnergyCMS();
  eventShape->addMissingEnergyCMS(missingEnergyCMS);

  float missingMass2 = missingEnergyCMS * missingEnergyCMS - missingMomentumCMS.Mag() * missingMomentumCMS.Mag();
  eventShape->addMissingMass2(missingMass2);

  float visibleEnergy = EventShapeModule::getVisibleEnergy();
  eventShape->addVisibleEnergy(visibleEnergy);
}

void EventShapeModule::endRun()
{
}

void EventShapeModule::terminate()
{
}

void EventShapeModule::getParticleMomentumLists(vector<string> particleLists)
{
  PCmsLabTransform T;

  m_particleMomentumList.clear();
  m_particleMomentumListCMS.clear();

  int nParticleLists = particleLists.size();
  B2DEBUG(10, "Number of ParticleLists to calculate Event Shape variables: " << nParticleLists);

  for (int i_pl = 0; i_pl != nParticleLists; ++i_pl) {
    string particleListName = particleLists[i_pl];
    B2DEBUG(10, "ParticleList: " << particleListName);
    StoreObjPtr<ParticleList> plist(particleListName);
    int m_part = plist->getListSize();
    for (int i = 0; i < m_part; i++) {
      const Particle* part = plist->getParticle(i);

      TLorentzVector p_lab = part->get4Vector();
      m_particleMomentumList.push_back(p_lab);

      TLorentzVector p_cms = T.rotateLabToCms() * p_lab;
      m_particleMomentumListCMS.push_back(p_cms);
    }
  }
  return;
}

TVector3 EventShapeModule::getThrustOfEvent()
{
  std::vector<TVector3> forThrust;
  forThrust.clear();
  int nParticles = m_particleMomentumListCMS.size();
  for (int i = 0; i < nParticles; ++i) {
    forThrust.push_back(m_particleMomentumListCMS.at(i).Vect());
  }
  TVector3 th = Thrust::calculateThrust(forThrust);
  return th;
}

TVector3 EventShapeModule::getMissingMomentum()
{
  PCmsLabTransform T;
  TLorentzVector beam = T.getBeamParams().getHER() + T.getBeamParams().getLER();
  TVector3 p = beam.Vect();
  int nParticles = m_particleMomentumList.size();
  for (int i = 0; i < nParticles; ++i) {
    p -= m_particleMomentumList.at(i).Vect();
  }
  return p;
}

TVector3 EventShapeModule::getMissingMomentumCMS()
{
  TVector3 p(0., 0., 0.);
  int nParticles = m_particleMomentumListCMS.size();
  for (int i = 0; i < nParticles; ++i) {
    p -= m_particleMomentumListCMS.at(i).Vect();
  }
  return p;
}

float EventShapeModule::getMissingEnergyCMS()
{
  PCmsLabTransform T;
  float ECMS = T.getCMSEnergy();
  int nParticles = m_particleMomentumListCMS.size();
  for (int i = 0; i < nParticles; ++i) {
    ECMS -= m_particleMomentumListCMS.at(i).E();
  }
  return ECMS;
}

float EventShapeModule::getVisibleEnergy()
{
  float visibleE = 0.0;
  int nParticles = m_particleMomentumListCMS.size();
  for (int i = 0; i < nParticles; ++i) {
    visibleE += m_particleMomentumListCMS.at(i).E();
  }
  return visibleE;
}