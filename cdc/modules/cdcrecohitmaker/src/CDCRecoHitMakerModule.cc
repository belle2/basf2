/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "cdc/modules/cdcrecohitmaker/CDCRecoHitMakerModule.h"

#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreDefs.h>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(CDCRecoHitMakerModule, "CDCRecoHitMaker")

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

CDCRecoHitMakerModule::CDCRecoHitMakerModule() : Module()
{
  //Set module properties
  setDescription("Prints the current event meta data information (exp, run, evt).");

  //Parameter definition
}


CDCRecoHitMakerModule::~CDCRecoHitMakerModule()
{

}


void CDCRecoHitMakerModule::initialize()
{
}


void CDCRecoHitMakerModule::beginRun()
{
}


void CDCRecoHitMakerModule::event()
{
}


void CDCRecoHitMakerModule::endRun()
{
}

void CDCRecoHitMakerModule::terminate()
{
}
