/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll, Martin Ritter                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/modules/GearboxModule.h>
#include <framework/gearbox/Gearbox.h>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(Gearbox)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

GearboxModule::GearboxModule() : Module()
{
  //Set module properties
  setDescription("Loads the Belle II detector parameters from a XML document.");

  m_backends.push_back("file:");

  //Parameter definition
  addParam("Backends",  m_backends, "The backend to use when looking for xml data",
           m_backends);
  addParam("Filename", m_filename, "The filename of the main xml file",
           string("geometry/Belle2.xml"));
}

void GearboxModule::initialize()
{
  Gearbox& gearbox = Gearbox::getInstance();
  gearbox.setBackends(m_backends);
  gearbox.open(m_filename);
}

void GearboxModule::beginRun()
{
  Gearbox& gearbox = Gearbox::getInstance();
  gearbox.close();
  gearbox.open(m_filename);
}
