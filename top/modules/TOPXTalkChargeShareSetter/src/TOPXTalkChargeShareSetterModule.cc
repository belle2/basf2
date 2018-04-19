/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Rikuya Okuto                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <top/modules/TOPXTalkChargeShareSetter/include/TOPXTalkChargeShareSetterModule.h>

/* --------------- WARNING ---------------------------------------------- *
If you have more complex parameter types in your class then simple int,
double or std::vector of those you might need to uncomment the following
include directive to avoid an undefined reference on compilation.
 * ---------------------------------------------------------------------- */
// #include <framework/core/ModuleParam.templateDetails.h>



using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(TOPXTalkChargeShareSetter)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

TOPXTalkChargeShareSetterModule::TOPXTalkChargeShareSetterModule() : Module()
{
  // Set module properties
  setDescription(R"DOC("Crosstalk & chargeshare flag setter


  )DOC");

  // Parameter definitions

}

TOPXTalkChargeShareSetterModule::~TOPXTalkChargeShareSetterModule()
{
}

void TOPXTalkChargeShareSetterModule::initialize()
{
}

void TOPXTalkChargeShareSetterModule::beginRun()
{
}

void TOPXTalkChargeShareSetterModule::event()
{
}

void TOPXTalkChargeShareSetterModule::endRun()
{
}

void TOPXTalkChargeShareSetterModule::terminate()
{
}


