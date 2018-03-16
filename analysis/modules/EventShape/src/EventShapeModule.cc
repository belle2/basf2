/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Michel Villanueva, Ami Rostomyan, Jorge Martinez         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <analysis/modules/EventShape/EventShapeModule.h>

#include <analysis/dataobjects/ParticleList.h>
#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/EventShape.h>

#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/TrackFitResult.h>
#include <mdst/dataobjects/ECLCluster.h>
#include <mdst/dataobjects/KLMCluster.h>
#include <mdst/dataobjects/PIDLikelihood.h>

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
  vector<TVector3> listOfParticles = EventShapeModule::getParticleList(m_particleLists);

  TVector3 thrustAxis = EventShapeModule::getThrustOfEvent(listOfParticles);
  eventShape->addThrustAxis(thrustAxis);
  eventShape->addThrust(thrustAxis.Mag());

  TVector3 missingMomentum = EventShapeModule::getMissingMomentum(listOfParticles);
  B2INFO("Missing " << missingMomentum.Mag());
  eventShape->addMissingMomentum(missingMomentum);
}

void EventShapeModule::endRun()
{
}

void EventShapeModule::terminate()
{
}

vector<TVector3> EventShapeModule::getParticleList(vector<string> particleLists)
{
  int nParticleLists = particleLists.size();
  B2INFO("Number of ParticleLists to calculate Event Shape variables: " << nParticleLists << " ");
  PCmsLabTransform T;
  vector<TVector3> forthrust;
  for (int i_pl = 0; i_pl != nParticleLists; ++i_pl) {
    string ParticleListName = particleLists[i_pl];
    B2DEBUG(10, "ParticleList " << ParticleListName);
    StoreObjPtr<ParticleList> plist(ParticleListName);
    int m_part = plist->getListSize();
    for (int i = 0; i < m_part; i++) {
      const Particle* part = plist->getParticle(i);
      B2DEBUG(19, "    Mdst source " << part->getMdstSource());
      if (part->getTrack())
        B2DEBUG(19, "    Track index: " << part->getTrack()->getArrayIndex());
      TVector3 p = part->getMomentum();
      TLorentzVector p_lab = part->get4Vector();
      TLorentzVector p_cms = T.rotateLabToCms() * p_lab;
      forthrust.push_back(p_cms.Vect());
    }
  }
  return forthrust;
}


TVector3 EventShapeModule::getThrustOfEvent(vector<TVector3> forthrust)
{
  TVector3 th = Thrust::calculateThrust(forthrust);
  return th;
}

TVector3 EventShapeModule::getMissingMomentum(vector<TVector3> forthrust)
{
  TVector3 beamMomentumCM(0., 0., 0.);
  int nParticles = forthrust.size();
  for (int i = 0; i < nParticles; ++i) {
    beamMomentumCM -= forthrust.at(i);
  }
  return beamMomentumCM;
}
