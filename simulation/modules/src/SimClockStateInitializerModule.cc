/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Gian Luca Pinna Angioni                                  *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <simulation/modules/SimClockStateInitializerModule.h>

#include <framework/core/Environment.h>
#include <framework/utilities/Utils.h>

#include <chrono>
#include <set>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(SimClockStateInitializer)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

SimClockStateInitializerModule::SimClockStateInitializerModule() : Module()
{
  //Set module properties
  setDescription(
    "Intialize the trigger offset data object used in simulation."
  );

  //Parameter definition
  addParam("seed", m_seed, "seed for random number", m_seed); //to be set

}

SimClockStateInitializerModule::~SimClockStateInitializerModule() = default;

void SimClockStateInitializerModule::initialize()
{
//Register the object in the data store
  m_clockStatePtr.registerInDataStore(DataStore::c_ErrorIfAlreadyRegistered);
}


void SimClockStateInitializerModule::beginRun()
{

}


void SimClockStateInitializerModule::event()
{

  // StoreObjPtr<SimClockState> m_clockStatePtrEv; /**< Output object. */
  m_clockStatePtr.create();
  m_clockStatePtr->update();

}
