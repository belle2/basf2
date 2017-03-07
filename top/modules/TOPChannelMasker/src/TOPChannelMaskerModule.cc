/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jan Strube, Sam Cunliffe                                 *
 * (jan.strube@pnnl.gov samuel.cunliffe@pnnl.gov)                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <top/modules/TOPChannelMasker/TOPChannelMaskerModule.h>


using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(TOPChannelMasker)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

TOPChannelMaskerModule::TOPChannelMaskerModule() : Module()
{
  // Set module properties
  setDescription("Masks dead PMs from the reconstruction");

  // Parameter definitions

}

TOPChannelMaskerModule::~TOPChannelMaskerModule()
{
}

void TOPChannelMaskerModule::initialize()
{
}

void TOPChannelMaskerModule::beginRun()
{
}

void TOPChannelMaskerModule::event()
{
}

void TOPChannelMaskerModule::endRun()
{
}

void TOPChannelMaskerModule::terminate()
{
}


