/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Gian Luca Pinna Angioni                                  *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <reconstruction/modules/TriggerTimeOffsetInitializer/TriggerTimeOffsetInitializerModule.h>

#include <framework/core/Environment.h>
#include <framework/utilities/Utils.h>

#include <chrono>
#include <set>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(TriggerTimeOffsetInitializer)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

TriggerTimeOffsetInitializerModule::TriggerTimeOffsetInitializerModule() : Module()
{
  //Set module properties
  setDescription(
    "Intialize the trigger offset data object used in simulation."
  );

  //Parameter definition
  addParam("seed", m_seed, "seed for random number", m_seed); //to be set

}

TriggerTimeOffsetInitializerModule::~TriggerTimeOffsetInitializerModule() = default;

void TriggerTimeOffsetInitializerModule::initialize()
{
  //Register the object in the data store
  m_trgOffSetPtr.registerInDataStore(DataStore::c_ErrorIfAlreadyRegistered);
}


void TriggerTimeOffsetInitializerModule::event()
{
  m_trgOffSetPtr.create();
  m_trgOffSetPtr->update();
}
