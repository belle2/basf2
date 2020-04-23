/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
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
    "Sets the event meta data information (exp, run, evt). You must use this "
    "module to tell basf2 about the number of events you want to generate, "
    "unless you have an input module that already does so. Note that all "
    "experiment/run combinations specified must be unique."
  );

  //Parameter definition
  addParam("seed", m_seed, "seed for random number", m_seed);

}

TriggerTimeOffsetInitializerModule::~TriggerTimeOffsetInitializerModule() = default;

void TriggerTimeOffsetInitializerModule::initialize()
{
  //Register the object in the data store
  m_trgOffSetPtr.registerInDataStore(DataStore::c_ErrorIfAlreadyRegistered);
  //steering file content overwritten via command line arguments?
}


void TriggerTimeOffsetInitializerModule::event()
{
  m_trgOffSetPtr.create();
}
