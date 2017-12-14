/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Pulvermacher                                   *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <boost/python.hpp>

#include <framework/modules/core/InteractiveModule.h>

#include <framework/logging/Logger.h>


using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(Interactive)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

InteractiveModule::InteractiveModule() : Module()
{
  setDescription("Start an interactive (I)python shell in each call of event().  Also imports the ROOT.Belle2 namespace for convenience, allowing you to use Belle2.PyStoreArray etc. directly.");
}

InteractiveModule::~InteractiveModule()
{
}

void InteractiveModule::event()
{
  //In case of exceptions in the embed() call, the imports may get lost (why?)
  //reimporting them ensures we can call embed() again in the next event
  if (PyRun_SimpleString("import interactive") == -1) {
    B2FATAL("'import interactive' failed.");
  }
  if (PyRun_SimpleString("from ROOT import Belle2") == -1) {
    B2FATAL("'from ROOT import Belle2' failed.");
  }

  B2INFO("Opening (I)Python shell, press Ctrl+D to close it. Press Ctrl+C first to exit basf2.");
  if (PyRun_SimpleString("interactive.embed()") == -1) {
    B2ERROR("embed() failed!");
  }
}
