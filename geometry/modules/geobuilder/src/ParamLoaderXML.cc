/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <geometry/modules/geobuilder/ParamLoaderXML.h>

#include <framework/core/ModuleManager.h>

#include <framework/gearbox/Gearbox.h>
#include <framework/gearbox/GearboxIOXML.h>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(ParamLoaderXML)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

ParamLoaderXML::ParamLoaderXML() : Module("ParamLoaderXML")
{
  //Set module properties
  setDescription("Loads the Belle II detector parameters from a XML document.");

  //Parameter definition
  addParam("InputFileXML",  m_filenameXML, string("Belle2.xml"), "The filename of the XML input file which should be loaded.");
  addParam("ParamCheck",  m_paramCheck, true, "Turns on/off parameter validation. Turn it off for speedup.");
}


ParamLoaderXML::~ParamLoaderXML()
{

}


void ParamLoaderXML::initialize()
{
  GearboxIOXML* gearboxIOXML = new GearboxIOXML();
  bool result = gearboxIOXML->open(m_filenameXML);

  if (result) {
    Gearbox& gearbox = Gearbox::Instance();
    gearbox.connect(gearboxIOXML);

    gearbox.enableParamCheck(m_paramCheck);

  } else {
    ERROR("Could not open XML file: " << m_filenameXML)
  }
}


void ParamLoaderXML::beginRun()
{

}


void ParamLoaderXML::event()
{

}


void ParamLoaderXML::endRun()
{

}


void ParamLoaderXML::terminate()
{

}
