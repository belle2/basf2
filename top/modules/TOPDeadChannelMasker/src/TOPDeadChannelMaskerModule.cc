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

#include <top/modules/TOPDeadChannelMasker/TOPDeadChannelMaskerModule.h>


using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(TOPDeadChannelMasker)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

TOPDeadChannelMaskerModule::TOPDeadChannelMaskerModule() : Module()
{
  // Set module properties
  setDescription("Masks dead PMs from the reconstruction");

  // Parameter definitions

}

TOPDeadChannelMaskerModule::~TOPDeadChannelMaskerModule()
{
}

void TOPDeadChannelMaskerModule::initialize()
{
}

void TOPDeadChannelMaskerModule::beginRun()
{
}

void TOPDeadChannelMaskerModule::event()
{
}

void TOPDeadChannelMaskerModule::endRun()
{
}

void TOPDeadChannelMaskerModule::terminate()
{
}


