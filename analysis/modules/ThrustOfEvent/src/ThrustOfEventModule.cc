/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: iorch                                                    *
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
  B2INFO("Number of ParticleLists to calculate Thrust " << nParticleLists << " ");
}

void ThrustOfEventModule::beginRun()
{
}

void ThrustOfEventModule::event()
{
  StoreObjPtr<ThrustOfEvent> thrust;
  // number of ParticleLists
  int nParticleLists = m_particleLists.size();

  
  // Aquí se guarda la magnitud del thrust y la dirección del thrust axis. -- Michel
  TVector3 thrustAxis(0.1,0.2,0.1);
  thrust->addThrustAxis(thrustAxis);
  thrust->addThrust(0.1);
}

void ThrustOfEventModule::endRun()
{
}

void ThrustOfEventModule::terminate()
{
}

float getThrustOfEvent(){
    std::srand(std::time(0)); // use current time as seed for random generator
    float random_variable0 = std::rand()/1.0;
    float random_variable1 = std::rand()/1.0;
    float random_variable = std::min(random_variable0,random_variable1)/
                            std::max(random_variable0,random_variable1);
    return random_variable;
}
