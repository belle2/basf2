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
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>

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
  setDescription("Loads the Belle II detector parameters from an XML document.");

  m_backends.push_back("file:");

  //Parameter definition
  addParam("backends",  m_backends, "The backends to use when looking for xml data. A backend can also contain a search path after ':'. (If none is given, '/data' will be used.)",
           m_backends);
  addParam("fileName", m_fileName, "The filename of the main xml file",
           string("geometry/Belle2.xml"));
}

void GearboxModule::initialize()
{
  //gearbox might need exp/run numbers
  StoreObjPtr<EventMetaData>::required();

  Gearbox& gearbox = Gearbox::getInstance();
  gearbox.setBackends(m_backends);
  gearbox.open(m_fileName);
}

void GearboxModule::beginRun()
{
  //if only file backend is used, we don't want to reread the data in every run
  if (m_backends.size() == 1 && m_backends[0] == "file:")
    return;

  Gearbox& gearbox = Gearbox::getInstance();
  gearbox.close();
  gearbox.open(m_fileName);
}
