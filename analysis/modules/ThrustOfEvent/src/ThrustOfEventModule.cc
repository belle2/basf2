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

#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/TrackFitResult.h>
#include <mdst/dataobjects/ECLCluster.h>
#include <mdst/dataobjects/KLMCluster.h>
#include <mdst/dataobjects/PIDLikelihood.h>

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

#include <framework/logging/Logger.h>

#include <iostream>

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
  addParam("particleList", m_particleList, "Name of the ParticleList");

}

ThrustOfEventModule::~ThrustOfEventModule()
{
}

void ThrustOfEventModule::initialize()
{
}

void ThrustOfEventModule::beginRun()
{
}

void ThrustOfEventModule::event()
{
}

void ThrustOfEventModule::endRun()
{
}

void ThrustOfEventModule::terminate()
{
}

float getThrustOfEvent(std::string m_particleList){
    std::srand(std::time(0)); // use current time as seed for random generator
    float random_variable0 = std::rand()/1.0;
    float random_variable1 = std::rand()/1.0;
    float random_variable = std::min(random_variable0,random_variable1)/
                            std::max(random_variable0,random_variable1);
    return random_variable;
}
