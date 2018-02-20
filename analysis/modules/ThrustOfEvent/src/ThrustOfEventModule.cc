/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jorge Martinez, Michel Villanueva                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <analysis/modules/ThrustOfEvent/ThrustOfEventModule.h>

#include <analysis/dataobjects/ParticleList.h>
#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/ThrustOfEvent.h>

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
REG_MODULE(ThrustOfEvent)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

ThrustOfEventModule::ThrustOfEventModule() : Module()
{
  // Set module properties
  setDescription("Module to compute Thrust of a particle list, mainly used to compute the thrust of a tau-taubar event.");

  // Parameter definitions
  addParam("particleLists", m_particleLists, "List of the ParticleLists", vector<string>());

}

ThrustOfEventModule::~ThrustOfEventModule()
{
}

void ThrustOfEventModule::initialize()
{
  StoreObjPtr<ThrustOfEvent> thrustOfEvt;
  thrustOfEvt.registerInDataStore();

  unsigned nParticleLists = m_particleLists.size();
  for (unsigned i = 0; i < nParticleLists; ++i) {
  }
  B2INFO("Number of ParticleLists to calculate Thrust: " << nParticleLists << " ");
}

void ThrustOfEventModule::beginRun()
{
}

void ThrustOfEventModule::event()
{
  StoreObjPtr<ThrustOfEvent> thrust;
  if (!thrust) thrust.create();

  TVector3 thrustAxis = ThrustOfEventModule::getThrustOfEvent(m_particleLists);
  thrust->addThrustAxis(thrustAxis);
  thrust->addThrust(thrustAxis.Mag());
}

void ThrustOfEventModule::endRun()
{
}

void ThrustOfEventModule::terminate()
{
}

TVector3 ThrustOfEventModule::getThrustOfEvent(vector<string> particleLists)
{
  int nParticleLists = particleLists.size();
  B2DEBUG(10, "ThrustOfEventModule: Getting Thrust of Event");
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

  TVector3 th = Thrust::calculateThrust(forthrust);


  return th;
}
