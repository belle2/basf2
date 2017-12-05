/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: iorch                                                    *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <ThrustOfEventModule.h>


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
  addParam("v m_pList", m_v m_pList, "Particle List used to compute the Thrust");

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


